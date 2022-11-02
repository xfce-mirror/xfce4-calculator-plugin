#!/usr/bin/awk -f

function abs(x) {
    return (x < 0) ? -x : x
}

BEGIN{
    expr = "'sqrt(0.0001524157875019052)+sqrt(75/3)+10*sqrt(1)+100*sqrt(0--0)'"
    ("../panel-plugin/calctest " expr) | getline res
    if (abs(res - 15.0123456789) > 1.0e-14) {
        print res
        exit 1
    } else
        exit 0
}
