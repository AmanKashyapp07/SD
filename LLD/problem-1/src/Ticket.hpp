#ifndef TICKET_HPP
#define TICKET_HPP

#include <string>
#include <chrono>
#include "Enums.hpp"
#include "Vehicle.hpp"

using std::string;
using TimePoint = std::chrono::system_clock::time_point;

/**
 * @brief Represents an immutable parking session ticket.
 * 
 * WHY THIS CLASS EXISTS:
 * Serves as the source of truth for an active parking session,
 * binding entry timestamp, vehicle details, and assigned spot ID together.
 */
class Ticket {
private:
    string ticketId;
    string licensePlate;
    VehicleType vehicleType;
    string spotId;
    TimePoint entryTime;

public:
    Ticket(const string& id, const string& plate, VehicleType vType, const string& sId, TimePoint entry);

    string getTicketId() const;
    string getLicensePlate() const;
    VehicleType getVehicleType() const;
    string getSpotId() const;
    TimePoint getEntryTime() const;
};

#endif // TICKET_HPP
