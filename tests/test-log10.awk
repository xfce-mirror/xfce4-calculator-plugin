#!/usr/bin/awk -f

function abs(x) {
    return (x < 0) ? -x : x
}

BEGIN{
    expr = "'log10(100000)+lg(1e20)'"
    ("calctest " expr) | getline res
    if (abs(res - 25) > 1.0e-14) {
        print res
        exit 1
    } else
        exit 0
}
