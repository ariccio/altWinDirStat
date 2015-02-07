altWinDirStat
=============

An unofficial modification of WinDirStat. Tremendous performance improvements.

This repository is used to be an ugly, hacky, bundle of crap - but now it's just a bundle of crap. 

A quick guide to the structure of this repository:

* Reference Code
  * Code that I referenced/studied early in development
  * None of it compiles as part of altWinDirStat
* WinDirStat
  * My branch, this contains the Visual Studio 2013 `.sln` file
  * *This is where the source code is!*
  * Has it's own, more detailed `README.md`
* Development Screenshots
  * Interesting things I saw while working on altWinDirStat
* filesystem-docs-n-stuff
  * All sorts of information on NTFS and NTFS internals
  * LOADS of good stuff in here!
  * Also has mirrors of any documentation that I mention in the source code
* stress-progs
  * A native application that I've built to stress test WinDirStat by creating an arbitrary number of randomly named files
  * Has it's own `.sln` file, and is developed concurrently (albeit sporadically)
* stress-scripts
  * A naive version of the aforementioned stress testing utility, written in Python
  * Turned out to be extremely slow, caused by a massive text-encoding bottleneck in Python
* *(many other files, not yet sorted)*
