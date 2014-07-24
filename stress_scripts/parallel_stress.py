#!/usr/bin/env python

"""
The main bottleneck in these stress scripts - believe it or not - is in unicode text encoding! The idea here was to somehow parallelize this process. Never really implemented it.

This doesn't really work.
"""

from __future__ import (absolute_import, division, print_function,
                        unicode_literals)
import os
import pip
import sys
import subprocess
import threading
import argparse

class stressObject(threading.Thread):
    def __init__(self):
        threading.Thread.__init__(self)
    def run(self):
        populate()
    

def populate():
    failed = []
    cmd = "C:\\Users\\Alexander Riccio\\Documents\\GitHub\\altWinDirStat\\populateDirWithSingleFiletype.py"
    exit_status = subprocess.check_output(cmd)
    return exit_status

def threadsFunc():
    threads = []
    [threads.append(stressObject()) for _ in range(8)]
    for thread in threads:
        thread.start()

def main():
    threadsFunc()
    
if __name__ == '__main__':
    main()
