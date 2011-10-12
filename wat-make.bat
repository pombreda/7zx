SET SRC=7zx.c 7zalloc.c 7zbcj2.c 7zbra86.c 7zbuf.c 7zbuf2.c 7zcrc.c 7zdecode.c 7zextrac.c 7zfile.c 7zheader.c 7zin.c 7zitem.c 7zlzmade.c 7zstream.c
wcl386 -q -os -s /bt=dos /l=pmodew             /d_MSC_VER /fm /fe=7zx-dos0 -"op stub=PMODEWI.EXE" %1 %2 %3 %SRC%
wcl386 -q -os -s /bt=dos /l=pmodew /dUSE_TINY  /d_MSC_VER /fm /fe=7zx-dos1 -"op stub=PMODEWI.EXE" %1 %2 %3 %SRC%
wcl386 -q -os -s                               /d_MSC_VER /fm /fe=7zx-wat0 %1 %2 %3 %SRC%
wcl386 -q -os -s /dUSE_WINDOWS_FILE            /d_MSC_VER /fm /fe=7zx-wat1 %1 %2 %3 %SRC%
rem upx --ultra-brute --strip-relocs=0 7zx.exe
