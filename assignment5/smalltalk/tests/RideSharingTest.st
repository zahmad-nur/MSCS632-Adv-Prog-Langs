"RideSharingTest.st - SUnit tests. Run with ./run_tests.sh"

TestCase subclass: RideSharingTest [
    cents: amount [ ^(amount * 100) rounded ]

    "------------------------------------------------------ pricing"

    testEachRideTypeOverridesFare [
        self assert: (self cents: (Ride id: 'A' from: 'x' to: 'y' miles: 6.0) fare) = 900.
        self assert: (self cents: (StandardRide id: 'B' from: 'x' to: 'y' miles: 6.0) fare) = 1300.
        self assert: (self cents: (PremiumRide id: 'C' from: 'x' to: 'y' miles: 6.0) fare) = 2700.
        self assert: (self cents: (PoolRide id: 'D' from: 'x' to: 'y' miles: 6.0 coRiders: 1) fare) = 975.
        self assert: (self cents: (PoolRide id: 'E' from: 'x' to: 'y' miles: 6.0 coRiders: 3) fare) = 845
    ]

    testMinimumFares [
        self assert: (self cents: (StandardRide id: 'A' from: 'x' to: 'y' miles: 1.0) fare) = 700.
        self assert: (self cents: (PremiumRide id: 'B' from: 'x' to: 'y' miles: 1.0) fare) = 1500.
        self assert: (self cents: (PoolRide id: 'C' from: 'x' to: 'y' miles: 1.0 coRiders: 6) fare) = 500
    ]

    testPoolDiscountCapsAtFortyPercent [
        self assert: (self cents: (PoolRide id: 'A' from: 'x' to: 'y' miles: 5.0 coRiders: 0) discountRate) = 20.
        self assert: (self cents: (PoolRide id: 'B' from: 'x' to: 'y' miles: 5.0 coRiders: 2) discountRate) = 30.
        self assert: (self cents: (PoolRide id: 'C' from: 'x' to: 'y' miles: 5.0 coRiders: 10) discountRate) = 40
    ]

    testInvalidRideInputIsRejected [
        self should: [StandardRide id: 'A' from: 'x' to: 'y' miles: 0]
             raise: SystemExceptions.InvalidArgument.
        self should: [PoolRide id: 'B' from: 'x' to: 'y' miles: 3.0 coRiders: -1]
             raise: SystemExceptions.InvalidArgument
    ]

    "------------------------------------------------- polymorphism"

    testPolymorphicCollection [
        | rides fares |
        rides := OrderedCollection new.
        rides
            add: (StandardRide id: 'S' from: 'x' to: 'y' miles: 6.0);
            add: (PremiumRide id: 'P' from: 'x' to: 'y' miles: 6.0 vehicle: 'Lux Black');
            add: (PoolRide id: 'L' from: 'x' to: 'y' miles: 6.0 coRiders: 2).
        fares := rides collect: [:r | self cents: r fare].
        self assert: fares asArray = #(1300 2700 910).
        self assert: rides first rideType = 'Standard'.
        self assert: ((rides at: 2) rideDetails indexOfSubCollection: 'vehicle: Lux Black') > 0.
        self assert: (rides last rideDetails indexOfSubCollection: '2 co-riders, 30% off') > 0
    ]

    testRequestingLocksInSubclassFare [
        | rider ride |
        rider := Rider id: 'U1' name: 'Test'.
        ride := PremiumRide id: 'P' from: 'x' to: 'y' miles: 6.0.
        rider requestRide: ride.
        self assert: ride isFareLocked.
        self assert: (self cents: ride lockedFare) = 2700.
        self assert: (self cents: rider totalSpent) = 2700
    ]

    "------------------------------------------------ encapsulation"

    testDriverKeepsAssignedRidesPrivate [
        | d copy |
        d := Driver id: 'D1' name: 'Test'.
        d addRide: (StandardRide id: 'S' from: 'x' to: 'y' miles: 2.0).
        "No accessor exists for the instance variable."
        self should: [d assignedRides] raise: MessageNotUnderstood.
        "#rides answers a copy, so changing it leaves the driver alone."
        copy := d rides.
        copy removeFirst.
        self assert: copy isEmpty.
        self assert: d rideCount = 1
    ]

    testRatingsAreValidatedAndAveraged [
        | d |
        d := Driver id: 'D1' name: 'Test' rating: 4 count: 4.
        d addRating: 5; addRating: 5.
        self assert: d rating = (26 / 6).
        self assert: d ratingCount = 6.
        self should: [d addRating: 0] raise: SystemExceptions.ArgumentOutOfRange.
        self should: [d addRating: 6] raise: SystemExceptions.ArgumentOutOfRange.
        self assert: d ratingCount = 6
    ]

    testRideLifecycleIsEnforced [
        | r |
        r := StandardRide id: 'S' from: 'x' to: 'y' miles: 2.0.
        self should: [r complete] raise: Error.
        r assignDriverId: 'D1' name: 'Test'.
        self should: [r assignDriverId: 'D2' name: 'Other'] raise: Error.
        r complete.
        self assert: r status = #completed
    ]

    "-------------------------------------------- favorites/matching"

    testFavoritesFirstThenTopRated [
        | app low high rider m1 m2 m3 |
        app := RideSharingSystem new.
        low := app addDriverId: 'D1' name: 'Low' rating: 4.50 count: 10.
        high := app addDriverId: 'D2' name: 'High' rating: 4.99 count: 10.
        rider := app addRiderId: 'U1' name: 'Rider'.
        rider addFavoriteDriver: 'D1'.

        m1 := app bookRide: (StandardRide id: 'R1' from: 'x' to: 'y' miles: 2.0) for: rider.
        self assert: m1 driver == low.
        self assert: m1 viaFavorite.

        "Favorite is busy now, so fall back to the highest-rated driver."
        m2 := app bookRide: (StandardRide id: 'R2' from: 'x' to: 'y' miles: 2.0) for: rider.
        self assert: m2 driver == high.
        self deny: m2 viaFavorite.

        "Nobody left."
        m3 := app bookRide: (StandardRide id: 'R3' from: 'x' to: 'y' miles: 2.0) for: rider.
        self deny: m3 found.

        "Finishing a ride frees the favorite again."
        app completeRide: 'R1' stars: 5.
        self assert: low isAvailable.
        self assert: (self cents: low totalEarnings) = 700
    ]

    testFavoritesHaveNoDuplicatesAndSupportRemoval [
        | r |
        r := Rider id: 'U1' name: 'Rider'.
        r addFavoriteDriver: 'D1'; addFavoriteDriver: 'D1'; addFavoriteDriver: 'D2'.
        self assert: r favoriteDrivers asArray = #('D1' 'D2').
        r removeFavoriteDriver: 'D1'.
        self deny: (r isFavorite: 'D1').
        self assert: (r isFavorite: 'D2')
    ]
]
