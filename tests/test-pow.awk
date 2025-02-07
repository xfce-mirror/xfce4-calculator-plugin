#!/usr/bin/awk -f

function abs(x) {
    return (x < 0) ? -x : x
}

BEGIN{
    "calctest '4^3**2'" | getline res
    if (abs(res - 4096) > 1.0e-15) {
        print res
        exit 1
    } else
        exit 0
}
