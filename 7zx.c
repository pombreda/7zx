/* 7zMain.c - Test application for 7z Decoder
2008-11-23 : Igor Pavlov : Public domain */

//#include <stdlib.h>
//#include <stdio.h>
#include <string.h>

#include "7zcrc.h"
#include "7zfile.h"
#include "7zversio.h"

#include "7zalloc.h"
#include "7zextrac.h"
#include "7zin.h"

#if defined(USE_WINDOWS_FILE)
#include <windows.h>

#define CR  "\r\n"

static int my_puts(const char *s) {
    DWORD ReadBytes;

    WriteFile( GetStdHandle(STD_OUTPUT_HANDLE), s, strlen(s), &ReadBytes, NULL );
    return (int)ReadBytes;
}


#elif defined(USE_TINY)
#include "TINYFILE.H"

#define CR  "\r\n"

static int my_puts(const char *s) {
    return tiny_write(HSTDOUT, strlen(s), s);
}

#else
#include <stdio.h>

#define CR  "\n"

#define my_puts(s) fputs((s), stdout)

#endif


#define PrintError(sz)  my_puts(CR "ERROR:" sz)


int MY_CDECL main(int numargs, char *args[])
{
  CFileInStream archiveStream;
  CLookToRead lookStream;
  CSzArEx db;
  SRes res;
  ISzAlloc allocImp;
  ISzAlloc allocTempImp;

  if (numargs == 1)
  {
    my_puts(CR "7z Decoder " MY_VERSION_COPYRIGHT_DATE CR "\tUsage: 7zx <archive_name>" CR);
    return 0;
  }
  if (numargs != 2)
  {
    PrintError("incorrect command");
    return 1;
  }

  if (InFile_Open(&archiveStream.file, args[1]))
  {
    PrintError("can not open input file");
    return 1;
  }

  
  FileInStream_CreateVTable(&archiveStream);
  LookToRead_CreateVTable(&lookStream, False);
  
  lookStream.realStream = &archiveStream.s;
  LookToRead_Init(&lookStream);

  allocImp.Alloc = SzAlloc;
  allocImp.Free = SzFree;

  allocTempImp.Alloc = SzAllocTemp;
  allocTempImp.Free = SzFreeTemp;

  CrcGenerateTable();

  SzArEx_Init(&db);
  res = SzArEx_Open(&db, &lookStream.s, &allocImp, &allocTempImp);
  if (res == SZ_OK)
  {
    const int extractCommand = 1;

    {
      UInt32 i;

      /*
      if you need cache, use these 3 variables.
      if you use external function, you can make these variable as static.
      */
      UInt32 blockIndex = 0xFFFFFFFF; /* it can have any value before first call (if outBuffer = 0) */
      Byte *outBuffer = 0; /* it must be 0 before first call for each new archive. */
      size_t outBufferSize = 0;  /* it can have any value before first call (if outBuffer = 0) */

      for (i = 0; i < db.db.NumFiles; i++)
      {
        size_t offset;
        size_t outSizeProcessed;
        CSzFileItem *f = db.db.Files + i;
        
        my_puts(CR);
        
        if (f->IsDir) {
          my_puts("Dir ");
          my_puts(f->Name);
          continue;
        }
        my_puts(f->Name);

        res = SzAr_Extract(&db, &lookStream.s, i,
            &blockIndex, &outBuffer, &outBufferSize,
            &offset, &outSizeProcessed,
            &allocImp, &allocTempImp);
        if (res != SZ_OK)
          break;
        if (1)
        {
          CSzFile outFile;
          size_t processedSize;
          char *fileName = f->Name;
          size_t nameLen = strlen(f->Name);
          for (; nameLen > 0; nameLen--)
            if (f->Name[nameLen - 1] == '/')
            {
              fileName = f->Name + nameLen;
              break;
            }
            
          if (OutFile_Open(&outFile, fileName))
          {
            PrintError("can not open output file");
            res = SZ_ERROR_FAIL;
            break;
          }
          processedSize = outSizeProcessed;
          if (File_Write(&outFile, outBuffer + offset, &processedSize) != 0 ||
              processedSize != outSizeProcessed)
          {
            PrintError("can not write output file");
            res = SZ_ERROR_FAIL;
            break;
          }
          if (File_Close(&outFile))
          {
            PrintError("can not close output file");
            res = SZ_ERROR_FAIL;
            break;
          }
        }
      }
      IAlloc_Free(&allocImp, outBuffer);
    }
  }
  SzArEx_Free(&db, &allocImp);

  File_Close(&archiveStream.file);
  if (res == SZ_OK)
  {
    my_puts(CR "\tEverything is Ok" CR);
    return 0;
  }
  if (res == SZ_ERROR_UNSUPPORTED)
    PrintError("decoder doesn't support this archive");
  else if (res == SZ_ERROR_MEM)
    PrintError("can not allocate memory");
  else if (res == SZ_ERROR_CRC)
    PrintError("CRC error");
  else
    my_puts(CR "ERROR !!!" CR);
  return res;
}
