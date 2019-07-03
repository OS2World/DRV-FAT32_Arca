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

int far pascal _loadds FS_CHDIR(
    unsigned short usFlag,      /* flag     */
    struct cdfsi far * pcdfsi,      /* pcdfsi   */
    struct cdfsd far * pcdfsd,      /* pcdfsd   */
    char far * pDir,            /* pDir     */
    unsigned short usCurDirEnd      /* iCurDirEnd   */
)
{
PVOLINFO pVolInfo;
POPENINFO pOpenInfo = NULL;
ULONG ulCluster;
PSZ   pszFile;
USHORT rc;
//BYTE     szDirLongName[ FAT32MAXPATH ];
PSZ    szDirLongName = NULL;

   _asm push es;

   MessageL(LOG_FS, "FS_CHDIR%m, flag %u", 0x000f, usFlag);

   szDirLongName = (PSZ)malloc((size_t)FAT32MAXPATH);
   if (!szDirLongName)
      {
      rc = ERROR_NOT_ENOUGH_MEMORY;
      goto FS_CHDIREXIT;
      }

   switch (usFlag)
      {
      case CD_VERIFY   :
         pDir = pcdfsi->cdi_curdir;
         usCurDirEnd = 0xFFFF;

      case CD_EXPLICIT :
         if (f32Parms.fMessageActive & LOG_FS)
            Message("CHDIR to %s", pDir);
         if (strlen(pDir) > FAT32MAXPATH)
            {
            rc = ERROR_FILENAME_EXCED_RANGE;
            goto FS_CHDIREXIT;
            }

         pVolInfo = GetVolInfoX(pDir);

         if (! pVolInfo)
            {
            pVolInfo = GetVolInfo(pcdfsi->cdi_hVPB);
            }

         if (! pVolInfo)
            {
            rc = ERROR_INVALID_DRIVE;
            goto FS_CHDIREXIT;
            }

         if (pVolInfo->fFormatInProgress)
            {
            rc = ERROR_ACCESS_DENIED;
            goto FS_CHDIREXIT;
            }

         if (IsDriveLocked(pVolInfo))
            {
            rc = ERROR_DRIVE_LOCKED;
            goto FS_CHDIREXIT;
            }

         if( usFlag == CD_EXPLICIT )
         {
            pOpenInfo = malloc(sizeof (OPENINFO));
            if (!pOpenInfo)
            {
                rc = ERROR_NOT_ENOUGH_MEMORY;
                goto FS_CHDIREXIT;
            }
            memset(pOpenInfo, 0, sizeof (OPENINFO));

            if( TranslateName(pVolInfo, 0L, NULL, pDir, szDirLongName, TRANSLATE_SHORT_TO_LONG ))
               strcpy( szDirLongName, pDir );

            pOpenInfo->pSHInfo = GetSH( szDirLongName, pOpenInfo);
            if (!pOpenInfo->pSHInfo)
            {
                rc = ERROR_TOO_MANY_OPEN_FILES;
                goto FS_CHDIREXIT;
            }
            //pOpenInfo->pSHInfo->sOpenCount++; //
            if (pOpenInfo->pSHInfo->fLock)
            {
                rc = ERROR_ACCESS_DENIED;
                goto FS_CHDIREXIT;
            }

            pOpenInfo->pVolInfo = GetVolInfoX(pDir);

            if (!pOpenInfo->pVolInfo)
               {
               pOpenInfo->pVolInfo = pVolInfo;
               }

            pVolInfo = pOpenInfo->pVolInfo;
         }

         ulCluster = FindDirCluster(pVolInfo,
            pcdfsi,
            pcdfsd,
            pDir,
            usCurDirEnd,
            FILE_DIRECTORY,
            &pszFile);

         if (ulCluster == pVolInfo->ulFatEof || *pszFile)
            {
            rc = ERROR_PATH_NOT_FOUND;
            goto FS_CHDIREXIT;
            }

         if (ulCluster == pVolInfo->BootSect.bpb.RootDirStrtClus && pVolInfo->hVBP)
            {
            rc = 0;
            goto FS_CHDIREXIT;
            }

         *(PULONG)pcdfsd = ulCluster;


         if( usFlag == CD_EXPLICIT )
         {
            pOpenInfo->pSHInfo->bAttr = FILE_DIRECTORY;
            *((PULONG)pcdfsd + 1 ) = ( ULONG )pOpenInfo;
         }
         rc = 0;
         break;

      case CD_FREE     :
         pOpenInfo = ( POPENINFO )*(( PULONG )pcdfsd + 1 );
         ReleaseSH( pOpenInfo );
         pOpenInfo = NULL;
         rc = 0;
         break;
      default :
         rc = ERROR_INVALID_FUNCTION;
         break;
      }

FS_CHDIREXIT:

   if (rc && pOpenInfo)
      {
      if (pOpenInfo->pSHInfo)
         ReleaseSH(pOpenInfo);
      else
         free(pOpenInfo);
      }

   if (szDirLongName)
      free(szDirLongName);

   MessageL(LOG_FS, "FS_CHDIR%m returned %u", 0x800f, rc);

   _asm pop es;

   return rc;
}

int far pascal _loadds FS_MKDIR(
    struct cdfsi far * pcdfsi,      /* pcdfsi   */
    struct cdfsd far * pcdfsd,      /* pcdfsd   */
    char far * pName,           /* pName    */
    unsigned short usCurDirEnd,     /* iCurDirEnd   */
    char far * pEABuf,          /* pEABuf   */
    unsigned short usFlags      /* flags    */
)
{
PVOLINFO pVolInfo;
ULONG    ulCluster;
ULONG    ulDirCluster;
PSZ      pszFile;
PDIRENTRY pDirEntry = NULL;
PDIRENTRY pDir;
PSHOPENINFO pDirSHInfo = NULL;
USHORT   rc;
PBYTE    pbCluster;
ULONG    ulBlock;

   _asm push es;

   usFlags = usFlags;

   MessageL(LOG_FS, "FS_MKDIR%m - %s", 0x0010, pName);

   pVolInfo = GetVolInfoX(pName);

   if (! pVolInfo)
      {
      pVolInfo = GetVolInfo(pcdfsi->cdi_hVPB);
      }

   if (! pVolInfo)
      {
      rc = ERROR_INVALID_DRIVE;
      goto FS_MKDIREXIT;
      }

   if (pVolInfo->fFormatInProgress)
      {
      rc = ERROR_ACCESS_DENIED;
      goto FS_MKDIREXIT;
      }

   if (IsDriveLocked(pVolInfo))
      {
      rc = ERROR_DRIVE_LOCKED;
      goto FS_MKDIREXIT;
      }
   if (!pVolInfo->fDiskCleanOnMount)
      {
      rc = ERROR_VOLUME_DIRTY;
      goto FS_MKDIREXIT;
      }
   if (pVolInfo->fWriteProtected)
      {
      rc = ERROR_WRITE_PROTECT;
      goto FS_MKDIREXIT;
      }

   if (strlen(pName) > FAT32MAXPATH)
      {
      rc = ERROR_FILENAME_EXCED_RANGE;
      goto FS_MKDIREXIT;
      }

   pDirEntry = (PDIRENTRY)malloc((size_t)sizeof(DIRENTRY));
   if (!pDirEntry)
      {
      rc = ERROR_NOT_ENOUGH_MEMORY;
      goto FS_MKDIREXIT;
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
      goto FS_MKDIREXIT;
      }

   ulCluster = FindPathCluster(pVolInfo, ulDirCluster, pszFile, pDirSHInfo, pDirEntry, NULL);
   if (ulCluster != pVolInfo->ulFatEof)
      {
      rc = ERROR_ACCESS_DENIED;
      goto FS_MKDIREXIT;
      }

   ulCluster = SetNextCluster( pVolInfo, FAT_ASSIGN_NEW, pVolInfo->ulFatEof);
   if (ulCluster == pVolInfo->ulFatEof)
      {
      rc = ERROR_DISK_FULL;
      goto FS_MKDIREXIT;
      }

   pbCluster = malloc((size_t)pVolInfo->ulBlockSize);
   if (!pbCluster)
      {
      SetNextCluster( pVolInfo, ulCluster, 0L);
      rc = ERROR_NOT_ENOUGH_MEMORY;
      goto FS_MKDIREXIT;
      }

   memset(pbCluster, 0, (size_t)pVolInfo->ulBlockSize);

      pDir = (PDIRENTRY)pbCluster;

      pDir->wCluster = LOUSHORT(ulCluster);
      pDir->wClusterHigh = HIUSHORT(ulCluster);
      pDir->bAttr = FILE_DIRECTORY;

   //rc = MakeDirEntry(pVolInfo, ulDirCluster, (PDIRENTRY)pbCluster, NULL, pszFile);
   rc = MakeDirEntry(pVolInfo, ulDirCluster, pDirSHInfo, (PDIRENTRY)pbCluster, pszFile);

   if (rc)
      {
      free(pbCluster);
      goto FS_MKDIREXIT;
      }

      memset(pDir->bFileName, 0x20, 11);
      memcpy(pDir->bFileName, ".", 1);

      memcpy(pDir + 1, pDir, sizeof (DIRENTRY));
      pDir++;

      memcpy(pDir->bFileName, "..", 2);
      if (ulDirCluster == pVolInfo->BootSect.bpb.RootDirStrtClus)
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

   rc = WriteBlock( pVolInfo, ulCluster, 0, pbCluster, DVIO_OPWRTHRU);

   if (rc)
      goto FS_MKDIREXIT;

   memset(pbCluster, 0, (size_t)pVolInfo->ulBlockSize);

   // zero-out remaining blocks
   for (ulBlock = 1; ulBlock < pVolInfo->ulClusterSize / pVolInfo->ulBlockSize; ulBlock++)
      {
      rc = WriteBlock( pVolInfo, ulCluster, ulBlock, pbCluster, DVIO_OPWRTHRU);
      if (rc)
         goto FS_MKDIREXIT;
      }

   free(pbCluster);

   if (f32Parms.fEAS && pEABuf && pEABuf != MYNULL)
      rc = usModifyEAS(pVolInfo, ulDirCluster, pDirSHInfo, pszFile, (PEAOP)pEABuf);

FS_MKDIREXIT:
   if (pDirEntry)
      free(pDirEntry);

   MessageL(LOG_FS, "FS_MKDIR%m returned %u", 0x8010, rc);

   _asm pop es;

   return rc;
}

int far pascal _loadds FS_RMDIR(
    struct cdfsi far * pcdfsi,      /* pcdfsi   */
    struct cdfsd far * pcdfsd,      /* pcdfsd   */
    char far * pName,           /* pName    */
    unsigned short usCurDirEnd      /* iCurDirEnd   */
)
{
PVOLINFO pVolInfo;
ULONG    ulCluster;
ULONG    ulNextCluster;
ULONG    ulDirCluster;
PSZ      pszFile;
PDIRENTRY pDirEntry = NULL;
PDIRENTRY pDir;
PDIRENTRY pWork, pMax;
USHORT   rc;
USHORT   usFileCount;
//BYTE     szLongName[ FAT32MAXPATH ];
PSZ     szLongName = NULL;
PSHOPENINFO pDirSHInfo = NULL;
PSHOPENINFO pSHInfo = NULL;

   _asm push es;

   MessageL(LOG_FS, "FS_RMDIR%m %s", 0x0011, pName);

   pVolInfo = GetVolInfoX(pName);

   if (! pVolInfo)
      {
      pVolInfo = GetVolInfo(pcdfsi->cdi_hVPB);
      }

   if (! pVolInfo)
      {
      rc = ERROR_INVALID_DRIVE;
      goto FS_RMDIREXIT;
      }
   if (pVolInfo->fFormatInProgress)
      {
      rc = ERROR_ACCESS_DENIED;
      goto FS_RMDIREXIT;
      }
   if (IsDriveLocked(pVolInfo))
      {
      rc = ERROR_DRIVE_LOCKED;
      goto FS_RMDIREXIT;
      }
   if (!pVolInfo->fDiskCleanOnMount)
      {
      rc = ERROR_VOLUME_DIRTY;
      goto FS_RMDIREXIT;
      }
   if (pVolInfo->fWriteProtected)
      {
      rc = ERROR_WRITE_PROTECT;
      goto FS_RMDIREXIT;
      }
   if (strlen(pName) > FAT32MAXPATH)
      {
      rc = ERROR_FILENAME_EXCED_RANGE;
      goto FS_RMDIREXIT;
      }

   pDirEntry = (PDIRENTRY)malloc((size_t)sizeof(DIRENTRY));
   if (!pDirEntry)
      {
      rc = ERROR_NOT_ENOUGH_MEMORY;
      goto FS_RMDIREXIT;
      }

   szLongName = (PSZ)malloc((size_t)FAT32MAXPATH);
   if (!szLongName)
      {
      rc = ERROR_NOT_ENOUGH_MEMORY;
      goto FS_RMDIREXIT;
      }

#if 1
   if( TranslateName(pVolInfo, 0L, NULL, pName, szLongName, TRANSLATE_SHORT_TO_LONG ))
      strcpy( szLongName, pName );

   rc = MY_ISCURDIRPREFIX( szLongName );
   if( rc )
     goto FS_RMDIREXIT;
#else
   rc = FSH_ISCURDIRPREFIX(pName);
   if (rc)
      goto FS_RMDIREXIT;
   rc = TranslateName(pVolInfo, 0L, NULL, pName, szName, TRANSLATE_AUTO);
   if (rc)
      goto FS_RMDIREXIT;
   rc = FSH_ISCURDIRPREFIX(szName);
   if (rc)
      goto FS_RMDIREXIT;
#endif

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
      goto FS_RMDIREXIT;
      }

   ulCluster = FindPathCluster(pVolInfo, ulDirCluster, pszFile, pDirSHInfo, pDirEntry, NULL);
   if ( ulCluster == pVolInfo->ulFatEof ||
       !(pDirEntry->bAttr & FILE_DIRECTORY) )
      {
      rc = ERROR_PATH_NOT_FOUND;
      goto FS_RMDIREXIT;
      }

   if ( pDirEntry->bAttr & FILE_READONLY )
      {
      rc = ERROR_ACCESS_DENIED;
      goto FS_RMDIREXIT;
      }

   pDir = malloc((size_t)pVolInfo->ulClusterSize);
   if (!pDir)
      {
      rc = ERROR_NOT_ENOUGH_MEMORY;
      goto FS_RMDIREXIT;
      }

   ulNextCluster = ulCluster;
   usFileCount = 0;
   while (ulNextCluster != pVolInfo->ulFatEof)
      {
      ULONG ulBlock;
      for (ulBlock = 0; ulBlock < pVolInfo->ulClusterSize / pVolInfo->ulBlockSize; ulBlock++)
         {
         rc = ReadBlock( pVolInfo, ulNextCluster, ulBlock, pDir, 0);
         if (rc)
            {
            free(pDir);
            goto FS_RMDIREXIT;
            }

            pWork = pDir;
            pMax = (PDIRENTRY)((PBYTE)pDir + pVolInfo->ulBlockSize);
            while (pWork < pMax)
               {
               if (pWork->bFileName[0] && pWork->bFileName[0] != DELETED_ENTRY &&
                   pWork->bAttr != FILE_LONGNAME)
                  {
                  if (memcmp(pWork->bFileName, ".       ", 8) &&
                      memcmp(pWork->bFileName, "..      ", 8))
                     usFileCount++;
                  }
               pWork++;
               }
         }
      ulNextCluster = GetNextCluster(pVolInfo, pSHInfo, ulNextCluster);
      if (!ulNextCluster)
         ulNextCluster = pVolInfo->ulFatEof;
      }
   free(pDir);
   if (usFileCount)
      {
      Message("Cannot RMDIR, contains %u files", usFileCount);
      rc = ERROR_ACCESS_DENIED;
      goto FS_RMDIREXIT;
      }

   if (f32Parms.fEAS)
      {
      rc = usDeleteEAS(pVolInfo, ulDirCluster, pDirSHInfo, pszFile);
      if (rc)
         goto FS_RMDIREXIT;
#if 0
      if (pDirEntry->fEAS == FILE_HAS_EAS || pDirEntry->fEAS == FILE_HAS_CRITICAL_EAS)
         pDirEntry->fEAS = FILE_HAS_NO_EAS;
#endif
      }

   rc = ModifyDirectory(pVolInfo, ulDirCluster, pDirSHInfo, MODIFY_DIR_DELETE,
      pDirEntry, NULL, pszFile, pszFile, DVIO_OPWRTHRU);
   //rc = ModifyDirectory(pVolInfo, ulDirCluster, pDirSHInfo, MODIFY_DIR_DELETE,
   //   pDirEntry, NULL, NULL, DVIO_OPWRTHRU);
   if (rc)
      goto FS_RMDIREXIT;

   DeleteFatChain(pVolInfo, ulCluster);

FS_RMDIREXIT:
   if (pDirEntry)
      free(pDirEntry);
   if (szLongName)
      free(szLongName);

   MessageL(LOG_FS, "FS_RMDIR%m returned %u", 0x8011, rc);

   _asm pop es;

   return rc;
}
