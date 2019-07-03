#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#define INCL_DOS
#define INCL_DOSDEVIOCTL
#define INCL_DOSDEVICES
#define INCL_DOSERRORS

#include "os2.h"
#include "portable.h"
#include "fat32ifs.h"

USHORT NameHash(USHORT *pszFilename, int NameLen);
USHORT GetChkSum16(const UCHAR *data, int bytes);
ULONG  GetChkSum32(const UCHAR *data, int bytes);

VOID MarkFreeEntries(PDIRENTRY pDirBlock, ULONG ulSize);
PDIRENTRY fSetLongName(PDIRENTRY pDir, PSZ pszName, BYTE bCheck);
PDIRENTRY CompactDir(PDIRENTRY pStart, ULONG ulSize, USHORT usNeededEntries);
USHORT GetFreeEntries(PDIRENTRY pDirBlock, ULONG ulSize);
USHORT DBCSStrlen( const PSZ pszStr );

USHORT GetBuf1Access(PVOLINFO pVolInfo, PSZ pszName);
VOID   ReleaseBuf1(PVOLINFO pVolInfo);
USHORT GetBuf2Access(PVOLINFO pVolInfo, PSZ pszName);
VOID   ReleaseBuf2(PVOLINFO pVolInfo);
USHORT GetBuf3Access(PVOLINFO pVolInfo, PSZ pszName);
VOID   ReleaseBuf3(PVOLINFO pVolInfo);


/******************************************************************
*
******************************************************************/
void FileSetSize(PVOLINFO pVolInfo, PDIRENTRY pDirEntry, ULONG ulDirCluster,
                 PSHOPENINFO pDirSHInfo, PSZ pszFile, ULONGLONG ullSize)
{
   pDirEntry->ulFileSize = ullSize & 0xffffffff;

   if (f32Parms.fFatPlus)
      {
      ULONGLONG ullTmp;
      ullTmp = (ULONGLONG)(pDirEntry->fEAS);
      ullTmp |= ((ullSize >> 32) & FILE_SIZE_MASK);
      pDirEntry->fEAS = (BYTE)ullTmp;
      pDirEntry->fEAS &= ~FILE_SIZE_EA;
      }

   if ( f32Parms.fFatPlus && f32Parms.fEAS && (ullSize >> 35) )
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
      *(PUSHORT)((PBYTE)((PFEA)pfealist->list + 1) + 13) = EAT_BINARY; // EA type
      *(PUSHORT)((PBYTE)((PFEA)pfealist->list + 1) + 15) = 8;          // length
      *(PULONGLONG)((PBYTE)((PFEA)pfealist->list + 1) + 17) = ullSize; // value

      rc = usModifyEAS(pVolInfo, ulDirCluster, pDirSHInfo, pszFile, &eaop);
      }
}


/******************************************************************
*
******************************************************************/
void FileGetSize(PVOLINFO pVolInfo, PDIRENTRY pDirEntry, ULONG ulDirCluster,
                 PSHOPENINFO pDirSHInfo, PSZ pszFile, PULONGLONG pullSize)
{
   *pullSize = pDirEntry->ulFileSize;

   if (f32Parms.fFatPlus)
      {
      *pullSize |= (((ULONGLONG)(pDirEntry->fEAS) & FILE_SIZE_MASK) << 32);
      }

   if ( f32Parms.fFatPlus && f32Parms.fEAS && (pDirEntry->fEAS & FILE_SIZE_EA) )
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

      rc = usGetEAS(pVolInfo, FIL_QUERYEASFROMLISTL, ulDirCluster, pDirSHInfo, pszFile, &eaop);

      if (rc)
         return;

      *pullSize = *(PULONGLONG)((PBYTE)((PFEA)pfealist->list + 1) + 17);
      }
}


/******************************************************************
*
******************************************************************/
ULONG FindDirCluster(PVOLINFO pVolInfo,
   struct cdfsi far * pcdfsi,       /* pcdfsi   */
   struct cdfsd far * pcdfsd,       /* pcdfsd   */
   PSZ pDir,
   USHORT usCurDirEnd,
   USHORT usAttrWanted,
   PSZ *pDirEnd)
{
//BYTE   szDir[FAT32MAXPATH];
PSZ    szDir;
ULONG  ulCluster;
ULONG  ulCluster2;
PDIRENTRY pDirEntry;
PSZ    pszPath = NULL;
PSZ    p;

   MessageL(LOG_FUNCS, "FindDirCluster%m for %s, CurDirEnd %u, AttrWanted %u",
            0x0031, pDir, usCurDirEnd, usAttrWanted );

   pszPath = malloc(CCHMAXPATHCOMP);

   if (! pszPath)
      {
      return ERROR_NOT_ENOUGH_MEMORY;
      }

   if (! pVolInfo->hVBP)
      {
      // FAT FS image mounted at some directory
      if (pDir[1] == ':')
         {
         // absolute path starting from a drive letter
         pDir += strlen(pVolInfo->pbMntPoint);
         }
      else if (pDir[0] == '\\')
         {
         // absolute path starting from backslash
         pDir++;
         }
      else
         {
         // relative path
         strcpy(pszPath, pcdfsi->cdi_curdir);
         strcat(pszPath, "\\");
         strcat(pszPath, pDir);
         pDir = pszPath + strlen(pVolInfo->pbMntPoint);
         }
      }

   if (pcdfsi && pVolInfo->hVBP &&
      (pcdfsi->cdi_flags & CDI_ISVALID) &&
      !(pcdfsi->cdi_flags & CDI_ISROOT) &&
      usCurDirEnd != 0xFFFF)
      {
      pDir += usCurDirEnd;
      ulCluster = *(PULONG)pcdfsd;
      }
   else
      {
      ulCluster = pVolInfo->BootSect.bpb.RootDirStrtClus;
      if (strlen(pDir) >= 2)
         {
         if (pDir[1] == ':')
            pDir += 2;
         }
      }

   pDirEntry = (PDIRENTRY)malloc((size_t)sizeof(DIRENTRY));
   if (!pDirEntry)
      {
      free(pszPath);
      return ERROR_NOT_ENOUGH_MEMORY;
      }

   if (*pDir == '\\')
      pDir++;

   p = strrchr(pDir, '\\');
   if (!p)
      p = pDir;
   szDir = (PSZ)malloc((size_t)FAT32MAXPATH);
   if (!szDir)
      {
      free(pszPath);
      free(pDirEntry);
      return pVolInfo->ulFatEof;
      }
   //memset(szDir, 0, sizeof szDir);
   memset(szDir, 0, FAT32MAXPATH);
   memcpy(szDir, pDir, p - pDir);
   if (*p && p != pDir)
      pDir = p + 1;
   else
      pDir = p;

   if (pDirEnd)
      *pDirEnd = pDir;

   ulCluster = FindPathCluster(pVolInfo, ulCluster, szDir, NULL, pDirEntry, NULL);
   if (ulCluster == pVolInfo->ulFatEof)
      {
      MessageL(LOG_FUNCS, "FindDirCluster%m for '%s', not found", 0x4002, szDir);
      free(pszPath);
      free(szDir);
      free(pDirEntry);
      return pVolInfo->ulFatEof;
      }
   if ( ulCluster != pVolInfo->ulFatEof &&
       !(pDirEntry->bAttr & FILE_DIRECTORY) )
      {
      MessageL(LOG_FUNCS, "FindDirCluster%m for '%s', not a directory", 0x4003, szDir);
      free(pszPath);
      free(szDir);
      free(pDirEntry);
      return pVolInfo->ulFatEof;
      }

   if (*pDir)
      {
      if (usAttrWanted != RETURN_PARENT_DIR && !strpbrk(pDir, "?*"))
         {
         ulCluster2 = FindPathCluster(pVolInfo, ulCluster, pDir, NULL, pDirEntry, NULL);
         if ( ulCluster2 != pVolInfo->ulFatEof &&
             ( (pDirEntry->bAttr & usAttrWanted) == usAttrWanted ) )
            {
            if (pDirEnd)
               *pDirEnd = pDir + strlen(pDir);
            free(pszPath);
            free(szDir);
            free(pDirEntry);
            return ulCluster2;
            }
         }
      }

   free(pszPath);
   free(szDir);
   free(pDirEntry);
   return ulCluster;
}

#define MODE_START  0
#define MODE_RETURN 1
#define MODE_SCAN   2

/******************************************************************
*
******************************************************************/
ULONG FindPathCluster(PVOLINFO pVolInfo, ULONG ulCluster, PSZ pszPath, PSHOPENINFO pSHInfo,
                      PDIRENTRY pDirEntry, PSZ pszFullName)
{
PSZ  pszLongName;
PSZ  pszPart;
PSZ  p;
BOOL fFound;
USHORT usMode;
PROCINFO ProcInfo;
USHORT usDirEntries = 0;
USHORT usMaxDirEntries = (USHORT)(pVolInfo->ulBlockSize / sizeof(DIRENTRY));
      // FAT12/FAT16/FAT32 case
      BYTE szShortName[13];
      PDIRENTRY pDir;
      PDIRENTRY pDirStart;
      PDIRENTRY pDirEnd;
      BYTE   bCheck;
      ULONG  ulSector;
      USHORT usSectorsRead;
      USHORT usSectorsPerBlock;

      MessageL(LOG_FUNCS, "FindPathCluster%m for %s, dircluster %lu", 0x0032, pszPath, ulCluster);

      if (ulCluster == 1)
         {
         // root directory starting sector
         ulSector = pVolInfo->BootSect.bpb.ReservedSectors +
            (ULONG)pVolInfo->BootSect.bpb.SectorsPerFat * pVolInfo->BootSect.bpb.NumberOfFATs;
         usSectorsPerBlock = (ULONG)pVolInfo->SectorsPerCluster /
            ((ULONG)pVolInfo->ulClusterSize / pVolInfo->ulBlockSize);
         usSectorsRead = 0;
         }

      if (pDirEntry)
         {
         memset(pDirEntry, 0, sizeof (DIRENTRY));
         pDirEntry->bAttr = FILE_DIRECTORY;
         }
      if (pszFullName)
         {
         memset(pszFullName, 0, FAT32MAXPATH);
         if (ulCluster == pVolInfo->BootSect.bpb.RootDirStrtClus)
            {
            pszFullName[0] = (BYTE)(pVolInfo->bDrive + 'A');
            pszFullName[1] = ':';
            pszFullName[2] = '\\';
            }
         }

      if (strlen(pszPath) >= 2)
         {
         if (pszPath[1] == ':')
            pszPath += 2;
         }

      pDirStart = malloc((size_t)pVolInfo->ulBlockSize);
      if (!pDirStart)
         {
         Message("FAT32: Not enough memory for cluster in FindPathCluster");
         return pVolInfo->ulFatEof;
         }
      pszLongName = malloc((size_t)FAT32MAXPATHCOMP * 2);
      if (!pszLongName)
         {
         Message("FAT32: Not enough memory for buffers in FindPathCluster");
         free(pDirStart);
         return pVolInfo->ulFatEof;
         }
      memset(pszLongName, 0, FAT32MAXPATHCOMP * 2);
      pszPart = pszLongName + FAT32MAXPATHCOMP;

      usMode = MODE_SCAN;
      GetProcInfo(&ProcInfo, sizeof ProcInfo);
      /*
         Allow EA files to be found!
      */
      if (ProcInfo.usPdb && f32Parms.fEAS && IsEASFile(pszPath))
         ProcInfo.usPdb = 0;

      while (usMode != MODE_RETURN && ulCluster != pVolInfo->ulFatEof)
         {
         usMode = MODE_SCAN;

         if (*pszPath == '\\')
            pszPath++;

         if (!strlen(pszPath))
            break;

         p = strchr(pszPath, '\\');
         if (!p)
            p = pszPath + strlen(pszPath);

         memset(pszPart, 0, FAT32MAXPATHCOMP);
         if (p - pszPath > FAT32MAXPATHCOMP - 1)
            {
            free(pDirStart);
            free(pszLongName);
            return pVolInfo->ulFatEof;
            }

         memcpy(pszPart, pszPath, p - pszPath);
         pszPath = p;

         memset(pszLongName, 0, FAT32MAXPATHCOMP);

         fFound = FALSE;
         while (usMode == MODE_SCAN && ulCluster != pVolInfo->ulFatEof)
            {
            ULONG ulBlock;
            for (ulBlock = 0; ulBlock < pVolInfo->ulClusterSize / pVolInfo->ulBlockSize; ulBlock++)
               {
               if (ulCluster == 1)
                  // reading root directory on FAT12/FAT16
                  ReadSector(pVolInfo, ulSector + ulBlock * usSectorsPerBlock, usSectorsPerBlock, (void *)pDirStart, 0);
               else
                  ReadBlock(pVolInfo, ulCluster, ulBlock, pDirStart, 0);
               pDir    = pDirStart;
               pDirEnd = (PDIRENTRY)((PBYTE)pDirStart + pVolInfo->ulBlockSize);

#ifdef CALL_YIELD
               Yield();
#endif

               usDirEntries = 0;
               //while (usMode == MODE_SCAN && pDir < pDirEnd)
               while (usMode == MODE_SCAN && usDirEntries < usMaxDirEntries)
                  {
                  if (pDir->bAttr == FILE_LONGNAME)
                     {
                     fGetLongName(pDir, pszLongName, FAT32MAXPATHCOMP, &bCheck);
                     }
                  else if ((pDir->bAttr & 0x0F) != FILE_VOLID)
                     {
                     MakeName(pDir, szShortName, sizeof szShortName);
                     FSH_UPPERCASE(szShortName, sizeof szShortName, szShortName);
                     if (strlen(pszLongName) && bCheck != GetVFATCheckSum(pDir))
                        memset(pszLongName, 0, FAT32MAXPATHCOMP);

                      /* support for the FAT32 variation of WinNT family */
                      if( !*pszLongName && HAS_WINNT_EXT( pDir->fEAS ))
                      {
                          PBYTE pDot;

                          MakeName( pDir, pszLongName, sizeof( pszLongName ));
                          pDot = strchr( pszLongName, '.' );

                          if( HAS_WINNT_EXT_NAME( pDir->fEAS )) /* name part is lower case */
                          {
                              if( pDot )
                                  *pDot = 0;

                              strlwr( pszLongName );

                              if( pDot )
                                  *pDot = '.';
                          }

                          if( pDot && HAS_WINNT_EXT_EXT( pDir->fEAS )) /* ext part is lower case */
                              strlwr( pDot + 1 );
                      }

                     if (!strlen(pszLongName))
                        strcpy(pszLongName, szShortName);

                     if (( strlen(pszLongName) && !stricmp(pszPart, pszLongName)) ||
                         !stricmp( pszPart, szShortName ))
                        {
                          if( pszFullName )
                              strcat( pszFullName, pszLongName );
                          fFound = TRUE;
                        }

                     if (fFound)
                        {
                        ulCluster = (ULONG)pDir->wClusterHigh * 0x10000L + pDir->wCluster;
                        ulCluster &= pVolInfo->ulFatEof;
                        if (strlen(pszPath))
                           {
                           if (pDir->bAttr & FILE_DIRECTORY)
                              {
                              if (pszFullName)
                                 strcat(pszFullName, "\\");
                              usMode = MODE_START;
                              break;
                              }
                           ulCluster = pVolInfo->ulFatEof;
                           }
                        else
                           {
                           if (pDirEntry)
                              memcpy(pDirEntry, pDir, sizeof (DIRENTRY));
                           }
                        usMode = MODE_RETURN;
                        break;
                        }
                     memset(pszLongName, 0, FAT32MAXPATHCOMP);
                     }
                  pDir++;
                  usDirEntries++;
                  if (ulCluster == 1 && usDirEntries > pVolInfo->BootSect.bpb.RootDirEntries)
                     break;
                  }
               if (usMode != MODE_SCAN)
                  break;
               }
            if (usMode != MODE_SCAN)
               break;
            if (ulCluster == 1)
               {
               // reading the root directory in case of FAT12/FAT16
               ulSector += pVolInfo->SectorsPerCluster;
               usSectorsRead += pVolInfo->SectorsPerCluster;
               if ((ULONG)usSectorsRead * pVolInfo->BootSect.bpb.BytesPerSector >=
                   (ULONG)pVolInfo->BootSect.bpb.RootDirEntries * sizeof(DIRENTRY))
                  // root directory ended
                  ulCluster = 0;
               }
            else
               {
               ulCluster = GetNextCluster(pVolInfo, NULL, ulCluster);
               }
            if (!ulCluster)
               ulCluster = pVolInfo->ulFatEof;
            }
         }
      free(pDirStart);
      free(pszLongName);
      if (ulCluster != pVolInfo->ulFatEof)
         MessageL(LOG_FUNCS, "FindPathCluster%m for %s found cluster %ld", 0x8032, pszPath, ulCluster);
      else
         MessageL(LOG_FUNCS, "FindPathCluster%m for %s returned EOF", 0x8033, pszPath);
      return ulCluster;
}


USHORT TranslateName(PVOLINFO pVolInfo, ULONG ulDirCluster, PSHOPENINFO pDirSHInfo, PSZ pszPath, PSZ pszTarget, USHORT usTranslate)
{
BYTE szShortName[13];
PSZ  pszLongName;
PSZ  pszUpperName;
PSZ  pszUpperPart;
PSZ  pszPart;
PSZ  p;
/* PSZ  pTar = pszTarget; */
PDIRENTRY pDir;
PDIRENTRY pDirStart;
PDIRENTRY pDirEnd;
BOOL fFound;
USHORT usMode;
BYTE   bCheck;
ULONG  ulCluster;
ULONG  ulSector;
USHORT usSectorsRead;
USHORT usSectorsPerBlock;
ULONG  ulDirEntries = 0;
USHORT usNumSecondary;
USHORT usFileAttr;
ULONG  ulRet;

   MessageL(LOG_FUNCS, "TranslateName%m: %s", 0x0034, pszPath);

   memset(pszTarget, 0, FAT32MAXPATH);
   if (strlen(pszPath) >= 2)
      {
      if (pszPath[1] == ':')
         {
         memcpy(pszTarget, pszPath, 2);
         pszTarget += 2;
         pszPath += 2;
         }
      }

   pDirStart = malloc((size_t)pVolInfo->ulBlockSize);
   if (!pDirStart)
      {
      Message("FAT32: Not enough memory for cluster in TranslateName");
      return ERROR_NOT_ENOUGH_MEMORY;
      }
   pszLongName = malloc((size_t)FAT32MAXPATHCOMP * 4);
   if (!pszLongName)
      {
      Message("FAT32: Not enough memory for buffers in TranslateName");
      free(pDirStart);
      return ERROR_NOT_ENOUGH_MEMORY;
      }
   memset(pszLongName, 0, FAT32MAXPATHCOMP * 4);

   pszPart      = pszLongName + FAT32MAXPATHCOMP;
   pszUpperPart = pszPart + FAT32MAXPATHCOMP;
   pszUpperName = pszUpperPart + FAT32MAXPATHCOMP;

   if (usTranslate == TRANSLATE_AUTO)
      {
      if (IsDosSession())
         usTranslate = TRANSLATE_SHORT_TO_LONG;
      else
         usTranslate = TRANSLATE_LONG_TO_SHORT;
      }

   if (ulDirCluster)
      ulCluster = ulDirCluster;
   else
      ulCluster = pVolInfo->BootSect.bpb.RootDirStrtClus;

   if (ulCluster == 1)
      {
      // root directory starting sector
      ulSector = pVolInfo->BootSect.bpb.ReservedSectors +
         (ULONG)pVolInfo->BootSect.bpb.SectorsPerFat * pVolInfo->BootSect.bpb.NumberOfFATs;
      usSectorsPerBlock = (ULONG)pVolInfo->SectorsPerCluster /
         ((ULONG)pVolInfo->ulClusterSize / pVolInfo->ulBlockSize);
      usSectorsRead = 0;
      }

   usMode = MODE_SCAN;
   while (usMode != MODE_RETURN && ulCluster != pVolInfo->ulFatEof)
      {
      usMode = MODE_SCAN;

      if (*pszPath == '\\')
         *pszTarget++ = *pszPath++;

      if (!strlen(pszPath))
         break;

      p = strchr(pszPath, '\\');
      if (!p)
         p = pszPath + strlen(pszPath);

      if (p - pszPath > FAT32MAXPATHCOMP - 1)
         {
         free(pDirStart);
         free(pszLongName);
         return ERROR_BUFFER_OVERFLOW;
         }

      memset(pszPart, 0, FAT32MAXPATHCOMP);
      memcpy(pszPart, pszPath, p - pszPath);
      FSH_UPPERCASE(pszPart, FAT32MAXPATHCOMP, pszUpperPart);
      pszPath = p;

      memset(pszLongName, 0, FAT32MAXPATHCOMP);
      fFound = FALSE;
      while (usMode == MODE_SCAN && ulCluster != pVolInfo->ulFatEof)
         {
         ULONG ulBlock;
         for (ulBlock = 0; ulBlock < pVolInfo->ulClusterSize / pVolInfo->ulBlockSize; ulBlock++)
            {
            if (ulCluster == 1)
               // reading root directory on FAT12/FAT16
               ReadSector(pVolInfo, ulSector + ulBlock * usSectorsPerBlock, usSectorsPerBlock, (void *)pDirStart, 0);
            else
               ReadBlock(pVolInfo, ulCluster, ulBlock, pDirStart, 0);
            pDir    = pDirStart;
            pDirEnd = (PDIRENTRY)((PBYTE)pDirStart + pVolInfo->ulBlockSize);

#ifdef CALL_YIELD
            Yield();
#endif

            while (usMode == MODE_SCAN && pDir < pDirEnd)
               {
                  if (pDir->bAttr == FILE_LONGNAME)
                     {
                     fGetLongName(pDir, pszLongName, FAT32MAXPATHCOMP, &bCheck);
                     }
                  else if ((pDir->bAttr & 0x0F) != FILE_VOLID)
                     {
                     MakeName(pDir, szShortName, sizeof szShortName);
                     FSH_UPPERCASE(szShortName, sizeof szShortName, szShortName);

                     if (bCheck != GetVFATCheckSum(pDir))
                        memset(pszLongName, 0, FAT32MAXPATHCOMP);

                     /* support for the FAT32 variation of WinNT family */
                     if ( !*pszLongName && HAS_WINNT_EXT( pDir->fEAS ))
                        {
                        PBYTE pDot;

                        MakeName( pDir, pszLongName, sizeof( pszLongName ));
                        pDot = strchr( pszLongName, '.' );

                        if ( HAS_WINNT_EXT_NAME( pDir->fEAS )) /* name part is lower case */
                           {
                              if( pDot )
                                  *pDot = 0;

                              strlwr( pszLongName );

                              if( pDot )
                                  *pDot = '.';
                           }

                        if( pDot && HAS_WINNT_EXT_EXT( pDir->fEAS )) /* ext part is lower case */
                            strlwr( pDot + 1 );
                        }

                     if (!strlen(pszLongName))
                        strcpy(pszLongName, szShortName);
                     FSH_UPPERCASE(pszLongName, FAT32MAXPATHCOMP, pszUpperName);

                     if (usTranslate == TRANSLATE_LONG_TO_SHORT) /* OS/2 session, translate to DOS */
                        {
                        if (!stricmp(pszUpperName, pszUpperPart) ||
                            !stricmp(szShortName,  pszUpperPart))
                           {
                           strcat(pszTarget, szShortName);
                           pszTarget += strlen(pszTarget);
                           fFound = TRUE;
                           }
                        }
                     else /* translate from DOS to OS/2 */
                        {
                        if (!stricmp(szShortName,  pszUpperPart) ||
                            !stricmp(pszUpperName, pszUpperPart))
                           {
                           strcat(pszTarget, pszLongName);
                           pszTarget += strlen(pszTarget);
                           fFound = TRUE;
                           }
                        }

                     if (fFound)
                        {
                        ulCluster = (ULONG)pDir->wClusterHigh * 0x10000L + pDir->wCluster;
                        ulCluster &= pVolInfo->ulFatEof;
                        if (strlen(pszPath))
                           {
                           if (pDir->bAttr & FILE_DIRECTORY)
                              {
                              usMode = MODE_START;
                              break;
                              }
                           ulCluster = pVolInfo->ulFatEof;
                           }
                        usMode = MODE_RETURN;
                        break;
                        }
                     memset(pszLongName, 0, FAT32MAXPATHCOMP);
                     }
               pDir++;
               ulDirEntries++;
               if (ulCluster == 1 && ulDirEntries > pVolInfo->BootSect.bpb.RootDirEntries)
                  break;
               }
            if (usMode != MODE_SCAN)
               break;
            }
         if (usMode != MODE_SCAN)
            break;
         if (ulCluster == 1)
            {
            // reading the root directory in case of FAT12/FAT16
            ulSector += pVolInfo->SectorsPerCluster;
            usSectorsRead += pVolInfo->SectorsPerCluster;
            if ((ULONG)usSectorsRead * pVolInfo->BootSect.bpb.BytesPerSector >=
                (ULONG)pVolInfo->BootSect.bpb.RootDirEntries * sizeof(DIRENTRY))
               // root directory ended
               ulCluster = 0;
            }
         else
            {
            ulCluster = GetNextCluster(pVolInfo, pDirSHInfo, ulCluster);
            }
         if (!ulCluster)
            ulCluster = pVolInfo->ulFatEof;
         if (ulCluster == pVolInfo->ulFatEof)
            strcat(pszTarget, pszPart);
         }
      }

   free(pDirStart);
   free(pszLongName);
   if (ulCluster == pVolInfo->ulFatEof)
      strcat(pszTarget, pszPath);
   return 0;
}

USHORT GetChkSum16(const UCHAR *data, int bytes)
{
   USHORT chksum = 0;
   int i;

   for (i = 0; i < bytes; i++)
      {
      if (i == 2 || i == 3)
         continue;

      chksum = ((chksum << 15) | (chksum >> 1)) + (USHORT)data[i];
      }

   return chksum;
}

ULONG GetChkSum32(const UCHAR *data, int bytes)
{
   ULONG chksum = 0;
   int i;

   for (i = 0; i < bytes; i++)
      chksum = ((chksum << 31) | (chksum >> 1)) + (ULONG)data[i];

   return chksum;
}

USHORT NameHash(USHORT *pszFilename, int NameLen)
{
   USHORT hash = 0;
   UCHAR  *data = (UCHAR *)pszFilename;
   int i;

   for (i = 0; i < NameLen * 2; i++)
      hash = ((hash << 15) | (hash >> 1)) + data[i];

   return hash;
}


USHORT ModifyDirectory(PVOLINFO pVolInfo, ULONG ulDirCluster, PSHOPENINFO pDirSHInfo,
                       USHORT usMode, PDIRENTRY pOld, PDIRENTRY pNew,
                       PSZ pszLongNameOld, PSZ pszLongNameNew, USHORT usIOMode)
{
USHORT    usEntriesNeeded;
USHORT    usFreeEntries;
ULONG     ulCluster;
ULONG     ulPrevCluster;
ULONG     ulPrevBlock;
ULONG     ulNextCluster = pVolInfo->ulFatEof;
USHORT    usClusterCount;
BOOL      fNewCluster;
ULONG     ulSector;
ULONG     ulPrevSector;
USHORT    usSectorsRead;
USHORT    usSectorsPerBlock;
ULONG     ulBytesToRead = 0;
ULONG     ulPrevBytesToRead = 0;
ULONG     ulBytesRemained;
USHORT    rc;

      PDIRENTRY pDirectory;
      PDIRENTRY pDir2;
      PDIRENTRY pWork, pWork2;
      PDIRENTRY pMax;
      PDIRENTRY pDirNew;
      PDIRENTRY pLNStart;

      MessageL(LOG_FUNCS, "ModifyDirectory%m DirCluster %ld, Mode = %d",
               0x0035, ulDirCluster, usMode);

      pDirNew = (PDIRENTRY)malloc((size_t)sizeof(DIRENTRY));
      if (!pDirNew)
         {
         return ERROR_NOT_ENOUGH_MEMORY;
         }

      if (usMode == MODIFY_DIR_RENAME ||
          usMode == MODIFY_DIR_INSERT)
         {
         if (!pNew || !pszLongNameNew)
            {
            Message("Modify directory: Invalid parameters 1");
            free(pDirNew);
            return ERROR_INVALID_PARAMETER;
            }

         memcpy(pDirNew, pNew, sizeof (DIRENTRY));
         if ((pNew->bAttr & 0x0F) != FILE_VOLID)
            {
            rc = MakeShortName(pVolInfo, ulDirCluster, pszLongNameNew, pDirNew->bFileName);
            if (rc == LONGNAME_ERROR)
               {
               Message("Modify directory: Longname error");
               free(pDirNew);
               return ERROR_FILE_EXISTS;
               }
            memcpy(pNew, pDirNew, sizeof (DIRENTRY));

            if (rc == LONGNAME_OFF)
               pszLongNameNew = NULL;
            }
         else
            pszLongNameNew = NULL;

         usEntriesNeeded = 1;
         if (pszLongNameNew)
#if 0
            usEntriesNeeded += strlen(pszLongNameNew) / 13 +
               (strlen(pszLongNameNew) % 13 ? 1 : 0);
#else
            usEntriesNeeded += ( DBCSStrlen( pszLongNameNew ) + 12 ) / 13;
#endif
         }

      if (usMode == MODIFY_DIR_RENAME ||
          usMode == MODIFY_DIR_DELETE ||
          usMode == MODIFY_DIR_UPDATE)
         {
         if (!pOld)
            {
            Message("Modify directory: Invalid parameter 2 ");
            free(pDirNew);
            return ERROR_INVALID_PARAMETER;
            }
         }

      pDirectory = (PDIRENTRY)malloc(2 * (size_t)pVolInfo->ulBlockSize);
      if (!pDirectory)
         {
         Message("Modify directory: Not enough memory");
         free(pDirNew);
         return ERROR_NOT_ENOUGH_MEMORY;
         }
      memset(pDirectory, 0, (size_t)pVolInfo->ulBlockSize);
      pDir2 = (PDIRENTRY)((PBYTE)pDirectory + pVolInfo->ulBlockSize);
      memset(pDir2, 0, (size_t)pVolInfo->ulBlockSize);

      ulCluster = ulDirCluster;
      pLNStart = NULL;
      ulPrevCluster = pVolInfo->ulFatEof;
      ulPrevBlock = 0;
      usClusterCount = 0;
      fNewCluster = FALSE;

      if (ulCluster == 1)
         {
         // root directory starting sector
         ulSector = pVolInfo->BootSect.bpb.ReservedSectors +
            (ULONG)pVolInfo->BootSect.bpb.SectorsPerFat * pVolInfo->BootSect.bpb.NumberOfFATs;
         usSectorsPerBlock = (ULONG)pVolInfo->SectorsPerCluster /
            ((ULONG)pVolInfo->ulClusterSize / pVolInfo->ulBlockSize);
         usSectorsRead = 0;
         ulBytesRemained = (ULONG)pVolInfo->BootSect.bpb.RootDirEntries * sizeof(DIRENTRY);
         }

      while (ulCluster != pVolInfo->ulFatEof)
         {
         ULONG ulBlock;
#ifdef CALL_YIELD
         Yield();
#endif
         usClusterCount++;
         for (ulBlock = 0;
              ulBlock < pVolInfo->ulClusterSize / pVolInfo->ulBlockSize &&
              ulCluster != pVolInfo->ulFatEof; ulBlock++)
            {
            if (!fNewCluster)
               {
               if (ulCluster == 1)
                  {
                  // reading root directory on FAT12/FAT16
                  rc = ReadSector(pVolInfo, ulSector + ulBlock * usSectorsPerBlock, usSectorsPerBlock, (void *)pDir2, usIOMode);
                  if (ulBytesRemained >= pVolInfo->ulBlockSize)
                     {
                     ulBytesToRead = pVolInfo->ulBlockSize;
                     ulBytesRemained -= pVolInfo->ulBlockSize;
                     }
                  else
                     {
                     ulBytesToRead = ulBytesRemained;
                     ulBytesRemained = 0;
                     }
                  }
               else
                  {
                  rc = ReadBlock(pVolInfo, ulCluster, ulBlock, pDir2, usIOMode);
                  ulBytesToRead = pVolInfo->ulBlockSize;
                  }
               if (rc)
                  {
                  free(pDirNew);
                  free(pDirectory);
                  return rc;
                  }
               }
            else
               {
               memset(pDir2, 0, (size_t)pVolInfo->ulBlockSize);
               fNewCluster = FALSE;
               }

            pMax = (PDIRENTRY)((PBYTE)pDirectory + pVolInfo->ulBlockSize + ulBytesToRead);

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
                           MessageL(LOG_FUNCS, " Updating cluster%m", 0x4009);
                           memcpy(pWork, pNew, sizeof (DIRENTRY));
                           if (ulCluster == 1)
                              // reading root directory on FAT12/FAT16
                              rc = WriteSector(pVolInfo, ulSector + ulBlock * usSectorsPerBlock, usSectorsPerBlock, (void *)pDir2, usIOMode);
                           else
                              rc = WriteBlock(pVolInfo, ulCluster, ulBlock, pDir2, usIOMode);
                           if (rc)
                              {
                              free(pDirNew);
                              free(pDirectory);
                              return rc;
                              }
                           ulCluster = pVolInfo->ulFatEof;
                           break;

                        case MODIFY_DIR_DELETE:
                        case MODIFY_DIR_RENAME:
                           MessageL(LOG_FUNCS, " Removing entry from cluster%m", 0x400a);
                           pWork2 = pLNStart;
                           while (pWork2 < pWork)
                              {
                              MessageL(LOG_FUNCS, "Deleting Longname entry.%m", 0x400b);
                              pWork2->bFileName[0] = DELETED_ENTRY;
                              pWork2++;
                              }
                           pWork->bFileName[0] = DELETED_ENTRY;

                           /*
                              Write previous cluster if LN start lies there
                           */
                           if (ulPrevCluster != pVolInfo->ulFatEof &&
                              pLNStart < pDir2)
                              {
                              if (ulPrevCluster == 1)
                                 // reading root directory on FAT12/FAT16
                                 rc = WriteSector(pVolInfo, ulPrevSector + ulPrevBlock * usSectorsPerBlock, usSectorsPerBlock, (void *)pDirectory, usIOMode);
                              else
                                 rc = WriteBlock(pVolInfo, ulPrevCluster, ulPrevBlock, pDirectory, usIOMode);
                              if (rc)
                                 {
                                 free(pDirNew);
                                 free(pDirectory);
                                 return rc;
                                 }
                              }

                           /*
                              Write current cluster
                           */
                           if (ulCluster == 1)
                              // reading root directory on FAT12/FAT16
                              rc = WriteSector(pVolInfo, ulSector + ulBlock * usSectorsPerBlock, usSectorsPerBlock, (void *)pDir2, usIOMode);
                           else
                              rc = WriteBlock(pVolInfo, ulCluster, ulBlock, pDir2, usIOMode);
                           if (rc)
                              {
                              free(pDirNew);
                              free(pDirectory);
                              return rc;
                              }

                           if (usMode == MODIFY_DIR_DELETE)
                              ulCluster = pVolInfo->ulFatEof;
                           else
                              {
                              usMode = MODIFY_DIR_INSERT;

                              pszLongNameOld = pszLongNameNew; //////

                              ulCluster = ulDirCluster;
                              ulBytesRemained = pVolInfo->BootSect.bpb.RootDirEntries * sizeof(DIRENTRY);
                              ulPrevCluster = pVolInfo->ulFatEof;
                              ulPrevSector = 0;
                              ulPrevBlock = 0;
                              usClusterCount = 0;
                              pLNStart = NULL;
                              continue;
                              }
                           break;
                        }
                     }

                  break;

               case MODIFY_DIR_INSERT:
                  if (ulPrevCluster != pVolInfo->ulFatEof && GetFreeEntries(pDirectory, ulPrevBytesToRead + ulBytesToRead) >= usEntriesNeeded)
                     {
                     BYTE bCheck = GetVFATCheckSum(pDirNew);

                     MessageL(LOG_FUNCS, " Inserting entry into 2 clusters%m", 0x400c);

                     pWork = CompactDir(pDirectory, ulPrevBytesToRead + ulBytesToRead, usEntriesNeeded);
                     pWork = fSetLongName(pWork, pszLongNameNew, bCheck);
                     memcpy(pWork, pDirNew, sizeof (DIRENTRY));

                     if (ulPrevCluster == 1)
                        // reading root directory on FAT12/FAT16
                        rc = WriteSector(pVolInfo, ulPrevSector + ulPrevBlock * usSectorsPerBlock, usSectorsPerBlock, (void *)pDirectory, usIOMode);
                     else
                        rc = WriteBlock(pVolInfo, ulPrevCluster, ulPrevBlock, pDirectory, usIOMode);
                     if (rc)
                        {
                        free(pDirNew);
                        free(pDirectory);
                        return rc;
                        }

                     if (ulCluster == 1)
                        // reading root directory on FAT12/FAT16
                        rc = WriteSector(pVolInfo, ulSector + ulBlock * usSectorsPerBlock, usSectorsPerBlock, (void *)pDir2, usIOMode);
                     else
                        rc = WriteBlock(pVolInfo, ulCluster, ulBlock, pDir2, usIOMode);
                     if (rc)
                        {
                        free(pDirNew);
                        free(pDirectory);
                        return rc;
                        }
                     ulCluster = pVolInfo->ulFatEof;
                     break;
                     }

                  usFreeEntries = GetFreeEntries(pDir2, ulBytesToRead);
                  if (usFreeEntries >= usEntriesNeeded)
                     {
                     BYTE bCheck = GetVFATCheckSum(pDirNew);

                     MessageL(LOG_FUNCS, " Inserting entry into 1 cluster%m", 0x400d);

                     pWork = CompactDir(pDir2, ulBytesToRead, usEntriesNeeded);
                     pWork = fSetLongName(pWork, pszLongNameNew, bCheck);
                     memcpy(pWork, pDirNew, sizeof (DIRENTRY));
                     if (ulCluster == 1)
                        // reading root directory on FAT12/FAT16
                        rc = WriteSector(pVolInfo, ulSector + ulBlock * usSectorsPerBlock, usSectorsPerBlock, (void *)pDir2, usIOMode);
                     else
                        rc = WriteBlock(pVolInfo, ulCluster, ulBlock, pDir2, usIOMode);
                     if (rc)
                        {
                        free(pDirNew);
                        free(pDirectory);
                        return rc;
                        }
                     ulCluster = pVolInfo->ulFatEof;
                     break;
                     }
                  else if (usFreeEntries > 0)
                     {
                     MarkFreeEntries(pDir2, ulBytesToRead);
                     if (ulCluster == 1)
                        // reading root directory on FAT12/FAT16
                        rc = WriteSector(pVolInfo, ulSector + ulBlock * usSectorsPerBlock, usSectorsPerBlock, (void *)pDir2, usIOMode);
                     else
                        rc = WriteBlock(pVolInfo, ulCluster, ulBlock, pDir2, usIOMode);
                     if (rc)
                        {
                        free(pDirNew);
                        free(pDirectory);
                        return rc;
                        }
                     }

                  break;
               }

            if (ulCluster != pVolInfo->ulFatEof)
               {
               ulPrevBytesToRead = ulBytesToRead;
               ulPrevCluster = ulCluster;
               ulPrevSector = ulSector;
               ulPrevBlock = ulBlock;
               memset(pDirectory, 0, (size_t)pVolInfo->ulBlockSize);
               memmove(pDirectory, pDir2, (size_t)ulBytesToRead);
               if (pLNStart)
                  pLNStart = (PDIRENTRY)((PBYTE)pLNStart - pVolInfo->ulBlockSize);

               if (ulCluster == 1)
                  {
                  // reading the root directory in case of FAT12/FAT16
                  ulSector += pVolInfo->SectorsPerCluster;
                  usSectorsRead += pVolInfo->SectorsPerCluster;
                  if ((ULONG)usSectorsRead * pVolInfo->BootSect.bpb.BytesPerSector >=
                      (ULONG)pVolInfo->BootSect.bpb.RootDirEntries * sizeof(DIRENTRY))
                     // root directory ended
                     ulNextCluster = 0;
                  else
                     ulNextCluster = 1;
                  }
               else
                  ulNextCluster = GetNextCluster(pVolInfo, NULL, ulCluster);
               if (!ulNextCluster)
                  ulNextCluster = pVolInfo->ulFatEof;
               if (ulNextCluster == pVolInfo->ulFatEof)
                  {
                  if (usMode == MODIFY_DIR_UPDATE ||
                      usMode == MODIFY_DIR_DELETE ||
                      usMode == MODIFY_DIR_RENAME)
                     {
                     if (ulBlock == pVolInfo->ulClusterSize / pVolInfo->ulBlockSize - 1)
                        {
                        free(pDirNew);
                        free(pDirectory);
                        return ERROR_FILE_NOT_FOUND;
                        }
                     else
                        {
                        if (ulBlock == pVolInfo->ulClusterSize / pVolInfo->ulBlockSize - 1)
                           ulCluster = ulNextCluster;
                        continue;
                        }
                     }
                  else
                     {
                     if (ulCluster == 1)
                        {
                        // no expanding for root directory in case of FAT12/FAT16
                        ulNextCluster = pVolInfo->ulFatEof;
                        }
                     else
                        ulNextCluster = SetNextCluster(pVolInfo, ulCluster, FAT_ASSIGN_NEW);
                     if (ulNextCluster == pVolInfo->ulFatEof)
                        {
                        free(pDirNew);
                        free(pDirectory);
                        Message("Modify Directory: Disk Full!");
                        return ERROR_DISK_FULL;
                        }
                     fNewCluster = TRUE;
                     }
                  }
               ulCluster = ulNextCluster;

               // clear the new cluster
               if (fNewCluster)
                  {
                  ULONG ulBlock2;
                  memset(pDir2, 0, (size_t)pVolInfo->ulBlockSize);
                  for (ulBlock2 = 0; ulBlock2 < pVolInfo->ulClusterSize / pVolInfo->ulBlockSize; ulBlock2++)
                     {
                     rc = WriteBlock(pVolInfo, ulCluster, ulBlock2, pDir2, usIOMode);
                     if (rc)
                        {
                        free(pDirNew);
                        free(pDirectory);
                        return rc;
                        }
                     }
                  }
               break;
               }
            if (ulCluster == pVolInfo->ulFatEof)
               break;
            if (ulBlock == pVolInfo->ulClusterSize / pVolInfo->ulBlockSize - 1)
               ulCluster = ulNextCluster;
            }
         if (ulCluster == pVolInfo->ulFatEof)
            break;
         if (ulBlock == pVolInfo->ulClusterSize / pVolInfo->ulBlockSize - 1)
            ulCluster = ulNextCluster;
         }

      free(pDirNew);
      free(pDirectory);
      return 0;
}
