#!/usr/bin/awk -f

BEGIN{
    expr = "'sqrt(-1)'"
    ("calctest " expr) | getline res
    if ((tolower(res) != "nan") && (tolower(res) != "-nan")) {
        print res
        exit 1
    } else
        exit 0
}
