#!/bin/bash

repoDir="/home/cluc/eecs440/ApeSearch/query-constraint/bin"

pages="${repoDir}/pages.txt"
index="${repoDir}/index.txt"
parser="${repoDir}/parser"

>$index

cat $pages | while read url 
do
    output="${repoDir}/tmp.html"
    curl -o $output $url

    echo "${url}">>${index}
    echo -e "\r\n">>${index}
    $parser "t" $output >> ${index}
    echo -e "\r\n">>${index}
    $parser "w" $output >> ${index}
    echo -e "\r\n">>${index}
    $parser "a" $output >> ${index}
    echo -e "\r\n">>${index}
    $parser "b" $output >> ${index}
    echo -e "\r\n\r\n">>${index}

    rm $output
done