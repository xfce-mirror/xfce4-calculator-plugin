#!/usr/bin/awk -f

function abs(x) {
    return (x < 0) ? -x : x
}

BEGIN{
    expr = "'0x0 + 0xabcdef + 0xFEDCBA - 0x7e9'"
    ("../panel-plugin/calctest " expr) | getline res
    if (abs(res - 27960000) > 1.0e-14) {
        print res
        exit 1
    } else
        exit 0
}
