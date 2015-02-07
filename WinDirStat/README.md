# PART OF THIS IS OUT OF DATE, AND WRONG. TODO: FIX

#### (alt)WinDirStat - Windows Directory Statistics
#### (c) 2003-2005 Bernhard Seifert (bseifert@users.sourceforge.net)
#### (c) 2014-2015 Alexander Riccio (alexander@riccio.com)

WinDirStat is a disk usage statistics viewer and cleanup tool for Windows (Vista & up). It shows disk, file and directory sizes in a treelist as well as graphically in a treemap, much like KDirStat and Sequoiaview.

altWinDirStat is:

- the program
  - windirstat.exe
and that's it!

I removed the [i18n](http://en.wikipedia.org/wiki/Internationalization_and_localization) support in vanilla WinDirStat, but I'd like to reintroduce it. The original system, using localized resource DLLs, was rather frustrating, in that it:

1. Required dynamic linking, and thus denied me the possibility of a single, standalone executable
2. Required the use of CString/CStringT (and thus heap/dynamic allocation)
3. Adds a layer of dynamic indirection, which impedes static analysis tools, and developer reasoning. Impeding static analysis and reasoning leads to [hard to spot bugs](https://github.com/ariccio/altWinDirStat/commit/639fefc715d094c6ca7c1770f2d991d836421ada#diff-f897a88848213c562f1eceba205b7af9L773), which **require** testing for discovery. I think that's silly, I think that relying of software testing is like attempting to prove Fermat's Last Theorem, as Prof. [John Conway describes it beginning at 7:30](http://vimeo.com/18216532), by testing all possible numbers - it's never going to work. **I place great importance on the clarity and analyzability of code**

I'd eventually like to reintroduce some form of i18n, albeit more cleanly. Currently, I've moved many important strings to a single namespace, `global_strings`, in `datastructures.h`. I'll use some combination of conditional compilation and possibly some `using`s, à la `using type_str = English::type_str;`, to support different languages.

I'm not yet sure exactly how to support asiatic/arabic/logographic/non-sequential/featural/alien scripts. It's an interesting problem.

is distributed under the terms of the GPL v2 (executables+source) respectively GNU FDL (help files).

See windirstat/res/license.txt and wdshelp/gnufdl.htm.

The tree-GIF was found in http://www.world-in-motion.de - I hope, I don't violate any copyright (tell me).

=============================================================================
If you make any changes or enhancements, find bugs or have feature requests, create an issue in [the issues section](https://github.com/ariccio/altWinDirStat/issues). 

Note, however, that not all proposed features will be taken into account as we want to keep WinDirStat what it is: a tool to view disk usage statistics, simple and intuitive. *Which is funny, because the vanilla version of WinDirStat had everything from the ability to send email, to a mini scripting language.*


=============================================================================
altWinDirStat is an application written in Visual C++ Visual Studio 2013, and (whatever the latest version of) the Windows Template Libraries.

It runs on Windows (Vista & up).

It shows [what's consuming space](http://xkcd.com/1360/) on your hard drive, in an intuitive manner.

Design and many details are based on KDirStat (kdirstat.sourceforge.net), and refactored from the vanilla WinDirStat

The directory tree is simultanously shown as a treelist and as a treemap.
One can effortlessly gain an impression of the proportions on the hard disk(s).

Major features:
* 3 views, Directory tree, Treemap and Extension list, coupled with each other,


This is a Microsoft Visual Studio 2013 Project.

=============================================================================
How to create a resource dll.
* **Don't**, this isn't a solved problem yet.

=============================================================================

For the translators give credit by giving the language's name in the respective language plus all the three-letter codes available for the language in ISO 639.

Find the list of already included languages in the table below:

 
 L-ID (hex)|Language(English)|Native name|3-letter ISO 639 code (all available)|
-----------|-----------------|-----------|-------------------------------------|
 0405      | Czech           | Ceština   | CES/CZE                             |
 0407      | German          | Deutsch   | DEU/GER                             |
 0409      | English         | English   | ENG                                 |
 040a      | Spanish         | Español   | ESL/SPA                             |
 040c      | French          | Français  | FRA/FRE                             |
 040e      | Hungarian       | Magyar    | HUN                                 |
 0410      | Italian         | Italiano  | ITA                                 |
 0415      | Polish          | Polski    | POL                                 |
 0419      | Russian         | Ðóññêèé   | RUS                                 |
 ------------------------------------------------------------------------------

To lookup further language codes look at:
-> http://www.w3.org/WAI/ER/IG/ert/iso639.htm

=============================================================================
testplan.txt may be useful for future releases.

