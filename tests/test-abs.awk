#!/usr/bin/awk -f

function abs(x) {
    return (x < 0) ? -x : x
}

BEGIN{
    expr = "'abs(0)+abs(-0.5)+abs(10)+abs(-300)'"
    "../panel-plugin/calctest " expr | getline res
    if (abs(res - 310.5) > 1.0e-14) {
        print res
        exit 1
    } else
        exit 0
}
