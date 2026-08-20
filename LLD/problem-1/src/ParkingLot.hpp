#ifndef PARKING_LOT_HPP
#define PARKING_LOT_HPP

#include <vector>
#include <unordered_map>
#include <memory>
#include <string>
#include "ParkingLevel.hpp"
#include "Ticket.hpp"
#include "Vehicle.hpp"
#include "PricingStrategy.hpp"

using std::string;
using std::vector;
using std::unordered_map;
using std::shared_ptr;
using TimePoint = std::chrono::system_clock::time_point;

/**
 * @brief Central Orchestrator / Facade for the Parking Lot.
 * 
 * WHY THIS CLASS EXISTS:
 * Manages the multi-level layout, tracks active tickets, delegates spot searches,
 * and coordinates vehicle entry, exit, and billing.
 */
class ParkingLot {
private:
    vector<shared_ptr<ParkingLevel>> levels;
    unordered_map<string, shared_ptr<Ticket>> activeTickets;
    shared_ptr<PricingStrategy> pricingStrategy;
    int ticketCounter;

public:
    explicit ParkingLot(shared_ptr<PricingStrategy> strategy);

    void addLevel(shared_ptr<ParkingLevel> level);
    void setPricingStrategy(shared_ptr<PricingStrategy> strategy);

    shared_ptr<Ticket> parkVehicle(shared_ptr<Vehicle> vehicle);
    double unparkVehicle(const string& ticketId, TimePoint exitTime);

    void displayStatus() const;
};

#endif // PARKING_LOT_HPP
