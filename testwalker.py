#!/usr/bin/env python
'''Checks out all commits up to a given destination and runs tests on each commit.

   This script may be useful after rebasing.
'''


import sys, os
from subprocess import check_output, check_call


def call_output(command):
    '''Run a command and return the standard output as a string'''
    return check_output(command, shell=True)


def call(command):
    '''Run a command'''
    return check_call(command, shell=True)


def get_commit_sha(name=None):
    '''Get the hash of the current commit or the given commit'''
    if name is None:
        name = ''
    result = call_output('git log %s -1 --pretty=oneline --color=never' % name)
    return result.split()[0]


def checkout(name):
    '''Checkout a commit'''
    call('git checkout %s' % name)


def main():
    args = sys.argv[1:]
    if len(args) != 1:
        raise ValueError('Expecting one argument, e.g. master')

    start = call_output('git rev-parse --abbrev-ref HEAD')
    destination = args[0]
    destination_sha = get_commit_sha(destination)
    while get_commit_sha() != destination_sha:
        # keep going until we reach the destination hash
        print get_commit_sha()
        # All the commands that should work after committing.
        call('./cleanfiles.sh')
        call('./setup.py build_ext -i')
        call('cd doc; make html')
        call('nosetests -v')
        # Go back one commit
        checkout('HEAD^')

    # Go back to the original commit
    checkout(start)


if __name__ == '__main__':
    main()
