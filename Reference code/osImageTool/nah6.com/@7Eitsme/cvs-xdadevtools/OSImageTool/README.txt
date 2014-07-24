OsImageTool

(c) 2003 XDA Developers
by itsme@xs4all.nl
web: http://www.xs4all.nl/~itsme/projects/xda/OsImageTool.html

INTRODUCTION

This tool is a replacement for XDARit, with less opportunity for the user
to damage their disks or devices.

When used interactively, you see 2 selection boxes, from which you
can either select a device or specify a file.
Devices can be a local sd card device, xda sd card device,
or xda ROM. The Sourcefile can be a .bin, .nb1, or nbf file,
the Destinationfile can be either a .nb1 or a .nbf file.

The program will check if the sdcard you are trying to write to
is large enough, not a harddisk, or too large, which may indicate
you are not writing to an sdcard. ( or maybe you are using this
software 2 years from now when 2+G cards will be common. )

This program will only create type 1 sd cards, which means you cannot
accidentally trash your bootloader with it.

Clicking 'COPY' will try to perform the action selected.
Clicking 'Cancel' will exit the program.


COMMANDLINE OPTIONS

    -r <source>
    -w <destination>
    -a                   Be as automatic as possible
    --register           Install as .nb1 and .nbf handler
    -h

<source> and <destination> may both be one of
   xdamemory
   xdasdslot
   localsdcard
   or a filename.

when 'localsdcard' is selected with '-a' (auto) and more than 1 sdcard
is found, OsImageTool will allow the user to select the desired card.


CONFIGURATION

   HKCR\.nb1 = 'Flash OsImageFile'
   HKCR\.nbf = 'Flash OsImageFile'
   HKCR\Flash OsImageFile\shell\Open\command
             = '"%apppath%" -a -r "%1" -w xdamemory'
   HKCR\Flash OsImageFile\shell\write to SD\command
             = '"%apppath%" -a -r "%1" -w localsdcard'

   HKCU\Software\XDA Developers\OsImageTool
        'Programme A Path'   = '%apppath%'
            * this is where the subdirectory for nk.nbf is searched for.
        'bootloader'         = '%apppath\bootloader_v5_15.nb0'
            * the bootloader used to create an .nbf
        'Programme A Command'= '%apppath%\start.bat'
            * the command used to execute Programme A
              this batchfile may for instance call adaptrom.exe
              to set things straight in the nbf.


FILES

  itsutils.dll

this is a small utility dll used to access the xda memory and sdcard.
source can be found on http://www.xs4all.nl/~itsme/projects/xda/tools.html

  OsImageTool.exe

this is the main executable.


DOWNLOAD

You can download the source + exe distribution from 
  http://www.xs4all.nl/~itsme/download/oit_100.zip.


INSTALLATION

From the source + exe distribution copy OsImageTool.exe and itsutils.dll
to the programme A directory as created by unpacking
jeff's-exe (http://cuba.calyx.nl/~jsummers/XDA/) to a directory in 
your "Program Files" directory. Then run 'OsImageTool --register'.


BUGS

Currently things may go wrong if you unplug / replug devices while
OsImageTool is already running.


HOW TO BUILD


To build this you may need to change the IncludeDirectories and
LibraryDirectories settings in vc++, they currently point to paths
on my machine, which may not have the same disks as yours.

You need the pocketpc2002 sdk ( for the activesync or rapi libaries )
from microsoft, and the boost ( from http://www.boost.org/ ) headerfiles. 
I am only using the header files from the boost distribution, no compilation 
of boost nescesary.


SOURCE CODE DESCRIPTION
  
This is a very raw description of how the various classes
relate to each other.

OsImageCopier 
    - can copy from OsImageReader to OsImageWriter

'OsImageReader' and 'OsImageWriter' use memory offsets 
      ( 80040000 for start of nb1 )
'SdCardDevice' uses device offsets

OsImageReader is subclassed by
    - SdOsImageReader
        * reads os images from a SdCardDevice
    - XDAOsImageReader
        * reads os image from device memory
    - FileOsImageReader, which is subclassed by:
        - BINOsImageReader
            * reads .xip and .bin files
        - LinearFileImageReader, which is subclassed by:
            - NB0ImageReader
            - NB1OsImageReader
            - NBFOsImageReader

OsImageWriter is subclassed by
    - SdOsImageWriter
        * writes osimage to a SdCardDevice
    - XDAOsImageWriter
        * creates nbf, and calls Programme A
    - FileOsImageWriter which is subclassed by 
        - LinearFileImageWriter which is subclassed by 
            - NB1FileOsImageWriter
            - NBFFileOsImageWriter

SdCardDevice is subclassed by
    - LocalSdCard  [ local disk devices ]
    - RemoteSdCard [ sdcard in xda ]

ComboBoxControl 
    - a cleaner interface to get/set data in a combobox

Configuration 
    - reads/writes configuration to the registry

DiskInfo
    - figures out what disks exist

ErrorHandler
    - keeps track of errors

SmartHandle
    - closes itself when it goes out of scope.

RemoteDevice
    - interface to rapi

Registry
    - easy interface to getting/setting registry values

I try to work with STL objects as much as possible, in most cases
a wrapper around the systemcalls is made for this.


...............................
todo
    - add target / source 'askme' to initially start with file selection 
      displayed
    - add tooltips to controls
    - why can I read way too much data from an sd card?
    - move all fileops to a class
    - split 'open' in 'open' and 'startimage'
    - split 'close' in 'close' and 'finishimage'
    - nbfwriter should not use 0x7fffffe0 as fileoffset, but needs 'out of band'
      method to write header.
    - add option to only allow writing of sd cards which already contain a 
      HTC$WALLABY signature
    - 'OsImage' naming is not entirely correct, same objects are used
      to read a bootloader.
    - maybe also add option to read bootloader from device, before
      taking the bootloader.nb0 file.
    - recheck device configuration when starting write.
    - calculate nbf checksum. ( instead of relying on adaptrom )
    - 'auto' may possibly be made more automatic.
    - I may need a 'message' or status message somewhere to describe what
      the program is doing.
    - give messageboxes the dialog as parent.
    - dialog should disappear when in 'auto' mode
    - SD card checksum is not correct
    - add FileImageSdDevice
    - osimagetool currently does not support writing sd cards with data above 31M.
       ( the saved contacts area, or in 1 case the first alpha release of ce.net
       this area contained the xip-chain )
    - 5.17 bootloader is not supported
    - check nbf and nb1 filesizes
    - replace calling of 'pput.exe' with copy-file-to-ce function
    - when waiting for activesync, 'no' doesn't cancel the dialog.
    - when selecting a file, the '.nb1' extension is not added.

    
[some research on how to relate drive letters to the device identification]

useful ioctls:

IOCTL_VOLUME_GET_VOLUME_DISK_EXTENTS
IOCTL_STORAGE_GET_DEVICE_NUMBER
IOCTL_STORAGE_QUERY_PROPERTY

usenet articles on lowlevel disk ioctls:

http://groups.google.com/groups?selm=%234UZnB9aCHA.1936%40tkmsftngp11
http://groups.google.com/groups?selm=d2ihfv4cgi2abb5f45ajpjf3l8vp4ooi4n%404ax.com
http://groups.google.com/groups?selm=OUnvnjLlCHA.1368%40tkmsftngp09
http://groups.google.com/groups?selm=984352256%40p24.f176.n450.z2.FidoNet.ftn
http://groups.google.com/groups?selm=3742BEBF.29D61EF2%40datan.de


