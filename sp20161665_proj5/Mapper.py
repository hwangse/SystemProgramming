#!/usr/bin/python
# -*- coding: utf-8 -*-

from sys import stdin, stdout

while True:
    
    line = stdin.readline().strip()
    
    if not line : break
    
    L = line.split(' ')
    if len(L) < 2 : 
        continue
    for i in range(len(L)-1):
        newStr=L[i]+' '+L[i+1]
        stdout.write(newStr + "\t1\n")
