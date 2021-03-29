#!/bin/bash

repoDir="/home/cluc/eecs440/ApeSearch/query-constraint"
pages="${repoDir}/bin/pages.txt"
index="${repoDir}/bin/index.txt"
parser="${repoDir}/bin/parser"

>$index

cat $pages | while read url 
do
    output="${repoDir}/bin/tmp.html"
    curl -o $output $url

    echo "${url}">>"${index}"
    echo "">>${index}
    $parser "t" $output
    $parser "w" $output
    $parser "a" $output
    $parser "b" $output
    echo "">>${index}
    echo "">>${index}

    rm $output
done