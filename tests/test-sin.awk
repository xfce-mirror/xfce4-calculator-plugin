#!/usr/bin/awk -f

function abs(x) {
    return (x < 0) ? -x : x
}

BEGIN{
    expr = "'sin(pi/2)'"
    ("../panel-plugin/calctest " expr) | getline res
    if (abs(res - 1) > 1.0e-14) {
        print res
        exit 1
    } else
        exit 0
}
