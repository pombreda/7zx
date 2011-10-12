#if !defined(NOTINY) && defined(__DJGPP__) && !defined(__STRICT_ANSI__)

  #include <crt0.h>

  #define UNUSED __attribute__((unused))

  char **__crt0_glob_function (char *arg UNUSED) { return 0; }
  void   __crt0_load_environment_file (char *progname UNUSED) { }

#endif
