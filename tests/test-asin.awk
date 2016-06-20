#!/usr/bin/awk -f

function abs(x) {
    return (x < 0) ? -x : x
}

BEGIN{
    expr = "'pi/asin(-1) + pi/arcsin(-1)'"
    "../panel-plugin/calctest " expr | getline res
    if (abs(res - -4) > 1.0e-14) {
        print res
        exit 1
    } else
        exit 0
}
