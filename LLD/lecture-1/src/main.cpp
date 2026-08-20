#include <iostream>
#include <memory>
#include <chrono>

#include "Enums.hpp"
#include "Vehicle.hpp"
#include "SpotFactory.hpp"
#include "PricingStrategy.hpp"
#include "ParkingLot.hpp"

using namespace std;
using namespace std::chrono;

int main() {
    cout << "======================================================\n";
    cout << "  MACHINE CODING DEMO: DESIGN A PARKING LOT SYSTEM\n";
    cout << "======================================================\n\n";

    // 1. Initialize Pricing Strategy (Strategy Pattern)
    // VehicleTypePricingStrategy charges: Bike = $10/hr, Car = $20/hr, Truck = $50/hr
    auto pricingStrategy = make_shared<VehicleTypePricingStrategy>(10.0, 20.0, 50.0);
    ParkingLot parkingLot(pricingStrategy);

    // 2. Add spots across Floor 1 and Floor 2 using SpotFactory (Factory Pattern)
    cout << "--> Initializing Parking Lot with multi-floor spots...\n";
    parkingLot.addSpot(SpotFactory::createSpot("F1-S1", 1, SpotType::SMALL));  // Bike Spot
    parkingLot.addSpot(SpotFactory::createSpot("F1-C1", 1, SpotType::MEDIUM)); // Car Spot
    parkingLot.addSpot(SpotFactory::createSpot("F1-T1", 1, SpotType::LARGE));  // Truck Spot
    
    parkingLot.addSpot(SpotFactory::createSpot("F2-S1", 2, SpotType::SMALL));  // Bike Spot
    parkingLot.addSpot(SpotFactory::createSpot("F2-C1", 2, SpotType::MEDIUM)); // Car Spot

    // Display initial status
    parkingLot.displayStatus();

    // 3. Create Vehicles
    auto bike1 = make_shared<Vehicle>("KA-01-HH-1234", VehicleType::MOTORCYCLE);
    auto car1 = make_shared<Vehicle>("MH-12-AB-9999", VehicleType::CAR);
    auto car2 = make_shared<Vehicle>("DL-03-CC-5555", VehicleType::CAR);
    auto car3 = make_shared<Vehicle>("KA-05-XX-0001", VehicleType::CAR); // Extra car to test Lot Full
    auto truck1 = make_shared<Vehicle>("HR-26-TR-8888", VehicleType::TRUCK);

    // 4. Park Vehicles & Issue Tickets
    cout << "--> Parking Vehicles...\n";
    auto ticket1 = parkingLot.parkVehicle(bike1);  // Should get F1-S1
    auto ticket2 = parkingLot.parkVehicle(car1);   // Should get F1-C1
    auto ticket3 = parkingLot.parkVehicle(car2);   // Should get F2-C1
    auto ticket4 = parkingLot.parkVehicle(truck1); // Should get F1-T1

    cout << "\n--> Testing Edge Case: Parking Car 3 when all Medium spots are full...\n";
    auto ticket5 = parkingLot.parkVehicle(car3);   // Should fail (Lot Full for Cars)

    parkingLot.displayStatus();

    // 5. Simulate Time Elapsed for Unparking (e.g. 3 Hours later)
    cout << "--> Simulating vehicle exits after 3 hours...\n";
    auto currentTime = system_clock::now();
    auto exitTime3HoursLater = currentTime + hours(3);

    if (ticket1) {
        parkingLot.unparkVehicle(ticket1->getTicketId(), exitTime3HoursLater);
    }
    if (ticket2) {
        parkingLot.unparkVehicle(ticket2->getTicketId(), exitTime3HoursLater);
    }

    // 6. Test Edge Case: Attempting to unpark with invalid ticket ID
    cout << "\n--> Testing Edge Case: Unparking with invalid Ticket ID...\n";
    parkingLot.unparkVehicle("TKT-INVALID-999", exitTime3HoursLater);

    // 7. Retrying Car 3 parking after Car 1 freed a spot
    cout << "\n--> Retrying Car 3 parking now that a Car spot is freed...\n";
    auto ticketCar3Retry = parkingLot.parkVehicle(car3); // Should succeed now!

    // Final Lot Status
    parkingLot.displayStatus();

    cout << "======================================================\n";
    cout << "  DEMO COMPLETED SUCCESSFULLY!\n";
    cout << "======================================================\n";

    return 0;
}
