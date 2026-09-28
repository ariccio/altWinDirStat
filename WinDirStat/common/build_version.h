// build_version.h - The version stamped into altWinDirStat.exe (VERSIONINFO + About box).
//
// CI overwrites this file before building (see .github/workflows/build.yml, "Stamp version"),
// using the release tag (vX.Y.Z -> X.Y.Z.<run number>) or 0.0.<run number> for untagged builds.
// The committed values below are what a local developer build reports.

#pragma once

#define AWDS_VER_MAJOR    0
#define AWDS_VER_MINOR    0
#define AWDS_VER_PATCH    0
#define AWDS_VER_BUILD    0
#define AWDS_VER_STRING   "0.0.0-dev"
#define AWDS_VER_WSTRING L"0.0.0-dev"
