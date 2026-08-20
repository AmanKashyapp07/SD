# System Design Building Block: API Protocols

Understanding how distributed systems talk to each other: REST, GraphQL, gRPC, WebSockets, Server-Sent Events, and Webhooks.

---

## 1. Start With the Problem

Imagine you are building a modern ride-sharing application like Uber or Lyft. Your system has three core features:
1. A rider browses available rides and requests a price estimate.
2. Once booked, the rider's phone must show the driver's live GPS location moving across a map in real time (updated every 1 second).
3. Internally, when a ride request arrives, your backend needs to query 15 internal microservices simultaneously (Pricing Engine, Fraud Detection, Driver Matching, Surge Calculator, User Profile, Payment Gateway) to approve and dispatch the trip in under 100 milliseconds.

Suppose you decide to use standard **REST APIs over HTTP/1.1 with JSON payloads** for everything because it is familiar and easy to build.

Here is what happens under the hood:

- **For the live GPS map:** Your mobile app has to send a new HTTP `GET /driver/location` request every single second (Short Polling). Each request initiates a new TCP connection (or keeps one alive), sends hundreds of bytes of repetitive HTTP headers (User-Agent, Authorization, Cookies), waits for the server to process, and receives a tiny payload: `{"lat": 37.77, "lng": -122.41}`. 90% of the network packets sent over mobile data are header overhead rather than actual GPS coordinates. Worse, if the driver is stuck at a red light and hasn't moved, the app still asks every second, wasting battery and cellular bandwidth while hammering your servers with millions of useless requests per minute.
- **For the internal microservices:** When a ride request arrives, your API Gateway makes 15 HTTP/1.1 REST calls to internal services. Because HTTP/1.1 does not support native multiplexing on a single TCP connection, your gateway opens dozens of separate TCP connections. Each service serializes large human-readable JSON strings and deserializes them back into memory. The JSON parsing overhead across 15 microservices adds 80 milliseconds of unnecessary CPU delay. The rider experiences a sluggish app, and during peak rainstorms, your servers crash under CPU exhaustion from parsing JSON text instead of doing actual business logic.

This painful failure demonstrates a fundamental rule of distributed systems: **There is no single "magic" protocol for all communication.** Using a request-response protocol for real-time streaming, or using heavy text-based protocols for internal microservice communication, introduces latency, CPU bottlenecks, and poor user experiences.

---

## 2. What It Is

An **API Protocol** is a standardized set of rules, message formats, and transport mechanisms that govern how two software systems exchange data over a network.

At its core, an API protocol defines three things:
1. **The Transport Layer & Connection Model:** How connections are opened and maintained (e.g., short-lived HTTP request-response vs. long-lived bidirectional TCP sockets).
2. **The Message Framing & Data Format:** How data is packaged and serialized across the wire (e.g., human-readable JSON/XML text vs. highly compressed binary byte streams like Protocol Buffers).
3. **The Interaction Pattern:** Who initiates communication and how data flows (Client-Pull, Server-Push, Bidirectional Streaming, or Asynchronous Event Notification).

Instead of treating the network as a generic pipe, choosing the right API protocol allows you to optimize for your exact bottleneck—whether that is minimizing network bandwidth on mobile devices, achieving sub-millisecond RPC latency across internal data centers, or pushing instant live updates to millions of connected web browsers.

---

## 3. How It Works — Step by Step

In modern system design, you will encounter six major API communication paradigms. Let us examine the mechanics, data framing, and transport behavior of each.

```
+-----------------------------------------------------------------------------+
|                        API PROTOCOLS AT A GLANCE                            |
+-------------------+--------------------+------------------+-----------------+
| PROTOCOL          | TRANSPORT          | DATA FORMAT      | DIRECTIONALITY  |
+-------------------+--------------------+------------------+-----------------+
| REST              | HTTP/1.1, HTTP/2   | JSON, XML, Text  | Request-Response|
| GraphQL           | HTTP (POST/GET)    | JSON (Schema)    | Request-Response|
| gRPC              | HTTP/2, HTTP/3     | Protobuf (Binary)| Bidirectional   |
| WebSocket         | TCP (Upgraded)     | Text / Binary    | Full-Duplex     |
| Server-Sent Events| HTTP/1.1, HTTP/2   | Text (Event-Stream)| Server-to-Client|
| Webhooks          | HTTP (POST)        | JSON             | Server-to-Server|
+-------------------+--------------------+------------------+-----------------+
```

---

### 1. REST (Representational State Transfer)

REST is an architectural style built directly on top of the HTTP protocol. It treats data as **resources** identified by unique URLs (URIs) and manipulates them using standard HTTP methods.

#### The Mechanics:
- `GET /users/123`: Retrieve user resource (Safe, Idempotent).
- `POST /users`: Create a new user (Not Idempotent).
- `PUT /users/123`: Replace existing user entirely (Idempotent).
- `PATCH /users/123`: Partially update existing user fields (Not necessarily Idempotent).
- `DELETE /users/123`: Remove user resource (Idempotent).

#### Wire Format Example:
```http
POST /api/v1/orders HTTP/1.1
Host: api.store.com
Authorization: Bearer eyJhbGciOi...
Content-Type: application/json
Content-Length: 58

{
  "item_id": "item_987",
  "quantity": 2,
  "currency": "USD"
}
```

#### Why it matters:
REST is universally understood by every browser, firewall, proxy, and programming language. It leverages native HTTP caching (`Cache-Control`, `ETag`) and standard status codes (`200 OK`, `201 Created`, `404 Not Found`, `500 Server Error`).

---

### 2. GraphQL (Client-Driven Query Language)

Created by Meta (Facebook) in 2012, GraphQL replaces fixed REST endpoints with a **single endpoint** (typically `POST /graphql`) where the client explicitly specifies the exact shape and fields of the data it needs.

#### The Problem GraphQL Solves:
In REST, calling `GET /users/123` returns a massive JSON payload with 40 fields (name, email, address, friends, settings, timestamps). If a mobile phone only needs the user's `name` and `profile_pic_url`, the remaining 38 fields are wasted network bandwidth (**Over-fetching**). Conversely, if you need a user's name and their latest 3 orders, REST forces you to make multiple requests (`GET /users/123` then `GET /users/123/orders`) (**Under-fetching** / N+1 problem).

#### The Mechanics (Schema & Resolver):
1. The backend defines a typed schema:
```graphql
type User {
  id: ID!
  name: String!
  avatarUrl: String
  orders(limit: Int): [Order]
}
```
2. The client sends a custom query requesting only what it requires:
```graphql
query GetUserProfile {
  user(id: "123") {
    name
    orders(limit: 2) {
      id
      totalAmount
    }
  }
}
```
3. The server executes resolver functions to gather the data from multiple databases or internal services and returns a single JSON object matching the exact requested structure.

---

### 3. gRPC (Google Remote Procedure Call)

Developed by Google, gRPC is an open-source, high-performance Remote Procedure Call (RPC) framework. Instead of calling URLs with HTTP verbs, a client invokes a method on a remote server as if it were a local in-memory function call.

```
+------------------+                    +------------------+
|   CLIENT (Go)    |                    |  SERVER (Java)   |
|                  |                    |                  |
|  client.GetOrder |                    |  implement       |
|      (orderId)   |                    |   GetOrder()     |
+--------+---------+                    +--------+---------+
         |                                       ^
         v                                       |
+------------------+                    +------------------+
| Generated Stub   |                    | Generated Stub   |
| (Protobuf Encode)|                    | (Protobuf Decode)|
+--------+---------+                    +--------+---------+
         |                                       ^
         |======== HTTP/2 Binary Frames =========|
         +---------------------------------------+
```

#### The Mechanics (Protobuf + HTTP/2):
1. **Contract-First Definition (`.proto` file):**
```protobuf
syntax = "proto3";

service OrderService {
  rpc GetOrder (OrderRequest) returns (OrderResponse);
}

message OrderRequest {
  string order_id = 1;
}

message OrderResponse {
  string order_id = 1;
  double amount = 2;
  string status = 3;
}
```
2. **Binary Serialization (Protocol Buffers):**
Instead of sending text `"order_id": "12345"` (which takes 20 bytes of ASCII text), Protobuf assigns field numbers (e.g., field tag `1`). It encodes keys as tiny 1-byte integer tags and serializes values in dense binary format.
3. **Transport via HTTP/2:**
gRPC uses HTTP/2 multiplexing, allowing hundreds of concurrent RPC calls to stream simultaneously over a single long-lived TCP connection without head-of-line blocking. It natively supports bidirectional streaming (Client Streaming, Server Streaming, Bidirectional Streaming).

---

### 4. WebSocket (Full-Duplex Persistent Sockets)

WebSocket provides a persistent, full-duplex, bidirectional communication channel over a single TCP connection.

#### The Mechanics (The HTTP Handshake Upgrade):
1. The client initiates a standard HTTP request with an `Upgrade` header:
```http
GET /chat HTTP/1.1
Host: server.com
Upgrade: websocket
Connection: Upgrade
Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==
Sec-WebSocket-Version: 13
```
2. The server responds with HTTP status `101 Switching Protocols`:
```http
HTTP/1.1 101 Switching Protocols
Upgrade: websocket
Connection: Upgrade
Sec-WebSocket-Accept: s3pPLMBiTxaQ9kYGzzhZRbK+xOo=
```
3. From this point forward, the HTTP protocol is discarded. The underlying TCP socket remains open. Both client and server can send lightweight binary or text frames (with only 2 to 10 bytes of frame overhead) at any millisecond without handshake or header overhead.

---

### 5. Server-Sent Events (SSE)

Server-Sent Events allow a server to push real-time text updates to a web client over a standard, long-lived HTTP connection.

#### The Mechanics:
- Unlike WebSockets, SSE is **unidirectional** (Server to Client only).
- The client makes a standard HTTP request with header `Accept: text/event-stream`.
- The server responds with `Content-Type: text/event-stream` and leaves the HTTP connection open indefinitely.
- Whenever new data occurs, the server sends a simple formatted text block:
```http
data: {"stock": "AAPL", "price": 185.50}

data: {"stock": "GOOG", "price": 142.20}
```
- Web browsers have built-in native support for SSE via the `EventSource` JavaScript API, including automatic reconnection and message ID tracking if the connection drops.

---

### 6. Webhooks (Asynchronous Server-to-Server Event Push)

A Webhook (often called a "Reverse API") is a mechanism where an external service sends an automated HTTP POST request to your server whenever a specific event happens.

#### The Mechanics:
- Instead of your server polling Stripe every 5 seconds asking *"Did customer X complete payment yet?"*, your server registers a callback URL with Stripe: `https://myapp.com/webhooks/stripe`.
- When the customer completes payment 20 minutes later, Stripe's servers execute an HTTP POST request to your registered URL with the event payload:
```json
{
  "event": "payment_intent.succeeded",
  "amount": 4900,
  "customer_id": "cus_N83x9Q"
}
```
- Your server verifies the cryptographic signature (HMAC header), returns `HTTP 200 OK` to acknowledge receipt, and processes the order.

---

## 4. Why This Design and Not Something Simpler?

To truly master API design in an interview, you must understand what naive solutions were attempted first and why they failed.

```
EVOLUTION OF REAL-TIME & INTER-SERVICE COMMUNICATION

1. Short Polling (HTTP/1.1)
   [Client] -- GET /status --> [Server] (No update)
   [Client] -- GET /status --> [Server] (No update)
   [Client] -- GET /status --> [Server] (Update ready!)
   -> Flaw: Wasted bandwidth, high latency, server CPU exhaustion.

2. Long Polling (Comet)
   [Client] -- GET /status ---------> [Server] (Holds connection open)
   [Client] <---- Status Update ------ [Server] (Closes connection)
   [Client] -- GET /status (Reopen) -> [Server]
   -> Flaw: Better, but still incurs repeated TCP/TLS handshakes and header overhead.

3. Persistent Full-Duplex (WebSocket / SSE)
   [Client] <==== Single Persistent TCP Socket ====> [Server]
   -> Solution: Zero per-message connection overhead, instant sub-millisecond push.

4. Internal Microservice RPC (JSON REST -> gRPC)
   JSON Text over HTTP/1.1  -->  Protobuf Binary over HTTP/2
   -> Solution: 7x-10x faster serialization, multiplexed connections, strict type safety.
```

### Why not use REST for everything?
REST with JSON is human-readable, easy to debug with `curl` or Postman, and universally compatible. However, for internal microservices processing billions of requests per day, text serialization/deserialization burns massive CPU cycles. Furthermore, HTTP/1.1's lack of multiplexing leads to thread starvation and connection pool exhaustion.

### Why not use WebSockets for everything?
WebSockets are powerful, but they are stateful. Load balancing stateful TCP sockets across autoscaling server clusters is complex (requiring Redis pub/sub backplanes or sticky sessions). WebSockets do not support native HTTP caching, automatic retries, or standard status codes. Using WebSockets for simple CRUD operations (like updating a user's email) is an anti-pattern that adds unnecessary architectural complexity.

---

## 5. Real-World Usage

Here is how top engineering organizations intentionally combine different API protocols across their architecture:

### 1. Netflix (gRPC for Internal Microservices)
- **Use Case:** Netflix operates thousands of microservice instances across AWS.
- **Why gRPC:** By migrating from REST/JSON to gRPC with Protocol Buffers for inter-service communication, Netflix achieved a 90% reduction in serialization CPU overhead and eliminated connection bottlenecks through HTTP/2 multiplexing.

### 2. Meta / Facebook (GraphQL for Mobile News Feed)
- **Use Case:** Loading complex, relational social media feeds on mobile devices operating on slow 3G/4G networks worldwide.
- **Why GraphQL:** A single feed item includes author details, post text, image assets, likes count, top 3 comments, and friendship status. GraphQL allows the mobile client to query all of this nested data in a single round-trip, downloading only the exact bytes required to render the screen.

### 3. Discord & Slack (WebSockets for Real-Time Chat)
- **Use Case:** Real-time messaging, typing indicators, and voice presence for millions of concurrent users.
- **Why WebSockets:** When a user types a message or enters a voice channel, waiting for a poll interval is unacceptable. Discord maintains persistent WebSocket connections to Elixir/Erlang gateway servers, allowing messages to be broadcast to active channel members in under 20 milliseconds.

### 4. Stripe (Webhooks for Payment Lifecycle Events)
- **Use Case:** Asynchronous payment processing (Credit Card approvals, bank transfers, subscription renewals).
- **Why Webhooks:** Bank payments can take minutes, hours, or days to clear. Polling Stripe's servers continuously for millions of pending transactions would overwhelm both Stripe and the merchant. Stripe uses Webhooks with automatic exponential backoff retries and cryptographic signature verification to notify merchants the instant a transaction completes.

---

## 6. Trade-offs

| Protocol | Transport | Best Suited For | Main Advantages | Main Disadvantages / When NOT to Use |
| :--- | :--- | :--- | :--- | :--- |
| **REST** | HTTP/1.1, HTTP/2 | Public APIs, CRUD operations, web services | Simple, universally supported, native HTTP caching, easy to debug | Over/under-fetching, heavy JSON overhead, no native push |
| **GraphQL** | HTTP (POST) | Complex client-facing apps, mobile backends (BFF) | No over/under-fetching, single endpoint, strong schema typing | Complex server resolvers, hard to cache via standard HTTP CDN, risk of expensive nested queries |
| **gRPC** | HTTP/2, HTTP/3 | Internal microservices, high-throughput RPC, polyglot backends | High binary performance, small payload size, multiplexing, bidirectional streaming | Not natively supported by web browsers without gRPC-Web proxy, binary payload is not human-readable |
| **WebSocket**| Persistent TCP | Live chat, multiplayer games, real-time trading dashboards | Full-duplex bidirectional, ultra-low frame overhead (2-10 bytes) | Stateful connections make horizontal scaling and load balancing complex, no built-in HTTP caching |
| **SSE** | HTTP/1.1, HTTP/2 | Live stock tickers, news feeds, LLM token streaming (ChatGPT) | Lightweight, unidirectional server push, built-in browser reconnection, works through corporate firewalls | Unidirectional only (client cannot push back over the same stream), text-only format |
| **Webhooks** | HTTP (POST) | Asynchronous 3rd-party integrations, payment processing | Event-driven, zero polling waste, simple server-to-server HTTP | Requires public callback endpoint, requires retry mechanisms and idempotency handling for failed deliveries |

---

## 7. Common Pitfalls & Misconceptions

### 1. Believing WebSockets Should Replace REST for High Performance
- **The Misconception:** "WebSockets are faster than REST, so we should build our entire mobile app API on WebSockets."
- **The Reality:** WebSockets eliminate HTTP headers and handshakes for continuous streaming, but they make your architecture **stateful**. You lose HTTP caching (Cloudflare/Akamai cannot cache WebSocket responses), API gateway route caching, and simple stateless load balancing. Use REST for standard CRUD and reserve WebSockets strictly for real-time bidirectional features.

### 2. Assuming GraphQL Eliminates Backend Database Queries
- **The Misconception:** "GraphQL gets all my data in one request, so my backend is automatically faster."
- **The Reality:** GraphQL solves network over-fetching between the *client and server*, but it can easily create severe **N+1 query problems** between your *server and database* if resolvers are poorly implemented. Without batching and caching tools like `DataLoader`, a single nested GraphQL query can trigger hundreds of database queries behind the scenes.

### 3. Thinking gRPC Replaces WebSockets in Web Browsers
- **The Misconception:** "We will use gRPC for our web frontend to get fast binary streaming directly to the browser."
- **The Reality:** Standard web browsers do not expose fine-grained control over raw HTTP/2 framing to JavaScript. To use gRPC in a web browser, you must use a translation layer like `gRPC-Web` combined with an Envoy reverse proxy, which converts gRPC into browser-compatible HTTP requests.

---

## 8. Common Interview Questions

### Question 1 (Basic): When would you choose Server-Sent Events (SSE) over WebSockets?
- **What the interviewer is testing for:** Do you understand the difference between unidirectional and bidirectional communication, and do you know how to choose the simpler, lighter tool?
- **Key Answer Points:** SSE is unidirectional (Server to Client), runs over standard HTTP, supports native browser reconnection via `EventSource`, and passes effortlessly through corporate firewalls and HTTP/2 proxies. If the client does not need to push data back over the same socket (e.g., ChatGPT streaming AI response tokens, live sports score tickers, stock feeds), SSE is dramatically simpler and more robust than WebSockets.

### Question 2 (Intermediate): How does gRPC achieve significantly higher throughput than REST with JSON?
- **What the interviewer is testing for:** Deep understanding of serialization mechanisms (Protobuf vs. JSON text) and transport protocols (HTTP/2 vs. HTTP/1.1).
- **Key Answer Points:** 
  1. Protocol Buffers serialize data into compact binary byte streams with integer tags instead of verbose string keys.
  2. Parsing binary Protobuf requires far fewer CPU cycles than parsing human-readable JSON text strings.
  3. gRPC runs over HTTP/2, which supports native binary framing, header compression (HPACK), and connection multiplexing (multiple concurrent requests sharing a single TCP connection).

### Question 3 (Advanced): How do you scale a WebSocket architecture to support 10 million concurrent users across multiple backend servers?
- **What the interviewer is testing for:** Understanding of stateful connection management, distributed session routing, and pub/sub backplanes.
- **Key Answer Points:**
  1. Use Layer 4 (TCP) or Layer 7 Load Balancers capable of handling WebSocket upgrades.
  2. Implement an in-memory session registry (e.g., Redis Cluster) mapping `UserID -> GatewayServer_ID`.
  3. Use a distributed Message Broker / Pub-Sub system (e.g., Redis Pub/Sub, Apache Kafka, or RabbitMQ) to broadcast messages across gateway servers so Server A can route a message to User B connected to Server F.

### Question 4 (Probing): How do you guarantee message delivery and prevent duplicate processing in Webhook systems?
- **What the interviewer is testing for:** Reliability engineering, at-least-once delivery, idempotency keys, and security.
- **Key Answer Points:**
  1. **Idempotency:** Every webhook payload must include a unique `event_id`. The receiving server checks an idempotency store (e.g., Redis or SQL table) before processing to ensure duplicate webhooks are ignored.
  2. **Exponential Backoff Retries:** If the receiver returns 5xx or times out, the sender retries using exponential backoff (e.g., retry after 5s, 30s, 5m, 1h).
  3. **Security (HMAC):** Payloads are signed with a shared secret in the header (e.g., `X-Signature-SHA256`) so the receiver can verify message authenticity.

---

## 9. How to Explain It in an Interview

When an interviewer asks: *"Which API protocol would you use for this system and why?"*, use this structured formula:

### Sample 4-Sentence Answer Script:
> "For this component, I recommend using **[Protocol Name]** because our primary constraint is **[Specific Constraint: e.g., low-latency real-time updates / high-throughput inter-service communication / flexible mobile client querying]**. 
> Unlike **[Alternative Protocol]**, which suffers from **[Specific Drawback: e.g., high polling overhead / heavy JSON CPU serialization / over-fetching]**, **[Protocol Name]** operates by **[How it works: e.g., streaming over a persistent HTTP/2 connection / compressing binary payloads via Protobuf]**. 
> The main trade-off is **[Specific Trade-off: e.g., stateful connection complexity / lack of native browser support]**, which we will mitigate by **[Mitigation Strategy: e.g., using Redis pub/sub for socket routing / placing an Envoy proxy at the edge]**."

### Why this structure works:
It proves you don't just pick technologies based on hype. You clearly state the **requirement**, contrast it with the **naive alternative**, explain the **mechanism**, acknowledge the **trade-off**, and provide a concrete **mitigation strategy**.

---

## 10. Related Building Blocks

To deepen your system design knowledge, explore these closely connected concepts:

1. **Load Balancers (Layer 4 vs. Layer 7):**
   - *Relationship:* Standard HTTP/REST calls can be load balanced per request (Layer 7). Persistent protocols like WebSockets or gRPC streams require Layer 4 TCP load balancing or specialized Layer 7 proxies (like Envoy or HAProxy) that understand HTTP/2 multiplexing and WebSocket upgrades.
2. **Message Queues & Event-Driven Architecture (Kafka / RabbitMQ):**
   - *Relationship:* While API protocols handle synchronous or real-time point-to-point communication (Client-to-Server or Server-to-Server), Message Queues handle asynchronous background processing, decoupling producers from consumers when an immediate response is not required.
3. **Data Serialization Formats (JSON vs. Protocol Buffers vs. Apache Avro):**
   - *Relationship:* The data format is directly tied to the protocol choice. Understanding how binary serialization encodes fields, supports backward/forward schema compatibility, and saves CPU cycles is essential for microservice architecture design.

---

## 11. Check Your Understanding

Test your grasp of this building block by answering these three conceptual questions:

1. **Scenario Analysis:** You are designing a real-time collaborative document editor like Google Docs. Users need to see each other's keystrokes and cursor positions with sub-50ms latency, while also periodically saving document snapshots. Which API protocols would you use for the live editing stream versus the document export/metadata service, and why?
2. **Mechanism Breakdown:** Why does HTTP/2 multiplexing eliminate the "head-of-line blocking" problem present in HTTP/1.1, and how does gRPC leverage this to handle hundreds of concurrent requests over a single TCP connection?
3. **Architecture Decision:** Your mobile application team complains that the app takes 4 seconds to load on mobile networks because it makes 8 sequential REST API calls to render the home screen. What are two distinct architectural solutions to solve this problem, and what are their trade-offs?
