# System Design Building Block: Database Indexing

A comprehensive guide to understanding how database indexes work under the hood: B+ Trees, Clustered vs. Non-Clustered Indexes, LSM-Trees, Composite Indexes, and Query Optimization.

---

## 1. Start With the Problem

Imagine you run an e-commerce platform like Amazon. Your PostgreSQL database contains an `orders` table with **50 million rows**. Each row stores an `order_id`, `user_id`, `total_amount`, `status`, and `created_at` timestamp.

A user logs in and visits their order history page, triggering this simple query:

```sql
SELECT * FROM orders WHERE user_id = 984512;
```

If the `user_id` column has **no index**, what happens inside the database engine?

Because the rows on disk are stored in arbitrary insertion order (a Heap file), the database has no idea where User `984512`'s orders are located. It has only one option: a **Full Table Scan (Sequential Scan)**.

The database engine must:
1. Load every single disk block containing the 50 million rows from storage into RAM.
2. At 200 bytes per row, that is **10 Gigabytes of raw data** read from disk.
3. Compare the `user_id` of every single row against `984512`.

On standard server SSDs, reading and scanning 10 GB takes **15 to 30 seconds** of pure disk I/O and 100% CPU core utilization.

Now imagine 500 users click "My Orders" at the same time. The database attempts to read $500 \times 10\text{ GB} = 5\text{ Terabytes}$ of disk data concurrently. The database connection pool exhausts, queries queue up, memory runs out, and the entire production database crashes.

This is the fundamental problem database indexing solves: **How do we locate 3 matching rows out of 50 million in under 2 milliseconds without scanning 10 GB of data from disk?**

---

## 2. What It Is

A **Database Index** is a specialized, auxiliary data structure (most commonly a **B+ Tree**) that maintains a sorted copy of specific column values along with direct pointers to the corresponding full table rows on disk.

Think of a database index exactly like the **index at the back of a 1,000-page textbook**:
- Without an index, if you want to find every mention of the word *"Polymorphism"*, you must read all 1,000 pages from front to back ($O(N)$ time).
- With an index, you flip to the back, look up the alphabetically sorted word *"Polymorphism"* in $O(\log N)$ time, find page numbers `[142, 388, 712]`, and flip directly to those exact pages.

An index trades **storage space** and **write performance** for **blazing-fast read speed**.

---

## 3. How It Works — Step by Step

To understand database indexing deeply, you must understand how data is organized on disk and how search algorithms traverse index nodes.

```
+-----------------------------------------------------------------------------+
|                        DATABASE INDEXING MECHANICS                          |
+-------------------+--------------------+------------------+-----------------+
| INDEX TYPE        | UNDERLYING STRUCT  | LOOKUP COMPLEXITY| BEST USE CASE   |
+-------------------+--------------------+------------------+-----------------+
| B+ Tree Index     | Self-balancing tree| O(log N)         | Equality, Range |
| Hash Index        | Hash Table         | O(1)             | Strict Equality |
| LSM-Tree Index    | Append-only Tables | O(log N)         | Write-Heavy NoSQL|
| Bitmap Index      | Bit arrays per val | O(1) bitwise AND | Low-Cardinality |
+-------------------+--------------------+------------------+-----------------+
```

---

### 1. The Core Data Structure: The B+ Tree

Almost all relational databases (MySQL InnoDB, PostgreSQL, Oracle, SQL Server) use a **B+ Tree** (a self-balancing, N-ary search tree) as their default indexing engine.

```
                  [ Root Node: Page 1 ]
                  |  100  |  200  |  300  |
                  /       |       |       \
        +--------+   +----+----+  +----+---+  +--------+
        |            |         |       |               |
   [ Page 10 ]  [ Page 11 ] [ Page 12 ] [ Page 13 ]
   | 20 | 50 |  |120| 160 | |220| 280 | |310| 390 |   <- Intermediate Nodes
        \            \         \       \
        [ Leaf 1 ]<->[ Leaf 2 ]<->[ Leaf 3 ]<->[ Leaf 4 ] <- Doubly Linked List
        [1..99]      [100..199]   [200..299]   [300..399]    (Holds Row Pointers)
```

#### Why B+ Trees Are Superior for Databases:
1. **Massive Fan-Out (Shallow Depth):** Unlike a binary search tree where each node has only 2 children, a B+ Tree node corresponds to a standard database page size (typically **16 KB**). A single 16 KB page can hold **1,000 keys and child pointers**.
   - Level 1 (Root): 1 page = 1,000 keys.
   - Level 2: 1,000 pages = 1,000,000 keys.
   - Level 3: 1,000,000 pages = **1,000,000,000 (1 Billion) keys**.
   - *Result:* Searching through 1 Billion rows requires reading only **3 disk pages** ($O(\log_{1000} N)$).
2. **Data Only in Leaves:** Internal nodes contain only routing keys, keeping them small enough to fit completely in RAM cache (Buffer Pool).
3. **Sequential Leaf Linked List:** All leaf nodes are linked horizontally in a **doubly linked list**. Range queries (`WHERE age BETWEEN 20 AND 30`) find the start key in 3 hops and then simply walk the linked list horizontally on disk with zero tree re-traversals.

---

### 2. Clustered vs. Non-Clustered Indexes

This is one of the most critical distinctions in database design:

```
CLUSTERED INDEX (The Table IS the Index)
[ Root ] -> [ Intermediate ] -> [ Leaf Node: Contains ENTIRE Row Data (ID, Name, Email, Address) ]

NON-CLUSTERED / SECONDARY INDEX (Auxiliary Lookup)
[ Root ] -> [ Intermediate ] -> [ Leaf Node: Contains Indexed Key + Pointer to Clustered Key ]
                                                            │
                                                            ▼
                                           [ Lookup Clustered Index (Bookmark Lookup) ]
```

#### A. Clustered Index (Primary Key Index)
- **Mechanics:** The physical storage order of the rows on disk is sorted by the clustered index column.
- **Rule:** A table can have **only ONE clustered index** (because physical data on disk can only be sorted in one order).
- In MySQL InnoDB, the `PRIMARY KEY` is always the Clustered Index. The leaf pages of the B+ Tree do not hold pointers—they hold the **actual complete table rows**.

#### B. Non-Clustered Index (Secondary Index)
- **Mechanics:** A separate B+ Tree structure created on non-primary columns (e.g., `CREATE INDEX idx_user_id ON orders(user_id)`).
- The leaf nodes of a secondary index store the indexed value (`user_id`) along with the **Clustered Key / Row Pointer** (`order_id`).
- **The Two-Step Traversal (Bookmark Lookup):**
  1. Search Secondary Index B+ Tree for `user_id = 984512` $\rightarrow$ finds `order_id = 55102`.
  2. Search Clustered Index B+ Tree for `order_id = 55102` $\rightarrow$ reads full row data (`total_amount`, `status`).

---

### 3. Worked Numerical Example: Table Scan vs. B+ Tree

Let us calculate the concrete disk I/O difference on our 50-million-row `orders` table:

- **Total Rows:** 50,000,000 rows
- **Row Size:** 200 Bytes
- **Disk Page Size:** 16 KB (holds 80 rows per page)
- **Total Table Pages:** $\frac{50,000,000}{80} = 625,000\text{ disk pages}$ ($10\text{ GB}$)

#### Without Index (Full Table Scan):
- Must read all **625,000 disk pages**.
- At 0.1 ms per random/sequential read batch $\approx$ **15 seconds**.

#### With B+ Tree Index on `user_id`:
- B+ Tree fan-out per 16 KB page $\approx$ 1,000 index entries.
- Depth of tree = $\log_{1000}(50,000,000) \approx 3\text{ levels}$.
- **Disk Reads Required:**
  1. Root node (Level 1): Cached in RAM = **0 ms**.
  2. Intermediate node (Level 2): Cached in RAM = **0 ms**.
  3. Leaf node (Level 3): Read 1 disk page = **1 disk read**.
  4. Fetch actual row from Clustered Table: Read 1 disk page = **1 disk read**.
- **Total Disk Reads = 2 page reads** ($\approx 32\text{ KB}$ instead of $10\text{ GB}$).
- **Execution Time:** **$1.2\text{ milliseconds}$** (a **12,500x speedup**).

---

## 4. Why This Design and Not Something Simpler?

```
WHY NOT SIMPLER ALTERNATIVES?

1. Sorted Flat File on Disk + Binary Search
   - Read Speed: O(log N) - Fast.
   - Flaw: Insert/Delete is O(N). Inserting one new row into the middle of a 10 GB file 
     requires shifting 5 GB of data on disk. Completely unusable for write workloads.

2. Hash Index (Hash Table)
   - Read Speed: O(1) - Ultra-fast equality (WHERE id = 5).
   - Flaw: Unordered. Cannot perform range queries (WHERE age >= 21) or sorting (ORDER BY).

3. B+ Tree (The Gold Standard)
   - Read Speed: O(log N) via shallow tree.
   - Write Speed: O(log N) via local page splits (only 16 KB page reorganizes, not entire table).
   - Range Queries: O(K) sequential scan along leaf linked list.
```

---

## 5. Real-World Usage

Here is how top tech companies optimize database indexing at massive scale:

### 1. Uber (Geospatial & Trip Indexing)
- **Use Case:** Matching riders with nearest drivers across millions of live GPS coordinates.
- **How they index:** Standard single-column B-Trees fail on 2D coordinates (`WHERE lat BETWEEN ... AND lng BETWEEN ...`). Uber uses **Spatial Indexes (R-Trees / Google S2 Geospatial Cell Indexes)**, mapping 2D Earth coordinates into 1D Hilbert Curve integer hashes indexed in B+ Trees.

### 2. Amazon (Covering Indexes for Checkout)
- **Use Case:** High-throughput order placement and inventory deduction during Prime Day.
- **How they index:** To prevent the slow second-hop "Bookmark Lookup" on secondary indexes, Amazon creates **Covering Indexes** (`CREATE INDEX idx_user_orders ON orders(user_id, status, total_amount)`). The query engine reads everything directly from the index leaf pages without ever touching the underlying table on disk.

### 3. Stripe (Idempotency Key Indexing)
- **Use Case:** Preventing duplicate credit card charges when network retries occur.
- **How they index:** Stripe creates a **Unique Index** on `(customer_id, idempotency_key)`. When a payment request arrives, the B+ Tree enforces unique constraint validation in under 1 millisecond before initiating payment processing.

### 4. Discord (LSM-Tree Indexing in ScyllaDB/Cassandra)
- **Use Case:** Storing trillions of chat messages across millions of Discord servers.
- **How they index:** B+ Trees suffer severe write-amplification under millions of concurrent message writes. Discord uses **LSM-Tree (Log-Structured Merge-Tree)** based wide-column stores where new messages append directly to in-memory Memtables, deferring sorting and indexing to background disk compactions.

---

## 6. Trade-offs

| Factor | B+ Tree Index | Hash Index | LSM-Tree (NoSQL) | Bitmap Index |
| :--- | :--- | :--- | :--- | :--- |
| **Point Lookup (`=`)** | $O(\log N)$ (Fast) | $O(1)$ (Fastest) | $O(\log N)$ (Moderate)| $O(1)$ bitwise |
| **Range Queries (`<, >, BETWEEN`)**| $O(\log N + K)$ (Excellent) | Not Supported | $O(\log N + K)$ (Good)| Poor |
| **Write / Insert Overhead**| Moderate (Page splits)| Low | Ultra-Low (Append-only)| High (Locks bitmap) |
| **Storage Overhead** | 10% – 30% extra space | 10% – 20% extra space| Low | Ultra-Low (Compressed)|
| **Best For** | General OLTP, SQL, Range queries | Key-Value lookups (Redis/Memory)| Write-heavy (Cassandra/Logs)| OLAP Data Warehouses (Low Cardinality)|

### The Cost of Indexing (Write Amplification & Storage Bloat):
Indexes are not free. Every index on a table:
1. **Slows down `INSERT`, `UPDATE`, and `DELETE`:** When you insert a row, the database must write the row to the clustered table *plus* update every single secondary B+ Tree index on that table.
2. **Consumes Disk & RAM:** A table with 10 secondary indexes can easily consume more disk space for its indexes than for the actual data itself.

---

## 7. Common Pitfalls & Misconceptions

### 1. The "Index Every Column" Anti-Pattern
- **The Misconception:** "Our queries are slow, so let's add an index to every single column in the table."
- **The Reality:** Adding 10 indexes to a high-throughput table cuts write performance by 5x–10x due to continuous B+ Tree page splits and disk synchronization. Only index columns that appear frequently in `WHERE`, `JOIN`, and `ORDER BY` clauses.

### 2. Violating the Leftmost Prefix Rule in Composite Indexes
- **The Misconception:** "I created a composite index on `(country, state, city)`, so queries on `city` will automatically be fast."
- **The Reality:** A composite B+ Tree is sorted by the first column first, then the second, then the third. 
  - `WHERE country = 'US' AND state = 'CA'` $\rightarrow$ **Uses Index.**
  - `WHERE country = 'US'` $\rightarrow$ **Uses Index.**
  - `WHERE city = 'San Francisco'` $\rightarrow$ **Cannot use index!** (Triggers a full table scan because keys are not sorted globally by `city`).

### 3. Wrapping Indexed Columns in Functions
- **The Misconception:** "I have an index on `created_at`, so `WHERE DATE(created_at) = '2026-08-20'` will use the index."
- **The Reality:** Applying a function (`DATE()`, `LOWER()`, `UPPER()`) forces the database to evaluate the function on every row, completely bypassing the B+ Tree index. Write the query as a range scan instead: `WHERE created_at >= '2026-08-20 00:00:00' AND created_at < '2026-08-21 00:00:00'`.

---

## 8. Common Interview Questions

### Question 1 (Basic): What is the difference between a Clustered Index and a Non-Clustered Index?
- **What the interviewer is testing for:** Basic understanding of physical vs. logical disk storage layout.
- **Key Answer Points:** A clustered index determines the physical order of rows on disk (the leaf nodes *are* the data pages), so there can only be one per table. A non-clustered index is a separate auxiliary B+ Tree whose leaf nodes store the indexed key along with a pointer (or primary key) back to the clustered row.

### Question 2 (Intermediate): What is a Covering Index and why is it so powerful?
- **What the interviewer is testing for:** Optimization techniques to eliminate secondary index bookmark lookups.
- **Key Answer Points:** A covering index contains all the columns requested in the `SELECT`, `WHERE`, and `ORDER BY` clauses of a query. Because all required data resides directly inside the index leaf nodes, the database engine satisfies the entire query from the index tree without performing a secondary lookup to the clustered table on disk.

### Question 3 (Advanced): Why do relational databases use B+ Trees instead of Binary Search Trees (Red-Black / AVL Trees) or Hash Tables?
- **What the interviewer is testing for:** Deep mechanical sympathy for disk I/O, page block architectures, and access patterns.
- **Key Answer Points:** 
  1. Red-Black trees have only 2 children per node, resulting in deep trees ($O(\log_2 N)$) that require 20–30 disk seeks for large datasets. B+ Trees have huge fan-outs (1,000+ children), keeping tree depth to 3–4 levels.
  2. Hash tables cannot execute range queries (`BETWEEN`, `>`, `<`) or sorting (`ORDER BY`).
  3. B+ Trees link all leaf nodes in a sequential doubly linked list, enabling lightning-fast range scans.

### Question 4 (Probing): What is "Index Cardinality" and what is the "Tipping Point" where a query planner ignores an index?
- **What the interviewer is testing for:** Understanding query optimizer cost models and index selectivity.
- **Key Answer Points:** 
  1. **Cardinality** is the uniqueness of values in a column. A column with high cardinality (e.g., `user_id`, `email`) has unique values; low cardinality (e.g., `gender`, `is_active`) has few distinct values.
  2. If a query matches more than **15%–30% of the entire table** (the Tipping Point), the database query optimizer deliberately ignores the secondary index and executes a full sequential scan because performing millions of random bookmark lookups is slower than scanning sequential pages.

---

## 9. How to Explain It in an Interview

When an interviewer asks: *"How would you optimize database read latency for this query?"*, use this structured formula:

### Sample 4-Sentence Answer Script:
> "To optimize this query, I recommend adding a **[Single-column / Composite / Covering]** B+ Tree index on **[Column Names]** because our primary access pattern involves **[Equality lookup / Range scan / Sorted retrieval]**. 
> Without an index, the query triggers an $O(N)$ full table scan across millions of disk blocks, whereas the B+ Tree reduces disk I/O to a 3-level tree traversal in $O(\log N)$ time. 
> To ensure maximum efficiency, we will order composite columns following the **Leftmost Prefix Rule** and include selected fields to create a **Covering Index**, avoiding secondary bookmark lookups. 
> The trade-off is a slight write penalty on inserts and updates, which is fully acceptable given our 95:5 read-to-write ratio."

### Why this structure works:
It proves you understand the **data structure choice**, the **disk I/O math**, advanced optimization techniques (**covering index / leftmost prefix**), and the **write-amplification trade-off**.

---

## 10. Related Building Blocks

To deepen your database design mastery, study these connected concepts:

1. **SQL vs. NoSQL (When to Use Which):**
   - *Relationship:* Relational databases rely heavily on B+ Trees for relational integrity and joins, while distributed NoSQL databases use LSM-Trees and Partition Keys to scale writes across distributed nodes.
2. **Database Sharding & Horizontal Partitioning:**
   - *Relationship:* Sharding partitions a table across multiple database servers based on a **Shard Key** (which acts as a global index routing queries to specific database nodes).
3. **In-Memory Caching (Redis / Memcached):**
   - *Relationship:* Sits in front of indexed databases. While a B+ Tree index reduces disk reads to 1–2 milliseconds, an in-memory cache returns data in **sub-millisecond microseconds**, shielding database B+ Trees from excessive read concurrency.

---

## 11. Check Your Understanding

Test your mastery of this building block by answering these three conceptual questions:

1. **Scenario Analysis:** You have a composite index defined as `CREATE INDEX idx_emp ON employees(department_id, salary, hire_date)`. Which of the following queries can fully use the index, partially use the index, or cannot use the index at all?
   - `Query A: WHERE department_id = 5 AND salary > 80000;`
   - `Query B: WHERE salary = 90000 AND hire_date = '2025-01-01';`
   - `Query C: WHERE department_id = 5 ORDER BY salary DESC;`
2. **Mechanism Breakdown:** Why is inserting random UUID primary keys into a clustered B+ Tree index significantly slower than inserting auto-incrementing sequential integers? (Hint: Think about B+ Tree page splits and disk fragmentation).
3. **Architecture Decision:** Your analytics dashboard runs an aggregation query scanning 100,000 rows every 5 seconds to calculate total sales by category. Why would adding a standard secondary index on `category` fail to significantly improve this query, and what database feature (e.g., Materialized Views or Columnar Storage) would solve it instead?
