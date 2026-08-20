#include "ParkingLot.hpp"
#include <iostream>
#include <iomanip>

using namespace std;

ParkingLot::ParkingLot(shared_ptr<PricingStrategy> strategy)
    : pricingStrategy(strategy), nextTicketNumber(1001) {}

void ParkingLot::setPricingStrategy(shared_ptr<PricingStrategy> strategy) {
    pricingStrategy = strategy;
}

void ParkingLot::addSpot(shared_ptr<ParkingSpot> spot) {
    spots.push_back(spot);
}

shared_ptr<Ticket> ParkingLot::parkVehicle(shared_ptr<Vehicle> vehicle) {
    // Search for first compatible and available parking spot
    for (auto& spot : spots) {
        if (!spot->isOccupied() && spot->isCompatibleWith(vehicle->getType())) {
            if (spot->park(vehicle)) {
                string ticketId = "TKT-" + to_string(nextTicketNumber++);
                auto entryTime = chrono::system_clock::now();
                auto ticket = make_shared<Ticket>(ticketId, vehicle->getLicensePlate(), vehicle->getType(), spot->getSpotId(), entryTime);
                activeTickets[ticketId] = ticket;

                cout << "[ENTRY SUCCESS] Vehicle " << vehicle->getLicensePlate()
                     << " (" << vehicleTypeToString(vehicle->getType()) << ")"
                     << " parked at Spot " << spot->getSpotId()
                     << " | Ticket Issued: " << ticketId << "\n";
                return ticket;
            }
        }
    }

    cout << "[ENTRY FAILED] Parking Lot Full for Vehicle " << vehicle->getLicensePlate()
         << " (" << vehicleTypeToString(vehicle->getType()) << ")\n";
    return nullptr;
}

double ParkingLot::unparkVehicle(const string& ticketId, TimePoint exitTime) {
    auto it = activeTickets.find(ticketId);
    if (it == activeTickets.end()) {
        cout << "[EXIT ERROR] Invalid or expired Ticket ID: " << ticketId << "\n";
        return -1.0;
    }

    auto ticket = it->second;

    // Find assigned spot and free it up
    for (auto& spot : spots) {
        if (spot->getSpotId() == ticket->getSpotId()) {
            spot->unpark();
            break;
        }
    }

    // Calculate fee via configured PricingStrategy
    double fee = pricingStrategy->calculateFee(*ticket, exitTime);

    // Remove from active tickets registry
    activeTickets.erase(it);

    cout << "[EXIT SUCCESS] Ticket: " << ticketId
         << " | Vehicle: " << ticket->getLicensePlate()
         << " freed Spot: " << ticket->getSpotId()
         << " | Total Fee: $" << fixed << setprecision(2) << fee << "\n";

    return fee;
}

void ParkingLot::displayStatus() const {
    cout << "\n================ PARKING LOT STATUS ================\n";
    int occupiedCount = 0;
    for (const auto& spot : spots) {
        cout << "Spot ID: " << left << setw(8) << spot->getSpotId()
             << " | Floor: " << spot->getFloorNumber()
             << " | Type: " << left << setw(18) << spotTypeToString(spot->getSpotType())
             << " | Status: " << (spot->isOccupied() ? "OCCUPIED (" + spot->getParkedVehicle()->getLicensePlate() + ")" : "AVAILABLE")
             << "\n";
        if (spot->isOccupied()) occupiedCount++;
    }
    cout << "Total Capacity: " << spots.size() << " | Occupied: " << occupiedCount << " | Available: " << (spots.size() - occupiedCount) << "\n";
    cout << "====================================================\n\n";
}
