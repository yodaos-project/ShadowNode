#!/usr/bin/env python

#convert jerry cpu profiler file into collapse file, which is input format of flamegraph.pl
import sys
import re

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

debug_info = {}

debug_info_file = open(sys.argv[2])
for line in debug_info_file:
    line = line.strip('\n');
    if line.endswith(':'):
        file_name = line.strip(':')
    else:
        m = re.match(r'(\+ ([a-zA-Z0-9_]*))? \[(\d+),(\d+)\] (\d+)',line)
        if(m):
            func_name = m.group(2) if m.group(2) else '<anonymous>'
            debug_info[m.group(5)] = func_name + '@' + file_name + '(' + m.group(3) + '.' + m.group(4) + ')'

for key in stack_time.keys():
    stack = key.split(',')
    stack.reverse()
    display_stack = map(lambda x: debug_info[x] if debug_info.has_key(x) else '<anonymous>', stack)
    print ";".join(display_stack) + ' ' + str(stack_time[key])

