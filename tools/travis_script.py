#!/usr/bin/env python

import os
import commands

from common_py.system.executor import Executor as ex


BUILDTYPES = ['debug', 'release']


def build_jerry():
    ex.check_run_cmd('./deps/jerry/tools/run-tests.py',
                     ['--unittests', '--jerry-test-suite'])


def build_iotjs(buildtype, args=[], env=[]):
    ex.check_run_cmd('./tools/build.py',
                     ['--clean', '--buildtype=' + buildtype] + args, env)


if __name__ == '__main__':
    test = os.getenv('OPTS')
    if test == 'host-linux':
        # run jerry test only on demand
        commit_range = os.getenv('TRAVIS_COMMIT_RANGE').partition('...')
        commit_head = commit_range[0]
        commit_base = commit_range[2]
        find_cmd = 'git diff ' + \
                   commit_head + ' ' + commit_base + \
                   ' | grep \"deps/jerry\"'
        return_code, find_output = commands.getstatusoutput(find_cmd)
        if find_output:
            build_jerry()
        for buildtype in BUILDTYPES:
            build_iotjs(buildtype, [
                '--run-test=full',
                '--no-check-valgrind'])

    elif test == "host-darwin":
        for buildtype in BUILDTYPES:
            build_iotjs(buildtype, [
                '--run-test=full',
                '--no-check-valgrind',
                '--profile=test/profiles/host-darwin.profile'])

    elif test == 'rpi2':
        for buildtype in BUILDTYPES:
            build_iotjs(buildtype, [
                        '--target-arch=arm',
                        '--target-board=rpi2',
                        '--profile=test/profiles/rpi2-linux.profile'])

    elif test == "no-snapshot":
        for buildtype in BUILDTYPES:
            build_iotjs(buildtype, [
                        '--run-test=full',
                        '--no-check-valgrind',
                        '--no-snapshot',
                        '--jerry-lto'])

    elif test == 'napi':
        for buildtype in BUILDTYPES:
            build_iotjs(buildtype, [
                '--run-test=full',
                '--no-check-valgrind',
                '--napi'])

    elif test == "coverity":
        ex.check_run_cmd('./tools/build.py', ['--clean'])
