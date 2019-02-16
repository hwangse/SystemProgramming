#!/usr/bin/python
# -*- coding: utf-8 -*-

from sys import stdin, stdout

while True:
    
    line = stdin.readline().strip()
    
    if line == "":
        break
    
    words = line.split(' ')
    for i in words:
        stdout.write(i + "\t1\n")