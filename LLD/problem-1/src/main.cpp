#include <iostream>
#include <memory>
#include <chrono>

#include "Enums.hpp"
#include "Vehicle.hpp"
#include "ParkingLevel.hpp"
#include "SpotFactory.hpp"
#include "PricingStrategy.hpp"
#include "ParkingLot.hpp"

using namespace std;
using namespace std::chrono;

int main() {
    cout << "======================================================\n";
    cout << "  MACHINE CODING DEMO: MULTI-LEVEL PARKING LOT SYSTEM\n";
    cout << "======================================================\n\n";

    // 1. Initialize Strategy Pattern (Pricing: Bike $10/hr, Car $20/hr, Truck $50/hr)
    auto pricingStrategy = make_shared<VehicleBasedPricingStrategy>(10.0, 20.0, 50.0);
    ParkingLot lot(pricingStrategy);

    // 2. Build multi-floor parking structure using Composition & Factory Pattern
    cout << "--> Constructing Multi-Level Parking Lot...\n";
    
    // Level 1: 1 Small spot, 1 Medium spot, 1 Large spot
    auto floor1 = make_shared<ParkingLevel>(1);
    floor1->addSpot(SpotFactory::createSpot("F1-S1", 1, SpotType::SMALL));
    floor1->addSpot(SpotFactory::createSpot("F1-C1", 1, SpotType::MEDIUM));
    floor1->addSpot(SpotFactory::createSpot("F1-T1", 1, SpotType::LARGE));
    lot.addLevel(floor1);

    // Level 2: 1 Small spot, 1 Medium spot
    auto floor2 = make_shared<ParkingLevel>(2);
    floor2->addSpot(SpotFactory::createSpot("F2-S1", 2, SpotType::SMALL));
    floor2->addSpot(SpotFactory::createSpot("F2-C1", 2, SpotType::MEDIUM));
    lot.addLevel(floor2);

    lot.displayStatus();

    // 3. Create Vehicles
    auto bike1 = make_shared<Vehicle>("KA-01-HH-1234", VehicleType::MOTORCYCLE);
    auto car1 = make_shared<Vehicle>("MH-12-AB-9999", VehicleType::CAR);
    auto car2 = make_shared<Vehicle>("DL-03-CC-5555", VehicleType::CAR);
    auto car3 = make_shared<Vehicle>("KA-05-XX-0001", VehicleType::CAR); // Extra car for testing Lot Full
    auto truck1 = make_shared<Vehicle>("HR-26-TR-8888", VehicleType::TRUCK);

    // 4. Park Vehicles & Generate Tickets
    cout << "--> Parking Vehicles...\n";
    auto ticket1 = lot.parkVehicle(bike1);  // Should get F1-S1
    auto ticket2 = lot.parkVehicle(car1);   // Should get F1-C1
    auto ticket3 = lot.parkVehicle(car2);   // Should get F2-C1
    auto ticket4 = lot.parkVehicle(truck1); // Should get F1-T1

    cout << "\n--> Testing Edge Case 1: Parking Car 3 when all Car spots are occupied...\n";
    auto ticket5 = lot.parkVehicle(car3);   // Should fail (Lot Full for Cars)

    lot.displayStatus();

    // 5. Simulate Unparking after 3 hours
    cout << "--> Simulating Vehicle Exits after 3 Hours...\n";
    auto currentTime = system_clock::now();
    auto exitTime3HoursLater = currentTime + hours(3);

    if (ticket1) {
        lot.unparkVehicle(ticket1->getTicketId(), exitTime3HoursLater);
    }
    if (ticket2) {
        lot.unparkVehicle(ticket2->getTicketId(), exitTime3HoursLater);
    }

    // 6. Test Edge Case 2: Unparking with invalid ticket
    cout << "\n--> Testing Edge Case 2: Unparking with invalid Ticket ID...\n";
    lot.unparkVehicle("TKT-INVALID-999", exitTime3HoursLater);

    // 7. Retrying Car 3 parking now that Car 1 spot is freed
    cout << "\n--> Retrying Car 3 parking after Spot F1-C1 was freed...\n";
    auto ticketCar3Retry = lot.parkVehicle(car3); // Should succeed now!

    lot.displayStatus();

    cout << "======================================================\n";
    cout << "  DEMO COMPLETED SUCCESSFULLY!\n";
    cout << "======================================================\n";

    return 0;
}
