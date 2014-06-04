rd zip /s/q
mkdir zip
mkdir zip\oit-source
mkdir zip\binaries
mkdir zip\utl-source
copy *.h zip\oit-source
copy *.rc zip\oit-source
copy *.sln zip\oit-source
copy *.vcproj zip\oit-source
copy *.rc zip\oit-source
copy README.txt zip
copy ..\..\nah6utils\itsutils.cpp zip\utl-source
copy ..\..\nah6utils\itsutils.h zip\utl-source
copy ..\..\nah6utils\checkdll.cpp zip\utl-source
copy ..\..\nah6utils\dllversion.h zip\utl-source
copy ..\..\nah6utils\pnewbootloader.cpp zip\utl-source
copy ..\..\nah6utils\bootloaderfix.bat zip\binaries

copy release\osimagetool.exe zip\binaries
copy ..\..\nah6utils\build\itsutils.dll zip\binaries
copy ..\..\nah6utils\build\pnewbootloader.exe zip\binaries
copy ..\..\nah6utils\build\bl515.nb0 zip\binaries\bootloader_v5_15.nb0
copy ..\..\nah6utils\README.fixbootloader zip
copy ..\..\nah6utils\mkarm.bat zip\utl-source
copy ..\..\nah6utils\mk.bat zip\utl-source
pushd zip
zip -r \tmp\xdatools .
popd
