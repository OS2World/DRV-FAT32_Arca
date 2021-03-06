// Fat32 formatter version 1.05
// (c) Tom Thornhill 2007,2008,2009
// This software is covered by the GPL.
// By using this tool, you agree to absolve Ridgecrop of an liabilities for lost data.
// Please backup any data you value before using this tool.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <math.h>

#include "fat32c.h"

/*
28.2  CALCULATING THE VOLUME SERIAL NUMBER

For example, say a disk was formatted on 26 Dec 95 at 9:55 PM and 41.94
seconds.  DOS takes the date and time just before it writes it to the
disk.

Low order word is calculated:               Volume Serial Number is:
    Month & Day         12/26   0c1ah
    Sec & Hundrenths    41:94   295eh               3578:1d02
                                -----
                                3578h

High order word is calculated:
    Hours & Minutes     21:55   1537h
    Year                1995    07cbh
                                -----
                                1d02h
*/


/*
This is the Microsoft calculation from FATGEN

    DWORD RootDirSectors = 0;
    DWORD TmpVal1, TmpVal2, FATSz;

    TmpVal1 = DskSize - ( ReservedSecCnt + RootDirSectors);
    TmpVal2 = (256 * SecPerClus) + NumFATs;
    TmpVal2 = TmpVal2 / 2;
    FATSz = (TmpVal1 + (TmpVal2 - 1)) / TmpVal2;

    return( FatSz );
*/

USHORT _Far16 _Pascal _loadds INIT16(HMODULE hmod, ULONG flag);

// disk file open handle
extern char msg;

extern BOOL fDiskImage;

DWORD get_fat_size_sectors ( format_params *params, DWORD DskSize,
                             DWORD ReservedSecCnt, DWORD SecPerClus,
                             DWORD NumFATs, DWORD BytesPerSect )
{
    ULONGLONG   Numerator, Denominator;
    ULONGLONG   FatElementSize2;
    ULONGLONG   FatSz;

    switch (params->bFatType)
    {
        case FAT_TYPE_FAT12:
            FatElementSize2 = 3;
            break;

        case FAT_TYPE_FAT16:
            FatElementSize2 = 4;
            break;

        case FAT_TYPE_FAT32:
            FatElementSize2 = 8;
    }

    // This is based on
    // http://hjem.get2net.dk/rune_moeller_barnkob/filesystems/fat.html
    // I've made the obvious changes for FAT32
    Numerator = FatElementSize2 * ( DskSize - ReservedSecCnt ) / 2;
    Denominator = ( SecPerClus * BytesPerSect ) + ( FatElementSize2 * NumFATs ) / 2;
    FatSz = Numerator / Denominator;

    // round up to alignment value
    if (params->align == 1)
       FatSz += 1;
    else
       FatSz = ((FatSz + params->align - 1) / params->align) * params->align;

    return( (DWORD) FatSz );
}


DWORD get_spc ( DWORD ClusterSizeKB, DWORD BytesPerSect )
{
    DWORD spc = ( ClusterSizeKB * 1024 ) / BytesPerSect;
    return( (DWORD) spc );
}

DWORD get_sectors_per_cluster ( format_params *params, LONGLONG DiskSizeBytes, DWORD BytesPerSect )
{
    DWORD ret = 0x01; // 1 sector per cluster
    LONGLONG DiskSizeMB = DiskSizeBytes / ( 1024*1024 );

    switch (params->bFatType)
    {
    case FAT_TYPE_FAT12:
        // 2 MB to 4 MB - 1 KB
        if ( DiskSizeMB > 2 )
            ret = get_spc( 1, BytesPerSect );  // ret = 0x2;

        // 4 MB to 8 MB - 2 KB
        if ( DiskSizeMB > 4 )
            ret = get_spc( 2, BytesPerSect );  // ret = 0x4;

        // 8 MB to 16 MB - 4 KB
        if ( DiskSizeMB > 8 )
            ret = get_spc( 4, BytesPerSect );  // ret = 0x8;

        // 16 MB to 32 MB - 8 KB
        if ( DiskSizeMB > 16 )
            ret = get_spc( 8, BytesPerSect );  // ret = 0x10;

        // 32 MB to 64 MB - 16 KB
        if ( DiskSizeMB > 32 )
            ret = get_spc( 16, BytesPerSect );  // ret = 0x20;

        // 64 MB to 128 MB - 32 KB
        if ( DiskSizeMB > 64 )
            ret = get_spc( 32, BytesPerSect );  // ret = 0x40;

        // 128 MB to 256 MB - 64 KB
        if ( DiskSizeMB > 128 )
            ret = get_spc( 64, BytesPerSect );  // ret = 0x80;

        break;

    case FAT_TYPE_FAT16:
        // 32 MB to 64 MB - 1 KB
        if ( DiskSizeMB > 32 )
            ret = get_spc( 1, BytesPerSect );  // ret = 0x2;

        // 64 MB to 128 MB - 2 KB
        if ( DiskSizeMB > 64 )
            ret = get_spc( 2, BytesPerSect ); // ret = 0x4;

        // 128 MB to 256 MB - 4 KB
        if ( DiskSizeMB > 128 )
            ret = get_spc( 4, BytesPerSect ); // ret = 0x8;

        // Larger than 256 MB - 8 KB
        if ( DiskSizeMB > 256 )
            ret = get_spc( 8, BytesPerSect );  // ret = 0x10;

        // Larger than 512 MB - 16 KB
        if ( DiskSizeMB > 512 )
            ret = get_spc( 16, BytesPerSect );  // ret = 0x20;

        // Larger than 1,024 MB - 32 KB
        if ( DiskSizeMB > 1024 )
            ret = get_spc( 32, BytesPerSect );  // ret = 0x40;

        // Larger than 2,048 MB - 64 KB
        if ( DiskSizeMB > 2048 )
            ret = get_spc( 64, BytesPerSect );  // ret = 0x80;

        break;

    case FAT_TYPE_FAT32:
        // 64 MB to 128 MB - 1 KB
        if ( DiskSizeMB > 64 )
            ret = get_spc( 1, BytesPerSect );  // ret = 0x2;

        // 128 MB to 256 MB - 2 KB
        if ( DiskSizeMB > 128 )
            ret = get_spc( 2, BytesPerSect );  // ret = 0x4;

        // 512 MB to 8,191 MB - 4 KB
        if ( DiskSizeMB > 256 )
            ret = get_spc( 4, BytesPerSect );  // ret = 0x8;

        // 8,192 MB to 16,383 MB - 8 KB
        if ( DiskSizeMB > 8192 )
            ret = get_spc( 8, BytesPerSect ); // ret = 0x10;

        // 16,384 MB to 32,767 MB 16 KB
        if ( DiskSizeMB > 16384 )
            ret = get_spc( 16, BytesPerSect ); // ret = 0x20;

        // Larger than 32,768 MB 32 KB
        if ( DiskSizeMB > 32768 )
            ret = get_spc( 32, BytesPerSect );  // ret = 0x40;

        break;

    }

    return( ret );
}

void zero_sectors ( PCDINFO pCD, DWORD Sector, DWORD BytesPerSect, DWORD NumSects) //, DISK_GEOMETRY* pdgDrive  )
{
    BYTE *pZeroSect;
    DWORD BurstSize;
    DWORD WriteSize;
    BOOL  ret;
    DWORD dwWritten;
    //LARGE_INTEGER Start, End, Ticks, Frequency;
    ULONGLONG Start, End, Ticks, Frequency;
    double fTime;
    double fBytesTotal;
    ULONGLONG qBytesTotal, qBytesWritten;
    float fPercentWritten, fPrevPercentWritten = 0;
    //char  Str[12];

    //BurstSize = pdgDrive->SectorsPerTrack * pdgDrive->TracksPerCylinder;
    BurstSize = 64; // 32K
    //BurstSize = 128; // 64K
    //BurstSize = 8; // 4k
    //BurstSize = 1; // one sector

    //printf("zero_sectors: \nhDevice=0x%lx, Sector=%lu, \nBytesPerSect=%lu, NumSects=%lu\n",
    //        hDevice, Sector, BytesPerSect, NumSects);

    mem_alloc((void **)&pZeroSect, BytesPerSect * BurstSize);

    //SeekToSector( pCD, Sector, BytesPerSect );

    query_freq( &Frequency );
    query_time( &Start );

    qBytesTotal     = NumSects * BytesPerSect;
    qBytesWritten   = 0;
    fPercentWritten = 0;

    while ( NumSects )
    {
        if ( NumSects > BurstSize )
            WriteSize = BurstSize;
        else
            WriteSize = NumSects;

        if (! fDiskImage)
        {
           SeekToSector( pCD, Sector, BytesPerSect );
        }

        ret = WriteSector2 ( pCD, Sector, BytesPerSect, pZeroSect, WriteSize );

        if ( ret )
            die ( "Failed to write", ret );

        qBytesWritten += WriteSize * BytesPerSect;

        fPercentWritten = ( 100 * qBytesWritten ) / qBytesTotal;

        if ( fPercentWritten - fPrevPercentWritten >= 1 )
        {
          // update progress indicator if it has grown by >= 5%
          fPrevPercentWritten = fPercentWritten;
          // percent written indication
          show_progress(fPercentWritten);
        }

        NumSects -= WriteSize;
        Sector   += WriteSize;
    }

    query_time( &End );

    Ticks = End - Start;
    fTime = (double) ( Ticks ) / Frequency;

    mem_free(pZeroSect, BytesPerSect * BurstSize);

    fBytesTotal = (double) qBytesTotal;
    show_message ( "\n\nWrote %I64d bytes in %.2f seconds, %.2f Megabytes/sec\n", 0, 0, 3,
                   qBytesTotal, fTime, fBytesTotal/(fTime*1024.0*1024.0) );
}

int format_volume (char *path, format_params *params)
{
    // First open the device
    char     *p;
    DWORD    i;
    HANDLE   hDevice;
    UCHAR    PartitionType = 0x0c;
    BOOL     bGPTMode = FALSE;

    int      cbRet;
    BOOL     bRet;
    BYTE     szString[12];

    // extended BPB
    struct extbpb dp = {0, 0, 32, 2, 0, 0, 0xf8, 0, 0, 0, 0, 0, {0}};

    // Recommended values
    //DWORD ReservedSectCount = 32; !!! create cmd line parameter !!!
    //DWORD NumFATs = 2;
    DWORD BackupBootSect = 6;
    DWORD VolumeId=0; // calculated before format
    DWORD FatSz;

    // // Calculated later
    //DWORD FatSize=0;
    //DWORD BytesPerSect=0;
    //DWORD SectorsPerCluster=0;
    //DWORD TotalSectors=0;
    DWORD SystemAreaSize=0;
    DWORD UserAreaSize=0;
    ULONGLONG qTotalSectors=0;

    // structures to be written to the disk
    FAT_BOOTSECTOR32 *pFAT32BootSect;
    FAT_BOOTSECTOR16 *pFATBootSect;
    FAT_FSINFO *pFAT32FsInfo;
    DWORD *pFirstSectOfFat;
    DWORD FatSize;

    // Debug temp vars
    ULONGLONG FatNeeded, ClusterCount;
    char c;
    PCDINFO pCD;

    static char volId[12] = {0};
    char *vol = volId;

    VolumeId = get_vol_id( );

    strncpy(vol, params->volume_label, 11);
    check_vol_label(path, (char **)&vol);

    mem_alloc((void **)&pCD, sizeof(CDINFO));
    if (!pCD)
       return FALSE;
    memset(pCD, 0, sizeof (CDINFO));

    if (fDiskImage)
       strcpy(pCD->szImage, path);

    // Open drive
    OpenDrive (pCD, path);
    bGPTMode = GetDriveParams(pCD, &dp);
    LockDrive(pCD);
    BeginFormat(pCD);
    //sectorio(hDevice);

    // Specify volume label
    if (!*vol)
        vol = get_vol_label(path, vol);

    vol = strupr(vol);

    // low level format
    if (params->long_format)
    {
       LowLevelFmt(pCD, params);
    }

    if (params->bFatType == FAT_TYPE_NONE)
    {
        ULONGLONG VolSizeMB;

        // FS type is not specified
        VolSizeMB = dp.TotalSectors;
        VolSizeMB *= dp.BytesPerSect;
        VolSizeMB /= 1024*1024;

        if (VolSizeMB < 4)
            params->bFatType = FAT_TYPE_FAT12;
        else if (VolSizeMB < 2048)
            params->bFatType = FAT_TYPE_FAT16;
        else
            params->bFatType = FAT_TYPE_FAT32;
    }

    if ( params->reserved_sectors )
        dp.ReservedSectCount = params->reserved_sectors;
    else
    {
        switch (params->bFatType)
        {
        case FAT_TYPE_FAT12:
        case FAT_TYPE_FAT16:
            dp.ReservedSectCount = 1;
            break;

        case FAT_TYPE_FAT32:
            dp.ReservedSectCount = 32;
            break;

        }
    }

    dp.ReservedSectCount = max(dp.ReservedSectCount, params->align);

    // Checks on Disk Size
    qTotalSectors = dp.TotalSectors;

    if (! params->sectors_per_cluster )
        params->sectors_per_cluster = get_sectors_per_cluster( params, ((LONGLONG)dp.TotalSectors) * dp.BytesPerSect, dp.BytesPerSect );

    FatSize = get_fat_size_sectors ( params, dp.TotalSectors,
        dp.ReservedSectCount, params->sectors_per_cluster,
        dp.NumFATs, dp.BytesPerSect );

    UserAreaSize = dp.TotalSectors - dp.ReservedSectCount - ((DWORD)dp.NumFATs*FatSize);

    if (params->bFatType < FAT_TYPE_FAT32)
        {
        if (!dp.RootDirEnt)
           // number of dir entries per cluster
           dp.RootDirEnt = (params->sectors_per_cluster * dp.BytesPerSect) / sizeof(DIRENTRY);

        if (params->align <= 2048)
           dp.RootDirEnt = max(dp.RootDirEnt, params->align * dp.BytesPerSect / sizeof(DIRENTRY));

        // root dir is excluded from user area on FAT12/FAT16
        UserAreaSize -= (ULONG)dp.RootDirEnt * sizeof(DIRENTRY) / dp.BytesPerSect +
        (((ULONG)dp.RootDirEnt * sizeof(DIRENTRY) % dp.BytesPerSect) ? 1 : 0);
        }

    ClusterCount = UserAreaSize / params->sectors_per_cluster;

    if ( (ClusterCount - 2 >= FAT12_BAD_CLUSTER && params->bFatType == FAT_TYPE_FAT12) ||
         (ClusterCount - 2 >= FAT16_BAD_CLUSTER && params->bFatType == FAT_TYPE_FAT16) ||
         (ClusterCount - 2 >= FAT32_BAD_CLUSTER && params->bFatType == FAT_TYPE_FAT32) ||
         FALSE )
    {
        die ( "The specified file system does not support the current volume size.\n", -2 );
    }

    // Sanity check for a cluster count of >2^28, since the upper 4 bits of the cluster values in
    // the FAT are reserved.
    if ( params->bFatType == FAT_TYPE_FAT12 && ClusterCount > 0xFFF )
    {
        die ( "This drive has more than 2^12 clusters, \n"
              "try to specify a larger cluster size or use \n"
              "the default (i.e. don't use -cXX)\n", -3 );
    }
    else if ( params->bFatType == FAT_TYPE_FAT16 && ClusterCount > 0xFFFF )
    {
        die ( "This drive has more than 2^16 clusters, \n"
              "try to specify a larger cluster size or use \n"
              "the default (i.e. don't use -cXX)\n", -3 );
    }
    else if ( params->bFatType == FAT_TYPE_FAT32 && ClusterCount > 0x0FFFFFFF )
    {
        die ( "This drive has more than 2^28 clusters, \n"
              "try to specify a larger cluster size or use \n"
              "the default (i.e. don't use -cXX)\n", -3 );
    }

    // Sanity check - < 64K clusters means that the volume will be misdetected as FAT16
    if ( params->bFatType == FAT_TYPE_FAT32 && ClusterCount < 65536 )
    {
        die ( "FAT32 must have at least 65536 clusters, \n"
              "try to specify a smaller cluster size or \n"
              "use the default (i.e. don't use -cXX)\n", -4  );
    }

    // Sanity check, make sure the fat is big enough
    // Convert the cluster count into a Fat sector count, and check the fat size value we calculated
    // earlier is OK.
    switch (params->bFatType)
    {
        case FAT_TYPE_FAT12:
            FatNeeded = ClusterCount * 3 / 2;
            break;

        case FAT_TYPE_FAT16:
            FatNeeded = ClusterCount * 2;
            break;

        case FAT_TYPE_FAT32:
            FatNeeded = ClusterCount * 4;
    }

    FatNeeded += (dp.BytesPerSect-1);
    FatNeeded /= dp.BytesPerSect;

    if ( FatNeeded > FatSize )
    {
        die ( "This drive is too big for this version\n"
              "of fat32format, check for an upgrade.\n", -5 );
    }

    // low end limit - 65536 sectors
    if ( (params->bFatType == FAT_TYPE_FAT32) && (qTotalSectors < 65536) )
    {
        // I suspect that most FAT32 implementations would mount this volume just fine, but the
        // spec says that we shouldn't do this, so we won't
        die ( "This drive is too small for FAT32 - there must be at least 64K clusters\n", -1);
    }

    if ( qTotalSectors >= 0xffffffff )
    {
        // This is a more fundamental limitation on FAT32 - the total sector count in the root dir
        // is 32bit. With a bit of creativity, FAT32 could be extended to handle at least 2^28 clusters
        // There would need to be an extra field in the FSInfo sector, and the old sector count could
        // be set to 0xffffffff. This is non standard though, the Windows FAT driver FASTFAT.SYS won't
        // understand this. Perhaps a future version of FAT32 and FASTFAT will handle this.
        die ( "This drive is too big for FAT32 - max 2TB supported\n", -1);
    }

    if (params->bFatType < FAT_TYPE_FAT32)
    {
        mem_alloc ( (void **)&pFATBootSect, dp.BytesPerSect );

        if ( !pFATBootSect )
            die ( "Failed to allocate memory", -2 );
    }
    else if (params->bFatType == FAT_TYPE_FAT32)
    {
        mem_alloc ( (void **)&pFAT32BootSect, dp.BytesPerSect );

        if ( !pFAT32BootSect )
            die ( "Failed to allocate memory", -2 );
    }

    mem_alloc ( (void **)&pFAT32FsInfo, dp.BytesPerSect );
    mem_alloc ( (void **)&pFirstSectOfFat, dp.BytesPerSect );

    if ( !pFAT32FsInfo || !pFirstSectOfFat )
        die ( "Failed to allocate memory", -2 );

    if (params->bFatType < FAT_TYPE_FAT32)
    {
        // fill out the boot sector and fs info
        pFATBootSect->sJmpBoot[0]=0xEB;
        pFATBootSect->sJmpBoot[1]=0x3C;
        pFATBootSect->sJmpBoot[2]=0x90;
        strcpy( pFATBootSect->sOEMName, "MSWIN4.1" );
        pFATBootSect->wBytsPerSec = (WORD) dp.BytesPerSect;

        pFATBootSect->bSecPerClus = (BYTE) params->sectors_per_cluster;
        pFATBootSect->wRsvdSecCnt = (WORD) dp.ReservedSectCount;
        pFATBootSect->bNumFATs = (BYTE) dp.NumFATs;
        pFATBootSect->wRootEntCnt = dp.RootDirEnt;

        if (dp.TotalSectors < 65536L)
            pFATBootSect->wTotSec16 = dp.TotalSectors;
        else
            pFATBootSect->wTotSec16 = 0;

        pFATBootSect->bMedia = 0xF8;
        pFATBootSect->wFATSz16 = (WORD)FatSize;
        pFATBootSect->wSecPerTrk = (WORD) dp.SectorsPerTrack;
        pFATBootSect->wNumHeads = (WORD) dp.TracksPerCylinder;
        pFATBootSect->dHiddSec = (DWORD) dp.HiddenSectors;
        //dp.TotalSectors = (DWORD)  (dp.PartitionLength / dp.BytesPerSect);

        if (dp.TotalSectors >= 65536L)
            pFATBootSect->dTotSec32 = dp.TotalSectors;
        else
            pFATBootSect->dTotSec32 = 0;

        pFATBootSect->bDrvNum = 0x80;
        pFATBootSect->Reserved1 = 0;
        pFATBootSect->bBootSig = 0x29;

        pFATBootSect->dBS_VolID = VolumeId;
        strncpy ( pFATBootSect->sVolLab, vol, 11 );

        if (params->bFatType == FAT_TYPE_FAT12)
            strncpy ( pFATBootSect->sBS_FilSysType, "FAT12   ", 8 );
        else
            strncpy ( pFATBootSect->sBS_FilSysType, "FAT16   ", 8 );

        ((BYTE*)pFATBootSect)[510] = 0x55;
        ((BYTE*)pFATBootSect)[511] = 0xaa;
    }
    else if (params->bFatType == FAT_TYPE_FAT32)
    {
        // fill out the boot sector and fs info
        pFAT32BootSect->sJmpBoot[0]=0xEB;
        pFAT32BootSect->sJmpBoot[1]=0x5A;
        pFAT32BootSect->sJmpBoot[2]=0x90;
        strcpy( pFAT32BootSect->sOEMName, "MSWIN4.1" );
        pFAT32BootSect->wBytsPerSec = (WORD) dp.BytesPerSect;

        pFAT32BootSect->bSecPerClus = (BYTE) params->sectors_per_cluster;
        pFAT32BootSect->wRsvdSecCnt = (WORD) dp.ReservedSectCount;
        pFAT32BootSect->bNumFATs = (BYTE) dp.NumFATs;
        pFAT32BootSect->wRootEntCnt = 0;
        pFAT32BootSect->wTotSec16 = 0;
        pFAT32BootSect->bMedia = 0xF8;
        pFAT32BootSect->wFATSz16 = 0;
        pFAT32BootSect->wSecPerTrk = (WORD) dp.SectorsPerTrack;
        pFAT32BootSect->wNumHeads = (WORD) dp.TracksPerCylinder;
        pFAT32BootSect->dHiddSec = (DWORD) dp.HiddenSectors;
        //dp.TotalSectors = (DWORD)  (dp.PartitionLength / dp.BytesPerSect);
        pFAT32BootSect->dTotSec32 = dp.TotalSectors;

        pFAT32BootSect->dFATSz32 = (WORD)FatSize;
        pFAT32BootSect->wExtFlags = 0;
        pFAT32BootSect->wFSVer = 0;
        pFAT32BootSect->dRootClus = 2;
        pFAT32BootSect->wFSInfo = 1;
        pFAT32BootSect->wBkBootSec = (WORD) BackupBootSect;
        pFAT32BootSect->bDrvNum = 0x80;
        pFAT32BootSect->Reserved1 = 0;
        pFAT32BootSect->bBootSig = 0x29;

        pFAT32BootSect->dBS_VolID = VolumeId;
        strncpy ( pFAT32BootSect->sVolLab, vol, 11 );
        strncpy ( pFAT32BootSect->sBS_FilSysType, "FAT32   ", 8 );

        ((BYTE*)pFAT32BootSect)[510] = 0x55;
        ((BYTE*)pFAT32BootSect)[511] = 0xaa;
    }

    /* FATGEN103.DOC says "NOTE: Many FAT documents mistakenly say that this 0xAA55 signature occupies the "last 2 bytes of
    the boot sector". This statement is correct if - and only if - BPB_BytsPerSec is 512. If BPB_BytsPerSec is greater than
    512, the offsets of these signature bytes do not change (although it is perfectly OK for the last two bytes at the end
    of the boot sector to also contain this signature)."

    Windows seems to only check the bytes at offsets 510 and 511. Other OSs might check the ones at the end of the sector,
    so we'll put them there too.
    */
    if ( dp.BytesPerSect != 512 )
    {
            ((BYTE*)pFAT32BootSect)[dp.BytesPerSect-2] = 0x55;
            ((BYTE*)pFAT32BootSect)[dp.BytesPerSect-1] = 0xaa;
    }

    if (params->bFatType == FAT_TYPE_FAT32)
    {
        // FSInfo sect
        pFAT32FsInfo->dLeadSig = 0x41615252;
        pFAT32FsInfo->dStrucSig = 0x61417272;
        pFAT32FsInfo->dFree_Count = (DWORD) -1;
        pFAT32FsInfo->dNxt_Free = (DWORD) -1;
        pFAT32FsInfo->dTrailSig = 0xaa550000;
    }

    // First FAT Sector
    if (params->bFatType == FAT_TYPE_FAT12)
    {
        pFirstSectOfFat[0] = 0x00fffff8;  // two first FAT entries, no root dir
    }
    else if (params->bFatType == FAT_TYPE_FAT16)
    {
        pFirstSectOfFat[0] = 0xfffffff8;  // two first FAT entries, no root dir
    }
    else if (params->bFatType == FAT_TYPE_FAT32)
    {
        pFirstSectOfFat[0] = 0x0ffffff8;  // Reserved cluster 1 media id in low byte
        pFirstSectOfFat[1] = 0x0fffffff;  // Reserved cluster 2 EOC
        pFirstSectOfFat[2] = 0x0fffffff;  // end of cluster chain for root dir
    }

    // Write boot sector, fats
    // Sector 0 Boot Sector
    // Sector 1 FSInfo
    // Sector 2 More boot code - we write zeros here
    // Sector 3 unused
    // Sector 4 unused
    // Sector 5 unused
    // Sector 6 Backup boot sector
    // Sector 7 Backup FSInfo sector
    // Sector 8 Backup 'more boot code'
    // zero'd sectors upto ReservedSectCount
    // FAT1  ReservedSectCount to ReservedSectCount + FatSize
    // ...
    // FATn  ReservedSectCount to ReservedSectCount + FatSize
    // RootDir - allocated to cluster2

    // Now we're commited - print some info first
    show_message ( "Size: %g MB %u sectors.\n", 0, 0, 2, (double) ((dp.TotalSectors / (1024*1024)) * dp.BytesPerSect), dp.TotalSectors );
    show_message ( "%d Bytes Per Sector, Cluster size %d bytes.\n", 0, 0, 2, dp.BytesPerSect, params->sectors_per_cluster * dp.BytesPerSect );

    show_message ( "Volume label is %1.\n", 0, 1375, 1, TYPE_STRING, vol );
    sprintf(szString, "%4.4X-%4.4X", HIUSHORT(VolumeId), LOUSHORT(VolumeId));
    show_message ( "The Volume Serial Number is %1.\n", 0, 1243, 1, TYPE_STRING, szString);

    show_message ( "%d Reserved Sectors, %lu Sectors per FAT, %d fats.\n", 0, 0, 3, dp.ReservedSectCount, FatSize, dp.NumFATs );

    // Work out the Cluster count

    sprintf(szString, "%lu", (dp.TotalSectors / 1024) * dp.BytesPerSect);
    show_message ( "%1 kilobytes total disk space.\n", 0, 568, 1, TYPE_STRING, szString);

    // fix up the FSInfo sector
    pFAT32FsInfo->dFree_Count = (UserAreaSize/params->sectors_per_cluster)-1;
    pFAT32FsInfo->dNxt_Free = 3; // clusters 0-1 resered, we used cluster 2 for the root dir

    sprintf(szString, "%lu", (pFAT32FsInfo->dFree_Count / 1024) * (params->sectors_per_cluster * dp.BytesPerSect));
    show_message ( "%1 kilobytes are available.\n", 0, 537, 1, TYPE_STRING, szString);

    sprintf(szString, "%d", 0);
    show_message ( "%1 kilobytes are in bad blocks.\n", 0, 536, 1, TYPE_STRING, szString);

    show_message( "\nFormatting...\n\n", 0, 534, 1, TYPE_STRING, path );

    // Once zero_sectors has run, any data on the drive is basically lost....

    // First zero out ReservedSect + FatSize * NumFats + SectorsPerCluster
    SystemAreaSize = (dp.ReservedSectCount+((DWORD)dp.NumFATs*FatSize) + params->sectors_per_cluster);

    zero_sectors( pCD, 0, dp.BytesPerSect, SystemAreaSize);

    show_message ( "Clearing out %d sectors for \nReserved sectors, fats and root cluster.\n", 0, 0, 1, SystemAreaSize );
    show_message ( "Initialising reserved sectors and FATs.\n", 0, 0, 0 );

    if (params->bFatType < FAT_TYPE_FAT32)
    {
        // write the boot sector
        WriteSector2 ( pCD, 0, dp.BytesPerSect, pFATBootSect, 1 );
    }
    else if (params->bFatType == FAT_TYPE_FAT32)
    {
        // Now we should write the boot sector and fsinfo twice, once at 0 and once at the backup boot sect position
        for ( i=0; i<2; i++ )
        {
            int SectorStart = (i==0) ? 0 : BackupBootSect;
            WriteSector2 ( pCD, SectorStart, dp.BytesPerSect, pFAT32BootSect, 1 );
            WriteSector2 ( pCD, SectorStart+1, dp.BytesPerSect, pFAT32FsInfo, 1 );
        }
    }
       // Write the first fat sector in the right places
       for ( i = 0; i < dp.NumFATs; i++ )
       {
          int SectorStart = dp.ReservedSectCount + (i * FatSize );
          WriteSector2 ( pCD, SectorStart, dp.BytesPerSect, pFirstSectOfFat, 1 );
       }

    // The filesystem recogniser in Windows XP doesn't use the partition type - in can be
    // set to pretty much anything other Os's like Dos (still useful for Norton Ghost!) and Windows ME might,
    // so we could fix it here
    // On the other hand, I'm not sure that exposing big partitions to Windows ME/98 is a very good idea
    // There are a couple of issues here -
    // 1) WinME/98 doesn't know about 48bit LBA, so IDE drives bigger than 137GB will cause it
    //    problems. Rather than refuse to mount them, it uses 28bit LBA which wraps
    //    around, so writing to files above the 137GB boundary will erase the FAT and root dirs.
    // 2) Win98 and WinME have 16 bit scandisk tools, which you need to disable, assuming you
    //    can get third party support for 48bit LBA, or use a USB external case, most of which
    //    will let you use a 48bit LBA drive.
    //    see http://www.48bitlba.com/win98.htm for instructions

    if ( !bGPTMode && pCD->bMediumType != MEDIUM_TYPE_CDROM)
    {
        BYTE type;

        switch (params->bFatType)
        {
        case FAT_TYPE_FAT12:
            type = 0x1;
            break;

        case FAT_TYPE_FAT16:
            if (pFATBootSect->dTotSec32)
                type = 0x6;
            else
                type = 0x4;
            break;

        case FAT_TYPE_FAT32:
            type = 0xc;
            break;

        }

        SetPartType (pCD, &dp, type);
    }

    RemountMedia ( pCD );
    UnlockDrive ( pCD );
    CloseDrive ( pCD );
    fflush(stdout);

    set_vol_label (path, vol);

    // free memory
    mem_free ( (void *)pFirstSectOfFat, dp.BytesPerSect );
    mem_free ( (void *)pFAT32FsInfo, dp.BytesPerSect );

    if (params->bFatType < FAT_TYPE_FAT32)
        mem_free ( (void *)pFATBootSect, dp.BytesPerSect );
    else if (params->bFatType == FAT_TYPE_FAT32)
        mem_free ( (void *)pFAT32BootSect, dp.BytesPerSect );

    return( TRUE );
}

static void usage( char *s )
{
        show_message ( "\nFat32format, ver. 1.07, \n"
                 "see http://www.ridgecrop.demon.co.uk/fat32format.htm\n"
                 "Modified and ported to OS/2 by osFree project \n"
                 "(http://osfree.org) for ufat32.dll.\n"
                 "This software is covered by the GPL.\n"
                 "Use with care - Ridgecrop are not liable\n"
		 "for data lost using this tool.\n\n"
                 "Usage:[c:\\] %s <d>: [options]\n\n"
                 "/C:<N> with different cluster sizes:\n"
                 "    N: sectors per cluster:\n"
                 "    1 ( max size 137GB ) \n"
                 "    2 ( max size 274GB )\n"
                 "    4 ( max size 549GB )\n"
                 "    8 ( max size 1TB.  )\n"
                 "    ... \n"
                 "  128 - use 128 sectors per cluster (64K clusters)\n"
                 "/P is specified by PM frontends\n"
                 "/V:<volume label>\n"
                 "/R:<reserved sectors>\n"
                 "/M:<mountpoint> specify a mountpoint of a mounted\n"
                 "    filesystem image. If specified, it is used\n"
                 "    instead of [c:\\].\n"
                 "/A:<align>\n"
                 "/T:<tracks>\n"
                 "/N:<sectors per track>\n"
                 "/F:<volume size>\n"
                 "/L  Low-level format for CDRW/DVD-RW/floppies\n"
                 "/? this help message\n\n", 0, 0, 1, s );

        exit(1);
}

void sig_handler (int sig);

int setup_signals (void)
{
#if defined(__OS2__) && defined(__DLL__)
    // setup 16-bit signals
    INIT16(0, 0);
#endif
    if (SIG_ERR == signal(SIGABRT, sig_handler)) {
       perror("Could not set SIGABRT");
       return EXIT_FAILURE;
    }

    if (SIG_ERR == signal(SIGBREAK, sig_handler)) {
       perror("Could not set SIGBREAK");
       return EXIT_FAILURE;
    }

    if (SIG_ERR == signal(SIGINT, sig_handler)) {
       perror("Could not set SIGINT");
       return EXIT_FAILURE;
    }

   if (SIG_ERR == signal(SIGFPE, sig_handler)) {
       perror("Could not set SIGFPE");
       return EXIT_FAILURE;
    }
    if (SIG_ERR == signal(SIGSEGV, sig_handler)) {
       perror("Could not set SIGSEGV");
       return EXIT_FAILURE;
    }
    if (SIG_ERR == signal(SIGILL, sig_handler)) {
       perror("Could not set SIGILL");
       return EXIT_FAILURE;
    }

    return 0;
}

int format(int argc, char *argv[], char *envp[])
{
    format_params p;
    char cVolume;
    int  i=1;
    char path[] = "Z:";
    char *s, *t;
    char key[12], val[12];

    // set up signal handlers
    if (setup_signals())
    {
      show_message ("Error setting the signal handler!\n", 0, 0, 0);
      quit (1);
    }

    memset( &p, 0, sizeof(p) );

    if ( argc < 2 )
        {
		usage( argv[0] );
        }

    cVolume = argv[1][0];

    if (!isalpha(cVolume) || argv[1][1] != ':')
        usage( argv[0] );

#if 0
    if ( cVolume != 'f' )
        die( "Debug - only F: can be formatted\n", -9 );
#endif

    path[0] = cVolume;
    path[1] = ':';
    path[2] = '\0';

    p.align = 1;

    for ( i = 2; i < argc; i++ )
    {
        if ( !((strlen(argv[i])>=2) && ((argv[i][0] == '-')||(argv[i][0] == '/'))) )
	    usage( argv[0] );

        memset(key, 0, 12);
        memset(val, 0, 12);

        if ( strlen(argv[i]) > 3 )
        {
            // move to ':'
            s = argv[i] + 1;
            t = strchr(s, ':');

            if (t >= s)
            {
                strncpy(key, s, t - s); key[t - s] = '\0';
                strncpy(val, t + 1, strlen(t + 1)); val[strlen(t + 1)] = '\0';
            }
            else
            {
                strncpy(key, s, 12);
                val[0] = '\0';
            }
        }

        if (!stricmp(key, "FS"))
        {
            if (!strcmp(strupr(val), "FAT12"))
                p.bFatType = FAT_TYPE_FAT12;
            else if (!strcmp(strupr(val), "FAT16"))
                p.bFatType = FAT_TYPE_FAT16;
            else if (!strcmp(strupr(val), "FAT32"))
                p.bFatType = FAT_TYPE_FAT32;
            else
                p.bFatType = FAT_TYPE_NONE;

            switch (p.bFatType)
            {
                case FAT_TYPE_FAT12:
                    p.ulFatEof  = FAT12_EOF;
                    p.ulFatEof2 = FAT12_EOF2;
                    p.ulFatBad  = FAT12_BAD_CLUSTER;
                    break;

                case FAT_TYPE_FAT16:
                    p.ulFatEof  = FAT16_EOF;
                    p.ulFatEof2 = FAT16_EOF2;
                    p.ulFatBad  = FAT16_BAD_CLUSTER;
                    break;

                case FAT_TYPE_FAT32:
                    p.ulFatEof  = FAT32_EOF;
                    p.ulFatEof2 = FAT32_EOF2;
                    p.ulFatBad  = FAT32_BAD_CLUSTER;
                    break;

                case FAT_TYPE_NONE:
                    ;
            }

            // skip /fs:... parameter
            continue;
        }

        switch ( toupper(argv[i][1]) )
        {
        case 'C':
                p.sectors_per_cluster = atol(val);
                if (p.bFatType <= FAT_TYPE_FAT32)
                   {
                   if (  (p.sectors_per_cluster != 1) &&  // 512 bytes, 0.5k
                      (p.sectors_per_cluster != 2)    &&  // 1K
                      (p.sectors_per_cluster != 4)    &&  // 2K
                      (p.sectors_per_cluster != 8)    &&  // 4K
                      (p.sectors_per_cluster != 16)   &&  // 8K
                      (p.sectors_per_cluster != 32)   &&  // 16K
                      (p.sectors_per_cluster != 64)   &&  // 32K
                      (p.sectors_per_cluster != 128)      // 64K ( Microsoft say don't use 64K or bigger);
                      )
                      {
                      show_message ( "Ignoring bad cluster size %d\n", 0, 0, 1, p.sectors_per_cluster );
                      p.sectors_per_cluster = 0;
                      usage( argv[0] );
                      }
                   }
                continue;
        case 'R':
                p.reserved_sectors = atol(val);
                if (! p.reserved_sectors)
                {
                    show_message( "Ignoring bad reserved sectors %d\n", 0, 0, 1, p.reserved_sectors );
                    p.reserved_sectors = 32;
                    usage( argv[0] );
                }
                continue;
        case 'H':
        case '?':
                usage( argv[0] );
        case 'V':
	        memcpy(p.volume_label, val, 12);
                continue;
        case 'P':
                msg = TRUE;
                continue;
        case 'A': // align
                {
                char  buf[256];

                if ( strrchr(strupr(val), 'K') )
                   {
                   memcpy(buf, val, strlen(val) - 1);
                   p.align = atol(buf) * 1024;
                   }
                else if ( strrchr(strupr(val), 'M') )
                   {
                   memcpy(buf, val, strlen(val) - 1);
                   p.align = atol(buf) * 1024 * 1024;
                   }
                else
                   {
                   p.align = atol(val);
                   }
                continue;
                }
        case 'T': // tracks
                p.tracks = atol(val);
                continue;
        case 'N': // sectors per track
                p.sectors_per_track = atol(val);
                continue;
        case 'F': // volume size
                p.size = atol(val);
                continue;
        case 'L': // long format
                p.long_format = TRUE;
                continue;
        case 'M': // specify a mount point	
                strcpy(path, val);
                fDiskImage = TRUE;
                continue;
        default:
	        usage( argv[0] );
        }
    }

    if ( format_volume( path, &p ) )
       show_message( "\n\n\nFormat completed successfully.\n\n\n", 0, 635, 0 );
    else
       show_message( "The specified disk did not finish formatting.\n", 0, 528, 0 );

    return 0;
}

#ifndef __DLL__
int main(int argc, char *argv[])
{
  return format(argc, argv, NULL);
}
#endif

void show_sig_string(int s)
{
    char *str = "";
    switch (s)
    {
        case SIGABRT:
          str="SIGABRT";
          break;
        case SIGBREAK:
          str="SIGBREAK";
          break;
        case SIGINT:
          str="SIGINT";
          break;
        case SIGFPE:
          str="SIGFPE";
          break;
        case SIGSEGV:
          str="SIGSEGV";
          break;
        case SIGILL:
          str="SIGILL";
          break;
        default:
          ;
    }
    printf("Signal: %d = %s\n", s, str);
}

void sig_handler (int sig)
{
    show_sig_string(sig);
    cleanup();
}
