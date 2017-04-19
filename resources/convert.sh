#!/bin/sh

OUT=../app/src/main/res
ITEMS=$(ls *.svg)

mkdir -p ${OUT}/drawable-hdpi
mkdir -p ${OUT}/drawable-mdpi
mkdir -p ${OUT}/drawable-xhdpi
mkdir -p ${OUT}/drawable-xxhdpi

for i in $ITEMS ; do
    filename=$(basename $i)
    n=$(echo $filename | cut -f 1 -d '.')
    inkscape -e ${OUT}/drawable-hdpi/${n%.svg}.png -w 48 -h 48 $i
    inkscape -e ${OUT}/drawable-mdpi/${n%.svg}.png -w 32 -h 32 $i
    inkscape -e ${OUT}/drawable-xhdpi/${n%.svg}.png -w 64 -h 64 $i
    inkscape -e ${OUT}/drawable-xxhdpi/${n%.svg}.png -w 96 -h 96 $i
done

inkscape -e ic_launcher.png -w 512 -h 512 ic_launcher.svg
