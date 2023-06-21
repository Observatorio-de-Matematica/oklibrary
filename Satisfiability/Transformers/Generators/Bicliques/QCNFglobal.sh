#!/usr/bin/env bash
# Niyozbek Obidov, 20.6.2023 (Swansea)
# Copyright 2023 Oliver Kullmann
# This file is part of the OKlibrary. OKlibrary is free software; you can redistribute
# it and/or modify it under the terms of the GNU General Public License as published by
# the Free Software Foundation and included in this library; either version 3 of the
# License, or any later version. */


echo "Please enter the directory of files"
read directory

echo "Please enter type of files to process"
read filetype

echo "Please enter the maximum number of files to process:"
read maxfiles

FILES=$(find -L $directory -name "*$filetype")
totalFiles=$(echo "$FILES" | wc -l)
echo "Total input files: $totalFiles"
globalVariables=0
filesCounter=0

for path in $FILES
do
	# echo "Processing $path"
    verdict=$(./QCNFglobal $path)
    (( filesCounter++ ))
    if [[ -z $verdict ]]
        then 
            # echo "Global variable!"
            (( globalVariables++ ))
        # else 
            # echo "Oh not a global variable!"
    fi

    if [[ $filesCounter -eq $maxfiles ]]
        then
            break
        fi
done

echo "Input files with global variables: $globalVariables"
