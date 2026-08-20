# Parking Lot System — Machine Coding / Low-Level Design (LLD)

This repository contains a full, interview-realistic implementation of the **Design a Parking Lot System** problem in C++, formatted for machine coding interview practice.

---

## Lecture 1: Course Overview

If you are new to Low-Level Design (LLD) and Machine Coding rounds, start with:
- [**`lecture.md`**](file:///Users/amankashyap/Documents/sys-design/sys-design-1/lecture.md) — **Lecture 1 Notes:** What machine coding rounds evaluate (What, Why, How), the 4-step interview framework, design pattern selection, and mental models.

---

## C++ File Structure & Readability FAQ

### What is a `.hpp` file?
- **`.hpp` stands for Header File in C++** (Header C Plus Plus).
- In C++, code is typically split into two types of files:
  1. **Header Files (`.hpp`)**: Declare the structure, interface, class properties, and method signatures (like a blueprint or table of contents).
  2. **Source Files (`.cpp`)**: Contain the actual implementation logic for those methods.
- Separating declarations (`.hpp`) from definitions (`.cpp`) enables modularity, avoids code duplication, and speeds up compiler build times.

### Why is `std::` removed from the `.cpp` files?
- Repeating `std::string`, `std::shared_ptr`, `std::cout` everywhere can clutter code during interviews and impair readability.
- We brought standard library components into scope using `using namespace std;` (or targeted `using` directives inside `.cpp` files), keeping the code clean, concise, and easy to read on a whiteboard or during live coding.

---

## How to Compile and Run

Ensure you have a C++ compiler supporting **C++17** or higher (`g++` or `clang++`).

### Option 1: Direct compilation with `g++`
```bash
# Navigate to the project folder
cd sys-design-1

# Compile all source files into an executable named 'parking_lot'
g++ -std=c++17 src/*.cpp -o parking_lot

# Execute the runnable demo
./parking_lot
```

### Option 2: Direct compilation with `clang++` (macOS default)
```bash
clang++ -std=c++17 src/*.cpp -o parking_lot && ./parking_lot
```

---

## Code Structure & Design Decision Cross-Reference

Below is the mapping between the theoretical concepts explained in [`problem.md`](file:///Users/amankashyap/Documents/sys-design/sys-design-1/problem.md) and their exact implementation in the C++ codebase:

| Design Decision / Pattern | C++ File(s) | Implementation Rationale |
| :--- | :--- | :--- |
| **Strategy Pattern** (Pricing) | [`PricingStrategy.hpp`](file:///Users/amankashyap/Documents/sys-design/sys-design-1/src/PricingStrategy.hpp), [`PricingStrategy.cpp`](file:///Users/amankashyap/Documents/sys-design/sys-design-1/src/PricingStrategy.cpp) | Abstract `PricingStrategy` interface with concrete strategies `FlatHourlyPricingStrategy` and `VehicleTypePricingStrategy`. Decouples billing logic from lot orchestration (Open/Closed Principle). |
| **Factory Pattern** (Spot Creation) | [`SpotFactory.hpp`](file:///Users/amankashyap/Documents/sys-design/sys-design-1/src/SpotFactory.hpp), [`SpotFactory.cpp`](file:///Users/amankashyap/Documents/sys-design/sys-design-1/src/SpotFactory.cpp) | Encapsulates instantiation of `ParkingSpot` objects. Hides spot construction details and allows future spot types (e.g. EV spots) without changing caller code. |
| **Domain Models & Encapsulation** | [`Vehicle.hpp`](file:///Users/amankashyap/Documents/sys-design/sys-design-1/src/Vehicle.hpp), [`ParkingSpot.hpp`](file:///Users/amankashyap/Documents/sys-design/sys-design-1/src/ParkingSpot.hpp), [`Ticket.hpp`](file:///Users/amankashyap/Documents/sys-design/sys-design-1/src/Ticket.hpp) | Immutable domain entities (`Vehicle`, `Ticket`) and stateful entities (`ParkingSpot`) enforcing clear Single Responsibility Principle (SRP). |
| **Facade / Central Orchestrator** | [`ParkingLot.hpp`](file:///Users/amankashyap/Documents/sys-design/sys-design-1/src/ParkingLot.hpp), [`ParkingLot.cpp`](file:///Users/amankashyap/Documents/sys-design/sys-design-1/src/ParkingLot.cpp) | Manages spots across floors, handles entry/exit workflows, tracks active tickets, and delegates fee calculations to the active strategy. |
| **Runnable Demo & Edge Case Testing** | [`main.cpp`](file:///Users/amankashyap/Documents/sys-design/sys-design-1/src/main.cpp) | Demonstrates multi-floor setup, spot allocation, ticket issuance, time-shifted fee calculation, and edge cases (Lot Full, Invalid Ticket). |
