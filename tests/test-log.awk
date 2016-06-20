#!/usr/bin/awk -f

function abs(x) {
    return (x < 0) ? -x : x
}

BEGIN{
    expr = "'log(2.71828182845904523536) + 100*ln(1) + log(5e-10) + ln(5e10)'"
    "../panel-plugin/calctest " expr | getline res
    if (abs(res - 4.21887582486820074478057) > 1.0e-14) {
        print res
        exit 1
    } else
        exit 0
}
