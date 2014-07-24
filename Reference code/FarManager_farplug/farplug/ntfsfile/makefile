!include project.ini

TOOLSDIR = ..\tools
FARSDK = farsdk

CPPFLAGS = -nologo -Zi -W3 -Gy -GS -GR -EHsc -MP -c
DEFINES = -DWIN32_LEAN_AND_MEAN -D_CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES=1 -D_WIN32_WINNT=0x0501 -D__A_IDXSZ_TYPE__=unsigned -D_CRT_SECURE_NO_WARNINGS -D_CRT_NON_CONFORMING_SWPRINTFS
LINKFLAGS = -nologo -debug -incremental:no -map -manifest:no -dynamicbase -nxcompat -largeaddressaware -dll
RCFLAGS = -nologo

!if "$(CPU)" == "AMD64" || "$(PLATFORM)" == "x64"
PLATFORM = x64
LIBSUFFIX = 64
RCFLAGS = $(RCFLAGS) -Dx64
!else
PLATFORM = x86
LINKFLAGS = $(LINKFLAGS) -safeseh
LIBSUFFIX = 32
!endif

!ifdef RELEASE
OUTDIR = Release
DEFINES = $(DEFINES) -DNDEBUG
CPPFLAGS = $(CPPFLAGS) -O2 -GL -MT
LINKFLAGS = $(LINKFLAGS) -opt:ref -opt:icf -LTCG
!else
OUTDIR = Debug
DEFINES = $(DEFINES) -DDEBUG
CPPFLAGS = $(CPPFLAGS) -Od -RTC1 -MTd
LINKFLAGS = $(LINKFLAGS) -fixed:no
LIBSUFFIX=$(LIBSUFFIX)d
!endif

OUTDIR = $(OUTDIR).$(PLATFORM)
INCLUDES = -I$(FARSDK) -I.. -I$(OUTDIR) -Ilzo\include -Iopenssl\include
CPPFLAGS = $(CPPFLAGS) -Fo$(OUTDIR)\ -Fd$(OUTDIR)\ $(INCLUDES) $(DEFINES)
RCFLAGS = $(RCFLAGS) $(INCLUDES) $(DEFINES)
LINKFLAGS = $(LINKFLAGS) -libpath:lzo\lib -libpath:openssl\lib

!ifdef BUILD
!include $(OUTDIR)\far.ini
!endif

OBJS = $(OUTDIR)\main.obj $(OUTDIR)\content.obj $(OUTDIR)\file_panel.obj $(OUTDIR)\ntfs_file.obj $(OUTDIR)\options.obj $(OUTDIR)\utils.obj $(OUTDIR)\volume.obj $(OUTDIR)\dlgapi.obj $(OUTDIR)\defragment.obj $(OUTDIR)\mftindex.obj $(OUTDIR)\filever.obj $(OUTDIR)\compress_files.obj $(OUTDIR)\volume_list.obj

LIBS = lzo2_$(LIBSUFFIX).lib libeay$(LIBSUFFIX).lib advapi32.lib mpr.lib version.lib imagehlp.lib crypt32.lib wintrust.lib

project: depfile $(OUTDIR)\far.ini
  $(MAKE) -nologo -$(MAKEFLAGS) build_project BUILD=1

distrib: depfile $(OUTDIR)\far.ini
  $(MAKE) -nologo -$(MAKEFLAGS) build_distrib BUILD=1

installer: depfile $(OUTDIR)\far.ini
  $(MAKE) -nologo -$(MAKEFLAGS) build_installer BUILD=1

build_project: $(OUTDIR)\$(MODULE).dll $(OUTDIR)\en.lng $(OUTDIR)\en.hlf $(OUTDIR)\ru.hlf

$(OUTDIR)\$(MODULE).dll: $(OUTDIR)\plugin.def $(OBJS) $(OUTDIR)\headers.pch $(OUTDIR)\version.res project.ini
  link $(LINKFLAGS) -def:$(OUTDIR)\plugin.def -out:$@ $(OBJS) $(OUTDIR)\headers.obj $(OUTDIR)\version.res $(LIBS)

$(OBJS): $(OUTDIR)\headers.pch

.cpp{$(OUTDIR)}.obj::
  $(CPP) $(CPPFLAGS) -Yuheaders.hpp -FIheaders.hpp -Fp$(OUTDIR)\headers.pch $<

$(OUTDIR)\headers.pch: headers.cpp headers.hpp
  $(CPP) $(CPPFLAGS) headers.cpp -Ycheaders.hpp -Fp$(OUTDIR)\headers.pch

depfile: $(OUTDIR) $(OUTDIR)\msg.h $(OUTDIR)\plugin.h
  $(TOOLSDIR)\gendep.exe $(INCLUDES) > $(OUTDIR)\dep.mak

$(OUTDIR)\msg.h $(OUTDIR)\en.lng: $(OUTDIR) $(OUTDIR)\en.msg
  $(TOOLSDIR)\msgc -in $(OUTDIR)\en.msg -out $(OUTDIR)\msg.h $(OUTDIR)\en.lng

$(OUTDIR)\version.res: $(OUTDIR)\version.rc
  $(RC) $(RCFLAGS) -fo$@ $**

PREPROC = $(TOOLSDIR)\preproc $** $@

$(OUTDIR)\version.rc: project.ini $(OUTDIR)\far.ini version.rc
  $(PREPROC)

$(OUTDIR)\plugin.def: project.ini plugin.def
  $(PREPROC)

$(OUTDIR)\en.msg: project.ini en.msg
  $(PREPROC)

$(OUTDIR)\en.hlf: project.ini en.hlf
  $(PREPROC)

$(OUTDIR)\ru.hlf: project.ini ru.hlf
  $(PREPROC)

$(OUTDIR)\file_id.diz: project.ini $(OUTDIR)\far.ini file_id.diz
  $(PREPROC)

$(OUTDIR)\plugin.h: project.ini $(OUTDIR)\far.ini plugin.h.h
  $(PREPROC)

$(OUTDIR)\far.ini: $(FARSDK)\plugin.hpp
  $(TOOLSDIR)\farver $** $@

COPY = copy $** $@

$(OUTDIR)\history_ru.txt: history_ru.txt
  $(COPY)

$(OUTDIR)\Renewal.xml: Renewal.xml
  $(COPY)


$(OUTDIR):
  if not exist $(OUTDIR) mkdir $(OUTDIR)

!ifdef BUILD
!include $(OUTDIR)\dep.mak
!endif


DISTRIB = $(OUTDIR)\$(MODULE)_$(VER_MAJOR).$(VER_MINOR).$(VER_PATCH)_uni
DISTRIB_FILES = .\$(OUTDIR)\$(MODULE).dll .\$(OUTDIR)\$(MODULE).map .\$(OUTDIR)\en.lng .\$(OUTDIR)\en.hlf .\$(OUTDIR)\ru.hlf .\$(OUTDIR)\file_id.diz .\$(OUTDIR)\history_ru.txt .\$(OUTDIR)\Renewal.xml
!if "$(PLATFORM)" != "x86"
DISTRIB = $(DISTRIB)_$(PLATFORM)
!endif
!ifndef RELEASE
DISTRIB = $(DISTRIB)_dbg
DISTRIB_FILES = $(DISTRIB_FILES) .\$(OUTDIR)\$(MODULE).pdb
!endif

build_distrib: $(DISTRIB).7z

$(DISTRIB).7z: $(DISTRIB_FILES) project.ini
  7z a -mx=9 $@ $(DISTRIB_FILES)

build_installer: $(DISTRIB).msi

LIGHTFLAGS = -nologo -cultures:en-us -spdb -sval -b installer
CANDLEFLAGS = -nologo -Iinstaller -I$(OUTDIR) -dSourceDir=$(OUTDIR) -dPlatform=$(PLATFORM) -dProduct="$(NAME)" -dVersion=$(VER_MAJOR).$(VER_MINOR).$(VER_PATCH) -dMinFarVersion=$(FAR_VER_MAJOR).$(FAR_VER_MINOR).$(FAR_VER_BUILD)
!ifdef RELEASE
LIGHTFLAGS = $(LIGHTFLAGS) -dcl:high
!else
LIGHTFLAGS = $(LIGHTFLAGS) -sh -cc $(OUTDIR) -reusecab
!endif
WIXOBJ = $(OUTDIR)\installer.wixobj $(OUTDIR)\ui.wixobj $(OUTDIR)\plugin.wixobj

$(DISTRIB).msi: $(DISTRIB_FILES) project.ini $(WIXOBJ) installer\ui_en-us.wxl installer\banner.jpg installer\dialog.jpg installer\exclam.ico installer\info.ico
  light $(LIGHTFLAGS) -loc installer\ui_en-us.wxl -out $@ $(WIXOBJ)

{installer}.wxs{$(OUTDIR)}.wixobj::
  candle $(CANDLEFLAGS) -out $(OUTDIR)\ $<

installer\installer.wxs installer\plugin.wxs: installer\installer.wxi $(OUTDIR)\plugin.wxi

$(OUTDIR)\plugin.wxi: project.ini installer\plugin.wxi.wxi
  $(PREPROC)

clean:
  if exist $(OUTDIR) rd /s /q $(OUTDIR)


.PHONY: project distrib installer build_project build_distrib build_installer depfile clean
.SUFFIXES: .wxs
