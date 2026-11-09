#include "Ride.h"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <sstream>
#include <stdexcept>

std::string toString(RideStatus status) {
    switch (status) {
        case RideStatus::Requested: return "requested";
        case RideStatus::Assigned:  return "assigned";
        case RideStatus::Completed: return "completed";
    }
    return "unknown";
}

std::string formatMoney(double amount) {
    long cents = std::lround(amount * 100.0);
    std::ostringstream out;
    out << '$' << cents / 100 << '.' << std::setw(2) << std::setfill('0') << cents % 100;
    return out.str();
}

std::string formatMiles(double miles) {
    long tenths = std::lround(miles * 10.0);
    std::ostringstream out;
    out << tenths / 10 << '.' << tenths % 10 << " mi";
    return out.str();
}

Ride::Ride(std::string rideID, std::string pickupLocation,
           std::string dropoffLocation, double distance)
    : rideID_(std::move(rideID)),
      pickupLocation_(std::move(pickupLocation)),
      dropoffLocation_(std::move(dropoffLocation)),
      distance_(distance) {
    if (distance_ <= 0.0) {
        throw std::invalid_argument("distance must be positive");
    }
}

double Ride::fare() const { return distance_ * 1.50; }

std::string Ride::rideType() const { return "Ride"; }

std::string Ride::rideDetails() const {
    std::ostringstream out;
    out << '[' << rideID_ << "] " << std::left << std::setw(8) << rideType()
        << " | " << pickupLocation_ << " -> " << dropoffLocation_
        << " | " << formatMiles(distance_)
        << " | " << formatMoney(lockedFare())
        << " | " << toString(status_);
    if (!driverName_.empty()) out << " (driver: " << driverName_ << ')';
    return out.str();
}

double Ride::lockInFare() {
    if (!fareLocked_) {
        fare_ = fare();  // virtual call -> subclass pricing
        fareLocked_ = true;
    }
    return fare_;
}

double Ride::lockedFare() const { return fareLocked_ ? fare_ : fare(); }

void Ride::assignDriver(const std::string& driverID, const std::string& driverName) {
    if (status_ != RideStatus::Requested) {
        throw std::logic_error("ride " + rideID_ + " already has a driver");
    }
    driverID_ = driverID;
    driverName_ = driverName;
    status_ = RideStatus::Assigned;
}

void Ride::complete() {
    if (status_ != RideStatus::Assigned) {
        throw std::logic_error("ride " + rideID_ + " must be assigned before completing");
    }
    status_ = RideStatus::Completed;
}

double Ride::atLeast(double amount, double minimum) { return std::max(amount, minimum); }
