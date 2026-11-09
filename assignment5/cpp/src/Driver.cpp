#include "Driver.h"

#include <algorithm>
#include <iomanip>
#include <sstream>
#include <stdexcept>

Driver::Driver(std::string driverID, std::string name,
               double startingRating, int startingRatingCount)
    : driverID_(std::move(driverID)),
      name_(std::move(name)),
      ratingTotal_(startingRating * startingRatingCount),
      ratingCount_(startingRatingCount) {}

void Driver::addRide(const std::shared_ptr<Ride>& ride) {
    if (!ride) throw std::invalid_argument("ride cannot be null");
    ride->assignDriver(driverID_, name_);
    assignedRides_.push_back(ride);
}

void Driver::completeRide(const std::string& rideID) {
    auto it = std::find_if(assignedRides_.begin(), assignedRides_.end(),
                           [&](const auto& r) { return r->rideID() == rideID; });
    if (it == assignedRides_.end()) {
        throw std::invalid_argument(name_ + " has no ride " + rideID);
    }
    (*it)->complete();
}

void Driver::addRating(int stars) {
    if (stars < 1 || stars > 5) {
        throw std::out_of_range("rating must be between 1 and 5 stars");
    }
    ratingTotal_ += stars;
    ++ratingCount_;
}

std::vector<std::shared_ptr<const Ride>> Driver::rides() const {
    return {assignedRides_.begin(), assignedRides_.end()};  // copy, read-only
}

bool Driver::isAvailable() const {
    return std::none_of(assignedRides_.begin(), assignedRides_.end(),
                        [](const auto& r) { return r->status() == RideStatus::Assigned; });
}

double Driver::totalEarnings() const {
    double total = 0.0;
    for (const auto& r : assignedRides_) {
        if (r->status() == RideStatus::Completed) total += r->lockedFare();
    }
    return total;
}

double Driver::rating() const {
    return ratingCount_ == 0 ? 5.0 : ratingTotal_ / ratingCount_;
}

std::string Driver::getDriverInfo() const {
    std::ostringstream out;
    out << "Driver " << driverID_ << ": " << name_
        << " | rating " << std::fixed << std::setprecision(2) << rating()
        << " (" << ratingCount_ << " ratings)"
        << " | rides: " << assignedRides_.size()
        << " | earned " << formatMoney(totalEarnings())
        << " | " << (isAvailable() ? "available" : "on a trip");
    return out.str();
}
