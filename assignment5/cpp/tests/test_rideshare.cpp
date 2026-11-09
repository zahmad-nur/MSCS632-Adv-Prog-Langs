// test_rideshare.cpp - unit tests (doctest, single header in tests/).
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include <cmath>
#include <memory>
#include <type_traits>
#include <vector>

#include "Driver.h"
#include "RideSharingSystem.h"
#include "RideTypes.h"
#include "Rider.h"

static long cents(double amount) { return std::lround(amount * 100); }

// ---------------------------------------------------------------- pricing
TEST_CASE("each ride type overrides fare() for a 6.0-mile trip") {
    CHECK(cents(Ride("A", "x", "y", 6.0).fare()) == 900);
    CHECK(cents(StandardRide("B", "x", "y", 6.0).fare()) == 1300);
    CHECK(cents(PremiumRide("C", "x", "y", 6.0).fare()) == 2700);
    CHECK(cents(PoolRide("D", "x", "y", 6.0, 1).fare()) == 975);
    CHECK(cents(PoolRide("E", "x", "y", 6.0, 3).fare()) == 845);
}

TEST_CASE("minimum fares apply to short trips") {
    CHECK(cents(StandardRide("A", "x", "y", 1.0).fare()) == 700);
    CHECK(cents(PremiumRide("B", "x", "y", 1.0).fare()) == 1500);
    CHECK(cents(PoolRide("C", "x", "y", 1.0, 6).fare()) == 500);
}

TEST_CASE("pool discount grows with co-riders and caps at 40%") {
    CHECK(PoolRide("A", "x", "y", 5.0, 0).discountRate() == doctest::Approx(0.20));
    CHECK(PoolRide("B", "x", "y", 5.0, 2).discountRate() == doctest::Approx(0.30));
    CHECK(PoolRide("C", "x", "y", 5.0, 10).discountRate() == doctest::Approx(0.40));
}

TEST_CASE("invalid ride input is rejected") {
    CHECK_THROWS_AS(StandardRide("A", "x", "y", 0.0), std::invalid_argument);
    CHECK_THROWS_AS(PoolRide("B", "x", "y", 3.0, -1), std::invalid_argument);
}

// ------------------------------------------------------------ polymorphism
TEST_CASE("fare() and rideDetails() dispatch through a base-class list") {
    std::vector<std::shared_ptr<Ride>> rides = {
        std::make_shared<StandardRide>("S", "x", "y", 6.0),
        std::make_shared<PremiumRide>("P", "x", "y", 6.0, "Lux Black"),
        std::make_shared<PoolRide>("L", "x", "y", 6.0, 2),
    };
    std::vector<long> fares;
    for (const auto& r : rides) fares.push_back(cents(r->fare()));
    CHECK(fares == std::vector<long>{1300, 2700, 910});

    CHECK(rides[0]->rideType() == "Standard");
    CHECK(rides[1]->rideDetails().find("vehicle: Lux Black") != std::string::npos);
    CHECK(rides[2]->rideDetails().find("2 co-riders, 30% off") != std::string::npos);
}

TEST_CASE("requesting a ride locks in the subclass fare") {
    Rider rider("U1", "Test");
    auto ride = std::make_shared<PremiumRide>("P", "x", "y", 6.0);
    rider.requestRide(ride);
    CHECK(ride->isFareLocked());
    CHECK(cents(ride->lockedFare()) == 2700);
    CHECK(cents(rider.totalSpent()) == 2700);
}

// ----------------------------------------------------------- encapsulation
template <class T>
concept CanTouchAssignedRides = requires(T& d) { d.assignedRides_; };

TEST_CASE("Driver keeps assignedRides private") {
    // Compile-time proof: outside code cannot name the private member.
    static_assert(!CanTouchAssignedRides<Driver>);
    // rides() hands out read-only pointers.
    static_assert(std::is_same_v<decltype(std::declval<Driver>().rides())::value_type,
                                 std::shared_ptr<const Ride>>);

    Driver d("D1", "Test");
    d.addRide(std::make_shared<StandardRide>("S", "x", "y", 2.0));
    auto copy = d.rides();
    copy.clear();
    CHECK(d.rideCount() == 1);
}

TEST_CASE("ratings are validated and averaged") {
    Driver d("D1", "Test", 4.0, 4);  // 16 stars over 4 ratings
    d.addRating(5);
    d.addRating(5);
    CHECK(d.rating() == doctest::Approx(26.0 / 6.0));
    CHECK(d.ratingCount() == 6);
    CHECK_THROWS_AS(d.addRating(0), std::out_of_range);
    CHECK_THROWS_AS(d.addRating(6), std::out_of_range);
    CHECK(d.ratingCount() == 6);
}

TEST_CASE("ride lifecycle is enforced") {
    StandardRide r("S", "x", "y", 2.0);
    CHECK_THROWS_AS(r.complete(), std::logic_error);  // not assigned yet
    r.assignDriver("D1", "Test");
    CHECK_THROWS_AS(r.assignDriver("D2", "Other"), std::logic_error);
    r.complete();
    CHECK(toString(r.status()) == "completed");
}

// -------------------------------------------------------- favorites/matching
TEST_CASE("favorite drivers are matched first, then the top-rated driver") {
    RideSharingSystem app;
    auto low  = app.addDriver("D1", "Low",  4.50, 10);
    auto high = app.addDriver("D2", "High", 4.99, 10);
    auto rider = app.addRider("U1", "Rider");
    rider->addFavoriteDriver("D1");

    auto m1 = app.bookRide(*rider, std::make_shared<StandardRide>("R1", "x", "y", 2.0));
    CHECK(m1.driver == low);
    CHECK(m1.viaFavorite);

    // Favorite is busy now, so fall back to the highest-rated driver.
    auto m2 = app.bookRide(*rider, std::make_shared<StandardRide>("R2", "x", "y", 2.0));
    CHECK(m2.driver == high);
    CHECK_FALSE(m2.viaFavorite);

    // Nobody left.
    auto m3 = app.bookRide(*rider, std::make_shared<StandardRide>("R3", "x", "y", 2.0));
    CHECK(m3.driver == nullptr);

    // Finishing a ride frees the favorite again.
    app.completeRide("R1", 5);
    CHECK(low->isAvailable());
    CHECK(cents(low->totalEarnings()) == 700);
}

TEST_CASE("favorites list has no duplicates and supports removal") {
    Rider r("U1", "Rider");
    r.addFavoriteDriver("D1");
    r.addFavoriteDriver("D1");
    r.addFavoriteDriver("D2");
    CHECK(r.favoriteDrivers() == std::vector<std::string>{"D1", "D2"});
    r.removeFavoriteDriver("D1");
    CHECK_FALSE(r.isFavorite("D1"));
    CHECK(r.isFavorite("D2"));
}
