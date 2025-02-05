#!/usr/bin/awk -f

function abs(x) {
    return (x < 0) ? -x : x
}

BEGIN{
    expr = "'3e3 + -2.3e+2 + 6e-2 + -2e-3 + 1e0 + 1e+0 + 1e-0'"
    ("calctest " expr) | getline res
    if (abs(res - 2773.058) > 1.0e-15) {
        print res
        exit 1
    } else
        exit 0
}
