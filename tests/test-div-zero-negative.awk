#!/usr/bin/awk -f

BEGIN{
    expr = "'-1/0'"
    ("../panel-plugin/calctest " expr) | getline res
    if (tolower(res) != "-inf") {
        print res
        exit 1
    } else
        exit 0
}
