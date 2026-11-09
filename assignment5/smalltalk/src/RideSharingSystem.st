"RideSharingSystem.st - ties riders, drivers and rides together.

 Matching rule: check the rider's favorite drivers first (in the rider's
 order of preference); if none is available, assign the highest-rated
 available driver."

Object subclass: RideMatch [
    | driver viaFavorite |

    RideMatch class >> driver: aDriver viaFavorite: aBoolean [
        ^self new setDriver: aDriver viaFavorite: aBoolean
    ]

    setDriver: aDriver viaFavorite: aBoolean [
        <category: 'private'>
        driver := aDriver.
        viaFavorite := aBoolean
    ]

    driver [ ^driver ]
    viaFavorite [ ^viaFavorite ]
    found [ ^driver notNil ]
]

Object subclass: RideSharingSystem [
    | drivers riders rides |

    RideSharingSystem class >> new [ ^super new init ]

    init [
        <category: 'private'>
        drivers := OrderedCollection new.
        riders := OrderedCollection new.
        rides := OrderedCollection new
    ]

    addDriverId: anID name: aName rating: aRating count: aCount [
        ^drivers add: (Driver id: anID name: aName rating: aRating count: aCount)
    ]

    addRiderId: anID name: aName [
        ^riders add: (Rider id: anID name: aName)
    ]

    findDriver: aDriverID [
        ^drivers detect: [:d | d driverID = aDriverID] ifNone: [nil]
    ]

    findDriverFor: aRider [
        <category: 'private'>
        | best |
        "1) Favorites first, in the rider's order of preference."
        aRider favoriteDrivers do: [:favID |
            | d |
            d := self findDriver: favID.
            (d notNil and: [d isAvailable])
                ifTrue: [^RideMatch driver: d viaFavorite: true]].
        "2) Otherwise the highest-rated available driver."
        drivers do: [:d |
            (d isAvailable and: [best isNil or: [d rating > best rating]])
                ifTrue: [best := d]].
        ^RideMatch driver: best viaFavorite: false
    ]

    bookRide: aRide for: aRider [
        "Rider requests the ride (fare locked in), then a driver is matched."
        | match |
        aRider requestRide: aRide.
        rides add: aRide.
        match := self findDriverFor: aRider.
        match found ifTrue: [match driver addRide: aRide].
        ^match
    ]

    completeRide: aRideID stars: stars [
        | ride driver |
        ride := rides detect: [:r | r rideID = aRideID]
                    ifNone: [^self error: 'unknown ride ', aRideID].
        driver := self findDriver: ride driverID.
        driver isNil ifTrue: [^self error: 'ride ', aRideID, ' has no driver'].
        driver completeRide: aRideID.
        driver addRating: stars
    ]

    allRides [ ^rides copy ]
    drivers [ ^drivers copy ]
]
