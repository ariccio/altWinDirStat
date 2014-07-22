#!/usr/bin/env python

"""
Update all the packages (in alphabetical order)
that you have installed globally with pip
(i.e. with `sudo pip install`).

Created by:
    Alexander Riccio (alexander@riccio.com)

Inspired by:
    http://pythonadventures.wordpress.com/2013/05/22/update-all-pip-packages/
    Jabba Laci, 2013--2014 (jabba.laci@gmail.com)
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
