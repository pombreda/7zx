SET SRC=7zx.c 7zalloc.c 7zbcj2.c 7zbra86.c 7zbuf.c 7zbuf2.c 7zcrc.c 7zdecode.c 7zextrac.c 7zfile.c 7zheader.c 7zin.c 7zitem.c 7zlzmade.c 7zstream.c
gcc -Os %SRC% -o 7zx-gcc0.exe
gcc -DUSE_WINDOWS_FILE -Os %SRC% -o 7zx-gcc1.exe
