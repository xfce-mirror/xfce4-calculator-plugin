#!/usr/bin/awk -f

BEGIN{
    expr = "'log10(-1)'"
    "../panel-plugin/calctest " expr | getline res
    if ((tolower(res) != "nan") && (tolower(res) != "-nan")) {
        print res
        exit 1
    } else
        exit 0
}
