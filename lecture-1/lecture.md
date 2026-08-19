# Lecture 1: Low-Level Design (LLD) & Machine Coding Master Guide

> **Instructor Note:** This folder (`sys-design-1`) is designed as a complete, self-sufficient module for studying Machine Coding and Low-Level Design (LLD) interview rounds using the "Design a Parking Lot System" as our primary running problem.

---

## 1. What is a Machine Coding / LLD Round?

A **Machine Coding Round** (or Low-Level Design round) is a 60–90 minute practical programming evaluation commonly used by companies such as Flipkart, Swiggy, Uber, Amazon, and Razorpay.

Unlike High-Level Design (HLD) rounds—which focus on distributed systems, databases, caches, and load balancers—LLD evaluates your ability to write **clean, modular, maintainable, and extensible code inside a single application service**.

### What You Must Deliver to Pass:
1. **Clarified Requirements:** An agreed-upon set of functional rules before typing any code.
2. **Class Blueprint & Boundaries:** Clean separation of domain entities and business logic.
3. **Design Pattern Application:** Decoupled architecture using SOLID principles.
4. **Compilable Code:** Modular source files (`.hpp` headers + `.cpp` implementation).
5. **Working Console Demo (`main.cpp`):** Execution demonstrating vehicle entry, spot allocation, ticket generation, unparking, fee calculation, and edge-case handling.

---

## 2. Why Companies Use This Round (The Evaluation Rubric)

Interviewers do not expect a massive enterprise app in 75 minutes. They evaluate five core pillars:

```
┌─────────────────────────────────────────────────────────────┐
│                 INTERVIEW EVALUATION RUBRIC                 │
├────────────────────────────────┬────────────────────────────┤
│ 1. SOLID Principles            │ SRP, OCP, LSP, ISP, DIP    │
│ 2. Design Pattern Selection    │ Strategy, Factory, Facade  │
│ 3. Code Organization           │ .hpp vs .cpp separation    │
│ 4. Edge Case Robustness        │ Full lot, invalid tickets  │
│ 5. Executable Proof            │ Clean main() console demo  │
└────────────────────────────────┴────────────────────────────┘
```

---

## 3. The 4-Step Machine Coding Framework (Time Budget)

Use this exact minute-by-minute blueprint during your 75-minute interview:

### Phase 1: Requirement Clarification (Minutes 0–10)
- **Do NOT start coding immediately.**
- Ask 5–6 targeted questions:
  - Supported vehicle types? (Motorcycles, Cars, Trucks)
  - Spot matching rules? (Small -> Motorcycle, Medium -> Car, Large -> Truck)
  - Pricing model? (Hourly, vehicle-dependent, flat rate)
  - Parking lot layout? (Multi-floor, designated spots per floor)
  - In-memory vs DB storage? (In-memory data structures)

### Phase 2: Class Modeling & Entity Boundaries (Minutes 10–20)
- List all nouns as candidate classes:
  - **`Vehicle`**: License plate & vehicle type.
  - **`ParkingSpot`**: Spot ID, floor number, size type, occupancy state.
  - **`Ticket`**: Unique ID, vehicle details, assigned spot ID, entry timestamp.
  - **`ParkingLot`**: Manager tracking spots across floors and active tickets.

### Phase 3: Pattern Selection & Interface Design (Minutes 20–30)
- Identify areas of change and decouple them:
  - **Pricing Rules Change Frequently** -> Use **Strategy Pattern** (`PricingStrategy`).
  - **Spot Creation May Change** -> Use **Factory Pattern** (`SpotFactory`).
  - **Complex Subsystem Control** -> Use **Facade Pattern** (`ParkingLot`).

### Phase 4: Coding & Execution Demo (Minutes 30–75)
- Write header declarations (`.hpp`) and implementation (`.cpp`).
- Build a working `main.cpp` to prove entry, exit, fee calculation, and edge cases.

---

## 4. SOLID Principles in Practice

Here is how SOLID principles are implemented in this repository:

1. **Single Responsibility Principle (SRP):**
   - `Vehicle` only holds vehicle data.
   - `Ticket` only holds parking session metadata.
   - `PricingStrategy` only calculates fees.
   - `ParkingLot` only coordinates operations.

2. **Open/Closed Principle (OCP):**
   - The pricing system is open for extension, closed for modification. You can add `PeakSurgePricingStrategy` or `EVChargingPricingStrategy` without editing a single line of `ParkingLot.cpp`.

3. **Liskov Substitution Principle (LSP):**
   - `VehicleTypePricingStrategy` and `FlatHourlyPricingStrategy` can be used interchangeably wherever `PricingStrategy` is expected.

4. **Interface Segregation Principle (ISP):**
   - `PricingStrategy` exposes only `calculateFee()`, keeping interfaces lean and focused.

5. **Dependency Inversion Principle (DIP):**
   - `ParkingLot` depends on the abstract `PricingStrategy` interface, not a concrete implementation.

---

## 5. Modern C++ Crash Course for Machine Coding

When writing C++ in an LLD interview, follow these modern conventions:

### Memory Management: Smart Pointers
- Use `std::shared_ptr<T>` when multiple objects share ownership (e.g. `ParkingLot` and `Ticket` referencing `Vehicle`).
- Use `std::make_shared<T>()` for clean allocation.

### Standard Containers
- `std::vector<shared_ptr<ParkingSpot>>`: Ordered list of parking slots.
- `std::unordered_map<string, shared_ptr<Ticket>>`: $O(1)$ fast lookup for ticket verification.

### Time Handling
- `std::chrono::system_clock::time_point`: Precise timestamp tracking for entry and exit.

### Clean Code Tip (`std::` usage)
- Separate declarations (`.hpp`) from logic (`.cpp`).
- Use `using namespace std;` inside `.cpp` files to eliminate repetitive `std::` clutter during live coding.

---

## 6. Complete Self-Study Roadmap inside `sys-design-1`

Everything you need to master this topic is contained within this folder:

```
sys-design-1/
├── lecture.md       <- YOU ARE HERE: Complete theoretical & interview framework
├── problem.md       <- Mock Interview Problem Sheet (<700 words) with 8 standard sections
├── README.md        <- Build instructions, .hpp FAQ, and pattern-to-code mapping
└── src/             <- Production-grade, compilable C++ source code
    ├── Enums.hpp               # VehicleType & SpotType definitions
    ├── Vehicle.hpp / .cpp      # Vehicle domain entity
    ├── ParkingSpot.hpp / .cpp  # Parking spot entity
    ├── Ticket.hpp / .cpp       # Ticket session record
    ├── SpotFactory.hpp / .cpp  # Factory Pattern implementation
    ├── PricingStrategy.hpp / .cpp # Strategy Pattern implementation
    ├── ParkingLot.hpp / .cpp   # Central Orchestrator (Facade)
    └── main.cpp                # Runnable console demo
```

### How to Compile & Run:
```bash
cd sys-design-1
g++ -std=c++17 src/*.cpp -o parking_lot && ./parking_lot
```

---

## 7. Next Steps & Mental Models

When reviewing this folder on your own:
1. Read [`problem.md`](file:///Users/amankashyap/Documents/sys-design/sys-design-1/problem.md) to understand how the problem is stated and clarified.
2. Read [`README.md`](file:///Users/amankashyap/Documents/sys-design/sys-design-1/README.md) to see how concepts map to files.
3. Inspect [`src/`](file:///Users/amankashyap/Documents/sys-design/sys-design-1/src) line by line to understand how header/implementation separation works in C++.
4. Practice re-implementing this parking lot system from scratch in 60 minutes without looking at the code!
