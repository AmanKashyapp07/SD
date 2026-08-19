# System Design: WhatsApp (High-Level Design)

A comprehensive guide for technical interviews, covering the end-to-end architecture of a global, real-time instant messaging platform.

---

## 1. Functional Requirements

What the system must do from the user perspective:

1. **One-on-One Chat:** Real-time text messaging between two users with low latency.
2. **Delivery Status Receipts:** Accurate message tracking indicators:
   - Single Tick: Message sent to server.
   - Double Tick: Message delivered to recipient's device.
   - Blue Ticks: Message read by recipient.
3. **Media Sharing:** Send and receive images, videos, voice notes, and documents.
4. **Group Chat:** Support multi-user conversations (up to 1,024 members) with real-time fan-out of messages.
5. **Last Seen & Online Presence:** Show real-time online status and timestamp of last activity.
6. **End-to-End Encryption (E2EE):** Only the sender and recipient can read messages; intermediate servers cannot inspect message contents (Signal Protocol).
7. **Offline Message Storage:** If a recipient is offline, the server holds the message temporarily until they come online, delivers it, and immediately deletes it from the server.
8. **Push Notifications:** Alert offline users of incoming messages via APNs (iOS) and FCM (Android).

---

## 2. Non-Functional Requirements

System quality attributes, constraints, and operational guarantees:

1. **Ultra-Low Latency:** Message delivery latency must be under 100 milliseconds over standard mobile networks.
2. **High Availability:** 99.99% uptime. The messaging service must always accept messages.
3. **Strict Message Ordering:** Messages within a chat session must be delivered in the exact chronological order they were sent.
4. **Zero Server Persistence for Delivered Messages:** To preserve user privacy and keep infrastructure lean, once a message is acknowledged as delivered by the recipient device, it is permanently deleted from server storage.
5. **Massive Scale:** Support 2+ billion registered users, 1+ billion daily active users, and 100+ billion messages per day.
6. **Network Efficiency & Battery Optimization:** Protocol must use minimal bandwidth and battery on mobile devices (lightweight payload serialization).

---

## 3. Back-of-the-Envelope Estimation

Let us calculate the storage, memory for persistent connections, requests per second (QPS), and network bandwidth required at WhatsApp scale.

### Core Assumptions
- Total active users = **2 Billion**
- Daily Active Users (DAU) = **1 Billion**
- Average messages sent per user per day = **50 messages**
- Percentage of messages containing media = **10%** (images, voice notes, videos)
- Average text message size = **100 Bytes** (including metadata like timestamp, IDs)
- Average media file size = **200 KB** (compressed)
- Peak traffic multiplier = **2x of average traffic**

### 1. Message Throughput (QPS)
- Total messages per day = $1\text{ Billion users} \times 50\text{ msgs/day} = 50\text{ Billion messages/day}$
- Total seconds per day = $86,400\text{ seconds} \approx 10^5\text{ seconds}$
- Average message QPS = $\frac{50\times 10^9\text{ msgs}}{86,400\text{ sec}} \approx 580,000\text{ messages/second}$
- Peak message QPS ($2\times$) $\approx$ **1.16 Million messages/second**

### 2. Concurrent Connections & Server RAM Estimation
- Assume **500 Million** users are simultaneously connected to chat servers via persistent TCP/WebSocket connections during peak hours.
- A single open connection (file descriptor + OS socket buffer + session state) consumes roughly **10 KB** of memory.
- Total RAM required for 500 Million open sockets:
  $$\text{Total RAM} = 500\text{ Million} \times 10\text{ KB} = 5\text{ TB of RAM}$$
- If one high-spec gateway server (e.g., 64 GB RAM) handles **2 Million concurrent connections** (achievable via Erlang/BEAM or modern Linux epoll tuning):
  $$\text{Total Chat Gateway Servers} = \frac{500\text{ Million}}{2\text{ Million}} = 250\text{ servers}$$
  *(This showcases why WhatsApp historically operated with astonishingly small engineering and server footprints).*

### 3. Media Storage & Bandwidth
- Total media messages per day = $10\% \text{ of } 50\text{ Billion} = 5\text{ Billion media files/day}$
- Daily media storage required = $5\text{ Billion} \times 200\text{ KB} = 1,000\text{ TB} = 1\text{ PB/day}$
- Media storage is retained on servers only for 14–30 days (ephemeral storage for pending downloads):
  $$\text{Active Media Storage (30 days)} = 30 \times 1\text{ PB} = 30\text{ PB}$$
- **Peak Egress Bandwidth for Media:**
  $$\text{Bandwidth} = \frac{1\text{ PB}}{86,400\text{ sec}} \approx 11.6\text{ GB/sec} \approx 93\text{ Gbps (Gigabits per second)}$$

---

## 4. High-Level Architecture

The architecture separates long-lived real-time connection handling from stateless business logic and ephemeral data storage.

### ASCII Architecture Diagram

```
+-----------------------------------------------------------------------------+
|                                CLIENT DEVICES                               |
|                     (Android, iOS, Web, Desktop Apps)                       |
+---------------------+-------------------------------+-----------------------+
                      |                               |
        1. Persistent TCP Connection                  | 2. Encrypted Media
        (WebSocket / Noise Protocol)                  |    Upload / Download
                      |                               |
                      v                               v
+------------------------------------+   +------------------------------------+
|        TCP LOAD BALANCER           |   |            MEDIA STORAGE           |
|            (HAProxy)               |   |     - S3 / Distributed Blob Store  |
+------------------+-----------------+   |     - Global CDN (Cloudflare/Edge) |
                   |                     +-----------------+------------------+
        +----------+-----------+                           ^
        |                      |                           |
        v                      v                           | Encrypted Blob
+----------------+     +----------------+                  |
| CHAT GATEWAY 1 |     | CHAT GATEWAY N |                  |
| (Erlang/Cowboy |     | (Erlang/Cowboy |                  |
| Socket Handler)|     | Socket Handler)|                  |
+-------+--------+     +-------+--------+                  |
        |                      |                           |
        +----------+-----------+                           |
                   |                                       |
                   v                                       |
+------------------------------------+                     |
|         SESSION SERVICE            |                     |
|  - Redis / Distributed Hash Table  |                     |
|  (Maps: UserID -> GatewayServerID) |                     |
+------------------+-----------------+                     |
                   |                                       |
        +----------+-----------+                           |
        |                      |                           |
        v                      v                           |
+----------------+     +----------------+                  |
| GROUP SERVICE  |     | PRESENCE SVC   |                  |
| (Membership &  |     | (Online/Last   |                  |
|  Fan-out)      |     |  Seen State)   |                  |
+-------+--------+     +-------+--------+                  |
        |                      |                           |
        v                      v                           |
+----------------+     +----------------+                  |
| EPHEMERAL MSG  |     | PUSH NOTIF SVC |                  |
| STORE          |     | (APNs / FCM)   +------------------+
| (Cassandra /   |     |                |
|  Mnesia)       |     +----------------+
+----------------+
```

---

## 5. Key Components & Why

### 1. Chat Gateway (Connection Handler Cluster)
- **What it does:** Maintains long-lived, persistent bidirectional TCP connections (WebSocket or raw TCP using Noise protocol) with every active client device.
- **Why this choice:** HTTP polling is wasteful on battery and network. Persistent sockets allow the server to instantly push incoming messages to the recipient without the recipient asking first. Technologies like Erlang/Elixir or C++ (epoll/kqueue) manage millions of concurrent lightweight processes with minimal overhead.

### 2. Session Service (Connection Router / Directory)
- **What it does:** Maintains an in-memory distributed registry mapping `UserID -> GatewayServer_IP/SocketID`.
- **Why this choice:** When User A on Gateway Server 1 sends a message to User B, Gateway Server 1 queries the Session Service to find that User B is currently connected to Gateway Server 4. It routes the message internally directly to Gateway Server 4. Powered by **Redis Cluster** or distributed hash tables.

### 3. Ephemeral Message Store (Offline Buffer)
- **What it does:** Temporarily holds messages for users who are currently offline or disconnected.
- **Why this choice:** WhatsApp does not act as a permanent cloud backup for text chats. Messages stay in the database **only until delivery receipt ACK** is received from the device, then they are deleted. Technologies: **Apache Cassandra** or **Mnesia** (built-in Erlang distributed DB) for fast, key-based writes and deletes.

### 4. Group Chat Service
- **What it does:** Resolves group conversations. When a message is sent to `GroupID_99`, the Group Service fetches the member list from a cache/database, duplicates the message references, and routes a copy to each member's active connection.
- **Why this choice:** Offloads fan-out compute from the main chat gateways so one large group message does not block 1-on-1 chats.

### 5. Presence Service (Last Seen & Online Status)
- **What it does:** Tracks whether a user is currently online or when they were last active.
- **Why this choice:** Instead of broadcasting presence updates to all contacts continuously (which causes an $O(N^2)$ traffic explosion), presence is fetched on-demand: when User A opens a chat screen with User B, User A subscribes to User B's presence state.

### 6. Media Storage & Content Delivery
- **What it does:** Stores encrypted media files on object storage (Amazon S3 / Blob storage) fronted by a CDN.
- **Why this choice:** Chat gateway servers should never handle heavy multi-megabyte media payloads over WebSocket connections. The media is uploaded via HTTP POST directly to blob storage, and only a lightweight encrypted URL pointer is sent through the chat socket.

---

## 6. How Real-Time Messaging Actually Works

### 1. One-on-One Message Flow (Online Recipient)

```
[Sender (Alice)]          [Chat Gateway A]       [Session Svc]      [Chat Gateway B]       [Recipient (Bob)]
      |                          |                     |                   |                       |
      |-- 1. Send Msg (Encrypted)-->                   |                   |                       |
      |<-- 2. ACK (Single Tick) --|                    |                   |                       |
      |                          |-- 3. Lookup Bob --->|                   |                       |
      |                          |<-- 4. Gateway B ----|                   |                       |
      |                          |                                         |                       |
      |                          |-------- 5. Route Msg to Gateway B ----->|                       |
      |                          |                                         |-- 6. Push to Bob ---->|
      |                          |                                         |<-- 7. ACK from Bob ---|
      |                          |<------- 8. Delivery Receipt (DoubleTick)-|                       |
      |<-- 9. Double Tick Update-|                                         |                       |
      |                          |                                         |                       |
      |                          |                                         |-- 8. Bob opens chat ->|
      |<-- 10. Blue Tick Update -|<------- 9. Read Receipt (Blue Tick)-----|                       |
```

1. **Client Encryption:** Alice's app encrypts the plaintext message using Bob's public key (Signal Protocol) locally on her device.
2. **Send over TCP:** Alice sends the ciphertext to her connected Gateway Server A.
3. **Server ACK:** Gateway A acknowledges receipt. Alice's UI shows a **Single Grey Tick**.
4. **Recipient Lookup:** Gateway A checks Session Service: "Where is Bob connected?" $\rightarrow$ Response: "Gateway Server B".
5. **Internal Transfer:** Gateway A forwards the message to Gateway Server B over a fast internal RPC/message bus.
6. **Push to Recipient:** Gateway B sends the message over Bob's open TCP connection.
7. **Delivery ACK:** Bob's device receives the message, stores it in its local SQLite database, and automatically sends an `ACK_DELIVERED` back to Gateway B.
8. **Double Tick Propagation:** The delivery confirmation routes back to Alice, updating her UI to **Double Grey Ticks**.
9. **Read Receipt:** When Bob opens the chat conversation, his app emits an `ACK_READ` event, which routes back to Alice to turn the ticks **Blue**.

### 2. Offline Recipient Flow
1. Gateway A looks up Bob in Session Service $\rightarrow$ Bob has no active socket connection (offline).
2. Gateway A writes the message into the **Ephemeral Message Store** with status `PENDING`.
3. Gateway A triggers the **Push Notification Service**, which sends an alert via Apple APNs or Google FCM ("You have a new message from Alice").
4. When Bob reconnects to the network, his device establishes a TCP connection with a Gateway server.
5. The Gateway queries the Ephemeral Message Store for all pending messages belonging to Bob, flushes them to his device in chronological order, and **immediately purges them from the database** once Bob's device confirms receipt.

### 3. End-to-End Encrypted Media Sharing Flow
1. **Local Encryption:** Alice selects an image. Her app generates a random symmetric key $K$, encrypts the image with $K$, and generates a cryptographic hash.
2. **Media Upload:** Alice's app uploads the encrypted image blob directly to Media Object Storage (S3/CDN) via HTTP POST.
3. **URL & Key Transmission:** Alice encrypts the symmetric key $K$ and the media URL using Bob's public key, and sends this tiny metadata payload as a standard text message over the chat socket.
4. **Download & Decryption:** Bob receives the text message, decrypts the metadata to obtain key $K$ and the download URL, downloads the encrypted blob from CDN, and decrypts the image locally on his device.
5. The server **never** has access to the decryption key $K$ or the unencrypted media content.

---

## 7. Bottlenecks & Solutions

### 1. The C10M Problem (Massive Concurrent Connections)
- **Problem:** Traditional thread-per-connection architectures (like standard Apache/Tomcat servers) crash after ~10,000 open sockets due to thread stack memory and CPU context-switching overhead.
- **Solution:** Asynchronous, non-blocking I/O event loops (**epoll** in Linux, **kqueue** in BSD) combined with actor-based concurrency (Erlang BEAM virtual machine). Erlang processes cost only ~300 bytes of memory each, allowing a single commodity server to hold 2+ million idle persistent TCP connections.

### 2. Group Message Fan-Out Explosion
- **Problem:** In a group of 1,024 members, 1 message sent means 1,023 messages must be distributed. If 100 people talk at once, traffic explodes ($100 \times 1,000 = 100,000$ operations).
- **Solution (Client-Side Fan-out vs Server Optimization):**
  - Sender encrypts the message once using a shared "Sender Key" (Signal Group Protocol).
  - The server acts strictly as a lightweight packet router: it duplicates the ciphertext packet to all connected member gateways without re-encrypting or decrypting.
  - Workers use batching queues so network socket writes are combined rather than executed individually.

### 3. "Last Seen" / Presence Update Storm
- **Problem:** If 100 Million users go online or offline within an hour, broadcasting status updates to all contacts in their address books would generate hundreds of billions of database writes and push events.
- **Solution:** **Pull/Subscription Model instead of Push Broadcast.** Presence is never broadcast globally. A user only subscribes to presence updates for contacts currently visible on their screen. If the app is minimized or the user is looking at another chat, all presence updates stop.

### 4. Message Ordering Guarantee
- **Problem:** Network jitter or reconnects can cause Message #2 to arrive before Message #1.
- **Solution:** Every message is tagged with a client-generated monotonic sequence number or timestamp paired with a conversation ID. The recipient's local database sorts messages by sequence number before rendering to the UI.

---

## 8. Follow-up Questions an Interviewer Might Ask

These are realistic interview follow-ups to think through on your own:

1. **Multi-Device Synchronization with E2EE:** How do you allow a user to use WhatsApp on both their phone and a web browser simultaneously while keeping all messages end-to-end encrypted without storing private keys on the server?
2. **Disappearing Messages Architecture:** How would you design disappearing messages that self-delete from both devices and the server after 24 hours, even if one user turns off their internet?
3. **VoIP Voice & Video Calling:** How does the architecture handle one-on-one and group voice/video calls using WebRTC, STUN, and TURN relay servers?
4. **Spam & Abuse Detection under E2EE:** If the server cannot read message contents, how does WhatsApp detect spammers, scammers, and mass automated broadcast abuse?
5. **Data Backup & Restore:** How does chat backup to Google Drive or iCloud work while preserving end-to-end encryption?
