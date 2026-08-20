# Company System Designs to Study — Placement Prep

Solid list of companies to study for system design placement prep, organized by what concept each teaches best.

## Foundational / Most Commonly Asked

- **URL Shortener (like Bitly/TinyURL)** — hashing, key generation, redirects, database choice
- **WhatsApp/Messenger** — real-time messaging, WebSockets, message delivery guarantees, online presence
- **Twitter/X** — news feed generation, fan-out on write vs read, timeline ranking
- **Instagram** — media storage, CDN usage, feed generation, follower graphs
- **YouTube/Netflix** — video storage, transcoding pipelines, CDN, adaptive streaming

## Storage and Consistency Heavy

- **Dropbox/Google Drive** — file sync, chunking, conflict resolution, metadata storage
- **Uber/Ola** — geospatial indexing (quad trees, geohashing), matching algorithms, real-time location tracking
- **Amazon/Flipkart (e-commerce)** — inventory management, order processing, payment consistency, search
- **Airbnb/Booking.com** — search and filtering at scale, booking consistency, availability calendars

## High Throughput / Scale-Focused

- **Google Search / web crawler** — crawling, indexing, ranking at scale
- **Ticketmaster/BookMyShow** — handling concurrent booking, seat locking, preventing overselling
- **Rate Limiter (used by APIs like Stripe/Twitter API)** — token bucket, sliding window algorithms

## Notification and Messaging Systems

- **Facebook/LinkedIn notification system** — fan-out, push notification delivery, batching
- **Slack/Discord** — channel-based messaging, presence, search within messages

## Payments and Financial Systems

- **Stripe/PayPal** — idempotency, distributed transactions, ledger design, fraud detection basics
- **Splitwise** — expense splitting, balance computation, graph simplification

## Distributed Systems Concepts (Asked at Senior Levels Too)

- **Distributed cache (like Redis at scale)** — eviction policies, consistency, replication
- **Distributed job scheduler (like Cron at scale)** — leader election, fault tolerance
- **Key-value store (like DynamoDB)** — partitioning, replication, consistency models (CAP theorem)

## How to Actually Study These

Rather than memorizing each design, focus on the recurring building blocks — since interviewers care more about your reasoning than a "correct" answer:

1. Load balancers and reverse proxies
2. Database choice (SQL vs NoSQL) and sharding strategies
3. Caching layers and cache invalidation
4. Message queues (Kafka, RabbitMQ) for async processing
5. CDNs for static/media content
6. Consistent hashing for distribution
7. CAP theorem trade-offs

## Suggested Approach

Pick 5-6 systems from the list above (mix of read-heavy, write-heavy, and real-time systems), design each from scratch on paper/whiteboard covering:

- Requirements
- Estimation
- High-level design
- Deep dive
- Trade-offs

You'll notice the same components reappearing across systems — that pattern recognition matters more than covering every company.