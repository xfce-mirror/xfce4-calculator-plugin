#!/usr/bin/awk -f

function abs(x) {
    return (x < 0) ? -x : x
}

BEGIN{
    expr = "'-cbrt(-8) + 10*cbrt(8) + 100*cbrt(0) + cbrt(0.729)'"
    ("../panel-plugin/calctest " expr) | getline res
    if (abs(res - 22.9) > 1.0e-14) {
        print res
        exit 1
    } else
        exit 0
}
