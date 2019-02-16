#!/usr/bin/python
# -*- coding: utf-8 -*-

from sys import stdin, stdout

library = {}

while True:

    line = stdin.readline().strip()
    if line == "":
        break

    word, cnt = line.split('\t')

    if word in library:
        library[word] += int(cnt)
    else:
        library[word] = int(cnt) 


for word in library:
    stdout.write(word + "\t" + str(library[word]) + "\n")