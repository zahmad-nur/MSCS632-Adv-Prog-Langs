"Extensions.st - small formatting helpers added to built-in classes.

 Smalltalk classes are open: we can add methods to Number and String
 without subclassing them. These keep the output identical to the C++
 version ($11.60, 5.2 mi, aligned columns)."

Number extend [
    asFixed: places [
        "Answer a string with exactly places digits after the decimal point."
        | scale scaled stream |
        scale := 10 raisedTo: places.
        scaled := (self * scale) rounded.
        stream := WriteStream on: String new.
        scaled < 0 ifTrue: [stream nextPut: $-. scaled := scaled negated].
        stream print: scaled // scale.
        places > 0 ifTrue: [
            stream nextPut: $.;
                nextPutAll: ((scaled \\ scale) printPaddedWith: $0 to: places)].
        ^stream contents
    ]

    asMoney [ ^'$', (self asFixed: 2) ]

    asMiles [ ^(self asFixed: 1), ' mi' ]
]

String extend [
    paddedTo: width [
        "Left-justify the receiver in a field of the given width."
        ^self size >= width
            ifTrue: [self]
            ifFalse: [self, (String new: width - self size withAll: $ )]
    ]

    rightAlignedTo: width [
        ^self size >= width
            ifTrue: [self]
            ifFalse: [(String new: width - self size withAll: $ ), self]
    ]
]
