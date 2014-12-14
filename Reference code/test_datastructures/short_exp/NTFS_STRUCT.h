// NTFS Structures

#include "stdafx.h"
//#include <Windows.h>
#include "Heap.h"


#define LONGINFO		1
#define SHORTINFO		2
#define SEARCHINFO		3
#define EXTRALONGINFO	4



#define POSIX_NAME      0
#define WIN32_NAME      1
#define DOS_NAME        2
#define WIN32DOS_NAME   3


#define NTFSDISK        1

// not supported
#define FAT32DISK       2
#define FATDISK         4
#define EXT2	        8

#define UNKNOWN         0xff99ff99

///WINHEX may be a great tool for this!



/* BOOT_BLOCK

*/
#pragma pack(push,1)

typedef struct {

	/*
	  NTFS Boot Sector Structure (for MBR formatted disks):
	  
	  Offset           Field Length          Field Name                 Notes

	  0x00             3   bytes             Jump Instruction
	  0x03             8   bytes             OEM ID
	  0x08             25  bytes             BPB
	  0x24             48  bytes             Extended BPB
	  0x54             426 bytes             Bootstrap Code
	  0x01FE           2   bytes             End of sector marker       ALWAYS set to 0x55AA

	  Remarks:
	  "At the end of the boot sector is a 2-byte structure called a signature word or end of sector marker, which is always set to 0x55AA" -("How NTFS works: Local File Systems")



	  GPT layout disks are DIFFERENT!!

	  Example data (also from "How NTFS works: Local File Systems"):

		Physical Sector: Cyl 0, Side 1, Sector 1
		00000000: EB 52 90 4E 54 46 53 20 - 20 20 20 00 02 08 00 00 .R.NTFS ..... ..
		00000010: 00 00 00 00 00 F8 00 00 - 3F 00 FF 00 3F 00 00 00 ........?...?...
		00000020: 00 00 00 00 80 00 80 00 - 1C 91 11 01 00 00 00 00 ................
		00000030: 00 00 04 00 00 00 00 00 - 11 19 11 00 00 00 00 00 ................
		00000040: F6 00 00 00 01 00 00 00 - 3A B2 7B 82 CD 7B 82 14 ........:.{..{..
		00000050: 00 00 00 00 FA 33 C0 8E - D0 BC 00 7C FB B8 C0 07 .....3.....|....
	 
	  Example data analysis:

	  Offset           Field Length          Field Name                 Notes
	  0x00             3   bytes             Jump Instruction
	              || || ||
	              VV VV VV
		00000000: EB 52 90 4E 54 46 53 20 - 20 20 20 00 02 08 00 00 .R.NTFS ..... ..
		00000010: 00 00 00 00 00 F8 00 00 - 3F 00 FF 00 3F 00 00 00 ........?...?...
		00000020: 00 00 00 00 80 00 80 00 - 1C 91 11 01 00 00 00 00 ................
		00000030: 00 00 04 00 00 00 00 00 - 11 19 11 00 00 00 00 00 ................
		00000040: F6 00 00 00 01 00 00 00 - 3A B2 7B 82 CD 7B 82 14 ........:.{..{..
		00000050: 00 00 00 00 FA 33 C0 8E - D0 BC 00 7C FB B8 C0 07 .....3.....|....





	  Offset           Field Length          Field Name                 Notes
	  0x03             8   bytes             OEM ID

	                       || || || || ||   || || ||
                           VV VV VV VV VV   VV VV VV
		00000000: EB 52 90 4E 54 46 53 20 - 20 20 20 00 02 08 00 00 .R.NTFS ..... ..
		00000010: 00 00 00 00 00 F8 00 00 - 3F 00 FF 00 3F 00 00 00 ........?...?...
		00000020: 00 00 00 00 80 00 80 00 - 1C 91 11 01 00 00 00 00 ................
		00000030: 00 00 04 00 00 00 00 00 - 11 19 11 00 00 00 00 00 ................
		00000040: F6 00 00 00 01 00 00 00 - 3A B2 7B 82 CD 7B 82 14 ........:.{..{..
		00000050: 00 00 00 00 FA 33 C0 8E - D0 BC 00 7C FB B8 C0 07 .....3.....|....


	  Offset           Field Length          Field Name                 Notes
	  0x0B             25  bytes             BPB

	                                                 || || || || ||
                                                     VV VV VV VV VV
		00000000: EB 52 90 4E 54 46 53 20 - 20 20 20 00 02 08 00 00 .R.NTFS ..... ..

		          || || || || || || || ||   || || || || || || || ||
		          VV VV VV VV VV VV VV VV   VV VV VV VV VV VV VV VV
		00000010: 00 00 00 00 00 F8 00 00 - 3F 00 FF 00 3F 00 00 00 ........?...?...

		          || || || ||
		          VV VV VV VV
		00000020: 00 00 00 00 80 00 80 00 - 1C 91 11 01 00 00 00 00 ................
		00000030: 00 00 04 00 00 00 00 00 - 11 19 11 00 00 00 00 00 ................
		00000040: F6 00 00 00 01 00 00 00 - 3A B2 7B 82 CD 7B 82 14 ........:.{..{..
		00000050: 00 00 00 00 FA 33 C0 8E - D0 BC 00 7C FB B8 C0 07 .....3.....|....


--------------------------------------------------------------------------------------------------------------------------------------------
		The BPB has MANY sub fields:



		  Offset           Field Length          Field Name                 Notes
		  0x0B             2  bytes              Bytes per sector           The size of a hardware sector, usually 512 Bytes

														 || ||
														 VV VV
			00000000: EB 52 90 4E 54 46 53 20 - 20 20 20 00 02 08 00 00 .R.NTFS ..... ..
			00000010: 00 00 00 00 00 F8 00 00 - 3F 00 FF 00 3F 00 00 00 ........?...?...
			00000020: 00 00 00 00 80 00 80 00 - 1C 91 11 01 00 00 00 00 ................
			00000030: 00 00 04 00 00 00 00 00 - 11 19 11 00 00 00 00 00 ................
			00000040: F6 00 00 00 01 00 00 00 - 3A B2 7B 82 CD 7B 82 14 ........:.{..{..
			00000050: 00 00 00 00 FA 33 C0 8E - D0 BC 00 7C FB B8 C0 07 .....3.....|....



		  Offset           Field Length          Field Name                 Notes
		  0x0D             1  byte               Sectors per cluster

														       ||
														       VV
			00000000: EB 52 90 4E 54 46 53 20 - 20 20 20 00 02 08 00 00 .R.NTFS ..... ..
			00000010: 00 00 00 00 00 F8 00 00 - 3F 00 FF 00 3F 00 00 00 ........?...?...
			00000020: 00 00 00 00 80 00 80 00 - 1C 91 11 01 00 00 00 00 ................
			00000030: 00 00 04 00 00 00 00 00 - 11 19 11 00 00 00 00 00 ................
			00000040: F6 00 00 00 01 00 00 00 - 3A B2 7B 82 CD 7B 82 14 ........:.{..{..
			00000050: 00 00 00 00 FA 33 C0 8E - D0 BC 00 7C FB B8 C0 07 .....3.....|....


		  Offset           Field Length          Field Name                 Notes
		  0x0E             2  bytes              RESERVED sectors           MUST be zero. NTFS will not mount if not zero. "Always 0 because NTFS places the boot sector at the beginning of the partition" - "How NTFS Works_Local File Systems"

														          || ||
														          VV VV
			00000000: EB 52 90 4E 54 46 53 20 - 20 20 20 00 02 08 00 00 .R.NTFS ..... ..
			00000010: 00 00 00 00 00 F8 00 00 - 3F 00 FF 00 3F 00 00 00 ........?...?...
			00000020: 00 00 00 00 80 00 80 00 - 1C 91 11 01 00 00 00 00 ................
			00000030: 00 00 04 00 00 00 00 00 - 11 19 11 00 00 00 00 00 ................
			00000040: F6 00 00 00 01 00 00 00 - 3A B2 7B 82 CD 7B 82 14 ........:.{..{..
			00000050: 00 00 00 00 FA 33 C0 8E - D0 BC 00 7C FB B8 C0 07 .....3.....|....


		  Offset           Field Length          Field Name                 Notes
		  0x10             3  bytes              N/A                        MUST be zero. NTFS will not mount if not zero.

			00000000: EB 52 90 4E 54 46 53 20 - 20 20 20 00 02 08 00 00 .R.NTFS ..... ..

			          || || ||
			          VV VV VV
			00000010: 00 00 00 00 00 F8 00 00 - 3F 00 FF 00 3F 00 00 00 ........?...?...
			00000020: 00 00 00 00 80 00 80 00 - 1C 91 11 01 00 00 00 00 ................
			00000030: 00 00 04 00 00 00 00 00 - 11 19 11 00 00 00 00 00 ................
			00000040: F6 00 00 00 01 00 00 00 - 3A B2 7B 82 CD 7B 82 14 ........:.{..{..
			00000050: 00 00 00 00 FA 33 C0 8E - D0 BC 00 7C FB B8 C0 07 .....3.....|....



		  Offset           Field Length          Field Name                 Notes
		  0x13             2  bytes              N/A                        MUST be zero. NTFS will not mount if not zero.

			00000000: EB 52 90 4E 54 46 53 20 - 20 20 20 00 02 08 00 00 .R.NTFS ..... ..

			                   || ||
			                   VV VV
			00000010: 00 00 00 00 00 F8 00 00 - 3F 00 FF 00 3F 00 00 00 ........?...?...
			00000020: 00 00 00 00 80 00 80 00 - 1C 91 11 01 00 00 00 00 ................
			00000030: 00 00 04 00 00 00 00 00 - 11 19 11 00 00 00 00 00 ................
			00000040: F6 00 00 00 01 00 00 00 - 3A B2 7B 82 CD 7B 82 14 ........:.{..{..
			00000050: 00 00 00 00 FA 33 C0 8E - D0 BC 00 7C FB B8 C0 07 .....3.....|....


		  Offset           Field Length          Field Name                 Notes
		  0x15             1  byte               Media Descriptor           F8 -> hard disk, F0 -> high-density 3.5-inch floppy disk

			00000000: EB 52 90 4E 54 46 53 20 - 20 20 20 00 02 08 00 00 .R.NTFS ..... ..

			                         ||
			                         VV
			00000010: 00 00 00 00 00 F8 00 00 - 3F 00 FF 00 3F 00 00 00 ........?...?...
			00000020: 00 00 00 00 80 00 80 00 - 1C 91 11 01 00 00 00 00 ................
			00000030: 00 00 04 00 00 00 00 00 - 11 19 11 00 00 00 00 00 ................
			00000040: F6 00 00 00 01 00 00 00 - 3A B2 7B 82 CD 7B 82 14 ........:.{..{..
			00000050: 00 00 00 00 FA 33 C0 8E - D0 BC 00 7C FB B8 C0 07 .....3.....|....


		  Offset           Field Length          Field Name                 Notes
		  0x16             2  bytes              RESERVED                   MUST be zero. NTFS will not mount if not zero.

			00000000: EB 52 90 4E 54 46 53 20 - 20 20 20 00 02 08 00 00 .R.NTFS ..... ..

			                            || ||
			                            VV VV
			00000010: 00 00 00 00 00 F8 00 00 - 3F 00 FF 00 3F 00 00 00 ........?...?...
			00000020: 00 00 00 00 80 00 80 00 - 1C 91 11 01 00 00 00 00 ................
			00000030: 00 00 04 00 00 00 00 00 - 11 19 11 00 00 00 00 00 ................
			00000040: F6 00 00 00 01 00 00 00 - 3A B2 7B 82 CD 7B 82 14 ........:.{..{..
			00000050: 00 00 00 00 FA 33 C0 8E - D0 BC 00 7C FB B8 C0 07 .....3.....|....


		  Offset           Field Length          Field Name                 Notes
		  0x18             2  bytes              N/A                        Don't care/Not checked

			00000000: EB 52 90 4E 54 46 53 20 - 20 20 20 00 02 08 00 00 .R.NTFS ..... ..

			                                    || ||
			                                    VV VV
			00000010: 00 00 00 00 00 F8 00 00 - 3F 00 FF 00 3F 00 00 00 ........?...?...
			00000020: 00 00 00 00 80 00 80 00 - 1C 91 11 01 00 00 00 00 ................
			00000030: 00 00 04 00 00 00 00 00 - 11 19 11 00 00 00 00 00 ................
			00000040: F6 00 00 00 01 00 00 00 - 3A B2 7B 82 CD 7B 82 14 ........:.{..{..
			00000050: 00 00 00 00 FA 33 C0 8E - D0 BC 00 7C FB B8 C0 07 .....3.....|....


		  Offset           Field Length          Field Name                 Notes
		  0x1A             2  bytes              N/A                        Don't care/Not checked

			00000000: EB 52 90 4E 54 46 53 20 - 20 20 20 00 02 08 00 00 .R.NTFS ..... ..

			                                          || ||
			                                          VV VV
			00000010: 00 00 00 00 00 F8 00 00 - 3F 00 FF 00 3F 00 00 00 ........?...?...
			00000020: 00 00 00 00 80 00 80 00 - 1C 91 11 01 00 00 00 00 ................
			00000030: 00 00 04 00 00 00 00 00 - 11 19 11 00 00 00 00 00 ................
			00000040: F6 00 00 00 01 00 00 00 - 3A B2 7B 82 CD 7B 82 14 ........:.{..{..
			00000050: 00 00 00 00 FA 33 C0 8E - D0 BC 00 7C FB B8 C0 07 .....3.....|....


		  Offset           Field Length          Field Name                 Notes
		  0x1C             4  bytes              N/A                        Don't care/Not checked

			00000000: EB 52 90 4E 54 46 53 20 - 20 20 20 00 02 08 00 00 .R.NTFS ..... ..

			                                                || || || ||
			                                                VV VV VV VV
			00000010: 00 00 00 00 00 F8 00 00 - 3F 00 FF 00 3F 00 00 00 ........?...?...
			00000020: 00 00 00 00 80 00 80 00 - 1C 91 11 01 00 00 00 00 ................
			00000030: 00 00 04 00 00 00 00 00 - 11 19 11 00 00 00 00 00 ................
			00000040: F6 00 00 00 01 00 00 00 - 3A B2 7B 82 CD 7B 82 14 ........:.{..{..
			00000050: 00 00 00 00 FA 33 C0 8E - D0 BC 00 7C FB B8 C0 07 .....3.....|....


		  Offset           Field Length          Field Name                 Notes
		  0x20             4  bytes              N/A                        MUST be zero. NTFS will not mount if not zero.

			00000000: EB 52 90 4E 54 46 53 20 - 20 20 20 00 02 08 00 00 .R.NTFS ..... ..
			00000010: 00 00 00 00 00 F8 00 00 - 3F 00 FF 00 3F 00 00 00 ........?...?...

			          || || || ||
			          VV VV VV VV
			00000020: 00 00 00 00 80 00 80 00 - 1C 91 11 01 00 00 00 00 ................
			00000030: 00 00 04 00 00 00 00 00 - 11 19 11 00 00 00 00 00 ................
			00000040: F6 00 00 00 01 00 00 00 - 3A B2 7B 82 CD 7B 82 14 ........:.{..{..
			00000050: 00 00 00 00 FA 33 C0 8E - D0 BC 00 7C FB B8 C0 07 .....3.....|....


	  --------------------------------------------------------------------------------
	  0x24->0x54 is the "Extended BPB"


		  Offset           Field Length          Field Name                 Notes
		  0x24             4  bytes              N/A                        Don't care/Not checked

			00000000: EB 52 90 4E 54 46 53 20 - 20 20 20 00 02 08 00 00 .R.NTFS ..... ..
			00000010: 00 00 00 00 00 F8 00 00 - 3F 00 FF 00 3F 00 00 00 ........?...?...

			                      || || || ||
			                      VV VV VV VV
			00000020: 00 00 00 00 80 00 80 00 - 1C 91 11 01 00 00 00 00 ................
			00000030: 00 00 04 00 00 00 00 00 - 11 19 11 00 00 00 00 00 ................
			00000040: F6 00 00 00 01 00 00 00 - 3A B2 7B 82 CD 7B 82 14 ........:.{..{..
			00000050: 00 00 00 00 FA 33 C0 8E - D0 BC 00 7C FB B8 C0 07 .....3.....|....


		  Offset           Field Length          Field Name                 Notes
		  0x28             8  bytes              Total Sectors              The number of sectors on the hard disk

			00000000: EB 52 90 4E 54 46 53 20 - 20 20 20 00 02 08 00 00 .R.NTFS ..... ..
			00000010: 00 00 00 00 00 F8 00 00 - 3F 00 FF 00 3F 00 00 00 ........?...?...

			                                    || || || || || || || ||
			                                    VV VV VV VV VV VV VV VV
			00000020: 00 00 00 00 80 00 80 00 - 1C 91 11 01 00 00 00 00 ................
			00000030: 00 00 04 00 00 00 00 00 - 11 19 11 00 00 00 00 00 ................
			00000040: F6 00 00 00 01 00 00 00 - 3A B2 7B 82 CD 7B 82 14 ........:.{..{..
			00000050: 00 00 00 00 FA 33 C0 8E - D0 BC 00 7C FB B8 C0 07 .....3.....|....


		  Offset           Field Length          Field Name                 Notes
		  0x30             8  bytes              Logical Cluster # of $MFT  Location of the Master File Table, by LCN

			00000000: EB 52 90 4E 54 46 53 20 - 20 20 20 00 02 08 00 00 .R.NTFS ..... ..
			00000010: 00 00 00 00 00 F8 00 00 - 3F 00 FF 00 3F 00 00 00 ........?...?...
			00000020: 00 00 00 00 80 00 80 00 - 1C 91 11 01 00 00 00 00 ................

			          || || || || || || || ||
			          VV VV VV VV VV VV VV VV
			00000030: 00 00 04 00 00 00 00 00 - 11 19 11 00 00 00 00 00 ................
			00000040: F6 00 00 00 01 00 00 00 - 3A B2 7B 82 CD 7B 82 14 ........:.{..{..
			00000050: 00 00 00 00 FA 33 C0 8E - D0 BC 00 7C FB B8 C0 07 .....3.....|....


		  Offset           Field Length          Field Name                 Notes
		  0x38             8  bytes              LCN of $MFTMirr  Location of the Master File Table, by LCN

			00000000: EB 52 90 4E 54 46 53 20 - 20 20 20 00 02 08 00 00 .R.NTFS ..... ..
			00000010: 00 00 00 00 00 F8 00 00 - 3F 00 FF 00 3F 00 00 00 ........?...?...
			00000020: 00 00 00 00 80 00 80 00 - 1C 91 11 01 00 00 00 00 ................

			                                    || || || || || || || ||
			                                    VV VV VV VV VV VV VV VV
			00000030: 00 00 04 00 00 00 00 00 - 11 19 11 00 00 00 00 00 ................
			00000040: F6 00 00 00 01 00 00 00 - 3A B2 7B 82 CD 7B 82 14 ........:.{..{..
			00000050: 00 00 00 00 FA 33 C0 8E - D0 BC 00 7C FB B8 C0 07 .....3.....|....


		  Offset           Field Length          Field Name                 Notes
		  0x40             1  byte               Clusters Per MFT record    The size of each record. For values 0x00->0x7F (positive numbers), the value of this byte is the number of clusters. For 0x80->0xFF (negative numbers), then 2^(the absolute value of the value of this byte) is the size of a file record in number of clusters.

			00000000: EB 52 90 4E 54 46 53 20 - 20 20 20 00 02 08 00 00 .R.NTFS ..... ..
			00000010: 00 00 00 00 00 F8 00 00 - 3F 00 FF 00 3F 00 00 00 ........?...?...
			00000020: 00 00 00 00 80 00 80 00 - 1C 91 11 01 00 00 00 00 ................
			00000030: 00 00 04 00 00 00 00 00 - 11 19 11 00 00 00 00 00 ................

			          ||
			          VV
			00000040: F6 00 00 00 01 00 00 00 - 3A B2 7B 82 CD 7B 82 14 ........:.{..{..
			00000050: 00 00 00 00 FA 33 C0 8E - D0 BC 00 7C FB B8 C0 07 .....3.....|....


		  Offset           Field Length          Field Name                 Notes
		  0x41             3  bytes              N/A                        Don't care/Not checked

			00000000: EB 52 90 4E 54 46 53 20 - 20 20 20 00 02 08 00 00 .R.NTFS ..... ..
			00000010: 00 00 00 00 00 F8 00 00 - 3F 00 FF 00 3F 00 00 00 ........?...?...
			00000020: 00 00 00 00 80 00 80 00 - 1C 91 11 01 00 00 00 00 ................
			00000030: 00 00 04 00 00 00 00 00 - 11 19 11 00 00 00 00 00 ................

			             || || ||
			             VV VV VV
			00000040: F6 00 00 00 01 00 00 00 - 3A B2 7B 82 CD 7B 82 14 ........:.{..{..
			00000050: 00 00 00 00 FA 33 C0 8E - D0 BC 00 7C FB B8 C0 07 .....3.....|....



		  Offset           Field Length          Field Name                 Notes
		  0x44             1  byte               Clusters Per Index Buffer  The size of each index buffer. Refer to the note after this segment.

			00000000: EB 52 90 4E 54 46 53 20 - 20 20 20 00 02 08 00 00 .R.NTFS ..... ..
			00000010: 00 00 00 00 00 F8 00 00 - 3F 00 FF 00 3F 00 00 00 ........?...?...
			00000020: 00 00 00 00 80 00 80 00 - 1C 91 11 01 00 00 00 00 ................
			00000030: 00 00 04 00 00 00 00 00 - 11 19 11 00 00 00 00 00 ................

			                      ||
			                      VV
			00000040: F6 00 00 00 01 00 00 00 - 3A B2 7B 82 CD 7B 82 14 ........:.{..{..
			00000050: 00 00 00 00 FA 33 C0 8E - D0 BC 00 7C FB B8 C0 07 .....3.....|....



			NOTE: This can be negative, which means that the size of the MFT/Index record is smaller than a cluster. In this case the size of the MFT/Index record in bytes is equal to 2^(-1 * Clusters per MFT/Index record). So for example if Clusters per MFT Record is 0xF6 (-10 in decimal), the MFT record size is 2^(-1 * -10) = 2^10 = 1024 bytes.


		  Offset           Field Length          Field Name                 Notes
		  0x45             3  bytes              N/A                        Don't care/Not checked

			00000000: EB 52 90 4E 54 46 53 20 - 20 20 20 00 02 08 00 00 .R.NTFS ..... ..
			00000010: 00 00 00 00 00 F8 00 00 - 3F 00 FF 00 3F 00 00 00 ........?...?...
			00000020: 00 00 00 00 80 00 80 00 - 1C 91 11 01 00 00 00 00 ................
			00000030: 00 00 04 00 00 00 00 00 - 11 19 11 00 00 00 00 00 ................

			                         || || ||
			                         VV VV VV
			00000040: F6 00 00 00 01 00 00 00 - 3A B2 7B 82 CD 7B 82 14 ........:.{..{..
			00000050: 00 00 00 00 FA 33 C0 8E - D0 BC 00 7C FB B8 C0 07 .....3.....|....


		  Offset           Field Length          Field Name                 Notes
		  0x48             8  bytes              Volume Serial Number       The serial number of this volume.

			00000000: EB 52 90 4E 54 46 53 20 - 20 20 20 00 02 08 00 00 .R.NTFS ..... ..
			00000010: 00 00 00 00 00 F8 00 00 - 3F 00 FF 00 3F 00 00 00 ........?...?...
			00000020: 00 00 00 00 80 00 80 00 - 1C 91 11 01 00 00 00 00 ................
			00000030: 00 00 04 00 00 00 00 00 - 11 19 11 00 00 00 00 00 ................

			                                    || || || || || || || ||
			                                    VV VV VV VV VV VV VV VV
			00000040: F6 00 00 00 01 00 00 00 - 3A B2 7B 82 CD 7B 82 14 ........:.{..{..
			00000050: 00 00 00 00 FA 33 C0 8E - D0 BC 00 7C FB B8 C0 07 .....3.....|....


		  Offset           Field Length          Field Name                 Notes
		  0x50             4  bytes              N/A                        Don't care/Not checked

			00000000: EB 52 90 4E 54 46 53 20 - 20 20 20 00 02 08 00 00 .R.NTFS ..... ..
			00000010: 00 00 00 00 00 F8 00 00 - 3F 00 FF 00 3F 00 00 00 ........?...?...
			00000020: 00 00 00 00 80 00 80 00 - 1C 91 11 01 00 00 00 00 ................
			00000030: 00 00 04 00 00 00 00 00 - 11 19 11 00 00 00 00 00 ................
			00000040: F6 00 00 00 01 00 00 00 - 3A B2 7B 82 CD 7B 82 14 ........:.{..{..

			          || || || ||
			          VV VV VV VV
			00000050: 00 00 00 00 FA 33 C0 8E - D0 BC 00 7C FB B8 C0 07 .....3.....|....


		  Offset           Field Length          Field Name                 Notes
		  0x54             426  bytes            Bootstrap Code             Don't care/Not checked

			00000000: EB 52 90 4E 54 46 53 20 - 20 20 20 00 02 08 00 00 .R.NTFS ..... ..
			00000010: 00 00 00 00 00 F8 00 00 - 3F 00 FF 00 3F 00 00 00 ........?...?...
			00000020: 00 00 00 00 80 00 80 00 - 1C 91 11 01 00 00 00 00 ................
			00000030: 00 00 04 00 00 00 00 00 - 11 19 11 00 00 00 00 00 ................
			00000040: F6 00 00 00 01 00 00 00 - 3A B2 7B 82 CD 7B 82 14 ........:.{..{..

			                      || || || ||   || || || || || || || ||
			                      VV VV VV VV   VV VV VV VV VV VV VV VV
			00000050: 00 00 00 00 FA 33 C0 8E - D0 BC 00 7C FB B8 C0 07 .....3.....|....
			                                               ^
			We don't care about all the rest of this stuff |


			</BPB>


	  </Boot Sector>

	  Size of:
	  UCHAR      1 Byte
	  USHORT     2 Bytes
	  ULONG      4 Bytes
	  ULONGLONG  8 Bytes

	  WORD  == unsigned short
	  DWORD == unsigned long
	  BYTE  == unsigned char
	*/
                                                //OFFSET      DECIMAL
	UCHAR              Jump_Instruction[3];//     0x000       000
	UCHAR              OEM_ID[8];//               0x003       003
	USHORT             BytesPerSector;//          0x00B       011
	UCHAR              SectorsPerCluster;//       0x00D       013
	USHORT             BootSectors;//             0x00E       014
	

	//could be UCHAR[5] for clarity
	UCHAR              Mbz1;//Must Be Zero??      0x010       016
	USHORT             Mbz2;//                    0x011       017
	USHORT             Reserved1;//               0x013       019

	UCHAR              Media_Descriptor;//        0x015       021
	
	USHORT             Mbz3;//                    0x016       022
	USHORT             SectorsPerTrack;//         0x018       024
	USHORT             NumberOfHeads;//           0x01A       026
	
	//labeled as "unused" in ntfsdoc.pdf
	ULONG              PartitionOffset;//         0x01C       028
	ULONG              Rserved2[2];//             0x020       032
	
	ULONGLONG          TotalSectors;//            0x028       040
	ULONGLONG          MftStartLcn;//             0x030       048
	ULONGLONG          Mft2StartLcn;//            0x038       056

	//"How NTFS Works_Local File Systems" refers to this as ONE byte in size? 
	ULONG              ClustersPerFileRecord;//   0x040       064
	
	ULONG              ClustersPerIndexBlock;//   0x044       068
	ULONGLONG          VolumeSerialNumber;//      0x048       072
	ULONG              NotUsedByNTFS;//           0x050       080
	UCHAR              Bootstrap_Code[426];
	USHORT             EndOfSectorMarker;//ALWAYS 0x55AA (decimal 21930)
}BOOT_BLOCK, *PBOOT_BLOCK;

#pragma pack(pop)


typedef struct{
	/* 
	  NTFS_RECORD_HEADER
	  type - 'FILE' 'INDX' 'BAAD' 'HOLE' *CHKD' - magic number!
	  Size of:
	  UCHAR      1 Byte
	  BYTE       1 Byte
	  USHORT     2 Bytes
	  WORD       2 Bytes
	  ULONG      4 Bytes
	  DWORD      4 Bytes
	  ULONGLONG  8 Bytes

	  WORD  == unsigned short
	  DWORD == unsigned long
	  BYTE  == unsigned char

	  "magic_number_MFT_record_header" is also referred to as: "fileSignature" (NTFS forensics.pdf), "Magic number" (ntfsdoc.pdf), "Magic Number" (MSHD.pptx, wk8.ppt), "Signature" (vorlesung_forensik_ws11-12_kap06_ntfs-handout.pdf, NTFS.ppt)

	  "UpdateSequenceArray_Offset" is also referred to as: "Offset to the update sequence" (wk8.ppt, MSHD.pptx, ntfsdoc.pdf), "Update Seq array offset" (ntfs_cheat_sheets.pdf), "wFixupOffset" (NTFS forensics.pdf), "Offset to fixup array" (NTFS.ppt)

	  "UpdateSequenceArray_Size" is also referred to as: "Size in words of Update Sequence Number & Array (S)" (ntfsdoc.pdf), "Number of entries in fixup array" (MSHD.pptx, NTFS.ppt, wk8.ppt), "wFixupSize" (NTFS forensics.pdf), "Update Seq array size" (ntfs_cheat_sheets.pdf)


	  "LogFileSequenceNumber" is also referred to as: "$LogFile Sequence Number" (ntfs_cheat_sheets.pdf), "n64LogSeqNumber" (NTFS forensics.pdf), "$LogFile Sequence number" (NTFS.ppt), "$LogFile Sequence Number (LSN)" (wk8.ppt, MSHD.pptx, ntfsdoc.pdf),
	*/
	//                                            OFFSET      DECIMAL

	//char[4] maybe?
	//ULONG  magic_number_MFT_record_header;// 0x000       000

	char  magic_number_MFT_record_header[4];//    0x000       000
	USHORT UpdateSequenceArray_Offset;//          0x004       004
	USHORT UpdateSequenceArray_Size;//            0x006       006
	USN    LogFileSequenceNumber;//               0x008       008
}NTFS_RECORD_HEADER, *PNTFS_RECORD_HEADER;

typedef struct{
	/*
	  FILE_RECORD_HEADER

	  Size of:
	  UCHAR      1 Byte
	  BYTE       1 Byte
	  USHORT     2 Bytes
	  WORD       2 Bytes
	  ULONG      4 Bytes
	  DWORD      4 Bytes
	  ULONGLONG  8 Bytes

	  WORD  == unsigned short
	  DWORD == unsigned long
	  BYTE  == unsigned char

	  "SequenceNumber" is also referred to as:"wSequence" (NTFS forensics.pdf), "Seq no" (ntfs_cheat_sheets.pdf)

	  "HardLink_Count" is also referred to as: "wHardLinks" (NTFS forensics.pdf)

	  "OffsetToFirstAttribute" is also referred to as: "wAttribOffset" (NTFS forensics.pdf)

	  "UsedSizeOfThisRecord" is also referred to as: "Used size of file record" (ntfs_cheat_sheets.pdf), "Real size of the FILE record" (ntfsdoc.pdf), "Used size of MFT entry" (NTFS.ppt, MSHD.pptx, wk8.ppt), "dwRecLength" (NTFS forensics.pdf)

	  "AllocatedBytesThisRecord" is also referred to as "dwAllLength" (ntfs forensics.pdf), "Allocated size of MFT entry" (wk8.ppt, MSHD.pptx, NTFS.ppt), "Allocated size of the FILE record" (ntfsdoc.pdf), "Allocated size of the FILE record" (ntfs_cheat_sheets.pdf)

	  "ReferenceBaseFileRecord" is also referred to as "n64BaseMftRec" (ntfs forensics.pdf)

	*/
	//                                            OFFSET      DECIMAL
	NTFS_RECORD_HEADER Ntfs;//                    0x000       000
	USHORT             SequenceNumber;//          0x010       016
	USHORT             HardLink_Count;//          0x012       018
	USHORT             OffsetToFirstAttribute;//  0x014       020
	
	// 0x0001 InUse; 0x0002 Directory
	USHORT             Flags;//                   0x016       022
	ULONG              UsedSizeOfThisRecord;//    0x018       024
	ULONG              AllocatedBytesThisRecord;//0x01C       028
	ULARGE_INTEGER     ReferenceBaseFileRecord;// 0x020       032
	USHORT             NextAttributeID;//         0x028       040
} FILE_RECORD_HEADER, *PFILE_RECORD_HEADER;

typedef struct {
	/*
	  FILE_RECORD_HEADER

	  Size of:
	  UCHAR      1 Byte
	  BYTE       1 Byte
	  USHORT     2 Bytes
	  WORD       2 Bytes
	  ULONG      4 Bytes
	  DWORD      4 Bytes
	  ULONGLONG  8 Bytes

	  WORD  == unsigned short
	  DWORD == unsigned long
	  BYTE  == unsigned char
	*/
	//                                            OFFSET      DECIMAL
	NTFS_RECORD_HEADER Ntfs;//                    0x000       000
	LONGLONG           logFileSeqNum;//           0x010       016
	LONGLONG           vcnOfIndex;//              0x018       024
	ULONG              indexEntryOffset;//        0x020       032
	LONGLONG           sizeOfIndexEntries;//      0x024       036
	LONGLONG           AllocatedSizeOfEntries;//  0x02C       044
	UCHAR              Flags;//                   0x034       052
	UCHAR              padding[ 3 ];//            0x035       053
	} INDEX_RECORD_HEADER, *PINDEX_RECORD_HEADER;



typedef enum {
	/*
	  ATTRIBUTE_TYPE enumeration
	  sizeof(ATTRIBUTE_TYPE) = 4 Bytes
	*/

	StandardInformation = 0x10,
	AttributeList       = 0x20,
	FileName            = 0x30,
	ObjectId            = 0x40,
	SecurityDescripter  = 0x50,
	VolumeName          = 0x60,
	VolumeInformation   = 0x70,
	Data                = 0x80,
	IndexRoot           = 0x90,
	IndexAllocation     = 0xA0,
	Bitmap              = 0xB0,
	ReparsePoint        = 0xC0,
	EAInformation       = 0xD0,
	EA                  = 0xE0,
	PropertySet         = 0xF0,
	LoggedUtilityStream = 0x100
} ATTRIBUTE_TYPE, *PATTRIBUTE_TYPE;

typedef struct{
	/*
	  ATTRIBUTE Structure
	  Size of:
	  UCHAR      1 Byte
	  BYTE       1 Byte
	  USHORT     2 Bytes
	  WORD       2 Bytes
	  ULONG      4 Bytes
	  DWORD      4 Bytes
	  ULONGLONG  8 Bytes

	  WORD  == unsigned short
	  DWORD == unsigned long
	  BYTE  == unsigned char
	*/

	//                                            OFFSET      DECIMAL
	ATTRIBUTE_TYPE     AttributeType;//           0x000       000
	ULONG              Length;//                  0x004       004
	BOOLEAN            Nonresident;//             0x008       008
	UCHAR              NameLength;//              0x009       009

	// Starts form the Attribute Offset
	USHORT             NameOffset;//              0x00A       010

	// 0x001 = Compressed
	USHORT             Flags; //                  0x00B       011
	USHORT             AttributeNumber;//         0x00C       012
} ATTRIBUTE, *PATTRIBUTE;

typedef struct {
	/*
	  ATTRIBUTE resident
	  Size of:
	  UCHAR      1 Byte
	  BYTE       1 Byte
	  USHORT     2 Bytes
	  WORD       2 Bytes
	  ULONG      4 Bytes
	  DWORD      4 Bytes
	  ULONGLONG  8 Bytes

	  WORD  == unsigned short
	  DWORD == unsigned long
	  BYTE  == unsigned char
	*/
	//                                            OFFSET      DECIMAL
	ATTRIBUTE          Attribute;//               0x000       000
	ULONG              ValueLength;//             0x010       016

	//Starts from the Attribute
	USHORT             ValueOffset;//             0x014       020
	
	//0x0001 Indexed
	USHORT             Flags;//                   0x016       022
} RESIDENT_ATTRIBUTE, *PRESIDENT_ATTRIBUTE;

typedef struct {
	/*
	  ATTRIBUTE nonresident
	  Size of:
	  UCHAR      1 Byte
	  BYTE       1 Byte
	  USHORT     2 Bytes
	  WORD       2 Bytes
	  ULONG      4 Bytes
	  DWORD      4 Bytes
	  ULONGLONG  8 Bytes

	  WORD  == unsigned short
	  DWORD == unsigned long
	  BYTE  == unsigned char
	*/
	//                                            OFFSET      DECIMAL
	ATTRIBUTE          Attribute;//               0x000       000
	ULONGLONG          LowVcn;//                  0x010       016
	ULONGLONG          HighVcn;//                 0x018       024
	USHORT             RunArrayOffset;//          0x020       032
	UCHAR              CompressionUnit;//         0x022       034
	UCHAR              AligmentOrReserved[5];//   0x023       035
	ULONGLONG          AllocatedSize;//           0x024       036
	ULONGLONG          DataSize;//                0x02C       044
	ULONGLONG          InitializedSize;//         0x034       052
	
	//Only when compressed
	ULONGLONG          CompressedSize;//          0x03C       060
} NONRESIDENT_ATTRIBUTE, *PNONRESIDENT_ATTRIBUTE;

/* 
	VolumeName - just a Unicode String
	Data = just data
	SecurityDescriptor - rarely found
	Bitmap - array of bits, which indicate the use of entries
*/

typedef struct {
	/*
	  STANDARD_INFORMATION
	  FILE_ATTRIBUTES_* like in windows.h, and is always resident
	*/
	FILETIME           CreationTime;
	FILETIME           ChangeTime;
	FILETIME           LastWriteTime;
	FILETIME           LastAccessTime;
	ULONG              FileAttributes;
	ULONG              AligmentOrReservedOrUnknown[3];
	ULONG              QuotaId;
	ULONG              SecurityID;
	ULONGLONG          QuotaCharge;
	USN                Usn;
} STANDARD_INFORMATION, *PSTANDARD_INFORMATION;

typedef struct {
	/*
	  STANDARD_INFORMATION
	  FILE_ATTRIBUTES_* like in windows.h, and is always resident
	*/
	ATTRIBUTE_TYPE     Attribute;
	USHORT             Length;
	UCHAR              NameLength;
	USHORT             NameOffset; // starts at structure begin
	ULONGLONG          LowVcn;
	ULONGLONG          FileReferenceNumber;
	USHORT             AttributeNumber;
	USHORT             AligmentOrReserved[3];
}ATTRIBUTE_LIST, *PATTRIBUTE_LIST;

typedef struct {
	/*
	  FILENAME_ATTRIBUTE
	  is always resident
	  ULONGLONG informations only updated, if name changes
	*/
	ULONGLONG          DirectoryFileReferenceNumber; //points to a MFT Index of a directory
	FILETIME           CreationTime; //saved on creation, changed when filename changes
	FILETIME           ChangeTime;
	FILETIME           LastWriteTime;
	FILETIME           LastAccessTime;
	ULONGLONG          AllocatedSize;
	ULONGLONG          DataSize; 
	ULONG              FileAttributes; // ditto
	ULONG              AligmentOrReserved;
	UCHAR              NameLength;
	UCHAR              NameType; // 0x01 Long 0x02 Short 0x00 Posix?
	WCHAR              Name[1];
}FILENAME_ATTRIBUTE, *PFILENAME_ATTRIBUTE;



/* MYSTRUCTS


*/

typedef struct
{
	LPCWSTR            FileName;
	DWORD              FileNameLength;
	ULARGE_INTEGER     ParentId;
	//ULARGE_INTEGER mftnumber; //Inode
	DWORD              Inode;
	WORD               flags;
	//HTREEITEM hItem;
	FILETIME           ctime;
	FILETIME           atime;
	FILETIME           mtime;
	FILETIME           rtime;

	ULONGLONG          filesize;
	ULONGLONG          allocfilesize;
	DWORD              attributes;
	DWORD              objAttrib;

}FILEINFORMATION, *PFILEINFORMATION;

typedef struct {
	LPCWSTR            FileName;
	USHORT             FileNameLength;
	USHORT             Flags;
	ULARGE_INTEGER     ParentId;
}SEARCHFILEINFO, *PSEARCHFILEINFO;

typedef struct {
	LPCWSTR            FileName;
	USHORT             FileNameLength;
	USHORT             Flags;
	ULARGE_INTEGER     ParentId;
	ULARGE_INTEGER     FileSize;
	LPARAM             UserData; 
	PVOID              ExtraData;
}SHORTFILEINFO, *PSHORTFILEINFO;

typedef struct experimental_SHORTFILEINFO experimental_SHORTFILEINFO;
typedef struct {
	/*
	  Hyper compact structure
	  order of magnitude = rough approx of file size
	  ~file size = 2^(order of magnitude)

	*/
	short              orderOfMagnitude;
	experimental_SHORTFILEINFO* parent_ptr;
	std::vector<experimental_SHORTFILEINFO*> exp_children;
	//ULARGE_INTEGER     ParentId;
}experimental_SHORTFILEINFO;



typedef struct 
{
	LPCWSTR            FileName;
	USHORT             FileNameLength;
	USHORT             Flags;
	ULARGE_INTEGER     ParentId;
	ULARGE_INTEGER     FileSize;
	LPARAM             UserData; 
	PVOID              ExtraData;

	FILETIME           CreationTime;
	FILETIME           AccessTime;
	FILETIME           WriteTime;
	FILETIME           ChangeTime;
	ULARGE_INTEGER     AllocatedFileSize;
	DWORD              FileAttributes;
	DWORD              Attributes;
}LONGFILEINFO, *PLONGFILEINFO;

typedef struct {
	HANDLE             fileHandle;
	DWORD              type;
	DWORD              IsLong;
	DWORD              filesSize;
	DWORD              realFiles;
	WCHAR              DosDevice;
	PHEAPBLOCK         heapBlock;
	union {
		struct{
			BOOT_BLOCK     bootSector;
			DWORD          BytesPerFileRecord;
			DWORD          BytesPerCluster;
			BOOL           complete;
			DWORD          sizeMFT;
			DWORD          entryCount;
			ULARGE_INTEGER MFTLocation;
			ULARGE_INTEGER MFT2Location;
			UCHAR          *MFT;
			UCHAR          *Bitmap;
			} NTFS;
		struct {
			DWORD FAT;
			} FAT;
		union {
			LONGFILEINFO   *lFiles;
			SHORTFILEINFO  *sFiles;
			SEARCHFILEINFO *fFiles;
			};
		};
}DISKHANDLE, *PDISKHANDLE;

typedef struct
{
	HWND               hWnd;
	DWORD              Value;
}STATUSINFO, *PSTATUSINFO;

VOID CallMe( PSTATUSINFO info, DWORD value );//Maybe?




/* MY FUNCTIONS

*/


typedef DWORD ( __cdecl *FETCHPROC)(PDISKHANDLE, PFILE_RECORD_HEADER, PUCHAR );


PDISKHANDLE            OpenDisk              ( LPCTSTR                disk                                                                                                                                   );
PDISKHANDLE            OpenDisk              ( WCHAR                  DosDevice                                                                                                                              );
BOOL                   FixFileRecord         ( PFILE_RECORD_HEADER    file                                                                                                                                   );
BOOL                   CloseDisk             ( PDISKHANDLE            disk                                                                                                                                   );
BOOL                   ReparseDisk           ( PDISKHANDLE            disk,     UINT                   option,  PSTATUSINFO info                                                                             );
ULONGLONG              LoadMFT               ( PDISKHANDLE            disk,     BOOL                   complete                                                                                              );
DWORD                  ParseMFT              ( PDISKHANDLE            disk,     UINT                   option,  PSTATUSINFO info                                                                             );
DWORD                  ReadNextCluster       ( PDISKHANDLE            disk,     PVOID                  buffer                                                                                                );
DWORD                  ReadDataLCN           ( PDISKHANDLE            disk,     ULONGLONG              lcn,     ULONG       count, PUCHAR     buffer                                                         );
DWORD                  ReadExternalData      ( PDISKHANDLE            disk,     PNONRESIDENT_ATTRIBUTE attr,    ULONGLONG   vcn,   ULONG      count,  PUCHAR    buffer                                       );
DWORD                  ReadData              ( PDISKHANDLE            disk,     PATTRIBUTE             attr,    PUCHAR      buffer                                                                           );
DWORD                  ReadMFTParse          ( PDISKHANDLE            disk,     PNONRESIDENT_ATTRIBUTE attr,    ULONGLONG   vcn,   ULONG      count,  PVOID     buffer, FETCHPROC   fetch, PSTATUSINFO info  );
DWORD                  ReadMFTLCN            ( PDISKHANDLE            disk,     ULONGLONG              lcn,     ULONG       count, PVOID      buffer, FETCHPROC fetch,  PSTATUSINFO info                     );
DWORD                  ProcessBuffer         ( PDISKHANDLE            disk,     PUCHAR                 buffer,  DWORD       size,  FETCHPROC  fetch                                                          );
LPWSTR                 GetPath               ( PDISKHANDLE            disk,     int                    id                                                                                                    );
LPWSTR                 GetCompletePath       ( PDISKHANDLE            disk,     int                    id                                                                                                    );
DWORD                  FetchSearchInfo       ( PDISKHANDLE            disk,     PFILE_RECORD_HEADER    fh,      PUCHAR      data                                                                             );
PUCHAR                 FindAttribute         ( PDISKHANDLE            disk,     ATTRIBUTE_TYPE         type                                                                                                  );
PATTRIBUTE             FindAttribute         ( PFILE_RECORD_HEADER    file,     ATTRIBUTE_TYPE         type                                                                                                  );
PFILENAME_ATTRIBUTE    FindFileName          ( PFILE_RECORD_HEADER    file,     USHORT                 type                                                                                                  );
LPCWSTR                GetFileName           ( PFILE_RECORD_HEADER    file                                                                                                                                   );
PATTRIBUTE             FindNextAttribute     ( PATTRIBUTE             att,      ATTRIBUTE_TYPE         type                                                                                                  );
ULONG                  RunLength             ( PUCHAR                 run                                                                                                                                    );
LONGLONG               RunLCN                ( PUCHAR                 run                                                                                                                                    );
ULONGLONG              RunCount              ( PUCHAR                 run                                                                                                                                    );
BOOL                   FindRun               ( PNONRESIDENT_ATTRIBUTE attr,     ULONGLONG              vcn,     PULONGLONG  lcn,   PULONGLONG count                                                          );
