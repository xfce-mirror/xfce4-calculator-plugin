#!/usr/bin/awk -f

function abs(x) {
    return (x < 0) ? -x : x
}

BEGIN{
    expr = "'log2(2) + 100*log2(1) + log2(0.00390625) + log2(4294967296)'"
    ("../panel-plugin/calctest " expr) | getline res
    if (abs(res - 25) > 1.0e-14) {
        print res
        exit 1
    } else
        exit 0
}
