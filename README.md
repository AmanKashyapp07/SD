# System Design & Machine Coding — Placement Preparation

A comprehensive, interview-focused repository covering High-Level Design (HLD) and Low-Level Design (LLD) / Machine Coding.

---

## Repository Structure

```
sys-design/
├── HLD/                          # High-Level Design (Distributed Systems & Scale)
│   ├── blocks/                   # Foundational Building Blocks (11-section in-depth guides)
│   │   ├── api-protocols.md      # REST, GraphQL, gRPC, WebSockets, SSE, Webhooks
│   │   ├── database-indexing.md  # B+ Trees, Clustered vs Non-Clustered, Composite, Covering
│   │   └── sql-vs-nosql.md       # Relational vs Document vs Key-Value vs Wide-Column vs Graph
│   ├── companies-design/         # Real-World System Architectures (8-section interview guides)
│   │   ├── netflix.md            # Video Streaming & Adaptive Bitrate (ABR)
│   │   ├── whatsapp.md           # Real-Time Messaging & End-to-End Encryption (E2EE)
│   │   └── url-shortener.md      # Scalable URL Shortener (TinyURL / Bitly)
│   ├── plan.md                   # Building Blocks Study Roadmap
│   └── plan_2.md                 # Company Designs Study Roadmap
│
├── LLD/                          # Low-Level Design & Machine Coding (Object-Oriented Design)
│   ├── parking-lot-system.md     # Parking Lot System (Comprehensive LLD Master Guide)
│   ├── problem-1/                # Parking Lot System (Working C++17 Modular Implementation)
│   │   ├── README.md             # Compilation & Pattern Mapping
│   │   └── src/                  # C++ Source Code (.hpp / .cpp)
│   └── plan.md                   # LLD Study Roadmap & Problems List
│
└── prompts.md                    # Standardized generation prompt templates
```

---

## 1. High-Level Design (HLD)

### Foundational Building Blocks (`HLD/blocks/`)
- [API Protocols (`api-protocols.md`)](file:///Users/amankashyap/Documents/sys-design/HLD/blocks/api-protocols.md)
- [Database Indexing (`database-indexing.md`)](file:///Users/amankashyap/Documents/sys-design/HLD/blocks/database-indexing.md)
- [SQL vs. NoSQL (`sql-vs-nosql.md`)](file:///Users/amankashyap/Documents/sys-design/HLD/blocks/sql-vs-nosql.md)

### Real-World Company Designs (`HLD/companies-design/`)
- [Netflix System Design (`netflix.md`)](file:///Users/amankashyap/Documents/sys-design/HLD/companies-design/netflix.md)
- [WhatsApp System Design (`whatsapp.md`)](file:///Users/amankashyap/Documents/sys-design/HLD/companies-design/whatsapp.md)
- [URL Shortener System Design (`url-shortener.md`)](file:///Users/amankashyap/Documents/sys-design/HLD/companies-design/url-shortener.md)

### Roadmaps
- [Building Blocks Roadmap (`plan.md`)](file:///Users/amankashyap/Documents/sys-design/HLD/plan.md)
- [Company Designs Roadmap (`plan_2.md`)](file:///Users/amankashyap/Documents/sys-design/HLD/plan_2.md)

---

## 2. Low-Level Design (LLD)

### Design Guides & Code Implementations (`LLD/`)
- [Parking Lot System Guide (`parking-lot-system.md`)](file:///Users/amankashyap/Documents/sys-design/LLD/parking-lot-system.md) — Multi-floor spot allocation, Strategy pattern for pricing, Composition vs Aggregation relationships.
- [Parking Lot System C++ Code (`problem-1/`)](file:///Users/amankashyap/Documents/sys-design/LLD/problem-1/README.md) — Production-grade, modular C++17 implementation with runnable demo in `src/main.cpp`.
- [LLD Preparation Roadmap (`plan.md`)](file:///Users/amankashyap/Documents/sys-design/LLD/plan.md) — Comprehensive list of foundational and intermediate LLD interview problems.

---

## 3. Prompts & Context Guide

- [Generation Prompts (`prompts.md`)](file:///Users/amankashyap/Documents/sys-design/prompts.md) — Standardized templates for adding new Building Blocks, Company Designs, or LLD problems.
