#!/usr/bin/awk -f

BEGIN{
    expr = "'log10(0)'"
    ("calctest " expr) | getline res
    if (tolower(res) != "-inf") {
        print res
        exit 1
    } else
        exit 0
}
