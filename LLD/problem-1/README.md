# Problem 1: Design a Parking Lot System (C++ Implementation)

A production-grade, modular C++17 implementation of the **Multi-Level Parking Lot System** problem for Machine Coding and Low-Level Design (LLD) placement interviews.

---

## 1. How to Compile and Run

Ensure you have a C++ compiler supporting **C++17** or higher (`g++` or `clang++`).

### Option 1: Direct compilation with `g++`
```bash
# Navigate to the problem folder
cd LLD/problem-1

# Compile all source files into an executable named 'parking_lot'
g++ -std=c++17 src/*.cpp -o parking_lot

# Run the executable demo
./parking_lot
```

### Option 2: Direct compilation with `clang++` (macOS default)
```bash
clang++ -std=c++17 src/*.cpp -o parking_lot && ./parking_lot
```

---

## 2. Code Structure & Design Pattern Cross-Reference

| File(s) | Role & Responsibilities | Design Pattern / OOP Concept |
| :--- | :--- | :--- |
| [`Enums.hpp`](file:///Users/amankashyap/Documents/sys-design/LLD/problem-1/src/Enums.hpp) | Defines `VehicleType` and `SpotType` with string formatting helpers. | Type Safety & Enums |
| [`Vehicle.hpp`](file:///Users/amankashyap/Documents/sys-design/LLD/problem-1/src/Vehicle.hpp), [`Vehicle.cpp`](file:///Users/amankashyap/Documents/sys-design/LLD/problem-1/src/Vehicle.cpp) | Domain model for vehicles entering the lot. | Encapsulation & Immutability |
| [`ParkingSpot.hpp`](file:///Users/amankashyap/Documents/sys-design/LLD/problem-1/src/ParkingSpot.hpp), [`ParkingSpot.cpp`](file:///Users/amankashyap/Documents/sys-design/LLD/problem-1/src/ParkingSpot.cpp) | Manages slot availability, sizing compatibility, and occupancy. | Aggregation (`Spot` holds `Vehicle`) |
| [`ParkingLevel.hpp`](file:///Users/amankashyap/Documents/sys-design/LLD/problem-1/src/ParkingLevel.hpp), [`ParkingLevel.cpp`](file:///Users/amankashyap/Documents/sys-design/LLD/problem-1/src/ParkingLevel.cpp) | Manages multi-floor slot layout and floor-level spot searches. | Composition (`Level` owns `Spots`) |
| [`Ticket.hpp`](file:///Users/amankashyap/Documents/sys-design/LLD/problem-1/src/Ticket.hpp), [`Ticket.cpp`](file:///Users/amankashyap/Documents/sys-design/LLD/problem-1/src/Ticket.cpp) | Immutable active parking session record holding timestamps and spot IDs. | State Tracking |
| [`SpotFactory.hpp`](file:///Users/amankashyap/Documents/sys-design/LLD/problem-1/src/SpotFactory.hpp), [`SpotFactory.cpp`](file:///Users/amankashyap/Documents/sys-design/LLD/problem-1/src/SpotFactory.cpp) | Encapsulates instantiation of parking spots based on size/type. | **Factory Pattern** |
| [`PricingStrategy.hpp`](file:///Users/amankashyap/Documents/sys-design/LLD/problem-1/src/PricingStrategy.hpp), [`PricingStrategy.cpp`](file:///Users/amankashyap/Documents/sys-design/LLD/problem-1/src/PricingStrategy.cpp) | Decouples fee calculation rules (`FlatHourlyPricingStrategy`, `VehicleBasedPricingStrategy`). | **Strategy Pattern** (Open/Closed Principle) |
| [`ParkingLot.hpp`](file:///Users/amankashyap/Documents/sys-design/LLD/problem-1/src/ParkingLot.hpp), [`ParkingLot.cpp`](file:///Users/amankashyap/Documents/sys-design/LLD/problem-1/src/ParkingLot.cpp) | Central entry point orchestrating entry, exit, ticket issuance, and billing. | **Facade Pattern** & Composition |
| [`main.cpp`](file:///Users/amankashyap/Documents/sys-design/LLD/problem-1/src/main.cpp) | Interactive demo verifying multi-level parking, ticket generation, unparking, fee calculation, and edge cases. | Test Harness |
