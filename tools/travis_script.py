#!/usr/bin/env python

import os

from common_py.system.filesystem import FileSystem as fs
from common_py.system.executor import Executor as ex
from common_py.system.platform import Platform
from check_tidy import check_tidy

platform = Platform()

DOCKER_ROOT_PATH = fs.join('/root')

# ShadowNode path in travis
TRAVIS_BUILD_PATH = fs.join(os.environ['TRAVIS_BUILD_DIR'])

# ShadowNode path in docker
DOCKER_SHADOW_NODE_PATH = fs.join(DOCKER_ROOT_PATH, 'workspace/shadow-node')
DOCKER_NAME = 'shadow_node_docker'
BUILDTYPES = ['debug', 'release']

# Common buildoptions for sanitizer jobs.
BUILDOPTIONS_SANITIZER = [
    '--buildtype=debug',
    '--clean',
    '--compile-flag=-fno-common',
    '--compile-flag=-fno-omit-frame-pointer',
    '--jerry-cmake-param=-DFEATURE_SYSTEM_ALLOCATOR=ON',
    '--jerry-cmake-param=-DJERRY_LIBC=OFF',
    '--no-snapshot',
    '--profile=test/profiles/host-linux.profile',
    '--run-test=full',
    '--target-arch=i686'
]

def run_docker():
    ex.check_run_cmd('docker', ['run', '-dit', '--privileged',
                     '--name', DOCKER_NAME, '-v',
                     '%s:%s' % (TRAVIS_BUILD_PATH, DOCKER_SHADOW_NODE_PATH),
                     '--add-host', 'test.mosquitto.org:127.0.0.1',
                     'iotjs/ubuntu:0.8'])

def exec_docker(cwd, cmd, env=[]):
    exec_cmd = 'cd %s && ' % cwd + ' '.join(cmd)
    docker_args = ['exec', '-it']
    for e in env:
        docker_args.append('-e')
        docker_args.append(e)

    docker_args += [DOCKER_NAME, 'bash', '-c', exec_cmd]
    ex.check_run_cmd('docker', docker_args)

def start_mosquitto_server():
    exec_docker(DOCKER_ROOT_PATH, ['mosquitto', '-d'])

def build_jerry():
    exec_docker(DOCKER_SHADOW_NODE_PATH, [
                './deps/jerry/tools/run-tests.py',
                '--unittests'], [])

def build_iotjs(buildtype, args=[], env=[]):
    exec_docker(DOCKER_SHADOW_NODE_PATH, [
                './tools/build.py',
                '--clean',
                '--buildtype=' + buildtype] + args, env)

if __name__ == '__main__':
    if os.getenv('RUN_DOCKER') == 'yes':
        run_docker()
        # install dbus and zlib
        exec_docker('/', ['apt-get', 'install', '-q', '-y',
                    'zlib1g-dev',
                    'dbus',
                    'libdbus-1-dev'], [])
        start_mosquitto_server()

    test = os.getenv('OPTS')
    if test == 'host-linux':
        build_jerry()
        for buildtype in BUILDTYPES:
            build_iotjs(buildtype, [
                        '--cmake-param=-DENABLE_MODULE_ASSERT=ON',
                        '--tests'])

    elif test == "host-darwin":
        for buildtype in BUILDTYPES:
            ex.check_run_cmd('./tools/build.py', [
                            '--tests',
                            '--no-check-valgrind',
                            '--buildtype=' + buildtype,
                            '--clean',
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
                        '--tests',
                        '--no-check-valgrind',
                        '--no-snapshot',
                        '--jerry-lto'])


    elif test == "coverity":
        ex.check_run_cmd('./tools/build.py', ['--clean'])
