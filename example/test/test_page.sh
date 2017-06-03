#!/bin/bash

function make_page
{
    echo '# <div align = center>Test</div>'
    echo
    echo '```'
    git log -1 | cat
    echo '```'
    echo
    
    echo '```'
    date
    ./test.sh
    echo '```'
    echo
}

make_page
