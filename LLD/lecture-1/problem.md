# Machine Coding / LLD: Design a Parking Lot System

## 1. What This Round Is
A Machine Coding (Low-Level Design) round is a 60–90 minute practical evaluation used by companies like Flipkart, Swiggy, and Uber. Interviewers evaluate your ability to translate high-level requirements into clean, modular, extensible, and working code. You are expected to produce compilable, runnable code with a working `main()` demo—diagrams or pseudo-code alone will result in a fail.

## 2. How the Problem Is Typically Phrased
"We want you to design and implement an automated Parking Lot system. It should manage vehicle entry, spot allocation, ticket generation, and fee calculation on exit. You have 75 minutes to write code that compiles and runs."

## 3. Clarifying Questions a Good Candidate Would Ask
1. **What vehicle types are supported?** (*Rationale: Determines enum scope and spot type matching logic.*)
2. **How are parking spots allocated?** (*Rationale: First-available, nearest to entrance, or strategy-based allocation?*)
3. **What is the pricing model?** (*Rationale: Flat rate, hourly, or vehicle-dependent rate? Need Strategy pattern?*)
4. **Is this a multi-floor parking lot?** (*Rationale: Dictates data structures for tracking floors and capacity.*)
5. **How are tickets generated and validated?** (*Rationale: Defines ticket attributes such as ID, entry timestamp, and spot ID.*)
6. **Do we need concurrency handling or persistent storage?** (*Rationale: In-memory design vs mutex/DB overhead for standard rounds.*)

## 4. Sample Finalized Requirements
- **Multi-floor Support:** The lot has multiple floors, each containing designated spots for Motorcycles, Cars, and Trucks.
- **Vehicle Entry:** Automatically assign the first available compatible spot (Motorcycle -> Small, Car -> Medium, Truck -> Large).
- **Ticket Generation:** Issue a unique ticket containing Ticket ID, Vehicle info, Spot ID, and Entry Timestamp upon entry.
- **Vehicle Exit & Fee Calculation:** Retrieve ticket, free up spot, calculate fee based on duration and vehicle type, and return receipt.
- **In-Memory Operations:** Clean object-oriented design with working in-memory state.

## 5. Design Decisions and Patterns Used
- **Strategy Pattern (`PricingStrategy`):** Decouples pricing algorithms from the `ParkingLot`. Allows easy addition of peak-hour, flat, or vehicle-based billing without modifying core lot logic (Open/Closed Principle).
- **Factory Pattern (`SpotFactory`):** Encapsulates creation logic for different `ParkingSpot` types based on size/vehicle type, hiding initialization details from `ParkingLot`.
- **Single Responsibility & Encapsulation:** `Ticket` manages time/spot metadata, `ParkingSpot` tracks state availability, and `ParkingLot` orchestrates operations.

## 6. What the Interviewer Is Silently Evaluating
- **Class Design & Abstraction:** Clear separation of domain models (`Vehicle`, `Ticket`, `ParkingSpot`).
- **SOLID Principles:** Single Responsibility, Open/Closed (via Strategy), Dependency Inversion.
- **Code Cleanliness & Maintainability:** Readable naming, modular files, proper encapsulation.
- **Edge Case Handling:** Lot full, invalid ticket ID, double unparking attempt.
- **Working Code:** Program compiles cleanly and runs a meaningful `main()` demo.

## 7. What NOT to Do
- **Jumping Straight to Coding:** Skipping clarification leads to wrong assumptions and rewrite penalty.
- **Monolithic God Class:** Placing all logic inside `ParkingLot` or `main()` violates SRP.
- **Over-Engineering:** Avoid microservices, databases, or complex thread pools unless requested.
- **Ignoring Edge Cases:** Failing to handle full lots or non-existent tickets gracefully.
- **Coding in Silence:** Failing to communicate your architectural thought process out loud.

## 8. Common Follow-up Extensions
1. **EV Spot Support:** Add electric charging spots with kWh charging fee calculation on exit.
2. **Dynamic Spot Allocation Strategy:** Support nearest-to-entrance allocation strategy (Strategy pattern).
3. **Reservation System:** Allow booking spots in advance with expiration timers.
4. **Multi-Payment Gateways:** Integrate Cash, UPI, and Credit Card payment processing methods.
