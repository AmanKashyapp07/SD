# System Design Building Block: SQL vs. NoSQL (When to Use Which)

A comprehensive guide to understanding database architectures: Relational Databases, Document Stores, Key-Value Stores, Wide-Column Databases, and Graph Databases.

---

## 1. Start With the Problem

Imagine you are the founding engineer at a rapidly growing fintech and e-commerce startup. Your application has two critical features:
1. **The Wallet & Checkout Engine:** Users deposit money into their wallet and transfer funds to purchase items.
2. **The Product Review & Telemetry Feed:** Millions of users scroll products, leave reviews, upload device telemetry, and track real-time clickstream events (generating 200,000 events every second).

Suppose you try to use a single database technology for everything.

### Scenario A: You choose MongoDB (NoSQL) for the Wallet
A user with a \$50 balance attempts to buy two \$40 items simultaneously from two different browser tabs. Both requests hit your application servers in the same millisecond. Because your NoSQL database does not enforce strict multi-document ACID isolation by default, Request 1 reads \$50 balance (approves purchase, balance becomes \$10), and Request 2 reads \$50 balance at the same instant (approves purchase, balance becomes \$10). The user gets \$80 worth of goods for \$50, and your company loses money due to a **double-spending race condition**.

### Scenario B: You choose PostgreSQL (SQL) for 200,000 Telemetry Events/sec
You create an events table with strict foreign key constraints, relational indexes, and ACID guarantees. As Black Friday traffic spikes to 200,000 writes per second, your single primary PostgreSQL server collapses. The CPU maxes out at 100% trying to update B+ Tree indexes and check transactional locks on disk for every single write. The database connection pool fills up, queries time out, and your entire application crashes.

This dual failure illustrates the core rule of database selection: **Databases are specialized tools designed around distinct trade-offs.** Choosing the wrong database model causes financial data corruption on one extreme, or complete system outages under high write throughput on the other.

---

## 2. What It Is

At the highest level, databases are categorized into two primary paradigms:

1. **Relational Databases (SQL):** Systems that store structured data in fixed tables consisting of rows and columns with predefined schemas. They enforce mathematical relations between tables using foreign keys and provide strict **ACID guarantees** (Atomicity, Consistency, Isolation, Durability) to ensure mathematical correctness.
2. **Non-Relational Databases (NoSQL):** Systems designed to handle unstructured, semi-structured, or high-velocity data across distributed clusters. They sacrifice rigid tabular schemas and immediate cross-table relational guarantees in exchange for **horizontal scalability, high write throughput, and flexible data models**.

Instead of viewing SQL vs. NoSQL as a competition, modern software architecture uses **Polyglot Persistence**—combining SQL and various NoSQL databases in the same system, assigning each workload to the database engine engineered specifically for that access pattern.

---

## 3. How It Works — Step by Step

To choose effectively in an interview, you must understand the underlying storage engines, indexing structures, and operational mechanics of SQL and the four major NoSQL database categories.

```
+-----------------------------------------------------------------------------+
|                      DATABASE PARADIGMS AT A GLANCE                         |
+-------------------+--------------------+------------------+-----------------+
| CATEGORY          | PRIMARY EXAMPLES   | CORE DATA MODEL  | PRIMARY USE CASE|
+-------------------+--------------------+------------------+-----------------+
| Relational (SQL)  | PostgreSQL, MySQL  | Tables / Rows    | ACID, Joins     |
| Key-Value (NoSQL) | Redis, DynamoDB    | Key -> Blob/JSON | Caching, Session|
| Document (NoSQL)  | MongoDB, CouchDB   | JSON / BSON Docs | Catalogs, CMS   |
| Wide-Column(NoSQL)| Cassandra, ScyllaDB| Rows + Columns   | Time-series, IoT|
| Graph (NoSQL)     | Neo4j, Amazon Neptune| Nodes & Edges  | Social Networks |
+-------------------+--------------------+------------------+-----------------+
```

---

### 1. Relational Databases (SQL) Mechanics

Relational databases structure data into normalized tables and use **B+ Tree storage engines** to provide fast $O(\log N)$ search, range queries, and strict transaction isolation.

```
SQL RELATIONAL MODEL (Normalized with Foreign Keys)

[Users Table]                          [Orders Table]
+---------+----------+                +----------+---------+--------+
| user_id | name     | <---- (1:N) ---| order_id | user_id | total  |
+---------+----------+                +----------+---------+--------+
| 1       | Alice    |                | 101      | 1       | $45.00 |
| 2       | Bob      |                | 102      | 1       | $12.50 |
+---------+----------+                +----------+---------+--------+
```

#### How SQL Executes an ACID Transaction:
1. **Atomicity & Write-Ahead Logging (WAL):** When a transaction modifies multiple rows (e.g., deducting \$50 from Account A and adding \$50 to Account B), the database first writes the intent to an append-only WAL on disk before touching data pages. If power cuts mid-way, the database replays or rolls back the WAL on reboot. Either all changes persist, or none do.
2. **Isolation & Locking (B+ Tree Pages):** The database acquires shared (read) or exclusive (write) locks on table rows or index pages to prevent concurrent transactions from reading uncommitted or conflicting state.
3. **Joins & Normalization:** Data is split into normalized tables to prevent duplication. When a user requests their order history, the query engine performs relational joins (`JOIN users ON orders.user_id = users.user_id`) in memory.

---

### 2. The Four Major NoSQL Models

NoSQL was born because B+ Tree locking and cross-table joins become massive bottlenecks when scaling writes horizontally across 100+ servers.

#### A. Key-Value Stores (e.g., Redis, AWS DynamoDB)
- **Mechanics:** The simplest database model. It operates like a massive, distributed hash table. You supply a string key (e.g., `user_session:9871`) and receive an opaque value (string, serialized JSON, or binary blob).
- **Lookup Complexity:** Constant time $O(1)$ read and write.
- **When to use:** Session token storage, shopping carts, rate-limiting counters, and user preferences.

#### B. Document Stores (e.g., MongoDB, Couchbase)
- **Mechanics:** Stores data as semi-structured, self-describing documents (typically JSON or BSON). Instead of normalizing data into 5 separate tables, all related information is embedded directly into a single document.
- **Example Document:**
```json
{
  "_id": "prod_456",
  "title": "Mechanical Keyboard",
  "price": 99.99,
  "attributes": {
    "switch_type": "Cherry MX Blue",
    "backlight": "RGB"
  },
  "tags": ["gaming", "peripherals"],
  "reviews": [
    { "user": "Alice", "rating": 5, "comment": "Great feel!" }
  ]
}
```
- **When to use:** E-commerce product catalogs with varying attributes per category, Content Management Systems (CMS), and user profiles where schema fields evolve rapidly.

#### C. Wide-Column Stores (e.g., Apache Cassandra, ScyllaDB)
- **Mechanics:** Uses an **LSM-Tree (Log-Structured Merge-Tree)** storage engine designed for massive write throughput. Writes are appended instantly into an in-memory `Memtable` and an append-only commit log on disk. Background compaction merges in-memory tables into immutable on-disk `SSTables`.
- **Partition Key & Clustering Key:** Data is horizontally partitioned across server nodes using a **Partition Key** (determines which node holds the data) and sorted on disk using a **Clustering Key** (determines row ordering within that node).
- **When to use:** Time-series telemetry, IoT sensor logging, chat history, messaging apps, and financial audit logs where writes reach hundreds of thousands of events per second.

```
LSM-TREE WRITE PIPELINE (Cassandra / Wide-Column)

[Incoming Write] 
       │
       ├─────────────────────────┐
       ▼                         ▼
[Commit Log on Disk]     [Memtable in RAM]
 (Append-Only Crash Log)  (Instant in-memory insert)
                                 │
                                 ▼ (When full, flush to disk)
                         [Immutable SSTables]
                         (Merged in background)
```

#### D. Graph Databases (e.g., Neo4j, Amazon Neptune)
- **Mechanics:** Treats data as **Nodes** (entities like Users, Places) connected by **Edges** (relationships like "FRIENDS_WITH", "LIKED", "PURCHASED"). 
- **Index-Free Adjacency:** Each node directly contains pointers to its connected neighbors on disk. Traversing 5 degrees of separation ("friends of friends of friends") requires following raw pointers in $O(K)$ time rather than computing expensive multi-table recursive SQL joins in $O(N^K)$ time.
- **When to use:** Social networks, fraud detection rings (detecting shared credit cards across different fake identities), knowledge graphs, and recommendation networks.

---

## 4. Why This Design and Not Something Simpler?

To excel in an interview, you must explain why a single database cannot excel at every task:

```
THE FUNDAMENTAL DATABASE TRADE-OFF SPECTRUM

       ACID & Complex Queries              Horizontal Scale & Write Throughput
  [───────────────────────────────▲───────────────────────────────]
  Relational (SQL)          Document (NoSQL)        Wide-Column / Key-Value
  (PostgreSQL / MySQL)      (MongoDB)               (Cassandra / DynamoDB)
  
  - Strict ACID Transactions - Flexible JSON Schema  - Distributed Masterless
  - Relational Joins         - Embedded Documents    - High Write Throughput (LSM)
  - Hard to scale writes     - Moderate Scale        - Eventual Consistency
```

### Why not use SQL for everything?
1. **Vertical Scaling Limits:** Relational databases maintain strict consistency and B+ Tree structures across tables. Scaling requires buying a bigger machine (more RAM, faster SSDs, more CPU cores). When traffic exceeds the capacity of the largest available server, horizontal sharding is required, which breaks foreign keys and multi-table joins.
2. **Schema Rigidity:** Adding a column to a table with 500 million rows can lock the database for hours, causing service downtime.

### Why not use NoSQL for everything?
1. **Lack of Native Cross-Table Joins:** In NoSQL, relationships must either be embedded (duplicating data) or resolved manually in application code by making multiple sequential network round-trips.
2. **Eventual Consistency & Data Duplication:** When you duplicate a user's address across 50 embedded order documents for fast reads, updating that address requires updating 50 documents. If an update fails mid-way, your data becomes inconsistent.

---

## 5. Real-World Usage

Here is how top engineering organizations combine SQL and NoSQL in production:

### 1. Amazon
- **Relational SQL (Aurora / PostgreSQL):** Used for financial ledger records, order billing, tax calculations, and fulfillment tracking where transactions must strictly adhere to ACID guarantees.
- **NoSQL Key-Value (DynamoDB):** Used for user shopping carts, session state, and product lookup where low-latency single-digit millisecond response times are required during peak Prime Day traffic.

### 2. Uber
- **Relational SQL (MySQL / Schemaless Layer):** Used for trip transactions, driver payouts, and rider account billing.
- **NoSQL Wide-Column (Cassandra / Pinot):** Used for ingesting real-time driver GPS coordinates, vehicle telemetry, and analytical trip heatmaps streaming from millions of mobile apps every second.

### 3. Netflix
- **Relational SQL (AWS Aurora MySQL):** Used for subscriber billing, credit card processing, account memberships, and subscription plan management.
- **NoSQL Wide-Column (Apache Cassandra):** Used for viewing history, bookmark timestamps, and playback telemetry across 250+ million user profiles worldwide.

### 4. LinkedIn
- **Graph Database (Neo4j / Custom Economic Graph):** Used for calculating 1st, 2nd, and 3rd-degree connection paths, "People You May Know", and company recruiter search graphs.

---

## 6. Trade-offs

| Factor | Relational (SQL) | Document Store | Key-Value Store | Wide-Column Store |
| :--- | :--- | :--- | :--- | :--- |
| **Primary Data Model** | Tables, Rows, Columns | JSON / BSON Docs | Key -> Opaque Value | Key -> Dynamic Columns |
| **Schema** | Rigid, Predefined | Flexible, Dynamic | None (Schemaless) | Column Families |
| **Transaction Model** | Full ACID (Multi-row) | Single-Doc ACID (Mostly) | Single-Key Atomic | Row-level Atomic (BASE) |
| **Query Complexity** | Rich (Joins, Aggregations) | Queries on nested fields | By Key only (Get/Put) | By Partition/Cluster Key |
| **Scaling Strategy** | Vertical (Sharding is hard) | Horizontal (Partitioning) | Horizontal (Partitioning)| Horizontal (Masterless) |
| **Write Performance** | Moderate (B+ Tree locks) | High | Ultra-High ($O(1)$) | Extreme (LSM-Tree) |
| **Best For** | Banking, ERP, E-commerce Checkout | Catalogs, CMS, User Profiles | Caching, Sessions, Carts | Time-series, Chat, IoT |

---

## 7. Common Pitfalls & Misconceptions

### 1. Believing "NoSQL is Always Faster Than SQL"
- **The Misconception:** "NoSQL is modern, so it is inherently faster than relational databases."
- **The Reality:** For indexed primary key lookups or small table queries, PostgreSQL and MySQL execute in under 1 millisecond. NoSQL is faster *only* under specific conditions: extreme write volumes (LSM-trees vs B+ trees) or distributed multi-node horizontal partitioning. If your data fits on a single machine, SQL is often faster and much easier to manage.

### 2. Believing "NoSQL Means No Structure or Schema"
- **The Misconception:** "In NoSQL, we can throw any random data into the database and ignore structure."
- **The Reality:** NoSQL shifts schema enforcement from the **database layer** to the **application code layer** (*Schema-on-Read* vs *Schema-on-Write*). If application engineers do not validate document shapes, the database fills with malformed data that crashes downstream microservices.

### 3. Believing "You Cannot Do Transactions in NoSQL"
- **The Misconception:** "NoSQL databases never support transactions."
- **The Reality:** Most modern NoSQL databases support **single-document / single-row ACID transactions**. Some systems (like MongoDB 4.0+ and DynamoDB Transactions) even support multi-document transactions, though at a noticeable performance and latency cost.

---

## 8. Common Interview Questions

### Question 1 (Basic): When should you strictly choose a SQL database over a NoSQL database?
- **What the interviewer is testing for:** Do you recognize the non-negotiable boundaries of data integrity, financial correctness, and complex relational structures?
- **Key Answer Points:** Choose SQL when:
  1. The domain requires strict **ACID transactions** across multiple entities (e.g., banking transfers, inventory deductions, double-entry bookkeeping).
  2. Data is heavily **relational and normalized** with frequent joins across multiple tables.
  3. The schema is well-defined, stable, and mathematical consistency is prioritized over infinite horizontal write scale.

### Question 2 (Intermediate): How does Cassandra achieve much higher write throughput than PostgreSQL?
- **What the interviewer is testing for:** Deep understanding of storage engines (LSM-Trees vs B+ Trees) and distributed replication architectures.
- **Key Answer Points:** 
  1. PostgreSQL uses a **B+ Tree**, requiring in-place page updates and disk locking for index reorganization.
  2. Cassandra uses an **LSM-Tree** with append-only writes: writes go to an in-memory Memtable and an append-only commit log with zero random disk I/O.
  3. Cassandra is **masterless (peer-to-peer)**: any node in the cluster can accept writes, distributing write traffic across all machines simultaneously.

### Question 3 (Advanced): How do you model a Many-to-Many relationship in MongoDB without relational joins?
- **What the interviewer is testing for:** Practical data modeling skills (Embedding vs Referencing) and understanding of document size limitations.
- **Key Answer Points:**
  1. **Embedding (1:Few):** If the relationship is bounded (e.g., a movie with 5 genres), embed an array of genre sub-documents directly inside the Movie document.
  2. **Referencing (1:Many / Many:Many unbounded):** If unbounded (e.g., Authors and Books), store an array of referenced IDs (`author_ids: [101, 102]`). Resolve in application code or use MongoDB `$lookup` (aggregation join).
  3. **Trade-off:** Embedding offers fast single-query reads but causes data duplication; referencing preserves normalization but requires multiple queries.

### Question 4 (Probing): What is the CAP Theorem, and how does it explain the difference between a SQL database and a distributed NoSQL database?
- **What the interviewer is testing for:** Theoretical foundation of distributed systems under network partitions.
- **Key Answer Points:**
  1. CAP states that in the event of a **Network Partition (P)**, a distributed system must choose between **Consistency (C)** (every read receives the latest write or errors) and **Availability (A)** (every request receives a non-error response without guarantee of latest data).
  2. Traditional SQL databases operate on a single primary node, favoring strong consistency (CP).
  3. Distributed NoSQL databases (like Cassandra) prioritize high availability (AP), using tunable eventual consistency so writes succeed even if some replica nodes are partitioned.

---

## 9. How to Explain It in an Interview

When an interviewer asks: *"Which database would you choose for this system and why?"*, use this structured formula:

### Sample 4-Sentence Answer Script:
> "For this component, I recommend using **[Database Name / Category]** because our primary access pattern is **[Specific Access Pattern: e.g., low-latency key-value lookups / high-write time-series ingestion / multi-table ACID transactions]**. 
> Unlike **[Alternative Database]**, which would suffer from **[Specific Bottleneck: e.g., B+ Tree write-locking / lack of relational joins / schema rigidity]**, **[Database Name]** handles this by **[Underlying Mechanism: e.g., append-only LSM-Tree writes / distributed hash partitioning / normalized foreign-key constraints]**. 
> The main trade-off is **[Specific Trade-off: e.g., lack of multi-row transactions / manual sharding overhead / eventual consistency]**, which we will manage by **[Mitigation Strategy: e.g., enforcing validation in our application service layer / using Redis caching in front]**."

### Why this structure works:
It proves you do not pick databases based on personal preference. You evaluate **access patterns**, contrast the choice with **competing database types**, explain the **storage engine reason**, and provide a realistic **trade-off mitigation**.

---

## 10. Related Building Blocks

To deepen your database architecture expertise, study these closely connected concepts:

1. **Database Indexing (B+ Trees vs. LSM-Trees vs. Hash Indexes):**
   - *Relationship:* The index structure directly determines whether a database is optimized for heavy reads, range scans, or extreme append-only write throughput.
2. **Database Sharding & Horizontal Partitioning:**
   - *Relationship:* When a single SQL or NoSQL node reaches capacity, data must be partitioned across nodes using range-based or hash-based sharding strategies (e.g., Consistent Hashing).
3. **Caching Strategies (Redis / Memcached):**
   - *Relationship:* Sits in front of both SQL and NoSQL databases to absorb 80%+ of read traffic, protecting primary storage engines from read exhaustion.

---

## 11. Check Your Understanding

Test your mastery of this building block by answering these three conceptual questions:

1. **Scenario Analysis:** You are designing the backend for an online banking app and an IoT fleet management tracker tracking 500,000 delivery vans. Which database categories would you select for the account ledger versus the vehicle GPS location feed, and what storage engine characteristics justify your choices?
2. **Mechanism Breakdown:** Explain why updating an existing record in a normalized SQL table with 5 B+ Tree indexes is significantly more resource-intensive than inserting a new log entry into a Cassandra wide-column table.
3. **Architecture Decision:** Your team wants to migrate a legacy product catalog from MySQL to MongoDB. What are two major architectural risks of this migration, and how should your team design document schemas to avoid data inconsistency?
