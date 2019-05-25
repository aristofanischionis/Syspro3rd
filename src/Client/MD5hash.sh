#!/bin/bash
# calculate md5 hash of a file, looking at its contents
# outputs a file with the same name and extension .md

if [[ $# -ne 1 ]] ; then
    echo 'the number of args provided is' $#
    exit 1
fi

file=$1
# change to: md5sum when in linux
md5 ${file} > ${file}.md5