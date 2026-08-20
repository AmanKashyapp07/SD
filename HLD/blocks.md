# System Design Building Blocks — Placement Prep

For placement-level system design prep, you typically need to master around **20-25 core building blocks**, grouped into these categories.

## 1. Networking & Communication (4)

- Load balancers (L4 vs L7)
- Reverse proxy vs forward proxy
- DNS and how routing works
- API protocols (REST, gRPC, WebSockets, GraphQL)

## 2. Storage & Databases (6)

- SQL vs NoSQL (when to use which)
- Database indexing
- Sharding / partitioning strategies
- Replication (master-slave, master-master)
- Database normalization vs denormalization
- Object storage (S3-like) vs block storage vs file storage

## 3. Caching (3)

- Cache placement (client, CDN, server-side, database cache)
- Cache eviction policies (LRU, LFU)
- Cache invalidation strategies (write-through, write-back, write-around)

## 4. Scalability Concepts (4)

- Horizontal vs vertical scaling
- Consistent hashing
- Rate limiting algorithms (token bucket, leaky bucket, sliding window)
- Data partitioning strategies

## 5. Asynchronous Processing (3)

- Message queues (Kafka, RabbitMQ, SQS)
- Pub-sub systems
- Batch vs stream processing

## 6. Distributed Systems Theory (4)

- CAP theorem
- Consistency models (strong, eventual, causal)
- Leader election
- Distributed locks

## 7. Reliability & Fault Tolerance (3)

- Redundancy and replication
- Circuit breakers
- Failover strategies (active-active, active-passive)

## 8. Security & Auth (2)

- Authentication vs authorization (OAuth, JWT)
- API gateway responsibilities

## How to Prioritize

You don't need equal depth everywhere. For most placement interviews (SDE-1/SDE-2 level), focus heavily on:

- Load balancing, caching, database sharding, and message queues — these show up in almost every design
- CAP theorem and consistency — interviewers love probing trade-offs here
- Rate limiting and consistent hashing — common as standalone questions too

The distributed systems theory section (leader election, distributed locks) matters more for senior/staff-level interviews or companies known for heavy system design rounds (Amazon, Google, Uber). For most placements, **15 solid building blocks** covering sections 1-5 will cover 80% of what gets asked.