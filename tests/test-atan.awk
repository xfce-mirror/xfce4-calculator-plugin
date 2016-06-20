#!/usr/bin/awk -f

function abs(x) {
    return (x < 0) ? -x : x
}

BEGIN{
    expr = "'pi/atan(-sqrt(3)) + pi/arctan(-sqrt(3))'"
    "../panel-plugin/calctest " expr | getline res
    if (abs(res - -6) > 1.0e-14) {
        print res
        exit 1
    } else
        exit 0
}
