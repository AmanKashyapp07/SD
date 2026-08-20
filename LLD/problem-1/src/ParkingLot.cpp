#include "ParkingLot.hpp"
#include <iostream>
#include <iomanip>

using namespace std;

ParkingLot::ParkingLot(shared_ptr<PricingStrategy> strategy)
    : pricingStrategy(strategy), ticketCounter(1000) {}

void ParkingLot::addLevel(shared_ptr<ParkingLevel> level) {
    levels.push_back(level);
}

void ParkingLot::setPricingStrategy(shared_ptr<PricingStrategy> strategy) {
    pricingStrategy = strategy;
}

shared_ptr<Ticket> ParkingLot::parkVehicle(shared_ptr<Vehicle> vehicle) {
    // Search levels sequentially for the lowest available compatible spot
    for (auto& level : levels) {
        auto spot = level->findAvailableSpot(vehicle->getType());
        if (spot && spot->park(vehicle)) {
            ticketCounter++;
            string ticketId = "TKT-" + to_string(ticketCounter);
            auto entryTime = chrono::system_clock::now();
            auto ticket = make_shared<Ticket>(ticketId, vehicle->getLicensePlate(), vehicle->getType(), spot->getSpotId(), entryTime);
            activeTickets[ticketId] = ticket;

            cout << "[ENTRY SUCCESS] Vehicle " << vehicle->getLicensePlate()
                 << " (" << vehicleTypeToString(vehicle->getType()) << ")"
                 << " parked at Spot " << spot->getSpotId()
                 << " on Floor " << level->getFloorNumber()
                 << " | Ticket Issued: " << ticketId << "\n";
            return ticket;
        }
    }

    cout << "[ENTRY FAILED] No available spot for Vehicle " << vehicle->getLicensePlate()
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

    // Find and free the allocated parking spot
    for (auto& level : levels) {
        for (auto& spot : level->getSpots()) {
            if (spot->getSpotId() == ticket->getSpotId()) {
                spot->unpark();
                break;
            }
        }
    }

    // Calculate fee via the configured strategy
    double fee = pricingStrategy->calculateFee(*ticket, exitTime);
    activeTickets.erase(it);

    cout << "[EXIT SUCCESS] Ticket: " << ticketId
         << " | Vehicle: " << ticket->getLicensePlate()
         << " freed Spot: " << ticket->getSpotId()
         << " | Total Fee: $" << fixed << setprecision(2) << fee << "\n";

    return fee;
}

void ParkingLot::displayStatus() const {
    cout << "\n================ PARKING LOT STATUS ================\n";
    int totalSpots = 0;
    int occupiedCount = 0;

    for (const auto& level : levels) {
        cout << "--- Floor " << level->getFloorNumber() << " ---\n";
        for (const auto& spot : level->getSpots()) {
            totalSpots++;
            cout << "  Spot ID: " << left << setw(8) << spot->getSpotId()
                 << " | Type: " << left << setw(18) << spotTypeToString(spot->getSpotType())
                 << " | Status: " << (spot->getIsOccupied() ? "OCCUPIED (" + spot->getParkedVehicle()->getLicensePlate() + ")" : "AVAILABLE")
                 << "\n";
            if (spot->getIsOccupied()) occupiedCount++;
        }
    }
    cout << "Total Capacity: " << totalSpots
         << " | Occupied: " << occupiedCount
         << " | Available: " << (totalSpots - occupiedCount) << "\n";
    cout << "====================================================\n\n";
}
