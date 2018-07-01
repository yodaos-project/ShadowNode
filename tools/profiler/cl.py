#!/usr/bin/env python

#convert jerry cpu profiler file into collapse file, which is input format of flamegraph.pl
import sys

f = open(sys.argv[1])

stack_time = {}

lines = f.readlines()

for i in range(len(lines)):
    current_line = lines[i].strip('\n')
    l = current_line.split(',',1)
    if stack_time.has_key(l[1]):
        time = stack_time[l[1]]
    else:
        time = 0.0
    time += float(l[0])
    stack_time[l[1]] = time

for key in stack_time.keys():
    time = stack_time[key]
    for key1 in stack_time.keys():
        if key1.endswith(key) and len(key) != len(key1):
            time -= stack_time[key1]
    stack_time[key] = time

for key in stack_time.keys():
    stack = key.split(',')
    stack.reverse();
    print ";".join(stack) + ' ' + str(stack_time[key])

