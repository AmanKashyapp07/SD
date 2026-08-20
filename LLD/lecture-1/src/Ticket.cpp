#include "Ticket.hpp"

using std::string;

Ticket::Ticket(const string& id, const string& plate, VehicleType vType, const string& sId, TimePoint entry)
    : ticketId(id), licensePlate(plate), vehicleType(vType), spotId(sId), entryTime(entry) {}

string Ticket::getTicketId() const {
    return ticketId;
}

string Ticket::getLicensePlate() const {
    return licensePlate;
}

VehicleType Ticket::getVehicleType() const {
    return vehicleType;
}

string Ticket::getSpotId() const {
    return spotId;
}

TimePoint Ticket::getEntryTime() const {
    return entryTime;
}
