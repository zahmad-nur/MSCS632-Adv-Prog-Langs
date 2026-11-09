#include "Rider.h"

#include <algorithm>
#include <sstream>
#include <stdexcept>

Rider::Rider(std::string riderID, std::string name)
    : riderID_(std::move(riderID)), name_(std::move(name)) {}

void Rider::requestRide(const std::shared_ptr<Ride>& ride) {
    if (!ride) throw std::invalid_argument("ride cannot be null");
    ride->lockInFare();  // upfront price, computed by the subclass's fare()
    requestedRides_.push_back(ride);
}

std::string Rider::viewRides() const {
    std::ostringstream out;
    out << "Ride history for " << name_ << " (" << riderID_ << "):\n";
    if (requestedRides_.empty()) {
        out << "  (no rides yet)\n";
    }
    for (const auto& ride : requestedRides_) {
        out << "  " << ride->rideDetails() << '\n';  // polymorphic call
    }
    out << "  Total spent: " << formatMoney(totalSpent());
    return out.str();
}

std::vector<std::shared_ptr<const Ride>> Rider::rides() const {
    return {requestedRides_.begin(), requestedRides_.end()};
}

double Rider::totalSpent() const {
    double total = 0.0;
    for (const auto& ride : requestedRides_) total += ride->lockedFare();
    return total;
}

void Rider::addFavoriteDriver(const std::string& driverID) {
    if (!isFavorite(driverID)) favoriteDrivers_.push_back(driverID);
}

void Rider::removeFavoriteDriver(const std::string& driverID) {
    favoriteDrivers_.erase(
        std::remove(favoriteDrivers_.begin(), favoriteDrivers_.end(), driverID),
        favoriteDrivers_.end());
}

bool Rider::isFavorite(const std::string& driverID) const {
    return std::find(favoriteDrivers_.begin(), favoriteDrivers_.end(), driverID)
           != favoriteDrivers_.end();
}
