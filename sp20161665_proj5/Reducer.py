#!/usr/bin/python
# -*- coding: utf-8 -*-

from sys import stdin, stdout

D=dict()

while True:

    line = stdin.readline().strip()
    if not line: break

    word, num = line.split('\t')

    if word in D:
        D[word] += int(num)
    else:
        D[word] = int(num) 


for word in sorted(D):
    stdout.write(word + "\t" + str(D[word]) + "\n")
