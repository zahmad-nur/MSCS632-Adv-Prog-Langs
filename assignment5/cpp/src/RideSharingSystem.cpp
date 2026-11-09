#include "RideSharingSystem.h"

#include <algorithm>
#include <stdexcept>

std::shared_ptr<Driver> RideSharingSystem::addDriver(const std::string& id,
                                                     const std::string& name,
                                                     double rating, int ratingCount) {
    auto driver = std::make_shared<Driver>(id, name, rating, ratingCount);
    drivers_.push_back(driver);
    return driver;
}

std::shared_ptr<Rider> RideSharingSystem::addRider(const std::string& id,
                                                   const std::string& name) {
    auto rider = std::make_shared<Rider>(id, name);
    riders_.push_back(rider);
    return rider;
}

std::shared_ptr<Driver> RideSharingSystem::findDriver(const std::string& driverID) const {
    for (const auto& d : drivers_) {
        if (d->driverID() == driverID) return d;
    }
    return nullptr;
}

MatchResult RideSharingSystem::findDriverFor(const Rider& rider) const {
    // 1) Favorites first, in the rider's order of preference.
    for (const auto& favID : rider.favoriteDrivers()) {
        auto driver = findDriver(favID);
        if (driver && driver->isAvailable()) return {driver, true};
    }
    // 2) Otherwise the highest-rated available driver.
    std::shared_ptr<Driver> best;
    for (const auto& d : drivers_) {
        if (d->isAvailable() && (!best || d->rating() > best->rating())) best = d;
    }
    return {best, false};
}

MatchResult RideSharingSystem::bookRide(Rider& rider, const std::shared_ptr<Ride>& ride) {
    rider.requestRide(ride);
    rides_.push_back(ride);
    MatchResult match = findDriverFor(rider);
    if (match.driver) match.driver->addRide(ride);
    return match;
}

void RideSharingSystem::completeRide(const std::string& rideID, int stars) {
    auto it = std::find_if(rides_.begin(), rides_.end(),
                           [&](const auto& r) { return r->rideID() == rideID; });
    if (it == rides_.end()) throw std::invalid_argument("unknown ride " + rideID);
    auto driver = findDriver((*it)->driverID());
    if (!driver) throw std::logic_error("ride " + rideID + " has no driver");
    driver->completeRide(rideID);
    driver->addRating(stars);
}
