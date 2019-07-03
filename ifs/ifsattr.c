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

/******************************************************************
*
******************************************************************/
int far pascal _loadds FS_FILEATTRIBUTE(
    unsigned short usFlag,      /* flag     */
    struct cdfsi far * pcdfsi,      /* pcdfsi   */
    struct cdfsd far * pcdfsd,      /* pcdfsd   */
    char far * pName,           /* pName    */
    unsigned short usCurDirEnd,     /* iCurDirEnd   */
    unsigned short far * pAttr  /* pAttr    */
)
{
PVOLINFO pVolInfo;
ULONG ulCluster;
ULONG ulDirCluster;
PSZ   pszFile;
PDIRENTRY pDirEntry = NULL;
PDIRENTRY pDirNew = NULL;
PSHOPENINFO pDirSHInfo = NULL;
USHORT rc;

   _asm push es;

   MessageL(LOG_FS, "FS_FILEATTRIBUTE%m, Flag = %X for %s", 0x000d, usFlag, pName);

   pVolInfo = GetVolInfoX(pName);

   if (! pVolInfo)
      {
      pVolInfo = GetVolInfo(pcdfsi->cdi_hVPB);
      }

   if (! pVolInfo)
      {
      rc = ERROR_INVALID_DRIVE;
      goto FS_FILEATTRIBUTEEXIT;
      }

   if (pVolInfo->fFormatInProgress)
      {
      rc = ERROR_ACCESS_DENIED;
      goto FS_FILEATTRIBUTEEXIT;
      }

   if (IsDriveLocked(pVolInfo))
      {
      rc = ERROR_DRIVE_LOCKED;
      goto FS_FILEATTRIBUTEEXIT;
      }

   pDirEntry = (PDIRENTRY)malloc((size_t)sizeof(DIRENTRY));
   if (!pDirEntry)
      {
      rc = ERROR_NOT_ENOUGH_MEMORY;
      goto FS_FILEATTRIBUTEEXIT;
      }
   pDirNew = (PDIRENTRY)malloc((size_t)sizeof(DIRENTRY));
   if (!pDirNew)
      {
      rc = ERROR_NOT_ENOUGH_MEMORY;
      goto FS_FILEATTRIBUTEEXIT;
      }

   if (strlen(pName) > FAT32MAXPATH)
      {
      rc = ERROR_FILENAME_EXCED_RANGE;
      goto FS_FILEATTRIBUTEEXIT;
      }

   ulDirCluster = FindDirCluster(pVolInfo,
      pcdfsi,
      pcdfsd,
      pName,
      usCurDirEnd,
      RETURN_PARENT_DIR,
      &pszFile);

   if (ulDirCluster == pVolInfo->ulFatEof)
      {
      rc = ERROR_PATH_NOT_FOUND;
      goto FS_FILEATTRIBUTEEXIT;
      }

   ulCluster = FindPathCluster(pVolInfo, ulDirCluster, pszFile, pDirSHInfo, pDirEntry, NULL);
   if (ulCluster == pVolInfo->ulFatEof)
      {
      rc = ERROR_FILE_NOT_FOUND;
      goto FS_FILEATTRIBUTEEXIT;
      }

   switch (usFlag)
      {
      case FA_RETRIEVE :
         *pAttr = pDirEntry->bAttr;
         rc = 0;
         break;

      case FA_SET     :
         {
         USHORT usMask;

         if (!pVolInfo->fDiskCleanOnMount)
            {
            rc = ERROR_VOLUME_DIRTY;
            goto FS_FILEATTRIBUTEEXIT;
            }

         if (pVolInfo->fWriteProtected)
            {
            rc = ERROR_WRITE_PROTECT;
            goto FS_FILEATTRIBUTEEXIT;
            }

         usMask = ~(FILE_READONLY | FILE_HIDDEN | FILE_SYSTEM | FILE_ARCHIVED);
         if (*pAttr & usMask)
            {
            rc = ERROR_ACCESS_DENIED;
            goto FS_FILEATTRIBUTEEXIT;
            }

            memcpy(pDirNew, pDirEntry, sizeof (DIRENTRY));

            if (pDirNew->bAttr & FILE_DIRECTORY)
               pDirNew->bAttr = (BYTE)(*pAttr | FILE_DIRECTORY);
            else
               pDirNew->bAttr = (BYTE)*pAttr;
            rc = ModifyDirectory(pVolInfo, ulDirCluster, pDirSHInfo, MODIFY_DIR_UPDATE,
               pDirEntry, pDirNew, pszFile, pszFile, 0);
            //rc = ModifyDirectory(pVolInfo, ulDirCluster, pDirSHInfo, MODIFY_DIR_UPDATE,
            //   pDirEntry, pDirNew, NULL, NULL, 0);
         break;
         }
      default:
         rc = ERROR_INVALID_FUNCTION;
         break;
      }

FS_FILEATTRIBUTEEXIT:
   if (pDirEntry)
      free(pDirEntry);
   if (pDirNew)
      free(pDirNew);

   MessageL(LOG_FS, "FS_FILEATTRIBUTE%m returned %d", 0x800d, rc);

   _asm pop es;

   return rc;
}


/******************************************************************
*
******************************************************************/
int far pascal _loadds FS_PATHINFO(
    unsigned short usFlag,      /* flag     */
    struct cdfsi far * pcdfsi,      /* pcdfsi   */
    struct cdfsd far * pcdfsd,      /* pcdfsd   */
    char far * pName,           /* pName    */
    unsigned short usCurDirEnd,     /* iCurDirEnd   */
    unsigned short usLevel,     /* level    */
    char far * pData,           /* pData    */
    unsigned short cbData       /* cbData   */
)
{
PVOLINFO pVolInfo;
ULONG ulCluster;
ULONG ulDirCluster;
PSZ   pszFile;
PDIRENTRY pDirEntry = NULL;
PSHOPENINFO pDirSHInfo = NULL;
USHORT usNeededSize;
USHORT rc;

   _asm push es;

   MessageL(LOG_FS, "FS_PATHINFO%m Flag = %d, Level = %d called for %s, cbData = %u",
            0x000e, usFlag, usLevel, pName, cbData);

   pVolInfo = GetVolInfoX(pName);

   if (! pVolInfo)
      {
      pVolInfo = GetVolInfo(pcdfsi->cdi_hVPB);
      }

   if (! pVolInfo)
      {
      rc = ERROR_INVALID_DRIVE;
      goto FS_PATHINFOEXIT;
      }

   if (pVolInfo->fFormatInProgress)
      {
      rc = ERROR_ACCESS_DENIED;
      goto FS_PATHINFOEXIT;
      }

   if (IsDriveLocked(pVolInfo))
      {
      rc = ERROR_DRIVE_LOCKED;
      goto FS_PATHINFOEXIT;
      }

   pDirEntry = (PDIRENTRY)malloc((size_t)sizeof(DIRENTRY));
   if (!pDirEntry)
      {
      rc = ERROR_NOT_ENOUGH_MEMORY;
      goto FS_PATHINFOEXIT;
      }

   if (strlen(pName) > FAT32MAXPATH)
      {
      rc = ERROR_FILENAME_EXCED_RANGE;
      goto FS_PATHINFOEXIT;
      }

   if (usLevel != FIL_NAMEISVALID)
      {
      ulDirCluster = FindDirCluster(pVolInfo,
         pcdfsi,
         pcdfsd,
         pName,
         usCurDirEnd,
         RETURN_PARENT_DIR,
         &pszFile);

      if (ulDirCluster == pVolInfo->ulFatEof)
         {
         rc = ERROR_PATH_NOT_FOUND;
         goto FS_PATHINFOEXIT;
         }

      }

   if (usFlag == PI_RETRIEVE)
      {
      //BYTE szFullName[FAT32MAXPATH];
      PSZ szFullName = (PSZ)malloc((size_t)FAT32MAXPATH);
      if (!szFullName)
         {
         rc = ERROR_NOT_ENOUGH_MEMORY;
         goto FS_PATHINFOEXIT;
         }

      if (usLevel != FIL_NAMEISVALID)
         {
         ulCluster = FindPathCluster(pVolInfo, ulDirCluster, pszFile, pDirSHInfo, pDirEntry, NULL);
         if (ulCluster == pVolInfo->ulFatEof)
            {
            rc = ERROR_FILE_NOT_FOUND;
            goto FS_PATHINFOEXIT;
            }

         if( TranslateName( pVolInfo, 0L, NULL, pName, szFullName, TRANSLATE_SHORT_TO_LONG ))
            strcpy( szFullName, pName );
         }

      switch (usLevel)
         {
         case FIL_STANDARD         :
            usNeededSize = sizeof (FILESTATUS);
            break;
         case FIL_STANDARDL        :
            usNeededSize = sizeof (FILESTATUS3L);
            break;
         case FIL_QUERYEASIZE      :
            usNeededSize = sizeof (FILESTATUS2);
            break;
         case FIL_QUERYEASIZEL     :
            usNeededSize = sizeof (FILESTATUS4L);
            break;
         case FIL_QUERYEASFROMLIST :
         case FIL_QUERYEASFROMLISTL :
         case FIL_QUERYALLEAS:
            usNeededSize = sizeof (EAOP);
            break;
         case FIL_NAMEISVALID:
            rc = 0;
            goto FS_PATHINFOEXIT;
         case 7:
            usNeededSize = strlen(szFullName) + 1;
            break;
         default                   :
            rc = ERROR_INVALID_LEVEL;
            goto FS_PATHINFOEXIT;
         }

      if (cbData < usNeededSize)
         {
         rc = ERROR_BUFFER_OVERFLOW;
         goto FS_PATHINFOEXIT;
         }

      rc = MY_PROBEBUF(PB_OPWRITE, pData, cbData);
      if (rc)
         {
         Message("Protection VIOLATION in FS_PATHINFO!\n");
         goto FS_PATHINFOEXIT;
         }

      switch (usLevel)
         {
         case FIL_STANDARD         :
            {
            PFILESTATUS pfStatus = (PFILESTATUS)pData;

            memset(pfStatus, 0, sizeof (FILESTATUS));

            pfStatus->fdateCreation = pDirEntry->wCreateDate;
            pfStatus->ftimeCreation = pDirEntry->wCreateTime;
            pfStatus->fdateLastAccess = pDirEntry->wAccessDate;
            pfStatus->fdateLastWrite = pDirEntry->wLastWriteDate;
            pfStatus->ftimeLastWrite = pDirEntry->wLastWriteTime;

            if (!(pDirEntry->bAttr & FILE_DIRECTORY))
               {
               pfStatus->cbFile = pDirEntry->ulFileSize;
               pfStatus->cbFileAlloc =
                  (pfStatus->cbFile / pVolInfo->ulClusterSize) * pVolInfo->ulClusterSize +
                  (pfStatus->cbFile % pVolInfo->ulClusterSize ? pVolInfo->ulClusterSize : 0);
               }

            pfStatus->attrFile = (USHORT)pDirEntry->bAttr;
            rc = 0;
            break;
            }

         case FIL_STANDARDL        :
            {
            PFILESTATUS3L pfStatus = (PFILESTATUS3L)pData;

            memset(pfStatus, 0, sizeof (FILESTATUS3L));

               pfStatus->fdateCreation = pDirEntry->wCreateDate;
               pfStatus->ftimeCreation = pDirEntry->wCreateTime;
               pfStatus->fdateLastAccess = pDirEntry->wAccessDate;
               pfStatus->fdateLastWrite = pDirEntry->wLastWriteDate;
               pfStatus->ftimeLastWrite = pDirEntry->wLastWriteTime;

               if (!(pDirEntry->bAttr & FILE_DIRECTORY))
                  {
                  FileGetSize(pVolInfo, pDirEntry, ulDirCluster, pDirSHInfo, pszFile, (PULONGLONG)&pfStatus->cbFile);
#ifdef INCL_LONGLONG
                  pfStatus->cbFileAlloc =
                     (pfStatus->cbFile / pVolInfo->ulClusterSize) * pVolInfo->ulClusterSize +
                     (pfStatus->cbFile % pVolInfo->ulClusterSize ? pVolInfo->ulClusterSize : 0);
#else
                  {
                  LONGLONG llRest;

                  pfStatus->cbFileAlloc = iDivUL(pfStatus->cbFile, pVolInfo->ulClusterSize);
                  pfStatus->cbFileAlloc = iMulUL(pfStatus->cbFileAlloc, pVolInfo->ulClusterSize);
                  llRest = iModUL(pfStatus->cbFile, pVolInfo->ulClusterSize);

                  if (iNeqUL(llRest, 0))
                     iAssignUL(&llRest, pVolInfo->ulClusterSize);
                  else
                     iAssignUL(&llRest, 0);

                  pfStatus->cbFileAlloc = iAdd(pfStatus->cbFileAlloc, llRest);
                  }
#endif
                  }

               pfStatus->attrFile = (USHORT)pDirEntry->bAttr;
            rc = 0;
            break;
            }

         case FIL_QUERYEASIZE      :
            {
            PFILESTATUS2 pfStatus = (PFILESTATUS2)pData;

            memset(pfStatus, 0, sizeof (FILESTATUS2));

               pfStatus->fdateCreation = pDirEntry->wCreateDate;
               pfStatus->ftimeCreation = pDirEntry->wCreateTime;
               pfStatus->fdateLastAccess = pDirEntry->wAccessDate;
               pfStatus->fdateLastWrite = pDirEntry->wLastWriteDate;
               pfStatus->ftimeLastWrite = pDirEntry->wLastWriteTime;
               if (!(pDirEntry->bAttr & FILE_DIRECTORY))
                  {
                  pfStatus->cbFile = pDirEntry->ulFileSize;
                  pfStatus->cbFileAlloc =
                     (pfStatus->cbFile / pVolInfo->ulClusterSize) * pVolInfo->ulClusterSize +
                     (pfStatus->cbFile % pVolInfo->ulClusterSize ? pVolInfo->ulClusterSize : 0);
                  }

               pfStatus->attrFile = (USHORT)pDirEntry->bAttr;
            if (!f32Parms.fEAS)
               {
               pfStatus->cbList = sizeof pfStatus->cbList;
               rc = 0;
               }
            else
               {
               rc = usGetEASize(pVolInfo, ulDirCluster, pDirSHInfo, pszFile, &pfStatus->cbList);
               }
            break;
            }

         case FIL_QUERYEASIZEL     :
            {
            PFILESTATUS4L pfStatus = (PFILESTATUS4L)pData;

            memset(pfStatus, 0, sizeof (FILESTATUS4L));

               pfStatus->fdateCreation = pDirEntry->wCreateDate;
               pfStatus->ftimeCreation = pDirEntry->wCreateTime;
               pfStatus->fdateLastAccess = pDirEntry->wAccessDate;
               pfStatus->fdateLastWrite = pDirEntry->wLastWriteDate;
               pfStatus->ftimeLastWrite = pDirEntry->wLastWriteTime;
               if (!(pDirEntry->bAttr & FILE_DIRECTORY))
                  {
                  FileGetSize(pVolInfo, pDirEntry, ulDirCluster, pDirSHInfo, pszFile, (PULONGLONG)&pfStatus->cbFile);
#ifdef INCL_LONGLONG
                  pfStatus->cbFileAlloc =
                     (pfStatus->cbFile / pVolInfo->ulClusterSize) * pVolInfo->ulClusterSize +
                     (pfStatus->cbFile % pVolInfo->ulClusterSize ? pVolInfo->ulClusterSize : 0);
#else
                  {
                  LONGLONG llRest;

                  pfStatus->cbFileAlloc = iDivUL(pfStatus->cbFile, pVolInfo->ulClusterSize);
                  pfStatus->cbFileAlloc = iMulUL(pfStatus->cbFileAlloc, pVolInfo->ulClusterSize);
                  llRest = iModUL(pfStatus->cbFile, pVolInfo->ulClusterSize);

                  if (iNeqUL(llRest, 0))
                     iAssignUL(&llRest, pVolInfo->ulClusterSize);
                  else
                     iAssignUL(&llRest, 0);

                  pfStatus->cbFileAlloc = iAdd(pfStatus->cbFileAlloc, llRest);
                  }
#endif
                  }

               pfStatus->attrFile = (USHORT)pDirEntry->bAttr;
            if (!f32Parms.fEAS)
               {
               pfStatus->cbList = sizeof pfStatus->cbList;
               rc = 0;
               }
            else
               {
               rc = usGetEASize(pVolInfo, ulDirCluster, pDirSHInfo, pszFile, &pfStatus->cbList);
               }
            break;
            }

         case FIL_QUERYEASFROMLIST:
         case FIL_QUERYEASFROMLISTL:
            {
            PEAOP pEA = (PEAOP)pData;
            PFEALIST pFEA = pEA->fpFEAList;
            rc = MY_PROBEBUF(PB_OPWRITE, (PBYTE)pFEA, sizeof pFEA->cbList);
            if (rc)
               {
               Message("Protection VIOLATION in FS_PATHINFO!\n");
               goto FS_PATHINFOEXIT;
               }
            rc = MY_PROBEBUF(PB_OPWRITE, (PBYTE)pFEA, (USHORT)pFEA->cbList);
            if (rc)
               {
               Message("Protection VIOLATION in FS_PILEINFO!\n");
               goto FS_PATHINFOEXIT;
               }
            if (!f32Parms.fEAS)
               {
               rc = usGetEmptyEAS(pszFile,pEA);
               }
            else
               {
               rc = usGetEAS(pVolInfo, usLevel, ulDirCluster, pDirSHInfo, pszFile, pEA);
               }

            break;
            }

         case FIL_QUERYALLEAS:
            {
            PEAOP pEA = (PEAOP)pData;
            PFEALIST pFEA = pEA->fpFEAList;
            rc = MY_PROBEBUF(PB_OPWRITE, (PBYTE)pFEA, sizeof pFEA->cbList);
            if (rc)
               {
               Message("Protection VIOLATION in FS_PATHINFO!\n");
               goto FS_PATHINFOEXIT;
               }
            rc = MY_PROBEBUF(PB_OPWRITE, (PBYTE)pFEA, (USHORT)pFEA->cbList);
            if (rc)
               {
               Message("Protection VIOLATION in FS_PATHINFO!\n");
               goto FS_PATHINFOEXIT;
               }
            if (!f32Parms.fEAS)
               {
               memset(pFEA, 0, (USHORT)pFEA->cbList);
               pFEA->cbList = sizeof pFEA->cbList;
               rc = 0;
               }
            else
               {
               rc = usGetEAS(pVolInfo, usLevel, ulDirCluster, pDirSHInfo, pszFile, pEA);
               }

            break;
            }

         case FIL_NAMEISVALID :
            rc = 0;
            break;

         case 7:
            strcpy(pData, szFullName);
            rc = 0;
            break;

         default :
            rc = ERROR_INVALID_LEVEL;
            break;
         }
      if (szFullName)
         free(szFullName);

      goto FS_PATHINFOEXIT;
      }


   if (usFlag & PI_SET)
      {
      if (!pVolInfo->fDiskCleanOnMount)
         {
         rc = ERROR_VOLUME_DIRTY;
         goto FS_PATHINFOEXIT;
         }
      if (pVolInfo->fWriteProtected)
         {
         rc = ERROR_WRITE_PROTECT;
         goto FS_PATHINFOEXIT;
         }

      rc = MY_PROBEBUF(PB_OPREAD, pData, cbData);
      if (rc)
         {
         Message("Protection VIOLATION in FS_PATHINFO!\n");
         goto FS_PATHINFOEXIT;
         }

      if (usLevel == FIL_STANDARD  || usLevel == FIL_QUERYEASIZE ||
          usLevel == FIL_STANDARDL || usLevel == FIL_QUERYEASIZEL)
         {
         ulCluster = FindPathCluster(pVolInfo, ulDirCluster, pszFile, pDirSHInfo, pDirEntry, NULL);
         if (ulCluster == pVolInfo->ulFatEof)
            {
            rc = ERROR_FILE_NOT_FOUND;
            goto FS_PATHINFOEXIT;
            }
         }

      switch (usLevel)
         {
         case FIL_STANDARD:
            {
            PFILESTATUS pfStatus = (PFILESTATUS)pData;
            USHORT usMask;
            PDIRENTRY pDirNew;

            pDirNew = (PDIRENTRY)malloc((size_t)sizeof(DIRENTRY));
            if (!pDirNew)
               {
               rc = ERROR_NOT_ENOUGH_MEMORY;
               goto FS_PATHINFOEXIT;
               }

            if (cbData < sizeof (FILESTATUS))
               {
               rc = ERROR_INSUFFICIENT_BUFFER;
               goto FS_PATHINFOEXIT;
               }

            usMask = FILE_READONLY | FILE_HIDDEN | FILE_SYSTEM | FILE_ARCHIVED;
            if ( pDirEntry->bAttr & FILE_DIRECTORY )
               usMask |= FILE_DIRECTORY;
            usMask = ~usMask;

            if (pfStatus->attrFile & usMask)
               {
               if (f32Parms.fMessageActive & LOG_FS)
                  Message("Trying to set invalid attr bits: %X", pfStatus->attrFile);
               rc = ERROR_ACCESS_DENIED;
               goto FS_PATHINFOEXIT;
               }

            memcpy(pDirNew, pDirEntry, sizeof (DIRENTRY));

            usMask = 0;
            if (memcmp(&pfStatus->fdateCreation, &usMask, sizeof usMask) ||
                memcmp(&pfStatus->ftimeCreation, &usMask, sizeof usMask))
               {
                  pDirNew->wCreateDate = pfStatus->fdateCreation;
                  pDirNew->wCreateTime = pfStatus->ftimeCreation;
               }

            if (memcmp(&pfStatus->fdateLastWrite, &usMask, sizeof usMask) ||
                memcmp(&pfStatus->ftimeLastWrite, &usMask, sizeof usMask))
               {
                  pDirNew->wLastWriteDate = pfStatus->fdateLastWrite;
                  pDirNew->wLastWriteTime = pfStatus->ftimeLastWrite;
               }

            if (memcmp(&pfStatus->fdateLastAccess, &usMask, sizeof usMask))
               {
                  pDirNew->wAccessDate = pfStatus->fdateLastAccess;
               }

               if (pDirNew->bAttr & FILE_DIRECTORY)
                  pDirNew->bAttr = (BYTE)(pfStatus->attrFile | FILE_DIRECTORY);
               else
                  pDirNew->bAttr = (BYTE)pfStatus->attrFile;

            rc = ModifyDirectory(pVolInfo, ulDirCluster, pDirSHInfo, MODIFY_DIR_UPDATE,
               pDirEntry, pDirNew, pszFile, pszFile, 0);
            //rc = ModifyDirectory(pVolInfo, ulDirCluster, pDirSHInfo, MODIFY_DIR_UPDATE,
            //   pDirEntry, pDirNew, NULL, NULL, 0);
            break;
            }

         case FIL_STANDARDL:
            {
            PFILESTATUS3L pfStatus = (PFILESTATUS3L)pData;
            USHORT usMask;
            PDIRENTRY pDirNew;

            pDirNew = (PDIRENTRY)malloc((size_t)sizeof(DIRENTRY));
            if (!pDirNew)
               {
               rc = ERROR_NOT_ENOUGH_MEMORY;
               goto FS_PATHINFOEXIT;
               }

            if (cbData < sizeof (FILESTATUS3L))
               {
               rc = ERROR_INSUFFICIENT_BUFFER;
               goto FS_PATHINFOEXIT;
               }

            usMask = FILE_READONLY | FILE_HIDDEN | FILE_SYSTEM | FILE_ARCHIVED;
            if ( pDirEntry->bAttr & FILE_DIRECTORY )
               usMask |= FILE_DIRECTORY;
            usMask = ~usMask;

            if (pfStatus->attrFile & usMask)
               {
               if (f32Parms.fMessageActive & LOG_FS)
                  Message("Trying to set invalid attr bits: %X", pfStatus->attrFile);
               rc = ERROR_ACCESS_DENIED;
               goto FS_PATHINFOEXIT;
               }

            memcpy(pDirNew, pDirEntry, sizeof (DIRENTRY));

            usMask = 0;
            if (memcmp(&pfStatus->fdateCreation, &usMask, sizeof usMask) ||
                memcmp(&pfStatus->ftimeCreation, &usMask, sizeof usMask))
               {
                  pDirNew->wCreateDate = pfStatus->fdateCreation;
                  pDirNew->wCreateTime = pfStatus->ftimeCreation;
               }

            if (memcmp(&pfStatus->fdateLastWrite, &usMask, sizeof usMask) ||
                memcmp(&pfStatus->ftimeLastWrite, &usMask, sizeof usMask))
               {
                  pDirNew->wLastWriteDate = pfStatus->fdateLastWrite;
                  pDirNew->wLastWriteTime = pfStatus->ftimeLastWrite;
               }

            if (memcmp(&pfStatus->fdateLastAccess, &usMask, sizeof usMask))
               {
                  pDirNew->wAccessDate = pfStatus->fdateLastAccess;
               }

               if (pDirNew->bAttr & FILE_DIRECTORY)
                  pDirNew->bAttr = (BYTE)(pfStatus->attrFile | FILE_DIRECTORY);
               else
                  pDirNew->bAttr = (BYTE)pfStatus->attrFile;

            rc = ModifyDirectory(pVolInfo, ulDirCluster, pDirSHInfo, MODIFY_DIR_UPDATE,
               pDirEntry, pDirNew, pszFile, pszFile, 0);
            //rc = ModifyDirectory(pVolInfo, ulDirCluster, pDirSHInfo, MODIFY_DIR_UPDATE,
            //   pDirEntry, pDirNew, NULL, NULL, 0);
            break;

            free(pDirNew);
            }

         case FIL_QUERYEASIZE      :
         case FIL_QUERYEASIZEL     :
            if (!f32Parms.fEAS)
               rc = 0;
            else
               {
#if 0
               PDIRENTRY pDirNew;
#endif
               rc = usModifyEAS(pVolInfo, ulDirCluster, pDirSHInfo, pszFile, (PEAOP)pData);
               if (rc)
                  goto FS_PATHINFOEXIT;
#if 0
               memcpy(pDirNew, pDirEntry, sizeof (DIRENTRY));
               pDirNew->wLastWriteDate.year = pGI->year - 1980;
               pDirNew->wLastWriteDate.month = pGI->month;
               pDirNew->wLastWriteDate.day = pGI->day;
               pDirNew->wLastWriteTime.hours = pGI->hour;
               pDirNew->wLastWriteTime.minutes = pGI->minutes;
               pDirNew->wLastWriteTime.twosecs = pGI->seconds / 2;
               rc = ModifyDirectory(pVolInfo, ulDirCluster, pDirSHInfo, MODIFY_DIR_UPDATE,
                  pDirEntry, pDirNew, pszFile, pszFile, 0);
               //rc = ModifyDirectory(pVolInfo, ulDirCluster, pDirSHInfo, MODIFY_DIR_UPDATE,
               //   pDirEntry, pDirNew, NULL, NULL, 0);
#endif
               }
            break;

         default          :
            rc = ERROR_INVALID_LEVEL;
            break;
         }
      }
   else
      rc = ERROR_INVALID_FUNCTION;

FS_PATHINFOEXIT:
   if (pDirEntry)
      free(pDirEntry);

   MessageL(LOG_FS, "FS_PATHINFO%m returned %u", 0x800e, rc);

   _asm pop es;

   return rc;
}
