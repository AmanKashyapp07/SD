# System Design: Netflix (High-Level Design)

A comprehensive guide for technical interviews, covering the end-to-end architecture of a global video streaming platform.

---

## 1. Functional Requirements

What the system must do from the user and content publisher perspective:

1. **Video Streaming:** Users can stream high-definition video on demand across different devices (Smart TVs, mobile phones, laptops, tablets).
2. **Adaptive Bitrate Streaming (ABR):** The system automatically adjusts video quality (from 360p to 4K) in real-time based on the user's internet bandwidth and screen resolution.
3. **Resume Playback (Bookmark State):** Users can pause a video on one device and resume seamlessly from the exact second on another device.
4. **Search and Discovery:** Users can search for movies, shows, genres, actors, and directors with low-latency auto-complete and typo tolerance.
5. **Personalized Recommendations:** Dynamic homepage tailored to individual user profiles based on viewing history, preferences, and similar user behaviors.
6. **Content Ingestion & Transcoding Pipeline:** Content creators/studios upload raw high-resolution master copies, which are split, transcoded into multiple formats and resolutions, and distributed globally.
7. **User Account & Billing Management:** User registration, multi-profile support per account, authentication, and subscription billing.

---

## 2. Non-Functional Requirements

System quality attributes, constraints, and performance guarantees:

1. **High Availability:** 99.99% uptime. Users should almost never see a downtime screen or service outage.
2. **Ultra-Low Latency Playback:** Video startup latency (time from clicking "Play" to the first frame appearing) must be under 200–500 milliseconds. Zero buffering or stuttering during playback.
3. **Eventual Consistency for Non-Critical Data:** Viewing history, ratings, and recommendations can be eventually consistent (a delay of a few seconds is acceptable).
4. **High Consistency for Critical Data:** User authentication, payment status, and profile active locks require strict consistency.
5. **Massive Scalability:** Able to handle 250+ million active subscribers and tens of millions of concurrent video streams during peak hours.
6. **Global Distribution:** Content must be cached and served close to users worldwide to minimize physical network latency.

---

## 3. Back-of-the-Envelope Estimation

Let us calculate the storage, requests per second (RPS), and network bandwidth required at Netflix scale.

### Core Assumptions
- Total active subscribers = **300 Million**
- Daily Active Users (DAU) = **100 Million**
- Average watch time per user per day = **2 hours**
- Average bitrate across all qualities (SD, HD, 4K) = **3 Mbps** (Megabits per second)
- Peak traffic multiplier = **2x of average traffic**

### 1. Concurrent Video Streams & Bandwidth
- Total watch hours per day = $100\text{ Million users} \times 2\text{ hours} = 200\text{ Million hours/day}$
- Total seconds per day = $86,400\text{ seconds} \approx 10^5\text{ seconds}$
- Average concurrent streams = $\frac{200\text{ Million hours} \times 3600\text{ sec}}{86,400\text{ sec}} \approx 8.33\text{ Million concurrent streams}$
- Peak concurrent streams ($2\times$) = **16.67 Million concurrent streams**

**Bandwidth at Peak:**
- Average stream bitrate = $3\text{ Mbps} = 0.375\text{ MB/s}$ (Megabytes per second)
- Total peak egress bandwidth = $16.67\text{ Million streams} \times 3\text{ Mbps} \approx 50\text{ Tbps}$ (Terabits per second)
- This proves why cloud egress alone is too expensive and why a custom CDN (Open Connect) is mandatory.

### 2. API Request Throughput (Control Plane)
- An active user makes roughly 15 API requests per session (browsing home page, metadata, search, heartbeats every 10 seconds for bookmark state).
- Total API calls per day = $100\text{ Million users} \times 20\text{ requests} = 2\text{ Billion requests/day}$
- Average API RPS = $\frac{2\times 10^9}{86,400} \approx 23,000\text{ requests/second}$
- Peak API RPS ($2\times$) $\approx$ **50,000 requests/second**

### 3. Video Content Storage
- Total movie and TV show titles = **15,000 titles**
- Average duration per title = **90 minutes**
- Raw master file per title = **500 GB**
- Number of encoded formats (resolutions: 360p, 480p, 720p, 1080p, 4K; codecs: H.264, VP9, AV1; audio tracks & subtitles) $\approx$ **120 variations per title**
- Average size of encoded set per title = **300 GB**
- Total storage for encoded library = $15,000\text{ titles} \times 300\text{ GB} = 4.5\text{ PB}$ (Petabytes)
- With backups and staging, total video storage is roughly **10 to 15 PB** (very manageable, since Netflix's library size is relatively fixed compared to YouTube).

---

## 4. High-Level Architecture

Netflix splits its architecture into two major domains:
1. **Control Plane (AWS Cloud):** Handles everything that happens *before* you click play (login, search, recommendations, billing, metadata, transcoding pipeline).
2. **Data Plane (Netflix Open Connect CDN):** Handles everything that happens *after* you click play (actual video bit streaming).

### ASCII Architecture Diagram

```
+-----------------------------------------------------------------------------+
|                                CLIENT DEVICES                               |
|            (Smart TVs, Web Browsers, Mobile Apps, Tablets, Consoles)        |
+----------------------+-------------------------------+----------------------+
                       |                               |
          1. API / Metadata Requests                   | 2. Video Streaming (Chunks)
                       |                               |
                       v                               v
+------------------------------------+   +------------------------------------+
|            API GATEWAY             |   |        OPEN CONNECT CDN            |
|       (Zuul / Spring Cloud)        |   |   (Custom Edge Appliance Servers   |
+------------------+-----------------+   |      inside local ISP networks)    |
                   |                     +-----------------+------------------+
        +----------+-----------+                           ^
        |                      |                           | Pre-positioned
        v                      v                           | Video Chunks
+----------------+     +----------------+                  |
| AUTH & USER    |     | SEARCH SERVICE |                  |
| SERVICE        |     | (Elasticsearch)|                  |
+-------+--------+     +-------+--------+                  |
        |                      |                           |
        v                      v                           |
+----------------+     +----------------+                  |
| PLAYBACK &     |     | RECOMMENDATION |                  |
| BOOKMARK SVC   |     | ENGINE (ML)    |                  |
+-------+--------+     +-------+--------+                  |
        |                      |                           |
        +----------+-----------+                           |
                   |                                       |
                   v                                       |
+------------------------------------+                     |
|           DATA STORAGE             |                     |
|  - EVCache / Redis (Session/Cache) |                     |
|  - Cassandra (Viewing History)     |                     |
|  - RDBMS / MySQL (Billing, Auth)   |                     |
+------------------+-----------------+                     |
                   |                                       |
                   v                                       |
+------------------------------------+                     |
|     CONTENT INGESTION PIPELINE     |                     |
|  - Raw Master Video Upload (S3)    |                     |
|  - Archer / Transcoder Pipeline    |                     |
|  - Chunking, DRM & Encryption      +---------------------+
+------------------------------------+
```

---

## 5. Key Components & Why

### 1. Edge CDN: Netflix Open Connect
- **What it does:** A custom-built global content delivery network consisting of specialized storage appliances (Open Connect Appliances / OCAs) placed directly inside Internet Service Provider (ISP) data centers globally.
- **Why this choice:** Traditional third-party CDNs (Akamai, Cloudflare) would cost hundreds of millions of dollars for 50+ Tbps egress. Open Connect caches 95%+ of traffic locally inside user ISPs, eliminating transit costs and cutting latency down to single-digit milliseconds.

### 2. API Gateway: Zuul
- **What it does:** Front door for all client API requests. Provides dynamic routing, rate limiting, SSL termination, authentication token validation, and traffic shedding during load spikes.
- **Why this choice:** Allows central enforcement of security policies and dynamic canary routing to test new microservice versions on subsets of users.

### 3. Databases (Polyglot Persistence)
- **Apache Cassandra (NoSQL / Wide-Column):** Used for user viewing history, bookmark points, and analytics. It provides masterless, peer-to-peer linear write scalability across multiple AWS regions with tunable consistency.
- **MySQL / AWS Aurora (RDBMS):** Used for billing, subscription plans, and user credentials where ACID compliance and strict transactional consistency are non-negotiable.
- **Elasticsearch:** Used for title search, actor indexing, auto-complete, and typo-tolerant search queries.

### 4. Caching Layer: EVCache (Enhanced Memcached / Redis)
- **What it does:** In-memory distributed caching tier sitting in front of Cassandra and microservices.
- **Why this choice:** Handles millions of reads per second for metadata, user profiles, and UI layout configs with sub-millisecond response times.

### 5. Message Bus & Stream Processing: Apache Kafka + Apache Flink
- **What it does:** Real-time event streaming. Every user interaction (click, play, pause, hover, search) emits an event into Kafka.
- **Why this choice:** Decouples event producers from consumers. Downstream systems (recommendation algorithms, billing trackers, quality-of-service telemetry) process events asynchronously without impacting video playback.

### 6. Video Ingestion & Transcoding Pipeline (Archer / Titus)
- **What it does:** Slices raw 4K video files into tiny 2-to-5-second segments across various dimensions (resolution, codec, bitrate) and encodes them in parallel on thousands of worker instances.
- **Why this choice:** Transcoding a 2-hour movie as one monolithic file takes hours. Slicing it into thousands of parallel chunks reduces processing time to minutes and allows per-scene optimization.

---

## 6. How Video Streaming Actually Works

### Adaptive Bitrate Streaming (ABR)
Modern video streaming does not download one giant MP4 file. Instead:
1. Videos are cut into **2 to 5 second chunks**.
2. Each chunk is pre-encoded at multiple bitrates (e.g., 360p @ 400 kbps, 720p @ 1.5 Mbps, 1080p @ 4 Mbps, 4K @ 15 Mbps).
3. A **Manifest file (M3U8 / MPD file)** lists the URLs for every chunk and its available bitrate variations.
4. The client video player dynamically inspects current network speed and CPU usage:
   - Strong WiFi $\rightarrow$ downloads 1080p / 4K chunks.
   - Sudden network drop $\rightarrow$ seamlessly switches down to 480p chunks for the next 2-second piece without stopping or showing a loading spinner.

Protocols used: **HLS (HTTP Live Streaming)** by Apple and **DASH (Dynamic Adaptive Streaming over HTTP)**.

### Step-by-Step Request Lifecycle (Clicking "Play" to First Frame)
1. **User clicks Play:** Client sends an API request to the API Gateway (`/play?title_id=123&user_id=456`).
2. **Authorization & Bookmark Fetch:** Playback Service validates user subscription and retrieves the last watched timestamp from EVCache/Cassandra.
3. **Manifest Generation:** Playback Service identifies the user's device capability, DRM key, and returns the manifest file containing URLs pointing to the optimal Open Connect Appliance (OCA) nearest to the user.
4. **Direct CDN Streaming:** The client bypasses AWS entirely and connects directly via HTTP/2 or HTTP/3 to the designated local ISP Open Connect server to fetch video chunks.
5. **Continuous Heartbeat:** Every 10 seconds, the client sends a background heartbeat updating the current playback position in the database.

---

## 7. Bottlenecks & Solutions

### 1. Thundering Herd on Major Releases (e.g., Stranger Things Season Premiere)
- **Problem:** Millions of users simultaneously hit "Play" at 12:00 AM, which would overwhelm origin servers and network links.
- **Solution (Proactive Caching / Push Model):** Netflix does not fetch video on-demand (Pull model) during a premiere. During non-peak hours (2:00 AM - 6:00 AM) before release day, Netflix pushes and pre-populates all chunks of the new season directly onto the local hard drives of every Open Connect Appliance worldwide.

### 2. Regional Outages & Disaster Recovery
- **Problem:** An entire AWS cloud region goes down due to a data center fiber cut or power failure.
- **Solution (Multi-Region Active-Active):** Netflix runs in multiple AWS regions simultaneously. User traffic can be evacuated and rerouted from US-East to US-West in under 5 minutes using DNS routing (Route53) and cross-region Cassandra replication.

### 3. High Storage & Bandwidth Costs of 4K Content
- **Problem:** High bitrates consume massive storage and ISP bandwidth.
- **Solution (Per-Title & Per-Shot Encoding):** An action movie with rapid motion requires a high bitrate (e.g. 5 Mbps for 1080p), but an animated cartoon or talk show requires much less (e.g. 1.8 Mbps for 1080p). Netflix calculates custom encoding recipes per scene/shot, cutting overall bandwidth by 20–30% without visible loss in quality.

### 4. Client-Side Failure Resilience
- **Problem:** Intermittent network drops on mobile networks.
- **Solution:** Clients maintain a local buffer of 20–30 seconds of forward video chunks. If connection drops momentarily, playback continues uninterrupted from the buffer.

---

## 8. Follow-up Questions an Interviewer Might Ask

These are real interview follow-ups to think through on your own:

1. **New Release Traffic Spike:** How would you design the system so that when 50 million users hit "Play" at the exact same minute for a new season, the recommendation service and metadata APIs don't crash?
2. **Personalized Recommendations at Scale:** How does Netflix generate personalized row arrangements (e.g., "Trending Now", "Continue Watching", "Top 10") in real-time within 50 milliseconds?
3. **Live Streaming Architecture:** How would the architecture change if Netflix decided to stream live sports (e.g., NFL games or live boxing) instead of on-demand content?
4. **DRM & Anti-Piracy:** How does Netflix prevent unauthorized downloading and screen-recording across millions of web and mobile clients?
5. **Offline Viewing (Downloads):** How would you design the download feature allowing users to watch movies on an airplane with expiring access?
