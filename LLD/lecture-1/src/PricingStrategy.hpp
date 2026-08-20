#ifndef PRICING_STRATEGY_HPP
#define PRICING_STRATEGY_HPP

#include <chrono>
#include "Ticket.hpp"

using TimePoint = std::chrono::system_clock::time_point;

/**
 * @brief Abstract interface for pricing algorithms.
 * 
 * WHY THIS PATTERN EXISTS (STRATEGY PATTERN):
 * Pricing rules change frequently in real business systems (e.g. hourly rates, vehicle-dependent rates,
 * weekend surge rates). Wrapping fee calculation behind an interface allows switching strategies at runtime
 * without modifying core ParkingLot code (adhering to Open/Closed Principle).
 */
class PricingStrategy {
public:
    virtual ~PricingStrategy() = default;
    virtual double calculateFee(const Ticket& ticket, TimePoint exitTime) const = 0;
};

/**
 * @brief Concrete strategy charging a flat hourly rate regardless of vehicle type.
 */
class FlatHourlyPricingStrategy : public PricingStrategy {
private:
    double hourlyRate;

public:
    explicit FlatHourlyPricingStrategy(double rate = 20.0);
    double calculateFee(const Ticket& ticket, TimePoint exitTime) const override;
};

/**
 * @brief Concrete strategy charging different hourly rates based on vehicle type.
 */
class VehicleTypePricingStrategy : public PricingStrategy {
private:
    double bikeRate;
    double carRate;
    double truckRate;

public:
    VehicleTypePricingStrategy(double bike = 10.0, double car = 20.0, double truck = 50.0);
    double calculateFee(const Ticket& ticket, TimePoint exitTime) const override;
};

#endif // PRICING_STRATEGY_HPP
