"Rider.st - a rider, their ride history and their favorite drivers.

 Favorites are an ordered list of driver IDs (first = most preferred).
 RideSharingSystem checks it before falling back to the highest-rated
 available driver."

Object subclass: Rider [
    | riderID name requestedRides favoriteDrivers |

    Rider class >> id: anID name: aName [ ^self new setId: anID name: aName ]

    setId: anID name: aName [
        <category: 'private'>
        riderID := anID.
        name := aName.
        requestedRides := OrderedCollection new.
        favoriteDrivers := OrderedCollection new
    ]

    "----------------------------------------------------- rides"

    requestRide: aRide [
        aRide lockInFare.   "upfront price from the subclass's #fare"
        requestedRides add: aRide.
        ^aRide
    ]

    viewRides [
        | s |
        s := WriteStream on: String new.
        s nextPutAll: 'Ride history for ', name, ' (', riderID, '):'; nl.
        requestedRides isEmpty ifTrue: [s nextPutAll: '  (no rides yet)'; nl].
        requestedRides do: [:ride |
            s nextPutAll: '  '; nextPutAll: ride rideDetails; nl].  "polymorphic"
        s nextPutAll: '  Total spent: ', self totalSpent asMoney.
        ^s contents
    ]

    rides [ ^requestedRides copy ]

    totalSpent [ ^requestedRides inject: 0 into: [:sum :r | sum + r lockedFare] ]

    "------------------------------------------------- favorites"

    addFavoriteDriver: aDriverID [
        (self isFavorite: aDriverID) ifFalse: [favoriteDrivers add: aDriverID]
    ]

    removeFavoriteDriver: aDriverID [
        favoriteDrivers remove: aDriverID ifAbsent: [nil]
    ]

    isFavorite: aDriverID [ ^favoriteDrivers includes: aDriverID ]

    favoriteDrivers [ ^favoriteDrivers copy ]

    riderID [ ^riderID ]
    name [ ^name ]
]
