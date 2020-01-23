#!/bin/sh
# encoding=utf-8
# author: xiaoke huang
# date: 2019/12/14


file_in_dir="testfile"
file_out_dir="testout"

if [ ! -d "./$file_in_dir/" ]; then
    echo $file_in_dir;
    mkdir $file_in_dir
fi

if [ ! -d "./$file_out_dir/" ]; then
    mkdir $file_out_dir
fi

# test each c file in $file_in_dir, then output to $file_out_dir
for i in `ls $file_in_dir`
do
# echo "`pwd`/$i"
./compiler < "./$file_in_dir/$i" > "./$file_out_dir/$i.out"
done