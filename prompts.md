# System Design & Machine Coding Generation Prompts

This document stores the exact prompt templates and architectural structures used across this repository. In future chat sessions, copy and paste the relevant prompt template to generate new study modules with consistent style, depth, and quality.

---

## Repository Structure Overview

```
sys-design/
├── HLD/                          # High-Level Design
│   ├── blocks/                   # Foundational System Design Building Blocks (11-section format)
│   ├── companies-design/         # Company High-Level System Designs (8-section format)
│   ├── plan.md                   # Building blocks study roadmap
│   └── plan_2.md                 # Company designs study roadmap
├── LLD/                          # Low-Level Design / Machine Coding
│   ├── lecture-1/                # Parking Lot System in C++ (4-part structure)
│   └── lecture-2/                # Future LLD problems...
├── prompts.md                    # Context & generation prompt templates
└── README.md                     # Repository index & navigation guide
```

---

## Type 1: System Design Building Block (`HLD/blocks/`)

**Target Directory:** `HLD/blocks/`  
**Target Filename:** `{{building-block-slug}}.md` (e.g., `consistent-hashing.md`, `rate-limiter.md`, `caching-strategies.md`)

### Copy-Paste Prompt Template:

```markdown
Create a markdown file named {{building-block-slug}}.md in the `HLD/blocks/` directory that teaches "{{BUILDING_BLOCK}}" as a system design concept for someone learning it for the first time while preparing for placement interviews at top tech companies (Google, Amazon, Meta, Microsoft, Uber, etc.).

This is a LEARNING document, not a quick-revision cheatsheet — prioritize genuine understanding over brevity. Explain things the way a good teacher would: build intuition first, then formalize it.

Strict formatting constraints:
- Do NOT use any emojis anywhere in the document.
- Write in a conversational, explanatory tone with full sentences and paragraphs.
- Use bold for key terms.

Structure the file with these 11 sections in exact order:

1. **Start With the Problem** — before defining anything, describe a concrete scenario where NOT having this concept causes a real failure/bottleneck. Make me feel the pain point before giving the solution.
2. **What It Is** — a clear definition, connected back to the problem scenario above.
3. **How It Works — Step by Step** — a detailed walkthrough of the mechanics. Include an ASCII or mermaid diagram. Explain the underlying algorithm or data structure with a concrete worked numerical/data example.
4. **Why This Design and Not Something Simpler** — explain what naive/simpler alternatives were tried first, why they fell short, and what this concept improves.
5. **Real-World Usage** — 3 to 4 named companies/products, explaining HOW they use it in production.
6. **Trade-offs** — a markdown table comparing this approach vs alternatives (pros, cons, when to use / when not to), followed by an explanatory paragraph.
7. **Common Pitfalls / Misconceptions** — 2 to 3 things learners commonly get wrong or oversimplify.
8. **Common Interview Questions** — 4 to 5 questions ordered from basic to probing, each followed by a note on what the interviewer is silently testing for.
9. **How to Explain It in an Interview** — a sample 3 to 4 sentence answer script, with a short note explaining why this structure works.
10. **Related Building Blocks** — 2 to 3 connected concepts explained with deep contextual relationships.
11. **Check Your Understanding** — 3 short self-test conceptual questions (not multiple choice) confirming genuine understanding.
```

---

## Type 2: Company High-Level Design (`HLD/companies-design/`)

**Target Directory:** `HLD/companies-design/`  
**Target Filename:** `{{company-slug}}.md` (e.g., `uber.md`, `instagram.md`, `youtube.md`, `twitter.md`)

### Copy-Paste Prompt Template:

```markdown
Create a markdown file named {{company-slug}}.md in the `HLD/companies-design/` directory covering the high-level system design of {{COMPANY_OR_SYSTEM}}, structured for placement interview revision.

Strict formatting constraints:
- Do NOT use any emojis anywhere in the document.
- Write in plain language tailored for a 4th-year engineering student preparing for tech placements.
- Avoid unnecessary jargon and generic filler — every line must teach something specific to how {{COMPANY_OR_SYSTEM}} actually works.

Structure the file with these 8 sections in exact order:

1. **Functional Requirements** — what the system must do from the user and platform perspective (5-7 clear functional bullets).
2. **Non-Functional Requirements** — scale, availability, latency targets, consistency models (ACID vs Eventual), and durability guarantees.
3. **Back-of-Envelope Estimation** — show actual math: active users, daily active users (DAU), peak QPS, concurrent connections/streams, storage required over time, and network bandwidth (ingress/egress).
4. **High-Level Architecture** — plain-English explanation plus an ASCII box-and-arrow component diagram showing the end-to-end flow from client to storage.
5. **Key Components & Why** — for each major piece (gateway, database choices, caching tier, message queues, specialized services, storage engines), explain what it does and why that exact technology choice makes sense at scale.
6. **How [Core Feature] Actually Works** — a deep-dive walkthrough of the primary technical lifecycle (e.g., how a video stream flows / how a message is routed / how rides are matched), including step-by-step mechanics and protocols.
7. **Bottlenecks & Solutions** — real engineering challenges at scale (e.g., thundering herd, hot keys, fan-out explosion, multi-region failover) and how they are mitigated.
8. **Follow-up Questions an Interviewer Might Ask** — 4 to 5 realistic interview follow-up questions listed WITHOUT answers (for self-thinking and interview preparation).
```

---

## Type 3: Low-Level Design / Machine Coding Lecture (`LLD/lecture-N/`)

**Target Directory:** `LLD/lecture-{{N}}/` (e.g., `LLD/lecture-2/`)  
**Running Problem:** e.g., "Design an Elevator System", "Design a Snake and Ladder Game", "Design a Splitwise Expense Sharing App", "Design a BookMyShow Ticket Booking System"

### Copy-Paste Prompt Template:

```markdown
Create a complete, self-contained educational module in a folder called `LLD/lecture-{{N}}` for teaching a Machine Coding / Low-Level Design (LLD) placement interview round, using the problem: "{{PROBLEM_NAME}}".

Use C++ (C++17) as the programming language.

Strict formatting constraints:
- Do NOT use any emojis anywhere in any markdown or code file.
- Keep C++ code clean, modular, and readable by avoiding repetitive `std::` prefixes in `.cpp` files (`using namespace std;` / targeted using directives).
- Separate header blueprints (`.hpp`) from implementation logic (`.cpp`).

Create the following files inside `LLD/lecture-{{N}}/`:

1. `problem.md` (STRICTLY under 700 words):
   - What this round is (60-90 min time limit, working compilable code required).
   - How the problem is phrased out loud by the interviewer.
   - 5-6 clarifying questions a good candidate asks and why each matters.
   - Sample finalized requirements agreed upon before coding.
   - Design decisions and patterns used (e.g., Strategy, State, Factory, Observer).
   - What the interviewer silently evaluates (SOLID, clean boundaries, edge cases).
   - What NOT to do (5 common mistakes).
   - 3-4 common follow-up extensions listed without solutions.

2. Working C++ solution inside `src/`:
   - Separate `.hpp` and `.cpp` files per class (domain entities, state models, factory/strategy interfaces, central orchestrator facade).
   - Concise comments in code explaining WHY each class/pattern exists.
   - A runnable `main.cpp` demonstrating end-to-end usage, state transitions, fee/result calculations, and edge-case handling (e.g., capacity full, invalid inputs).

3. `lecture.md` (Master Lecture Notes & Interview Framework):
   - Conversational, mentor-style explanation of What, Why, and How.
   - The 4-step interview time budget framework (0-10m Clarification, 10-20m Class Modeling, 20-30m Pattern Selection, 30-75m Coding & Demo).
   - SOLID principles applied to this exact problem.
   - Modern C++ interview cheat sheet (smart pointers, standard containers, thread-safety/chrono).
   - Step-by-step breakdown of how the C++ classes interact.

4. `README.md`:
   - Compilation and execution commands (`g++ -std=c++17 src/*.cpp -o demo && ./demo`).
   - C++ `.hpp` vs `.cpp` readability notes.
   - Cross-reference table mapping design decisions to specific C++ source files.

Verify that the code compiles cleanly and executes before finishing.
```
