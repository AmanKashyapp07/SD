# System Design: URL Shortener (High-Level Design)

A comprehensive guide for technical interviews, covering the end-to-end architecture of a scalable, high-throughput URL shortening service (like TinyURL or Bitly).

---

## 1. Functional Requirements

What the system must do from the user perspective:

1. **URL Shortening:** Given a long URL, the system generates a unique, shorter alias (e.g., `https://tiny.url/xyz123`).
2. **URL Redirection:** When a user visits the short URL, the system immediately redirects them to the original long URL.
3. **Custom URL Alias (Optional):** Users can optionally provide their own custom short alias (e.g., `https://tiny.url/my-portfolio`), provided it is not already taken.
4. **Expiration Time (TTL):** Users can optionally specify an expiration date after which the short link becomes invalid.
5. **Basic Analytics & Metrics:** Track click count, timestamp of access, referring source, and geographical location for each short link.

---

## 2. Non-Functional Requirements

System quality attributes, constraints, and operational guarantees:

1. **Ultra-Low Latency Redirection:** Redirection must happen in under 10–20 milliseconds to provide a seamless browsing experience.
2. **High Availability:** 99.999% uptime. If the redirection service goes down, every linked service across the web breaks.
3. **Extremely Read-Heavy Workload:** The ratio of reads (URL redirections) to writes (URL creations) is typically **100:1** or higher.
4. **Non-Predictable / Non-Guessable URLs:** Short URLs should not be easily sequential or guessable (to prevent attackers from scraping or discovering private links).
5. **High Durability:** Once created, a URL mapping must never be lost.

---

## 3. Back-of-the-Envelope Estimation

Let us calculate the storage, requests per second (QPS), and caching requirements over a 5-year operational horizon.

### Core Assumptions
- New URLs shortened per month = **100 Million URLs/month**
- Read-to-write ratio = **100:1** (for every 1 URL created, it is clicked 100 times)
- Short URL lifespan (retention period) = **5 years**
- Character set for short URL = Base62 (`[0-9, a-z, A-Z]`)

### 1. Write and Read Throughput (QPS)
- **Write QPS (URL Creation):**
  $$\text{Write QPS} = \frac{100\text{ Million}}{30\text{ days} \times 86,400\text{ sec}} \approx 40\text{ writes/second}$$
  - Peak Write QPS ($2\times$) $\approx$ **100 writes/second** (very low write load).

- **Read QPS (URL Redirection):**
  $$\text{Read QPS} = 40\text{ writes/sec} \times 100 \approx 4,000\text{ reads/second}$$
  - Peak Read QPS ($2.5\times$) $\approx$ **10,000 reads/second**.

### 2. Base62 URL Length Calculation
We need enough unique character combinations to support 5 years of URL creation:
$$\text{Total URLs in 5 Years} = 100\text{ Million/month} \times 12\text{ months} \times 5\text{ years} = 6\text{ Billion URLs}$$

Using **Base62** encoding (62 possible alphanumeric characters: 10 digits + 26 lowercase + 26 uppercase):
- 6 characters: $62^6 \approx 56.8\text{ Billion}$ unique combinations (sufficient).
- 7 characters: $62^7 \approx 3.52\text{ Trillion}$ unique combinations (industry standard for headroom and security).

Therefore, a **7-character string** provides more than enough capacity.

### 3. Storage Estimation (5 Years)
Each database record contains:
- `short_key`: 7 bytes
- `long_url`: 500 bytes (average)
- `user_id`: 8 bytes
- `created_at`: 8 bytes
- `expires_at`: 8 bytes
- Metadata / overhead: ~50 bytes
- **Total size per record $\approx$ 580 Bytes $\approx$ 0.6 KB**

Total storage needed for 6 Billion records over 5 years:
$$\text{Total 5-Year Storage} = 6\text{ Billion} \times 0.6\text{ KB} = 3.6\text{ TB}$$
*(3.6 TB easily fits into a small distributed database cluster).*

### 4. Cache Memory Estimation (80/20 Rule)
According to the Pareto Principle, 20% of the short URLs generate 80% of the daily redirection traffic.
- Daily redirections = $4,000\text{ reads/sec} \times 86,400\text{ sec} \approx 345\text{ Million requests/day}$
- Number of URLs to cache (20% of daily active URLs) $\approx 69\text{ Million URLs}$
- Cache Memory required = $69\text{ Million} \times 0.6\text{ KB} \approx 41.4\text{ GB of RAM}$
*(41.4 GB easily fits onto a single modern Redis server, though we use a replicated Redis cluster for high availability).*

---

## 4. High-Level Architecture

The system uses a clean separation between the write path (URL creation) and the read path (URL redirection), backed by an asynchronous analytics pipeline.

### ASCII Architecture Diagram

```
+-----------------------------------------------------------------------------+
|                                CLIENT BROWSERS                              |
+----------------------+-------------------------------+----------------------+
                       |                               |
        1. POST /api/shorten (Write)                   | 2. GET /xyz123 (Read)
                       |                               |
                       v                               v
+-----------------------------------------------------------------------------+
|                               LOAD BALANCER                                 |
|                       (Nginx / AWS Application LB)                          |
+----------------------+-------------------------------+----------------------+
                       |                               |
                       v                               v
+------------------------------------+   +------------------------------------+
|          WRITE SERVICE             |   |           READ SERVICE             |
|     (URL Creation & Validation)    |   |     (Redirection Handler)          |
+------------------+-----------------+   +-----------------+------------------+
                   |                                       |
        +----------+-----------+                           | 1. Check Cache
        |                      |                           v
        v                      v                 +--------------------+
+----------------+     +----------------+        |    REDIS CACHE     |
| KEY GENERATION |     | PRIMARY DB     |        | (Hot URL Mappings) |
| SERVICE (KGS)  |     | (DynamoDB /    |<-------+--------------------+
| (Token Range)  |     |  PostgreSQL)   |          2. Fallback on Miss
+----------------+     +-------+--------+
                               |
                               | 3. Emits Click Event (Async)
                               v
                     +--------------------+
                     |    APACHE KAFKA    |
                     +---------+----------+
                               |
                               v
                     +--------------------+
                     | ANALYTICS CONSUMER |
                     | (Aggregates Clicks)|
                     +---------+----------+
                               |
                               v
                     +--------------------+
                     |  CLICKHOUSE / DB   |
                     |  (Analytics Store) |
                     +--------------------+
```

---

## 5. Key Components & Why

### 1. Key Generation Strategy: Pre-Generated Keys vs Hash Collisions
There are two primary approaches to generate short keys:

- **Approach A: Hash-and-Truncate (MD5/SHA256):**
  - Compute `MD5(long_url)` and take the first 7 characters.
  - *Drawback:* Different URLs can produce the same first 7 characters (hash collision). Checking the database for collisions and appending salts adds database read latency and complexity.

- **Approach B: Key Generation Service (KGS) / Distributed Counter (Recommended):**
  - A standalone service pre-generates billions of unique 7-character Base62 keys in advance and stores them in a `Key-DB`.
  - When the Write Service needs a key, it simply takes the next available key from memory without hashing or collision checking.
  - *Why this choice:* Eliminates collision checks, guarantees $O(1)$ key generation, and prevents race conditions between concurrent requests.

### 2. Primary Database: NoSQL Key-Value vs Relational
- **NoSQL Key-Value / Wide-Column (Amazon DynamoDB / Apache Cassandra):**
  - *Why this choice:* The data model is simple: a single lookup key (`short_key`) mapping to a value (`long_url`). No relational joins or complex queries are needed. NoSQL provides sub-5ms lookups, linear horizontal scaling, and built-in TTL support to automatically delete expired URLs.
- **Relational Alternative (PostgreSQL / MySQL):**
  - Works perfectly fine for small-to-medium scale ($<10\text{ TB}$) using a simple indexed table. However, horizontal sharding by `short_key` is required as data grows.

### 3. In-Memory Caching Layer: Redis Cluster
- **What it does:** Caches frequently accessed short URLs with an **LRU (Least Recently Used)** eviction policy.
- **Why this choice:** Over 80% of redirect requests hit the cache directly, serving responses in under 2 milliseconds and protecting the database from read spikes.

### 4. Asynchronous Analytics Pipeline (Kafka + ClickHouse)
- **What it does:** Tracks click statistics (timestamp, user agent, IP address, country).
- **Why this choice:** Analytics must **never** block the critical redirection path. The Read Service writes an event to an Apache Kafka topic asynchronously, and background stream workers aggregate statistics into a columnar analytics database (like ClickHouse or TimescaleDB).

---

## 6. How URL Redirection Actually Works

### 1. HTTP 301 vs HTTP 302 Redirection: Critical Interview Concept
When the server redirects the browser to the original URL, it returns an HTTP redirect status code:

| Status Code | Meaning | Browser Behavior | Impact on Analytics |
| :--- | :--- | :--- | :--- |
| **HTTP 301** | *Moved Permanently* | Browser caches the long URL permanently. Future clicks bypass our server completely. | **Bad for analytics:** We cannot track subsequent clicks. Saves server bandwidth. |
| **HTTP 302 / 307** | *Found / Temporary Redirect* | Browser does not cache the redirect. Every single click hits our server first. | **Ideal for analytics:** Every click is registered. Slight increase in server load. |

**Interview Verdict:** Use **HTTP 302 (or 307)** if tracking click analytics is a requirement; use **HTTP 301** if minimizing server load and bandwidth is the primary priority.

### 2. Step-by-Step URL Shortening Flow (Write Path)
1. User sends `POST /api/shorten` with `{ "long_url": "https://example.com/very/long/path" }`.
2. Write Service queries the **Key Generation Service (KGS)** to fetch a pre-generated 7-character key (e.g., `aB3x9Q1`).
3. Write Service saves `{ "short_key": "aB3x9Q1", "long_url": "https://example.com/very/long/path", "created_at": NOW() }` into the Database.
4. Write Service inserts the mapping into Redis Cache.
5. Returns `https://tiny.url/aB3x9Q1` to the client.

### 3. Step-by-Step Redirection Flow (Read Path)
1. User clicks `https://tiny.url/aB3x9Q1`.
2. Browser sends `GET /aB3x9Q1` to the Load Balancer.
3. Read Service checks the **Redis Cache** for key `aB3x9Q1`.
   - **Cache Hit:** Retrieves `long_url` directly from memory ($<2\text{ms}$).
   - **Cache Miss:** Queries Database, populates Redis cache for future requests.
4. Read Service emits an asynchronous click event to **Apache Kafka** (containing IP, User-Agent, Timestamp).
5. Read Service returns HTTP `302 Found` with header `Location: https://example.com/very/long/path`.
6. Browser immediately navigates to the long URL.

---

## 7. Bottlenecks & Solutions

### 1. KGS Single Point of Failure and Token Exhaustion
- **Problem:** If the Key Generation Service goes down, new URL creation halts. If two server instances request a key simultaneously, they might receive the same token.
- **Solution (Range-Based Token Allocation):**
  - Use Apache ZooKeeper to manage token ranges.
  - Server 1 is allocated range `[1 to 1,000,000]`.
  - Server 2 is allocated range `[1,000,001 to 2,000,000]`.
  - Each server dispenses tokens locally from memory without cross-server synchronization. When a server uses up its range, it requests a new block from ZooKeeper.

### 2. Cache Stampede (Hot Key Problem)
- **Problem:** A celebrity tweets a shortened link, causing 50,000 concurrent requests within 5 seconds. If the key expires from the cache at that moment, all 50,000 requests hit the database simultaneously, causing a database crash.
- **Solution:**
  - **Mutex / Single-Flight Locking:** Only the first request is allowed to fetch from the database and populate the cache; all other requests wait for the cache to update.
  - **Pre-Warming & Longer TTL for Trending Keys:** Trending links have their cache TTL dynamically extended.

### 3. Handling Expired URLs & Database Cleanup
- **Problem:** Deleting billions of expired records using `DELETE FROM urls WHERE expires_at < NOW()` causes massive table locks and performance degradation.
- **Solution:**
  - **Lazy Deletion:** When a user visits an expired link, check the timestamp. If expired, return HTTP 404 and delete the record on the spot.
  - **Background Scheduled Cleanup:** Run a lightweight cron worker during low-traffic hours (e.g., 3:00 AM) that deletes expired entries in small batches.
  - **Database Native TTL:** In DynamoDB or Cassandra, configure automatic TTL deletion handled internally by the storage engine.

### 4. Malicious URLs, Spam, and Phishing
- **Problem:** Attackers use URL shorteners to disguise phishing links or malware downloads.
- **Solution:**
  - Before shortening, validate the target domain against Google Safe Browsing API or internal blacklists.
  - Implement rate limiting per IP / User Account using Token Bucket algorithm in Redis.

---

## 8. Follow-up Questions an Interviewer Might Ask

These are realistic interview follow-ups to think through on your own:

1. **Custom Alias Handling:** How would you handle a user requesting a custom alias (e.g., `https://tiny.url/black-friday`)? How do you ensure high performance while preventing race conditions when two users request the same custom alias at the exact same millisecond?
2. **Real-Time Click Analytics Dashboard:** How would you design a real-time dashboard showing the link creator a live count of clicks grouped by country, browser, and referrer over the last 24 hours?
3. **Link Editing and URL Deletion:** If a user updates their long URL or deletes a short link, how do you handle cache invalidation globally across multiple data centers?
4. **Preventing Link Guessing / Security:** If short URLs are generated from sequential numbers, attackers can crawl all links from `0000000` to `9999999`. How do you make generated tokens unpredictable while preserving Base62 efficiency?
5. **Multi-Region Active-Active Deployment:** How would you deploy this service across US, Europe, and Asia so users experience sub-10ms redirects regardless of geographical location?
