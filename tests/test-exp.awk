#!/usr/bin/awk -f

function abs(x) {
    return (x < 0) ? -x : x
}

BEGIN{
    expr = "'exp(0) + exp(1) + exp(0.2) + exp(5)'"
    "../panel-plugin/calctest " expr | getline res
    if (abs(res - 153.3528436891958185) > 1.0e-14) {
        print res
        exit 1
    } else
        exit 0
}
