#include <malloc.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "fat32c.h"

#define RETURN_PARENT_DIR 0xFFFF

#define MODIFY_DIR_INSERT 0
#define MODIFY_DIR_DELETE 1
#define MODIFY_DIR_UPDATE 2
#define MODIFY_DIR_RENAME 3

#define LONGNAME_OFF         0
#define LONGNAME_OK          1
#define LONGNAME_MAKE_UNIQUE 2
#define LONGNAME_ERROR       3

typedef struct _EAOP EAOP;
typedef struct _EAOP *PEAOP;
typedef struct _FEALIST FEALIST;
typedef struct _FEALIST *PFEALIST;

BYTE rgValidChars[]="01234567890 ABCDEFGHIJKLMNOPQRSTUVWXYZ!#$%&'()-_@^`{}~";

ULONG ReadSector(PCDINFO pCD, ULONG ulSector, ULONG nSectors, PBYTE pbSector);
ULONG ReadCluster(PCDINFO pCD, ULONG ulCluster, PVOID pbCluster);
ULONG WriteSector(PCDINFO pCD, ULONG ulSector, ULONG nSectors, PBYTE pbSector);
ULONG WriteCluster(PCDINFO pCD, ULONG ulCluster, PVOID pbCluster);
ULONG ReadFatSector(PCDINFO pCD, ULONG ulSector);
ULONG WriteFatSector(PCDINFO pCD, ULONG ulSector);
ULONG GetNextCluster(PCDINFO pCD, PSHOPENINFO pSHInfo, ULONG ulCluster, BOOL fAllowBad);
BOOL  GetDiskStatus(PCDINFO pCD);
ULONG GetFreeSpace(PCDINFO pCD);
BOOL  MarkDiskStatus(PCDINFO pCD, BOOL fClean);
ULONG FindDirCluster(PCDINFO pCD, PSZ pDir, USHORT usCurDirEnd, USHORT usAttrWanted, PSZ *pDirEnd);
ULONG FindPathCluster(PCDINFO pCD, ULONG ulCluster, PSZ pszPath, PSHOPENINFO pSHInfo,
                      PDIRENTRY pDirEntry, PSZ pszFullName);
APIRET ModifyDirectory(PCDINFO pCD, ULONG ulDirCluster, PSHOPENINFO pDirSHInfo,
                       USHORT usMode, PDIRENTRY pOld, PDIRENTRY pNew,
                       PSZ pszLongNameOld, PSZ pszLongNameNew);
APIRET MarkFileEAS(PCDINFO pCD, ULONG ulDirCluster, PSHOPENINFO pDirSHInfo, PSZ pszFileName, BYTE fEAS);
USHORT GetSetFileEAS(PCDINFO pCD, USHORT usFunc, PMARKFILEEASBUF pMark);
BOOL fGetLongName(PDIRENTRY pDir, PSZ pszName, USHORT wMax, PBYTE pbCheck);
USHORT QueryUni2NLS( USHORT usPage, USHORT usChar );
BYTE GetVFATCheckSum(PDIRENTRY pDir);
APIRET MakeShortName(PCDINFO pCD, ULONG ulDirCluster, PSZ pszLongName, PSZ pszShortName);
PDIRENTRY fSetLongName(PDIRENTRY pDir, PSZ pszLongName, BYTE bCheck);
USHORT DBCSStrlen( const PSZ pszStr );
BOOL IsDBCSLead( UCHAR uch );
VOID Translate2OS2(PUSHORT pusUni, PSZ pszName, USHORT usLen);
USHORT Translate2Win(PSZ pszName, PUSHORT pusUni, USHORT usLen);
USHORT QueryNLS2Uni( USHORT usCode );
APIRET SetNextCluster(PCDINFO pCD, ULONG ulCluster, ULONG ulNext);
VOID MakeName(PDIRENTRY pDir, PSZ pszName, USHORT usMax);
USHORT GetFreeEntries(PDIRENTRY pDirBlock, ULONG ulSize);
PDIRENTRY CompactDir(PDIRENTRY pStart, ULONG ulSize, USHORT usEntriesNeeded);
VOID MarkFreeEntries(PDIRENTRY pDirBlock, ULONG ulSize);
ULONG GetFreeCluster(PCDINFO pCD);
APIRET SetFileSize(PCDINFO pCD, PFILESIZEDATA pFileSize);
USHORT RecoverChain2(PCDINFO pCD, ULONG ulCluster, PBYTE pData, USHORT cbData);
USHORT MakeDirEntry(PCDINFO pCD, ULONG ulDirCluster, PSHOPENINFO pDirSHInfo,
                    PDIRENTRY pNew, PSZ pszName);
BOOL DeleteFatChain(PCDINFO pCD, ULONG ulCluster);
BOOL UpdateFSInfo(PCDINFO pCD);
ULONG MakeFatChain(PCDINFO pCD, PSHOPENINFO pSHInfo, ULONG ulPrevCluster, ULONG ulClustersRequested, PULONG pulLast);
USHORT MakeChain(PCDINFO pCD, PSHOPENINFO pSHInfo, ULONG ulFirstCluster, ULONG ulSize);
APIRET MakeFile(PCDINFO pCD, ULONG ulDirCluster, PSHOPENINFO pDirSHInfo, PSZ pszOldFile, PSZ pszFile, PBYTE pBuf, ULONG cbBuf);
APIRET MakeDir(PCDINFO pCD, ULONG ulDirCluster, PDIRENTRY pDir, PSZ pszFile);
BOOL IsCharValid(char ch);

UCHAR GetFatType(PBOOTSECT pSect);
ULONG GetFatEntrySec(PCDINFO pCD, ULONG ulCluster);
ULONG GetFatEntriesPerSec(PCDINFO pCD);
ULONG GetFatEntry(PCDINFO pCD, ULONG ulCluster);
void  SetFatEntry(PCDINFO pCD, ULONG ulCluster, ULONG ulValue);
ULONG GetFatEntryBlock(PCDINFO pCD, ULONG ulCluster, USHORT usBlockSize);
ULONG GetFatEntriesPerBlock(PCDINFO pCD, USHORT usBlockSize);
ULONG GetFatSize(PCDINFO pCD);
ULONG GetFatEntryEx(PCDINFO pCD, PBYTE pFatStart, ULONG ulCluster, USHORT usBlockSize);
void  SetFatEntryEx(PCDINFO pCD, PBYTE pFatStart, ULONG ulCluster, ULONG ulValue, USHORT usBlockSize);

USHORT ReadBmpSector(PCDINFO pCD, ULONG ulSector);
USHORT WriteBmpSector(PCDINFO pCD, ULONG ulSector);
ULONG GetAllocBitmapSec(PCDINFO pCD, ULONG ulCluster);
BOOL GetBmpEntry(PCDINFO pCD, ULONG ulCluster);
VOID SetBmpEntry(PCDINFO pCD, ULONG ulCluster, BOOL fState);
BOOL ClusterInUse2(PCDINFO pCD, ULONG ulCluster);
BOOL MarkCluster2(PCDINFO pCD, ULONG ulCluster, BOOL fState);
BOOL fGetLongName1(PSZ pszName, USHORT wMax);
VOID MarkFreeEntries1(ULONG ulSize);
ULONG GetLastCluster(PCDINFO pCD, ULONG ulCluster);
USHORT fGetAllocBitmap(PCDINFO pCD, PULONG pulFirstCluster, PULONGLONG pullLen);
void SetSHInfo1(PCDINFO pCD, PSHOPENINFO pSHInfo);
APIRET DelFile(PCDINFO pCD, PSZ pszFilename);
void FileGetSize(PCDINFO pCD, PDIRENTRY pDirEntry, ULONG ulDirCluster, PSHOPENINFO pDirSHInfo, PSZ pszFile, PULONGLONG pullSize);
void FileSetSize(PCDINFO pCD, PDIRENTRY pDirEntry, ULONG ulDirCluster, PSHOPENINFO pDirSHInfo, PSZ pszFile, ULONGLONG ullSize);
USHORT usModifyEAS(PCDINFO pCD, ULONG ulDirCluster, PSHOPENINFO pDirSHInfo, PSZ pszFileName, PEAOP pEAOP);
USHORT usGetEAS(PCDINFO pCD, USHORT usLevel, ULONG ulDirCluster, PSHOPENINFO pDirSHInfo, PSZ pszFileName, PEAOP pEAOP);

void set_datetime(DIRENTRY *pDir);

/******************************************************************
*
******************************************************************/
BOOL IsCharValid(char ch)
{
    return (BOOL)strchr(rgValidChars, ch);
}

/******************************************************************
*
******************************************************************/
/* Get the last-character. (sbcs/dbcs) */
int lastchar(const char *string)
{
    UCHAR *s;
    unsigned int c = 0;
    int i, len = strlen(string);
    s = (UCHAR *)string;
    for(i = 0; i < len; i++)
    {
        c = *(s + i);
        if(IsDBCSLead(( UCHAR )c))
        {
            c = (c << 8) + ( unsigned int )*(s + i + 1);
            i++;
        }
    }
    return c;
}

/******************************************************************
*
******************************************************************/
BOOL GetDiskStatus(PCDINFO pCD)
{
   ULONG ulStatus;
   BOOL  fStatus;

   if (ReadFatSector(pCD, 0L))
      return FALSE;

   ulStatus = GetFatEntry(pCD, 1);

   if (ulStatus & pCD->ulFatClean)
      fStatus = TRUE;
   else
      fStatus = FALSE;

   return fStatus;
}

/******************************************************************
*
******************************************************************/
ULONG ReadFatSector(PCDINFO pCD, ULONG ulSector)
{
   ULONG  ulSec = ulSector * 3;
   USHORT usNumSec = 3;
   APIRET rc;

   // read multiples of three sectors,
   // to fit a whole number of FAT12 entries
   // (ulSector is indeed a number of 3*512
   // bytes blocks, so, it is needed to multiply by 3)

   // A 360 KB diskette has only 2 sectors per FAT
   if (pCD->BootSect.bpb.BigSectorsPerFat < 3)
      {
      if (ulSector > 0)
         return ERROR_SECTOR_NOT_FOUND;
      else
         {
         ulSec = 0;
         usNumSec = pCD->BootSect.bpb.BigSectorsPerFat;
         }
      }

   if (pCD->ulCurFATSector == ulSector)
      return 0;

   if (ulSec >= pCD->BootSect.bpb.BigSectorsPerFat)
      return ERROR_SECTOR_NOT_FOUND;

   rc = ReadSector(pCD, pCD->ulActiveFatStart + ulSec, usNumSec,
      pCD->pbFATSector);
   if (rc)
      return rc;

   pCD->ulCurFATSector = ulSector;

   return 0;
}

/******************************************************************
*
******************************************************************/
ULONG WriteFatSector(PCDINFO pCD, ULONG ulSector)
{
   USHORT usFat;
   ULONG  ulSec = ulSector * 3;
   USHORT usNumSec = 3;
   APIRET rc;

   if (pCD->ulCurFATSector != ulSector)
      return ERROR_SECTOR_NOT_FOUND;

   // read multiples of three sectors,
   // to fit a whole number of FAT12 entries
   // (ulSector is indeed a number of 3*512
   // bytes blocks, so, it is needed to multiply by 3)

   // A 360 KB diskette has only 2 sectors per FAT
   if (pCD->BootSect.bpb.BigSectorsPerFat < 3)
      {
      if (ulSector > 0)
         return ERROR_SECTOR_NOT_FOUND;
      else
         {
         ulSec = 0;
         usNumSec = pCD->BootSect.bpb.BigSectorsPerFat;
         }
      }

   if (ulSec >= pCD->BootSect.bpb.BigSectorsPerFat)
      return ERROR_SECTOR_NOT_FOUND;

   for (usFat = 0; usFat < pCD->BootSect.bpb.NumberOfFATs; usFat++)
      {
      rc = WriteSector(pCD, pCD->ulActiveFatStart + ulSec, usNumSec,
         pCD->pbFATSector);

      if (rc)
         return rc;

      if (pCD->BootSect.bpb.ExtFlags & 0x0080)
         break;

      ulSec += pCD->BootSect.bpb.BigSectorsPerFat;
      }

   return 0;
}

ULONG GetLastCluster(PCDINFO pCD, ULONG ulCluster)
{
ULONG  ulReturn = 0;

   //if (f32Parms.fMessageActive & LOG_FUNCS)
   //   Message("GetLastCluster");

   if (!ulCluster)
      return pCD->ulFatEof;

   //if (GetFatAccess(pCD, "GetLastCluster"))
   //   return pCD->ulFatEof;

   ulReturn = ulCluster;
   while (ulCluster != pCD->ulFatEof)
      {
      ulReturn = ulCluster;
      ulCluster = GetNextCluster(pCD, NULL, ulCluster, FALSE);
      //ulSector = GetFatEntrySec(pCD, ulCluster);

      //if (ulSector != pCD->ulCurFatSector)
      //   ReadFatSector(pCD, ulSector);

      //ulCluster = GetFatEntry(pCD, ulCluster);

      if (ulCluster >= pCD->ulFatEof2 && ulCluster <= pCD->ulFatEof)
         break;
      if (! ulCluster)
         {
         //Message("The file ends with NULL FAT entry!");
         break;
         }
      }
   //ReleaseFat(pCD);
   return ulReturn;
}

/******************************************************************
*
******************************************************************/
ULONG GetFreeSpace(PCDINFO pCD)
{
   ULONG ulSector = 0;
   ULONG ulCluster = 0;
   ULONG ulTotalFree;
   ULONG ulNextFree = 0;
   ULONG ulNextCluster = 0;

   ulTotalFree = 0;
   for (ulCluster = 2; ulCluster < pCD->ulTotalClusters + 2; ulCluster++)
      {
      //ulSector = GetFatEntrySec(pCD, ulCluster);
      //if (ulSector != pCD->ulCurFATSector)
      //   ReadFatSector(pCD, ulSector);
      //ulNextCluster = GetFatEntry(pCD, ulCluster);
      //if (ulNextCluster == 0)
      if (!ClusterInUse2(pCD, ulCluster))
         {
         ulTotalFree++;
         if (!ulNextFree)
            ulNextFree = ulCluster;
         }
      }

   pCD->FSInfo.ulFreeClusters = ulTotalFree;
   pCD->FSInfo.ulNextFreeCluster = ulNextFree;
   UpdateFSInfo(pCD);

   return ulTotalFree;
}

/******************************************************************
*
******************************************************************/
BOOL MarkDiskStatus(PCDINFO pCD, BOOL fClean)
{
   static BYTE bSector[SECTOR_SIZE * 8] = {0};
   USHORT usFat;
   ULONG ulNextCluster = 0;
   PBYTE pbSector;

   if (pCD->ulCurFATSector != 0)
      {
      if (ReadFatSector(pCD, 0))
         return FALSE;
      }

   ulNextCluster = GetFatEntry(pCD, 1);

   if (fClean)
      {
      ulNextCluster |= pCD->ulFatClean;
      }
   else
      {
      ulNextCluster  &= ~pCD->ulFatClean;
      }

   SetFatEntry(pCD, 1, ulNextCluster);

   if (WriteFatSector(pCD, 0))
      return FALSE;

   return TRUE;
}

/******************************************************************
*
******************************************************************/
USHORT GetSetFileEAS(PCDINFO pCD, USHORT usFunc, PMARKFILEEASBUF pMark)
{
   ULONG ulDirCluster;
   PSHOPENINFO pDirSHInfo = NULL;
   PSZ   pszFile;

   ulDirCluster = FindDirCluster(pCD,
      pMark->szFileName,
      0xFFFF,
      RETURN_PARENT_DIR,
      &pszFile);

   if (ulDirCluster == pCD->ulFatEof)
      return ERROR_PATH_NOT_FOUND;

   if (usFunc == FAT32_QUERYEAS)
      {
      ULONG ulCluster;
      DIRENTRY DirEntry;

      ulCluster = FindPathCluster(pCD, ulDirCluster, pszFile, pDirSHInfo, &DirEntry, NULL);
      if (ulCluster == pCD->ulFatEof)
         return ERROR_FILE_NOT_FOUND;
         pMark->fEAS = DirEntry.fEAS;
      return 0;
      }

   return MarkFileEAS(pCD, ulDirCluster, pDirSHInfo, pszFile, pMark->fEAS);
}


/************************************************************************
*
************************************************************************/
APIRET ModifyDirectory0(PCDINFO pCD, ULONG ulDirCluster, USHORT usMode, PDIRENTRY pOld, PDIRENTRY pNew, PSZ pszLongNameOld, PSZ pszLongNameNew)
{
   PDIRENTRY pDirectory;
   PDIRENTRY pDir2;
   PDIRENTRY pWork, pWork2;
   PDIRENTRY pMax;
   USHORT    usEntriesNeeded;
   USHORT    usFreeEntries;
   DIRENTRY  DirNew;
   ULONG     ulCluster;
   ULONG     ulPrevCluster;
   ULONG     ulNextCluster;
   PDIRENTRY pLNStart;
   APIRET    rc;
   USHORT    usClusterCount;
   BOOL      fNewCluster;
   ULONG     ulSector;
   ULONG     ulPrevSector;
   USHORT    usSectorsRead;
   ULONG     ulBytesToRead;
   ULONG     ulPrevBytesToRead = 0;
   ULONG     ulBytesRemained;

   if (usMode == MODIFY_DIR_RENAME ||
       usMode == MODIFY_DIR_INSERT)
      {
      if (!pNew || !pszLongNameNew)
         return ERROR_INVALID_PARAMETER;

      memcpy(&DirNew, pNew, sizeof (DIRENTRY));
      if ((pNew->bAttr & 0x0F) != FILE_VOLID)
         {
         rc = MakeShortName(pCD, ulDirCluster, pszLongNameNew, DirNew.bFileName);
         if (rc == LONGNAME_ERROR)
            return ERROR_FILE_EXISTS;
         set_datetime(&DirNew);
         memcpy(pNew, &DirNew, sizeof (DIRENTRY));

         if (rc == LONGNAME_OFF)
            pszLongNameNew = NULL;
         }
      else
         pszLongNameNew = NULL;

      usEntriesNeeded = 1;
      if (pszLongNameNew)
         {
#if 0
         usEntriesNeeded += strlen(pszLongNameNew) / 13 +
            (strlen(pszLongNameNew) % 13 ? 1 : 0);
#else
         usEntriesNeeded += ( DBCSStrlen( pszLongNameNew ) + 12 ) / 13;
#endif
         }
      }

   if (usMode == MODIFY_DIR_RENAME ||
       usMode == MODIFY_DIR_DELETE ||
       usMode == MODIFY_DIR_UPDATE)
      {
      if (!pOld)
         return ERROR_INVALID_PARAMETER;
      }

   pDirectory = (PDIRENTRY)malloc(2 * pCD->ulClusterSize);
   if (!pDirectory)
      return ERROR_NOT_ENOUGH_MEMORY;

   memset(pDirectory, 0, pCD->ulClusterSize);
   pDir2 =(PDIRENTRY)((PBYTE)pDirectory + pCD->ulClusterSize);
   memset(pDir2, 0, pCD->ulClusterSize);

   ulCluster = ulDirCluster;
   pLNStart = NULL;
   ulPrevCluster = pCD->ulFatEof;
   usClusterCount = 0;
   fNewCluster = FALSE;

   if (ulCluster == 1)
      {
      // root directory starting sector
      ulSector = pCD->BootSect.bpb.ReservedSectors +
         pCD->BootSect.bpb.SectorsPerFat * pCD->BootSect.bpb.NumberOfFATs;
      usSectorsRead = 0;
      ulBytesRemained = pCD->BootSect.bpb.RootDirEntries * sizeof(DIRENTRY);
      }

   while (ulCluster != pCD->ulFatEof)
      {
#ifdef CALL_YIELD
      //Yield();
#endif

      usClusterCount++;
      if (!fNewCluster)
         {
         if (ulCluster == 1)
            {
            // reading root directory on FAT12/FAT16
            rc = ReadSector(pCD, ulSector, pCD->SectorsPerCluster, (void *)pDir2);
            if (ulBytesRemained >= pCD->ulClusterSize)
               {
               ulBytesToRead = pCD->ulClusterSize;
               ulBytesRemained -= pCD->ulClusterSize;
               }
            else
               {
               ulBytesToRead = ulBytesRemained;
               ulBytesRemained = 0;
               }
            }
         else
            {
            rc = ReadCluster(pCD, ulCluster, pDir2);
            ulBytesToRead = pCD->ulClusterSize;
            }
         if (rc)
            {
            free(pDirectory);
            return rc;
            }
         }
      else
         {
         memset(pDir2, 0, pCD->ulClusterSize);
         fNewCluster = FALSE;
         }

      pMax = (PDIRENTRY)((PBYTE)pDirectory + pCD->ulClusterSize + ulBytesToRead);

      switch (usMode)
         {
         case MODIFY_DIR_RENAME :
         case MODIFY_DIR_UPDATE :
         case MODIFY_DIR_DELETE :

            /*
               Find old entry
            */

            pWork = pDir2;
            while (pWork != pMax)
               {
               if (pWork->bFileName[0] && pWork->bFileName[0] != DELETED_ENTRY)
                  {
                  if (pWork->bAttr == FILE_LONGNAME)
                     {
                     if (!pLNStart)
                        pLNStart = pWork;
                     }
                  else if ((pWork->bAttr & 0x0F) != FILE_VOLID)
                     {
                     if (!memcmp(pWork->bFileName, pOld->bFileName, 11) &&
                         pWork->wCluster     == pOld->wCluster &&
                         pWork->wClusterHigh == pOld->wClusterHigh)
                        {
                        if (!pLNStart)
                           pLNStart = pWork;
                        break;
                        }
                     pLNStart = NULL;
                     }
                  else
                     pLNStart = NULL;
                  }
               else
                  pLNStart = NULL;
               pWork++;
               }

            if (pWork != pMax)
               {
               switch (usMode)
                  {
                  case MODIFY_DIR_UPDATE:
                     memcpy(pWork, pNew, sizeof (DIRENTRY));
                     set_datetime(pWork);
                     if (ulCluster == 1)
                        // reading root directory on FAT12/FAT16
                        rc = WriteSector(pCD, ulSector, pCD->SectorsPerCluster, (void *)pDir2);
                     else
                        rc = WriteCluster(pCD, ulCluster, (void *)pDir2);
                     if (rc)
                        {
                        free(pDirectory);
                        return rc;
                        }
                     ulCluster = pCD->ulFatEof;
                     break;

                  case MODIFY_DIR_DELETE:
                  case MODIFY_DIR_RENAME:
                     pWork2 = pLNStart;
                     while (pWork2 < pWork)
                        {
                        pWork2->bFileName[0] = DELETED_ENTRY;
                        pWork2++;
                        }
                     pWork->bFileName[0] = DELETED_ENTRY;

                     /*
                        Write previous cluster if LN start lies there
                     */
                     if (ulPrevCluster != pCD->ulFatEof &&
                        pLNStart < pDir2)
                        {
                        if (ulPrevCluster == 1)
                           // reading root directory on FAT12/FAT16
                           rc = WriteSector(pCD, ulPrevSector, pCD->SectorsPerCluster, (void *)pDirectory);
                        else
                           rc = WriteCluster(pCD, ulPrevCluster, (void *)pDirectory);
                        if (rc)
                           {
                           free(pDirectory);
                           return rc;
                           }
                        }

                     /*
                        Write current cluster
                     */
                     if (ulCluster == 1)
                        // reading root directory on FAT12/FAT16
                        rc = WriteSector(pCD, ulSector, pCD->SectorsPerCluster, (void *)pDir2);
                     else
                        rc = WriteCluster(pCD, ulCluster, (void *)pDir2);
                     if (rc)
                        {
                        free(pDirectory);
                        return rc;
                        }

                     if (usMode == MODIFY_DIR_DELETE)
                        ulCluster = pCD->ulFatEof;
                     else
                        {
                        usMode = MODIFY_DIR_INSERT;
                        pszLongNameOld = pszLongNameNew;
                        ulCluster = ulDirCluster;
                        ulBytesRemained = pCD->BootSect.bpb.RootDirEntries * sizeof(DIRENTRY);
                        ulPrevCluster = pCD->ulFatEof;
                        ulPrevSector = 0;
                        usClusterCount = 0;
                        pLNStart = NULL;
                        continue;
                        }
                     break;
                  }
               }

            break;

         case MODIFY_DIR_INSERT:
            if (ulPrevCluster != pCD->ulFatEof && GetFreeEntries(pDirectory, ulPrevBytesToRead + ulBytesToRead) >= usEntriesNeeded)
               {
               BYTE bCheck = GetVFATCheckSum(&DirNew);

               pWork = (PDIRENTRY)CompactDir(pDirectory, ulPrevBytesToRead + ulBytesToRead, usEntriesNeeded);
               pWork = (PDIRENTRY)fSetLongName(pWork, pszLongNameNew, bCheck);
               memcpy(pWork, &DirNew, sizeof (DIRENTRY));
               if (ulPrevCluster == 1)
                  // reading root directory on FAT12/FAT16
                  rc = WriteSector(pCD, ulPrevSector, pCD->SectorsPerCluster, (void *)pDirectory);
               else
                  rc = WriteCluster(pCD, ulPrevCluster, (void *)pDirectory);
               if (rc)
                  {
                  free(pDirectory);
                  return rc;
                  }

               if (ulCluster == 1)
                  // reading root directory on FAT12/FAT16
                  rc = WriteSector(pCD, ulSector, pCD->SectorsPerCluster, (void *)pDir2);
               else
                  rc = WriteCluster(pCD, ulCluster, (void *)pDir2);
               if (rc)
                  {
                  free(pDirectory);
                  return rc;
                  }
               ulCluster = pCD->ulFatEof;
               break;
               }

            usFreeEntries = GetFreeEntries(pDir2, ulBytesToRead);
            if (usFreeEntries >= usEntriesNeeded)
               {
               BYTE bCheck = GetVFATCheckSum(&DirNew);

               pWork = (PDIRENTRY)CompactDir(pDir2, ulBytesToRead, usEntriesNeeded);
               pWork = (PDIRENTRY)fSetLongName(pWork, pszLongNameNew, bCheck);
               memcpy(pWork, &DirNew, sizeof (DIRENTRY));
               if (ulCluster == 1)
                  // reading root directory on FAT12/FAT16
                  rc = WriteSector(pCD, ulSector, pCD->SectorsPerCluster, (void *)pDir2);
               else
                  rc = WriteCluster(pCD, ulCluster, (void *)pDir2);
               if (rc)
                  {
                  free(pDirectory);
                  return rc;
                  }
               ulCluster = pCD->ulFatEof;
               break;
               }
            else if (usFreeEntries > 0)
               {
               MarkFreeEntries(pDir2, ulBytesToRead);
               if (ulCluster == 1)
                  // reading root directory on FAT12/FAT16
                  rc = WriteSector(pCD, ulSector, pCD->SectorsPerCluster, (void *)pDir2);
               else
                  rc = WriteCluster(pCD, ulCluster, (void *)pDir2);
               if (rc)
                  {
                  free(pDirectory);
                  return rc;
                  }
               }

            break;
         }

      if (ulCluster != pCD->ulFatEof)
         {
         ulPrevBytesToRead = ulBytesToRead;
         ulPrevCluster = ulCluster;
         ulPrevSector = ulSector;
         memset(pDirectory, 0, pCD->ulClusterSize);
         memmove(pDirectory, pDir2, ulBytesToRead);
         if (pLNStart)
            pLNStart = (PDIRENTRY)((PBYTE)pLNStart - pCD->ulClusterSize);

         if (ulCluster == 1)
            {
            // reading the root directory in case of FAT12/FAT16
            ulSector += pCD->SectorsPerCluster;
            usSectorsRead += pCD->SectorsPerCluster;
            if (usSectorsRead * pCD->BootSect.bpb.BytesPerSector >=
                pCD->BootSect.bpb.RootDirEntries * sizeof(DIRENTRY))
               // root directory ended
               ulNextCluster = 0;
            else
               ulNextCluster = 1;
            }
         else
            ulNextCluster = GetNextCluster(pCD, NULL, ulCluster, TRUE);
         if (!ulNextCluster)
            ulNextCluster = pCD->ulFatEof;
         if (ulNextCluster == pCD->ulFatEof)
            {
            if (usMode == MODIFY_DIR_UPDATE ||
                usMode == MODIFY_DIR_DELETE ||
                usMode == MODIFY_DIR_RENAME)
               {
               free(pDirectory);
               return ERROR_FILE_NOT_FOUND;
               }

            if (ulCluster == 1)
               // no expanding for root directory in case of FAT12/FAT16
               ulNextCluster = pCD->ulFatEof;
            else
               ulNextCluster = SetNextCluster(pCD, ulCluster, FAT_ASSIGN_NEW);
            if (ulNextCluster == pCD->ulFatEof)
               {
               free(pDirectory);
               return ERROR_DISK_FULL;
               }
            fNewCluster = TRUE;
            }
         ulCluster = ulNextCluster;
         }
      }

   free(pDirectory);
   return 0;
}

APIRET ModifyDirectory(PCDINFO pCD, ULONG ulDirCluster, PSHOPENINFO pDirSHInfo,
                       USHORT usMode, PDIRENTRY pOld, PDIRENTRY pNew,
                       PSZ pszLongNameOld, PSZ pszLongNameNew)
{
APIRET rc;

         rc = ModifyDirectory0(pCD, ulDirCluster, usMode, pOld, pNew,
                               pszLongNameOld, pszLongNameNew);
   return rc;
}


/******************************************************************
*
******************************************************************/
APIRET MakeShortName(PCDINFO pCD, ULONG ulDirCluster, PSZ pszLongName, PSZ pszShortName)
{
   USHORT usLongName;
   PSZ pLastDot;
   PSZ pFirstDot;
   PSZ p;
   USHORT usIndex;
   BYTE szShortName[12];
   PSZ  pszUpper;
   APIRET rc;

   usLongName = LONGNAME_OFF;
   memset(szShortName, 0x20, 11);
   szShortName[11] = 0;

   /*
      Uppercase the longname
   */
   usIndex = strlen(pszLongName) + 5;
   pszUpper = malloc(usIndex);
   if (!pszUpper)
      return LONGNAME_ERROR;

   strupr(pszLongName); // !!! @todo DBCS/Unicode

   /* Skip all leading dots */
   p = pszUpper;
   while (*p == '.')
      p++;

   pLastDot  = strrchr(p, '.');
   pFirstDot = strchr(p, '.');

   if (!pLastDot)
      pLastDot = pszUpper + strlen(pszUpper);
   if (!pFirstDot)
      pFirstDot = pLastDot;

   /*
      Is the name a valid 8.3 name ?
   */
   if ((!strcmp(pszLongName, pszUpper)) &&
      pFirstDot == pLastDot &&
      pLastDot - pszUpper <= 8 &&
      strlen(pLastDot) <= 4)
      {
      PSZ p = pszUpper;

      if (*p != '.')
         {
         while (*p)
            {
            if (*p < 128 && ! IsCharValid(*p) && *p != '.')
               break;
            p++;
            }
         }

      if (!(*p))
         {
         memset(szShortName, 0x20, sizeof szShortName);
         szShortName[11] = 0;
         memcpy(szShortName, pszUpper, pLastDot - pszUpper);
         if (*pLastDot)
            memcpy(szShortName + 8, pLastDot + 1, strlen(pLastDot + 1));

         memcpy(pszShortName, szShortName, 11);
         free(pszUpper);
         return usLongName;
         }
      }

   usLongName = LONGNAME_OK;

   if (pLastDot - pszUpper > 8)
      usLongName = LONGNAME_MAKE_UNIQUE;

   szShortName[11] = 0;


   usIndex = 0;
   p = pszUpper;
   while (usIndex < 11)
      {
      if (!(*p))
         break;

      if (usIndex == 8 && p <= pLastDot)
         {
         if (p < pLastDot)
            usLongName = LONGNAME_MAKE_UNIQUE;
         if (*pLastDot)
            p = pLastDot + 1;
         else
            break;
         }

      while (*p == 0x20)
         {
         usLongName = LONGNAME_MAKE_UNIQUE;
         p++;
         }
      if (!(*p))
         break;

      if (*p >= 128)
         {
         szShortName[usIndex++] = *p;
         }
      else if (*p == '.')
         {
         /*
            Skip all dots, if multiple dots are in the
            name create an unique name
         */
         if (p == pLastDot)
            usIndex = 8;
         else
            usLongName = LONGNAME_MAKE_UNIQUE;
         }
      else if (IsCharValid(*p))
         szShortName[usIndex++] = *p;
      else
         {
         szShortName[usIndex++] = '_';
         }
      p++;
      }
   if (strlen(p))
      usLongName = LONGNAME_MAKE_UNIQUE;

   free(pszUpper);

   p = szShortName;
   for( usIndex = 0; usIndex < 8; usIndex++ )
      if( IsDBCSLead( p[ usIndex ]))
         usIndex++;

   if( usIndex > 8 )
      p[ 7 ] = 0x20;

   p = szShortName + 8;
   for( usIndex = 0; usIndex < 3; usIndex++ )
      if( IsDBCSLead( p[ usIndex ]))
         usIndex++;

   if( usIndex > 3 )
      p[ 2 ] = 0x20;

   if (usLongName == LONGNAME_MAKE_UNIQUE)
      {
      USHORT usNum;
      BYTE   szFileName[25];
      BYTE   szNumber[18];
      ULONG ulCluster;
      PSZ p;
      USHORT usPos1, usPos2;

      for (usPos1 = 8; usPos1 > 0; usPos1--)
         if (szShortName[usPos1 - 1] != 0x20)
            break;

      for (usNum = 1; usNum < 32000; usNum++)
         {
         memset(szFileName, 0, sizeof szFileName);
         memcpy(szFileName, szShortName, 8);

         /*
            Find last blank in filename before dot.
         */

         memset(szNumber, 0, sizeof szNumber);
         itoa(usNum, szNumber, 10);

         usPos2 = 7 - (strlen(szNumber));
         if (usPos1 && usPos1 < usPos2)
            usPos2 = usPos1;

         for( usIndex = 0; usIndex < usPos2; usIndex++ )
            if( IsDBCSLead( szShortName[ usIndex ]))
               usIndex++;

         if( usIndex > usPos2 )
            usPos2--;

         strcpy(szFileName + usPos2, "~");
         strcat(szFileName, szNumber);

         if (memcmp(szShortName + 8, "   ", 3))
            {
            strcat(szFileName, ".");
            memcpy(szFileName + strlen(szFileName), szShortName + 8, 3);
            p = szFileName + strlen(szFileName);
            while (p > szFileName && *(p-1) == 0x20)
               p--;
            *p = 0;
            }
         ulCluster = FindPathCluster(pCD, ulDirCluster, szFileName, NULL, NULL, NULL);
         if (ulCluster == pCD->ulFatEof)
            break;
         }
      if (usNum < 32000)
         {
         p = strchr(szFileName, '.');
#if 0
         if (p && p - szFileName < 8)
            memcpy(szShortName, szFileName, p - szFileName);
         else
            memccpy(szShortName, szFileName, 0, 8 );
         }
#else
         if( !p )
            p = szFileName + strlen( szFileName );

         memcpy(szShortName, szFileName, p - szFileName);
         memset( szShortName + ( p - szFileName ), 0x20, 8 - ( p - szFileName ));
#endif
         }
      else
         return LONGNAME_ERROR;
      }
   memcpy(pszShortName, szShortName, 11);
   return usLongName;
}

/******************************************************************
*
******************************************************************/
PDIRENTRY fSetLongName(PDIRENTRY pDir, PSZ pszLongName, BYTE bCheck)
{
   USHORT usNeededEntries;
   PDIRENTRY pRet;
   BYTE bCurEntry;
   PLNENTRY pLN;
   USHORT usIndex;
   USHORT uniName[13];
   USHORT uniEnd[13];
   PUSHORT p;

   if (!pszLongName || !strlen(pszLongName))
      return pDir;

#if 0
   usNeededEntries = strlen(pszLongName) / 13 +
      (strlen(pszLongName) % 13 ? 1 : 0);
#else
   usNeededEntries = ( DBCSStrlen( pszLongName ) + 12 ) / 13;
#endif

   if (!usNeededEntries)
      return pDir;

   pDir += (usNeededEntries - 1);
   pRet = pDir + 1;
   pLN = (PLNENTRY)pDir;

   bCurEntry = 1;
   while (*pszLongName)
      {
#if 0
      USHORT usLen;
#endif
      pLN->bNumber = bCurEntry;
      if (DBCSStrlen(pszLongName) <= 13)
         pLN->bNumber += 0x40;
      pLN->wCluster = 0L;
      pLN->bAttr = FILE_LONGNAME;
      pLN->bReserved = 0;
      pLN->bVFATCheckSum = bCheck;

#if 0
      usLen = strlen(pszLongName);
      if (usLen > 13)
         usLen = 13;
#endif

      memset(uniEnd, 0xFF, sizeof uniEnd);
      memset(uniName, 0, sizeof uniName);

      pszLongName += Translate2Win(pszLongName, uniName, 13);

      p = uniName;
      for (usIndex = 0; usIndex < 5; usIndex ++)
         {
         pLN->usChar1[usIndex] = *p;
         if (*p == 0)
            p = uniEnd;
         p++;
         }

      for (usIndex = 0; usIndex < 6; usIndex ++)
         {
         pLN->usChar2[usIndex] = *p;
         if (*p == 0)
            p = uniEnd;
         p++;
         }

      for (usIndex = 0; usIndex < 2; usIndex ++)
         {
         pLN->usChar3[usIndex] = *p;
         if (*p == 0)
            p = uniEnd;
         p++;
         }

      pLN--;
      bCurEntry++;
      }

   return pRet;
}

/******************************************************************
*
******************************************************************/
USHORT DBCSStrlen( const PSZ pszStr )
{
   USHORT usLen;
   USHORT usIndex;
   USHORT usRet;

   usLen = strlen( pszStr );
   usRet = 0;
   for( usIndex = 0; usIndex < usLen; usIndex++ )
      {
         if( IsDBCSLead( pszStr[ usIndex ]))
            usIndex++;

         usRet++;
      }

   return usRet;
}


/******************************************************************
*
******************************************************************/
APIRET SetNextCluster(PCDINFO pCD, ULONG ulCluster, ULONG ulNext)
{
   ULONG ulNextCluster = 0;
   BOOL fUpdateFSInfo;
   ULONG ulReturn;
   APIRET rc;

   ulReturn = ulNext;
   if (ulCluster == FAT_ASSIGN_NEW)
      {
      /*
         A new seperate CHAIN is started.
      */
      ulCluster = GetFreeCluster(pCD);
      if (ulCluster == pCD->ulFatEof)
         return pCD->ulFatEof;
      ulReturn = ulCluster;
      ulNext = pCD->ulFatEof;
      }

   else if (ulNext == FAT_ASSIGN_NEW)
      {
      /*
         An existing chain is extended
      */
      ulNext = SetNextCluster(pCD, FAT_ASSIGN_NEW, pCD->ulFatEof);
      if (ulNext == pCD->ulFatEof)
         return pCD->ulFatEof;
      ulReturn = ulNext;
      }

   if (ReadFatSector(pCD, GetFatEntrySec(pCD, ulCluster)))
      {
      return pCD->ulFatEof;
      }

   fUpdateFSInfo = FALSE;
   ulNextCluster = GetFatEntry(pCD, ulCluster);
   if (ulNextCluster && !ulNext)
      {
      fUpdateFSInfo = TRUE;
      pCD->FSInfo.ulFreeClusters++;
      }
   if (ulNextCluster == 0 && ulNext)
      {
      fUpdateFSInfo = TRUE;
      pCD->FSInfo.ulNextFreeCluster = ulCluster;
      pCD->FSInfo.ulFreeClusters--;
      }

   SetFatEntry(pCD, ulCluster, ulNext);

   rc = WriteFatSector(pCD, GetFatEntrySec(pCD, ulCluster));
   if (rc)
      {
      return pCD->ulFatEof;
      }

   if (fUpdateFSInfo)
      UpdateFSInfo(pCD);

   return ulReturn;
}

/******************************************************************
*
******************************************************************/
USHORT GetFreeEntries(PDIRENTRY pDirBlock, ULONG ulSize)
{
   USHORT usCount;
   PDIRENTRY pMax;
   BOOL bLoop;

   pMax = (PDIRENTRY)((PBYTE)pDirBlock + ulSize);
   usCount = 0;
   bLoop = pMax == pDirBlock;
   while (( pDirBlock != pMax ) || bLoop )
      {
      if (!pDirBlock->bFileName[0] || pDirBlock->bFileName[0] == DELETED_ENTRY)
         usCount++;
      bLoop = FALSE;
      pDirBlock++;
      }

   return usCount;
}

/******************************************************************
*
******************************************************************/
PDIRENTRY CompactDir(PDIRENTRY pStart, ULONG ulSize, USHORT usEntriesNeeded)
{
   PDIRENTRY pTar, pMax, pFirstFree;
   USHORT usFreeEntries;
   BOOL bLoop;

   pMax = (PDIRENTRY)((PBYTE)pStart + ulSize);
   bLoop = pMax == pStart;
   pFirstFree = pMax;
   usFreeEntries = 0;
   while (( pFirstFree != pStart ) || bLoop )
      {
      if (!(pFirstFree-1)->bFileName[0])
         usFreeEntries++;
      else
         break;
      bLoop = FALSE;
      pFirstFree--;
      }

   if ((( pFirstFree == pStart ) && !bLoop ) || (pFirstFree - 1)->bAttr != FILE_LONGNAME)
      if (usFreeEntries >= usEntriesNeeded)
         return pFirstFree;

   /*
      Leaving longname entries at the end
   */
   while ((( pFirstFree != pStart ) || bLoop ) && (pFirstFree - 1)->bAttr == FILE_LONGNAME)
   {
      bLoop = FALSE;
      pFirstFree--;
   }

   usFreeEntries = 0;
   pTar = pStart;
   while ((( pStart != pFirstFree ) || bLoop ) && usFreeEntries < usEntriesNeeded)
      {
      if (pStart->bFileName[0] && pStart->bFileName[0] != DELETED_ENTRY)
         {
         if (pTar != pStart)
            *pTar = *pStart;
         pTar++;
         }
      else
         usFreeEntries++;

      bLoop = FALSE;
      pStart++;
      }
   if (pTar != pStart)
      {
#if 1
      USHORT usEntries = 0;
      PDIRENTRY p;

      for( p = pStart; ( p != pFirstFree ) /*|| bLoop */; p++ )
        {
            /*bLoop = FALSE;*/
            usEntries++;
        }
      memmove(pTar, pStart, usEntries * sizeof (DIRENTRY));
#else
      memmove(pTar, pStart, (pFirstFree - pStart) * sizeof (DIRENTRY));
#endif
      pFirstFree -= usFreeEntries;
      memset(pFirstFree, DELETED_ENTRY, usFreeEntries * sizeof (DIRENTRY));
      }

   return pFirstFree;
}

/******************************************************************
*
******************************************************************/
VOID MarkFreeEntries(PDIRENTRY pDirBlock, ULONG ulSize)
{
   PDIRENTRY pMax;

   pMax = (PDIRENTRY)((PBYTE)pDirBlock + ulSize);
   while (pDirBlock != pMax)
      {
      if (!pDirBlock->bFileName[0])
         pDirBlock->bFileName[0] = DELETED_ENTRY;
      pDirBlock++;
      }
}

/******************************************************************
*
******************************************************************/
ULONG GetFreeCluster(PCDINFO pCD)
{
   ULONG ulStartCluster;
   ULONG ulCluster;
   BOOL fStartAt2;

   if (pCD->FSInfo.ulFreeClusters == 0L)
      return pCD->ulFatEof;

   fStartAt2 = FALSE;
   //ulCluster = pCD->FSInfo.ulNextFreeCluster + 1;
   ulCluster = pCD->FSInfo.ulNextFreeCluster;
   if (!ulCluster || ulCluster >= pCD->ulTotalClusters + 2)
      {
      fStartAt2 = TRUE;
      ulCluster = 2;
      ulStartCluster = pCD->ulTotalClusters + 2;
      }
   else
      {
      ulStartCluster = ulCluster;
      }

   //while (GetNextCluster(pCD, NULL, ulCluster, TRUE))
   while (ClusterInUse2(pCD, ulCluster))
      {
      ulCluster++;
      if (fStartAt2 && ulCluster >= ulStartCluster)
         return pCD->ulFatEof;

      if (ulCluster >= pCD->ulTotalClusters + 2)
         {
         if (!fStartAt2)
            {
            ulCluster = 2;
            fStartAt2 = TRUE;
            }
         else
            {
            return pCD->ulFatEof;
            }
         }
      }
   return ulCluster;
}

/******************************************************************
*
******************************************************************/
APIRET SetFileSize(PCDINFO pCD, PFILESIZEDATA pFileSize)
{
   ULONG ulDirCluster;
   PSZ   pszFile;
   ULONG ulCluster;
   DIRENTRY DirEntry;
   DIRENTRY DirNew;
   ULONG ulClustersNeeded;
   ULONG ulClustersUsed;
   PSHOPENINFO pSHInfo = NULL;
   SHOPENINFO DirSHInfo;
   PSHOPENINFO pDirSHInfo = NULL;
   APIRET rc;

   ulDirCluster = FindDirCluster(pCD,
      pFileSize->szFileName,
      0xFFFF,
      RETURN_PARENT_DIR,
      &pszFile);

   if (ulDirCluster == pCD->ulFatEof)
      return ERROR_PATH_NOT_FOUND;

   ulCluster = FindPathCluster(pCD, ulDirCluster, pszFile,
      pDirSHInfo, &DirEntry, NULL);
   if (ulCluster == pCD->ulFatEof)
      return ERROR_FILE_NOT_FOUND;
   if (!ulCluster)
      pFileSize->ullFileSize = 0L;

   ulClustersNeeded = pFileSize->ullFileSize / pCD->ulClusterSize;
   if (pFileSize->ullFileSize % pCD->ulClusterSize)
      ulClustersNeeded++;

   if (pFileSize->ullFileSize > 0 )
      {
      ulClustersUsed = 1;
      while (ulClustersUsed < ulClustersNeeded)
         {
         ULONG ulNextCluster = GetNextCluster(pCD, pSHInfo, ulCluster, TRUE);
         if (!ulNextCluster)
            break;
         ulCluster = ulNextCluster;
         if (ulCluster == pCD->ulFatEof)
            break;
         ulClustersUsed++;
         }
      if (ulCluster == pCD->ulFatEof)
         pFileSize->ullFileSize = ulClustersUsed * pCD->ulClusterSize;
      else
         SetNextCluster(pCD, ulCluster, pCD->ulFatEof);
      }

   memcpy(&DirNew, &DirEntry, sizeof (DIRENTRY));
      FileSetSize(pCD, &DirNew, ulDirCluster, pDirSHInfo, pszFile, pFileSize->ullFileSize);

   if (!pFileSize->ullFileSize)
      {
         DirNew.wCluster = 0;
         DirNew.wClusterHigh = 0;
      }


   rc = ModifyDirectory(pCD, ulDirCluster, pDirSHInfo, MODIFY_DIR_UPDATE,
      &DirEntry, &DirNew, pszFile, pszFile);

   return rc;
}


/******************************************************************
*
******************************************************************/
ULONG MakeDir(PCDINFO pCD, ULONG ulDirCluster, PDIRENTRY pDir, PSZ pszFile)
{
ULONG ulCluster;
PVOID pbCluster;
PSHOPENINFO pDirSHInfo = NULL;
APIRET rc;

   ulCluster = SetNextCluster(pCD, FAT_ASSIGN_NEW, pCD->ulFatEof);
   if (ulCluster == pCD->ulFatEof)
      {
      ulCluster = pCD->ulFatEof;
      goto MKDIREXIT;
      }

   pbCluster = malloc(pCD->ulClusterSize);
   if (!pbCluster)
      {
      SetNextCluster( pCD, ulCluster, 0L);
      ulCluster = pCD->ulFatEof;
      goto MKDIREXIT;
      }

   memset(pbCluster, 0, pCD->ulClusterSize);

      pDir = (PDIRENTRY)pbCluster;

      pDir->wCluster = LOUSHORT(ulCluster);
      pDir->wClusterHigh = HIUSHORT(ulCluster);
      pDir->bAttr = FILE_DIRECTORY;

   rc = MakeDirEntry(pCD, ulDirCluster, pDirSHInfo, (PDIRENTRY)pbCluster, pszFile);
   if (rc)
      {
      free(pbCluster);
      ulCluster = pCD->ulFatEof;
      goto MKDIREXIT;
      }

      memset(pDir->bFileName, 0x20, 11);
      memcpy(pDir->bFileName, ".", 1);

      memcpy(pDir + 1, pDir, sizeof (DIRENTRY));
      pDir++;

      memcpy(pDir->bFileName, "..", 2);
      if (ulDirCluster == pCD->BootSect.bpb.RootDirStrtClus)
         {
         pDir->wCluster = 0;
         pDir->wClusterHigh = 0;
         }
      else
         {
         pDir->wCluster = LOUSHORT(ulDirCluster);
         pDir->wClusterHigh = HIUSHORT(ulDirCluster);
         }
      pDir->bAttr = FILE_DIRECTORY;

   rc = WriteCluster(pCD, ulCluster, pbCluster);

   if (rc)
      {
      ulCluster = pCD->ulFatEof;
      goto MKDIREXIT;
      }

MKDIREXIT:
   return ulCluster;
}

/******************************************************************
*
******************************************************************/
USHORT RecoverChain2(PCDINFO pCD, ULONG ulCluster, PBYTE pData, USHORT cbData)
{
   static ULONG ulDirCluster = 0;
   DIRENTRY DirEntry;
   BYTE     szFileName[14];
   USHORT   usNr;
   PSHOPENINFO pDirSHInfo = NULL;

   memset(&DirEntry, 0, sizeof (DIRENTRY));

   memcpy(DirEntry.bFileName, "FOUND   000", 11);
   strcpy(szFileName, "FOUND.000");
   for (usNr = 0; usNr <= 999; usNr++)
      {
      USHORT iPos = 8;
      USHORT usNum = usNr;

         while (usNum)
            {
            szFileName[iPos] = (BYTE)((usNum % 10) + '0');
            usNum /= 10;
            iPos--;
            }
         if (FindPathCluster(pCD, pCD->BootSect.bpb.RootDirStrtClus,
            szFileName, NULL, NULL, NULL) == pCD->ulFatEof)
            break;
      }
   if (usNr > 999)
      return ERROR_FILE_EXISTS;
      memcpy(DirEntry.bExtention, szFileName + 6, 3);


      set_datetime(&DirEntry);

   if (!ulDirCluster)
      ulDirCluster = MakeDir(pCD, pCD->BootSect.bpb.RootDirStrtClus, &DirEntry, szFileName);
   if (ulDirCluster == pCD->ulFatEof)
      return ERROR_DISK_FULL;

   memset(&DirEntry, 0, sizeof (DIRENTRY));
   memcpy(DirEntry.bFileName, "FILE0000CHK", 11);
   strcpy(szFileName, "FILE0000.CHK");
   for (usNr = 0; usNr < 9999; usNr++)
      {
      USHORT iPos = 7;
      USHORT usNum = usNr;

         while (usNum)
            {
            szFileName[iPos] = (BYTE)((usNum % 10) + '0');
            usNum /= 10;
            iPos--;
            }
         if (FindPathCluster(pCD, ulDirCluster,
            szFileName, NULL, NULL, NULL) == pCD->ulFatEof)
            break;
      }
   if (usNr == 9999)
      return ERROR_FILE_EXISTS;
      memcpy(DirEntry.bFileName, szFileName, 8);
   if (pData)
      strncpy(pData, szFileName, cbData);

      set_datetime(&DirEntry);
      DirEntry.wCluster = LOUSHORT(ulCluster);
      DirEntry.wClusterHigh = HIUSHORT(ulCluster);
   while (ulCluster != pCD->ulFatEof)
      {
      ULONG ulNextCluster;
         {
         ULONGLONG ullSize;
         FileGetSize(pCD, &DirEntry, ulDirCluster, NULL, szFileName, &ullSize);
         ullSize += pCD->ulClusterSize;
         FileSetSize(pCD, &DirEntry, ulDirCluster, NULL, szFileName, ullSize);
         }
      ulNextCluster = GetNextCluster(pCD, pDirSHInfo, ulCluster, TRUE);
      if (!ulNextCluster)
         {
         SetNextCluster(pCD, ulCluster, pCD->ulFatEof);
         ulCluster = pCD->ulFatEof;
         }
      else
         ulCluster = ulNextCluster;
      }

   return MakeDirEntry(pCD,
      ulDirCluster, NULL,
      &DirEntry, szFileName);
}

/******************************************************************
*
******************************************************************/
USHORT MakeDirEntry(PCDINFO pCD, ULONG ulDirCluster, PSHOPENINFO pDirSHInfo,
                    PDIRENTRY pNew, PSZ pszName)
{
      set_datetime(pNew);

   return ModifyDirectory(pCD, ulDirCluster, pDirSHInfo, MODIFY_DIR_INSERT,
      NULL, pNew, pszName, pszName);
}

/******************************************************************
*
******************************************************************/
BOOL DeleteFatChain(PCDINFO pCD, ULONG ulCluster)
{
   ULONG ulNextCluster;
   ULONG ulSector;
   ULONG ulBmpSector = 0;
   ULONG ulClustersFreed;
   APIRET rc;

   if (!ulCluster)
      return TRUE;

   ulSector = GetFatEntrySec(pCD, ulCluster);
   ReadFatSector(pCD, ulSector);

   ulClustersFreed = 0;
   while (!(ulCluster >= pCD->ulFatEof2 && ulCluster <= pCD->ulFatEof))
      {
#ifdef CALL_YIELD
      //Yield();
#endif

      if (!ulCluster || ulCluster == pCD->ulFatBad)
         {
         break;
         }
      ulSector = GetFatEntrySec(pCD, ulCluster);
      if (ulSector != pCD->ulCurFATSector)
         {
         rc = WriteFatSector(pCD, pCD->ulCurFATSector);
         if (rc)
            return FALSE;
         ReadFatSector(pCD, ulSector);
         }
      ulNextCluster = GetFatEntry(pCD, ulCluster);

      SetFatEntry(pCD, ulCluster, 0L);

      ulClustersFreed++;
      ulCluster = ulNextCluster;
      }
   rc = WriteFatSector(pCD, pCD->ulCurFATSector);
   if (rc)
      return FALSE;

   pCD->FSInfo.ulFreeClusters += ulClustersFreed;
   UpdateFSInfo(pCD);

   return TRUE;
}


/******************************************************************
*
******************************************************************/
BOOL UpdateFSInfo(PCDINFO pCD)
{
   PBYTE bSector = malloc(pCD->BootSect.bpb.BytesPerSector);

   if (pCD->BootSect.bpb.FSinfoSec == 0xFFFF)
      {
      free(bSector);
      return TRUE;
      }

   // no FSInfo sector on FAT12/FAT16
   if (pCD->bFatType != FAT_TYPE_FAT32)
      {
      free(bSector);
      return TRUE;
      }

   if (!ReadSector(pCD, pCD->BootSect.bpb.FSinfoSec, 1, bSector))
      {
      memcpy(bSector + FSINFO_OFFSET, (void *)&pCD->FSInfo, sizeof (BOOTFSINFO));
      if (!WriteSector(pCD, pCD->BootSect.bpb.FSinfoSec, 1, bSector))
         {
         free(bSector);
         return TRUE;
         }
      }

   free(bSector);
   return FALSE;
}

/******************************************************************
*
******************************************************************/
ULONG MakeFatChain(PCDINFO pCD, PSHOPENINFO pSHInfo, ULONG ulPrevCluster, ULONG ulClustersRequested, PULONG pulLast)
{
ULONG  ulCluster = 0;
ULONG  ulFirstCluster = 0;
ULONG  ulStartCluster = 0;
ULONG  ulLargestChain;
ULONG  ulLargestSize;
ULONG  ulReturn;
ULONG  ulSector = 0;
ULONG  ulNextCluster = 0;
ULONG  ulBmpSector = 0;
BOOL   fStartAt2;
BOOL   fContiguous;

   if (!ulClustersRequested)
      return pCD->ulFatEof;

   if (pCD->FSInfo.ulFreeClusters < ulClustersRequested)
      return pCD->ulFatEof;

   ulReturn = pCD->ulFatEof;
   fContiguous = TRUE;
   for (;;)
      {
      ulLargestChain = pCD->ulFatEof;
      ulLargestSize = 0;

      //ulFirstCluster = pCD->FSInfo.ulNextFreeCluster + 1;
      ulFirstCluster = pCD->FSInfo.ulNextFreeCluster;
      if (ulFirstCluster < 2 || ulFirstCluster >= pCD->ulTotalClusters + 2)
         {
         fStartAt2 = TRUE;
         ulFirstCluster = 2;
         ulStartCluster = pCD->ulTotalClusters + 3;
         }
      else
         {
         ulStartCluster = ulFirstCluster;
         fStartAt2 = FALSE;
         }

      for (;;)
         {
#ifdef CALL_YIELD
         //Yield();
#endif
         /*
            Find first free cluster
         */
            while (ulFirstCluster < pCD->ulTotalClusters + 2)
               {
               ulSector = GetFatEntrySec(pCD, ulFirstCluster);
               ulNextCluster = GetFatEntry(pCD, ulFirstCluster);
               if (ulSector != pCD->ulCurFATSector)
                  ReadFatSector(pCD, ulSector);
               if (!ulNextCluster)
                  break;
               ulFirstCluster++;
               }

         if (fStartAt2 && ulFirstCluster >= ulStartCluster)
            break;

         if (ulFirstCluster >= pCD->ulTotalClusters + 2)
            {
            if (fStartAt2)
               break;
            ulFirstCluster = 2;
            fStartAt2 = TRUE;
            continue;
            }


         /*
            Check if chain is long enough
         */

            for (ulCluster = ulFirstCluster ;
                     ulCluster < ulFirstCluster + ulClustersRequested &&
                     ulCluster < pCD->ulTotalClusters + 2;
                           ulCluster++)
               {
               ulSector = GetFatEntrySec(pCD, ulCluster);
               ulNextCluster = GetFatEntry(pCD, ulCluster);
               if (ulSector != pCD->ulCurFATSector)
                  ReadFatSector(pCD, ulSector);
               if (ulNextCluster)
                  break;
               }

         if (ulCluster != ulFirstCluster + ulClustersRequested)
            {
            /*
               Keep the largests chain found
            */
            if (ulCluster - ulFirstCluster > ulLargestSize)
               {
               ulLargestChain = ulFirstCluster;
               ulLargestSize  = ulCluster - ulFirstCluster;
               }
            ulFirstCluster = ulCluster;
            continue;
            }

         /*
            Chain found long enough
         */
         if (ulReturn == pCD->ulFatEof)
            ulReturn = ulFirstCluster;

         if (MakeChain(pCD, pSHInfo, ulFirstCluster, ulClustersRequested)) //// here
            goto MakeFatChain_Error;

         if (ulPrevCluster != pCD->ulFatEof)
            {
            if (SetNextCluster(pCD, ulPrevCluster, ulFirstCluster) == pCD->ulFatEof)
               goto MakeFatChain_Error;
            }

         if (pulLast)
            *pulLast = ulFirstCluster + ulClustersRequested - 1;
         return ulReturn;
         }

      /*
         We get here only if no free chain long enough was found!
      */
      fContiguous = FALSE;

      if (ulLargestChain != pCD->ulFatEof)
         {
         ulFirstCluster = ulLargestChain;
         if (ulReturn == pCD->ulFatEof)
            ulReturn = ulFirstCluster;

         if (MakeChain(pCD, pSHInfo, ulFirstCluster, ulLargestSize))
            goto MakeFatChain_Error;

         if (ulPrevCluster != pCD->ulFatEof)
            {
            if (SetNextCluster(pCD, ulPrevCluster, ulFirstCluster) == pCD->ulFatEof)
               goto MakeFatChain_Error;
            }

         ulPrevCluster        = ulFirstCluster + ulLargestSize - 1;
         ulClustersRequested -= ulLargestSize;
         }
      else
         break;
      }

MakeFatChain_Error:

   if (ulReturn != pCD->ulFatEof)
      DeleteFatChain(pCD, ulReturn);

   return pCD->ulFatEof;
}

/******************************************************************
*
******************************************************************/
USHORT MakeChain(PCDINFO pCD, PSHOPENINFO pSHInfo, ULONG ulFirstCluster, ULONG ulSize)
{
ULONG ulSector = 0;
ULONG ulBmpSector = 0;
ULONG ulLastCluster = 0;
ULONG ulNextCluster = 0;
ULONG  ulCluster = 0;
USHORT rc;

   ulLastCluster = ulFirstCluster + ulSize - 1;

   ulSector = GetFatEntrySec(pCD, ulFirstCluster);
   if (ulSector != pCD->ulCurFATSector)
      ReadFatSector(pCD, ulSector);

   for (ulCluster = ulFirstCluster; ulCluster < ulLastCluster; ulCluster++)
      {
      if (! pSHInfo || ! pSHInfo->fNoFatChain)
         {
         ulSector = GetFatEntrySec(pCD, ulCluster);
         if (ulSector != pCD->ulCurFATSector)
            {
            rc = WriteFatSector(pCD, pCD->ulCurFATSector);
            if (rc)
               return rc;
            ReadFatSector(pCD, ulSector);
            }
         ulNextCluster = GetFatEntry(pCD, ulCluster);
         if (ulNextCluster)
            return ERROR_SECTOR_NOT_FOUND;
         SetFatEntry(pCD, ulCluster, ulCluster + 1);
         }
      }

      ulSector = GetFatEntrySec(pCD, ulCluster);
      if (ulSector != pCD->ulCurFATSector)
         {
         rc = WriteFatSector(pCD, pCD->ulCurFATSector);
         if (rc)
            return rc;
         ReadFatSector(pCD, ulSector);
         }
      ulNextCluster = GetFatEntry(pCD, ulCluster);
      if (ulNextCluster && pCD->bFatType <= FAT_TYPE_FAT32)
         return ERROR_SECTOR_NOT_FOUND;

      SetFatEntry(pCD, ulCluster, pCD->ulFatEof);
      rc = WriteFatSector(pCD, pCD->ulCurFATSector);
      if (rc)
         return rc;

   //pCD->FSInfo.ulNextFreeCluster = ulCluster;
   pCD->FSInfo.ulNextFreeCluster = ulCluster + 1;
   pCD->FSInfo.ulFreeClusters   -= ulSize;

   return 0;
}

BOOL ClusterInUse2(PCDINFO pCD, ULONG ulCluster)
{

   if (ulCluster >= pCD->ulTotalClusters + 2)
      {
      //Message("An invalid cluster number %8.8lX was found\n", ulCluster);
      return TRUE;
      }

      {
      ULONG ulNextCluster;
      ulNextCluster = GetNextCluster(pCD, NULL, ulCluster, FALSE);
      if (ulNextCluster)
         return TRUE;
      else
         return FALSE;
      }

   return FALSE;
}

BOOL ClusterInUse(PCDINFO pCD, ULONG ulCluster)
{
ULONG ulOffset;
USHORT usShift;
BYTE bMask;

   if (ulCluster >= pCD->ulTotalClusters + 2)
      {
      printf("An invalid cluster number %8.8lX was found.\n", ulCluster);
      return TRUE;
      }

   ulCluster -= 2;
   ulOffset = ulCluster / 8;
   usShift = (USHORT)(ulCluster % 8);
   bMask = (BYTE)(1 << usShift);
   if (pCD->pFatBits[ulOffset] & bMask)
      return TRUE;
   else
      return FALSE;
}


/******************************************************************
*
******************************************************************/
APIRET MakeFile(PCDINFO pCD, ULONG ulDirCluster, PSHOPENINFO pDirSHInfo, PSZ pszOldFile, PSZ pszFile, PBYTE pBuf, ULONG cbBuf)
{
   ULONG ulClustersNeeded;
   ULONG ulCluster, ulOldCluster;
   DIRENTRY OldOldEntry, OldNewEntry, OldEntry, NewEntry;
   char pszOldFileName[256] = {0};
   char pszFileName[256] = {0};
   APIRET rc;
   char file_exists = 0;
   int i;

   if (cbBuf)
      {
      ulClustersNeeded = cbBuf / pCD->ulClusterSize +
         (cbBuf % pCD->ulClusterSize ? 1 : 0);

      if (pszOldFile)
         {
         strcpy(pszOldFileName, pszOldFile);

         ulOldCluster = FindPathCluster(pCD, ulDirCluster, pszOldFileName, pDirSHInfo, &OldOldEntry, NULL);

         if (ulOldCluster != pCD->ulFatEof)
            {
            DeleteFatChain(pCD, ulOldCluster);
            ModifyDirectory(pCD, ulDirCluster, pDirSHInfo, MODIFY_DIR_DELETE, &OldOldEntry, NULL, pszOldFile, pszOldFile);
            }
         }

      if (pszFile)
         strcpy(pszFileName, pszFile);

      ulCluster = FindPathCluster(pCD, ulDirCluster, pszFileName, pDirSHInfo, &OldEntry, NULL);

      if (ulCluster != pCD->ulFatEof)
         {
         file_exists = 1;
         memcpy(&NewEntry, &OldEntry, sizeof(DIRENTRY));
         if (!pszOldFile)
            DeleteFatChain(pCD, ulCluster);
         else
            {
            memcpy(&OldNewEntry, &OldEntry, sizeof(DIRENTRY));
            // rename chkdsk.log to chkdsk.old
            rc = ModifyDirectory(pCD, ulDirCluster, pDirSHInfo, MODIFY_DIR_RENAME,
               &OldEntry, &OldNewEntry, pszFile, pszOldFileName);
            if (rc)
               goto MakeFileEnd;
            }
         }
      else
         {
         memset(&NewEntry, 0, sizeof(DIRENTRY));
         }

      ulCluster = MakeFatChain(pCD, NULL, pCD->ulFatEof, ulClustersNeeded, NULL);

      if (ulCluster != pCD->ulFatEof)
         {
               NewEntry.wCluster = LOUSHORT(ulCluster);
               NewEntry.wClusterHigh = HIUSHORT(ulCluster);
               FileSetSize(pCD, &NewEntry, ulDirCluster, pDirSHInfo, pszFile, cbBuf);
         }
      else
         {
            rc = ERROR_DISK_FULL;
            goto MakeFileEnd;
         }
      }

   if (! file_exists || pszOldFile)
   {
      rc = MakeDirEntry(pCD, ulDirCluster, pDirSHInfo, &NewEntry, pszFileName);
   }
   else
   {
      rc = ModifyDirectory(pCD, ulDirCluster, NULL, MODIFY_DIR_UPDATE,
              &OldEntry, &NewEntry, pszFileName, pszFileName);
   }

   if (rc)
      goto MakeFileEnd;

   for (i = 0; i < ulClustersNeeded; i++)
      {
      rc = WriteCluster(pCD, ulCluster, pBuf);

      if (rc)
         goto MakeFileEnd;

      pBuf += pCD->ulClusterSize;
      ulCluster = GetNextCluster(pCD, NULL, ulCluster, TRUE);
      }

   UpdateFSInfo(pCD);

MakeFileEnd:
   return rc;
}

APIRET DelFile(PCDINFO pCD, PSZ pszFilename)
{
PSZ pszFile;
DIRENTRY DirEntry;
SHOPENINFO DirSHInfo;
PSHOPENINFO pDirSHInfo = NULL;
ULONG ulDirCluster;
ULONG ulCluster;
APIRET rc;

   ulDirCluster = FindDirCluster(pCD,
      pszFilename,
      0xffff,
      RETURN_PARENT_DIR,
      &pszFile);

   if (ulDirCluster == pCD->ulFatEof)
      {
      rc = ERROR_PATH_NOT_FOUND;
      goto DeleteFileExit;
      }

   ulCluster = FindPathCluster(pCD, ulDirCluster, pszFile, pDirSHInfo,
                               &DirEntry, NULL);
   if (ulCluster == pCD->ulFatEof)
      {
      rc = ERROR_FILE_NOT_FOUND;
      goto DeleteFileExit;
      }

   if ( DirEntry.bAttr & FILE_DIRECTORY )
      {
      rc = ERROR_ACCESS_DENIED;
      goto DeleteFileExit;
      }

   rc = ModifyDirectory(pCD, ulDirCluster, pDirSHInfo, MODIFY_DIR_DELETE,
                        &DirEntry, NULL, pszFilename, pszFilename);
   if (rc)
      goto DeleteFileExit;

   if (ulCluster)
      DeleteFatChain(pCD, ulCluster);
   rc = 0;

DeleteFileExit:
   return rc;
}

/************************************************************************
*
************************************************************************/
void FileSetSize(PCDINFO pCD, PDIRENTRY pDirEntry, ULONG ulDirCluster, PSHOPENINFO pDirSHInfo, PSZ pszFile, ULONGLONG ullSize)
{
   pDirEntry->ulFileSize = ullSize & 0xffffffff;

   //if (f32Parms.fFatPlus)
      {
      ULONGLONG ullTmp;
      ullTmp = (ULONGLONG)(pDirEntry->fEAS);
      ullSize >>= 32;
      ullTmp |= (ullSize & FILE_SIZE_MASK);
      pDirEntry->fEAS = (BYTE)ullTmp;
      }

   if ( (ullSize >> 35) )
      {
      // write EAs
      EAOP eaop;
      BYTE pBuf[sizeof(FEALIST) + 13 + 4 + sizeof(ULONGLONG)];
      PFEALIST pfealist = (PFEALIST)pBuf;
      APIRET rc;

      pDirEntry->fEAS |= FILE_SIZE_EA | FILE_HAS_EAS;

      memset(&eaop, 0, sizeof(eaop));
      memset(pfealist, 0, sizeof(pBuf));
      eaop.fpFEAList = pfealist;
      pfealist->cbList = sizeof(pBuf);
      pfealist->list[0].fEA = FEA_NEEDEA; // critical EA
      strcpy((PBYTE)((PFEA)pfealist->list + 1), "FAT_PLUS_FSZ");
      pfealist->list[0].cbName  = 13 - 1;
      pfealist->list[0].cbValue = 4 + sizeof(ULONGLONG);
      *(PUSHORT)((PBYTE)((PFEA)pfealist->list + 1) + 13) = EAT_BINARY;  // EA type
      *(PUSHORT)((PBYTE)((PFEA)pfealist->list + 1) + 15) = 8;          // length
      *(PULONGLONG)((PBYTE)((PFEA)pfealist->list + 1) + 17) = ullSize; // value

      rc = usModifyEAS(pCD, ulDirCluster, pDirSHInfo, pszFile, &eaop);
      }
}


/************************************************************************
*
************************************************************************/
void FileGetSize(PCDINFO pCD, PDIRENTRY pDirEntry, ULONG ulDirCluster, PSHOPENINFO pDirSHInfo, PSZ pszFile, PULONGLONG pullSize)
{
   *pullSize = pDirEntry->ulFileSize;

   //if (f32Parms.fFatPlus)
      {
      *pullSize |= (((ULONGLONG)(pDirEntry->fEAS) & FILE_SIZE_MASK) << 32);
      }

   if ( (pDirEntry->fEAS & FILE_SIZE_EA) )
      {
      // read EAs
      EAOP eaop;
      BYTE pBuf1[sizeof(GEALIST) + 12];
      PGEALIST pgealist = (PGEALIST)pBuf1;
      BYTE pBuf2[sizeof(FEALIST) + 13 + 4 + sizeof(ULONGLONG)];
      PFEALIST pfealist = (PFEALIST)pBuf2;
      APIRET rc;
      int i;

      memset(&eaop, 0, sizeof(eaop));
      memset(pgealist, 0, sizeof(pBuf1));
      memset(pfealist, 0, sizeof(pBuf2));
      eaop.fpGEAList = pgealist;
      eaop.fpFEAList = pfealist;
      pgealist->cbList = sizeof(GEALIST) + 12;
      pgealist->list[0].cbName = 12;
      strcpy(pgealist->list[0].szName, "FAT_PLUS_FSZ");
      pfealist->cbList = sizeof(pBuf2);

      rc = usGetEAS(pCD, FIL_QUERYEASFROMLISTL, ulDirCluster, pDirSHInfo, pszFile, &eaop);

      if (rc)
         return;

      *pullSize = *(PULONGLONG)((PBYTE)((PFEA)pfealist->list + 1) + 17);
      }
}
