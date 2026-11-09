#!/usr/bin/env bash
# Run the Smalltalk demo. Requires GNU Smalltalk: brew install gnu-smalltalk
cd "$(dirname "$0")"
gst src/Extensions.st src/Ride.st src/RideTypes.st src/Driver.st \
    src/Rider.st src/RideSharingSystem.st demo.st
