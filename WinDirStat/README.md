# MOST OF THIS IS OUT OF DATE, AND WRONG. I've fixed small parts, that's it. TODO: FIX

#### WinDirStat - Windows Directory Statistics
#### (c) 2003-2005 Bernhard Seifert (bseifert@users.sourceforge.net)


This project, as vanilla WinDirStat was made up of:

- the program [executables]
  - windirstat (English; default)
- resource DLLs [executables]
  - wdsr0405 (Czech)
  - wdsr0407 (German)
  - wdsr040a (Spanish)
  - wdsr040c (French)
  - wdsr040e (Hungarian)
  - wdsr0410 (Italian)
  - wdsr0415 (Polish)
  - wdsr0419 (Russian)
- helper/utility programs [executables]
  - setup,
  - linkcounter
- help files
  - wdsh0407 (German)
  - wdsh040e (Hungarian)
  - wdsh0415 (Polish)
  - wdshelp  (English; default)

altWinDirStat is:

- the program
  - windirstat.exe
and that's it!

I removed the [i18n](http://en.wikipedia.org/wiki/Internationalization_and_localization) support in vanilla WinDirStat, but I'd like to reintroduce it. The original system, using localized resource DLLs, was rather frustrating, in that it:
1. Required dynamic linking, and thus denied me the possibility of a single, standalone executable
2. Required the use of CString/CStringT (and thus heap/dynamic allocation)
3. Adds a layer of dynamic indirection, which impedes static analysis tools, and developer reasoning. Impeding static analysis and reasoning leads to [hard to spot bugs](https://github.com/ariccio/altWinDirStat/commit/639fefc715d094c6ca7c1770f2d991d836421ada#diff-f897a88848213c562f1eceba205b7af9L773), which **require** testing for discovery. I think that's silly, I think that relying of software testing is like attempting to prove Fermat's Last Theorem, as Prof. [John Conway describes it beginning at 7:30](http://vimeo.com/18216532), by testing all possible numbers - it's never going to work. **I place great importance on the clarity and analyzability of code**

I'd eventually like to reintroduce some form of i18n, albeit more cleanly.

is distributed under the terms of the GPL v2 (executables+source)
respectively GNU FDL (help files).

See windirstat/res/license.txt and wdshelp/gnufdl.htm.

The tree-GIF was found in http://www.world-in-motion.de - I hope, I
don't violate any copyright (tell me).


=============================================================================
WinDirStat is a disk usage statistics viewer and cleanup tool for MS Windows XP (all current variants). It shows disk, file and directory sizes in a treelist as well as graphically in a treemap, much like KDirStat and Sequoiaview.


=============================================================================
If you make any changes or enhancements, find bugs or have feature requests, please contact us. Note, however, that not all proposed features will be taken into account as we want to keep WinDirStat what it is: a tool to view disk usage statistics and to clean up your disk, simple and intuitive.


=============================================================================
WinDirStat is an application written in Visual C++ using MFC 7.0.
It runs on MS Windows (9x, NT, 2000, XP).

It shows where all your disk space is gone and helps you clean it up.

Design and many details are based on KDirStat (kdirstat.sourceforge.net).
WinDirStat is "a KDirStat re-programmed for MS Windows".

The directory tree is simultanously shown as a treelist and as a treemap.
One can effortlessly gain an impression of the proportions on the hard disk(s).

Major features:
* 3 views, Directory tree, Treemap and Extension list, coupled with each other,
* Built-in cleanup actions including Open, Delete, Show Properties,
* User defined cleanup actions (command line based),
* Language is English by default; further translations can be added as resource DLLs,
* Online-Help,
* A little setup.exe which installs the files and shortcuts.


=============================================================================
This is a Microsoft Visual Studio 2013 Project.

Projects included in the workspace:

 Component name | File name         | Description
----------------|-------------------|------------
 linkcounter	| linkercounter.exe | Updates LINKCOUNT in common/version.h
 setup		| setup.exe         | Setup helper
 wdsh0407	| wdsh0407.chm      | German Helpfile
 wdsh040e	| wdsh040e.chm      | Hungarian Helpfile
 wdsh0415       | wdsh0415.chm      | Polish Helpfile
 wdshelp	| windirstat.chm    | English Helpfile  
 wdsr0405	| wdsr0407.dll      | Czech Resource DLL
 wdsr0407	| wdsr0407.dll      | German Resource DLL
 wdsr040a	| wdsr040a.dll      | Spanish Resource DLL
 wdsr040c	| wdsr040c.dll      | French Resource DLL
 wdsr040e	| wdsr040e.dll      | Hungarian Resource DLL
 wdsr0410	| wdsr0415.dll      | Italian Resource DLL
 wdsr0415	| wdsr0415.dll      | Polish Resource DLL 
 wdsr0419	| wdsr0419.dll      | Russian Resource DLL
 windirstat	| windirstat.exe    | (including English resources).

The Microsoft redistributable file shfolder.dll is also included.

I've commented
- every source file
- every class
- every data member.

I haven't commented most member functions, maybe because my working English is not so good (I mostly would have duplicated the function name).

=============================================================================
How to create a resource dll.

* Determine the language id xx und sub-language id yy as defined in winnt.h.

* Create a new project wdsrxxyy.dll: Visual C++ Project
  - Win 32 Project - Dll - empty project.

* Copy windirstat.rc, resource.h and res/*.* into the
  wdsrxxyy-Folder respective the res-subfolder.

* In the linker options - advanced set Resource Only DLL to Yes.

* In Text Include 3 adjust the LANGUAGE

* Remove the manifest and license.txt

* Translate the rc-File

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

