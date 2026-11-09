"Driver.st - a driver and the rides they have been assigned.

 Encapsulation: assignedRides has no accessor. Other objects add rides with
 #addRide: and read them with #rides, which answers a copy, so changing the
 copy never changes the driver's own list. The rating only changes through
 #addRating:, which checks the 1-5 star range."

Object subclass: Driver [
    | driverID name ratingTotal ratingCount assignedRides |

    Driver class >> id: anID name: aName [
        ^self id: anID name: aName rating: 5 count: 0
    ]

    Driver class >> id: anID name: aName rating: startingRating count: startingCount [
        ^self new setId: anID name: aName rating: startingRating count: startingCount
    ]

    setId: anID name: aName rating: startingRating count: startingCount [
        <category: 'private'>
        driverID := anID.
        name := aName.
        ratingTotal := startingRating * startingCount.
        ratingCount := startingCount.
        assignedRides := OrderedCollection new
    ]

    "----------------------------------------------------- rides"

    addRide: aRide [
        aRide assignDriverId: driverID name: name.
        assignedRides add: aRide.
        ^aRide
    ]

    completeRide: aRideID [
        | ride |
        ride := assignedRides detect: [:r | r rideID = aRideID]
                    ifNone: [^self error: name, ' has no ride ', aRideID].
        ride complete
    ]

    rides [ ^assignedRides copy ]

    rideCount [ ^assignedRides size ]

    isAvailable [
        "Available when no ride is currently in progress."
        ^(assignedRides detect: [:r | r status = #assigned] ifNone: [nil]) isNil
    ]

    totalEarnings [
        ^(assignedRides select: [:r | r status = #completed])
            inject: 0 into: [:sum :r | sum + r lockedFare]
    ]

    "--------------------------------------------------- ratings"

    addRating: stars [
        (stars between: 1 and: 5) ifFalse: [
            ^SystemExceptions.ArgumentOutOfRange signalOn: stars
                mustBeBetween: 1 and: 5].
        ratingTotal := ratingTotal + stars.
        ratingCount := ratingCount + 1
    ]

    rating [ ^ratingCount = 0 ifTrue: [5] ifFalse: [ratingTotal / ratingCount] ]

    ratingCount [ ^ratingCount ]

    "---------------------------------------------------- display"

    getDriverInfo [
        ^'Driver ', driverID, ': ', name,
            ' | rating ', (self rating asFixed: 2),
            ' (', ratingCount printString, ' ratings)',
            ' | rides: ', assignedRides size printString,
            ' | earned ', self totalEarnings asMoney,
            ' | ', (self isAvailable ifTrue: ['available'] ifFalse: ['on a trip'])
    ]

    driverID [ ^driverID ]
    name [ ^name ]
]
