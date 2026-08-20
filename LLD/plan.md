# LLD (Low-Level Design) — Placement Prep

LLD is about designing the internals of a single component or service — class-level and object-level design, not system architecture. Interviewers care about clean OOP, extensibility, and correct use of design patterns, not scale.

## Core Concepts to Know Before Practicing Problems

- **OOP fundamentals** — encapsulation, inheritance, polymorphism, abstraction (and how to justify each choice, not just define them)
- **SOLID principles** — Single Responsibility, Open/Closed, Liskov Substitution, Interface Segregation, Dependency Inversion
- **UML basics** — class diagrams, relationships (association, aggregation, composition), sequence diagrams
- **Design patterns (most asked)**
  - Creational: Singleton, Factory, Builder
  - Structural: Adapter, Decorator, Facade
  - Behavioral: Strategy, Observer, State, Command
- **Concurrency basics** — thread safety, when Singleton needs to be thread-safe, producer-consumer problems

## Classic LLD Problems (Foundational — Start Here)

- **Parking Lot System** — classes: `ParkingLot`, `Level`, `Slot`, `Vehicle`, `Ticket`; tests OOP modeling and Strategy pattern for slot allocation
- **Library Management System** — book, member, borrowing rules, fine calculation
- **Elevator System** — scheduling algorithm, state management, multiple elevators/requests
- **Vending Machine** — state pattern (idle, selecting, dispensing, out-of-stock)
- **Tic-Tac-Toe / Chess Game Engine** — board representation, move validation, win-condition checking
- **ATM Machine** — transaction handling, state management, security basics

## Intermediate LLD Problems

- **Splitwise (Expense Sharing)** — bridges HLD and LLD; models `User`, `Expense`, `Balance`, split logic (equal/percentage/exact), balance simplification via graph reduction
- **BookMyShow / Movie Ticket Booking** — seat locking, concurrency handling, avoiding double booking
- **Cab Booking (Uber/Ola core logic)** — driver-rider matching logic, fare calculation, ride state machine
- **Food Delivery App (Swiggy/Zomato core logic)** — order lifecycle, restaurant-menu modeling, delivery assignment
- **Hotel Booking System** — room availability, booking conflicts, pricing rules
- **Notification System** — Observer pattern, multiple channels (SMS, email, push)

## Advanced / Company-Favorite LLD Problems

- **Rate Limiter (implementation, not just algorithm)** — token bucket class design, thread safety
- **Cache with Eviction Policy (LRU/LFU implementation)** — actual class + data structure design, not just the concept
- **Logging Framework** — Singleton + Strategy for log levels/destinations
- **File System (in-memory)** — Composite pattern for files/folders
- **Chess Engine (full)** — polymorphism for piece movement rules, move history, check/checkmate detection
- **Snake and Ladder Game** — board setup, dice roll, player movement, win condition

## Design Patterns Mapped to Problems (Study Together)

| Pattern | Problem It Commonly Shows Up In |
|---|---|
| Singleton | Logging framework, configuration manager, cache manager |
| Factory | Vehicle creation in parking lot, notification channel creation |
| Strategy | Fare calculation, sorting/matching algorithms, payment methods |
| Observer | Notification system, stock ticker updates |
| State | Vending machine, elevator, traffic light, ride status |
| Decorator | Adding toppings/features (e.g., coffee shop ordering system) |
| Builder | Constructing complex objects (e.g., building a meal order, HTTP request) |

## How to Practice Each Problem

For every problem, go through this sequence rather than jumping straight to code:

1. **Clarify requirements** — what exactly needs to be modeled, what's out of scope
2. **Identify core entities/classes** — nouns in the problem become candidate classes
3. **Define relationships** — inheritance vs composition vs association
4. **Apply SOLID + relevant design pattern(s)** — justify why, not just which
5. **Write the class diagram** — attributes and key methods only, not full implementation
6. **Code the core logic** — focus on the 2-3 methods that matter most (e.g., booking logic, not getters/setters)
7. **Discuss extensibility** — "if we added X feature, what would change?"

## How to Prioritize

- Start with the **classic problems** (Parking Lot, Library, Elevator, Vending Machine) — these teach the pattern of thinking interviewers expect
- Move to **intermediate problems** once comfortable — these are the most commonly asked at actual placements (Splitwise and BookMyShow show up frequently)
- Save **advanced problems** for later — these are more common in senior or dedicated LLD-round-heavy companies
- Practice **explaining trade-offs out loud** (e.g., "why Strategy over if-else chains") — this is what separates a pass from a borderline in LLD rounds