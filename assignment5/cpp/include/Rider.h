// Rider.h - a rider, their ride history and their favorite drivers.
//
// Favorites are stored as an ordered list of driver IDs (first = most
// preferred). RideSharingSystem checks this list before falling back to the
// highest-rated available driver.
#pragma once

#include <memory>
#include <string>
#include <vector>

#include "Ride.h"

class Rider {
public:
    Rider(std::string riderID, std::string name);

    void requestRide(const std::shared_ptr<Ride>& ride);  // locks in the fare
    std::string viewRides() const;                        // ride history
    std::vector<std::shared_ptr<const Ride>> rides() const;
    double totalSpent() const;

    void addFavoriteDriver(const std::string& driverID);
    void removeFavoriteDriver(const std::string& driverID);
    bool isFavorite(const std::string& driverID) const;
    std::vector<std::string> favoriteDrivers() const { return favoriteDrivers_; }

    const std::string& riderID() const { return riderID_; }
    const std::string& name() const { return name_; }

private:
    std::string riderID_;
    std::string name_;
    std::vector<std::shared_ptr<Ride>> requestedRides_;
    std::vector<std::string> favoriteDrivers_;
};
