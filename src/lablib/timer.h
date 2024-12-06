/********************************************************************
  NAME
    timer.h - prototypes for timer utilities for DOS and UNIX

  AUTHOR
    DLS, APR-1994
********************************************************************/

#ifdef __cplusplus
extern "C" {
#endif
  unsigned int trGetTime();
  void trSleep(int sleep_ms);
  void trDateToDays(int day, int month, int year);
  
#ifdef __cplusplus
}
#endif

