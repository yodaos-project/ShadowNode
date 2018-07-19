#!/usr/bin/env python

"""
convert jerry cpu profiler file into collapse file,
which is input format of flamegraph.pl
"""

import sys
import re

# input format
# eachline is a call stack info
# time, bytecode_1,..., bytecode_n-1, bytecode_n
# bytecode_n call bytecode_n-1, ... call bytecode_1
# time is top frame bytecode_n's time

PERF_FILE = open(sys.argv[1])

STACK_TIME = {}

LINES = PERF_FILE.readlines()

# Merge all same call stack and put into hashmap
for line in LINES:
    current_line = line.strip('\n')
    stack_line = current_line.split(',', 1)
    if STACK_TIME.has_key(stack_line[1]):
        time = STACK_TIME[stack_line[1]]
    else:
        time = 0.0
    time += float(stack_line[0])
    STACK_TIME[stack_line[1]] = time

# For each call stack, subtract all call stack whose depth is just 1 smaller.
# The resut is self time.
for line, time in STACK_TIME.items():
    for line1, time1 in STACK_TIME.items():
        if line1.endswith(line):
            words = line.split(',')
            words1 = line1.split(',')
            if len(words) + 1 == len(words1):
                time -= time1
    STACK_TIME[line] = time

# parse debug info dump file
DEBUG_INFO = {}
DEBUG_INFO_FILE = open(sys.argv[2])
for line in DEBUG_INFO_FILE:
    line = line.strip('\n')
    if line.endswith(':'):
        file_name = line.strip(':')
    else:
        m = re.match(r'(\+ ([a-zA-Z0-9_]*))?( )*(\[(\d+),(\d+)\])?( )*(\d+)', line)
        # m.group(1) (\+ ([a-zA-Z0-9_]*))
        # m.group(2) ([a-zA-Z0-9_]*)
        # m.group(3) ( )
        # m.group(4) (\[(\d+),(\d+)\])
        # m.group(5) (\d+)
        # m.group(6) (\d+)
        # m.group(7) ( )
        # m.group(8) (\d+)
        if m:
            func_name = m.group(2) if m.group(2) else m.group(8)
            lineinfo = '(' + m.group(5) + ':' + m.group(6) + ')' if m.group(4) else '()'
            DEBUG_INFO[m.group(8)] = func_name + '@' + file_name + lineinfo

# map call stack into human readable
for line, time in STACK_TIME.items():
    stack = line.split(',')
    stack.reverse()
    display_stack = [DEBUG_INFO[x] if DEBUG_INFO.has_key(x) else x \
                    for x in stack]
    print ";".join(display_stack) + ' ' + str(STACK_TIME[line])
