/*************
 * mcxdbug.h *
 *************/


typedef struct {
    char *name;
    int  stacksize;
} MCXTASK;
#ifdef _DBUG_
const MCXTASK mcxtask[12]={
    "clkdrv", 18,
    "dev", 96,
    "maintenance", 72,
    "keypad", 100,
    "mdb", 72,
    "main", 224,
    "initmdb", 100,
    "pollmdb", 136,
    "display", 64,
    "periodic", 104,
    "vend1", 120,
    "vend2", 120};
#else
extern MCXTASK mcxtask[];
#endif
