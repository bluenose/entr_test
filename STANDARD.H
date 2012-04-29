/************************************************************************
*                         T Y P E D E F S                               *
************************************************************************/
typedef unsigned char   BYTE;
typedef unsigned int    UINT;
typedef unsigned short  WORD;
typedef unsigned long   DWORD;

/************************************************************************
*                         U N I O N S                                   *
************************************************************************/
typedef union {WORD  w;   BYTE b[2];} UNIONWB;
typedef union {DWORD dw;  WORD w[2];  BYTE b[4];} UNIONDWB;


/************************************************************************
*                         D E F I N E S                                 *
************************************************************************/
#define TRUE       1
#define FALSE      0

#define DEBUG_TEST 1

#define SPIE       0x80   /* spi interrupt enable         */
#define SPE        0x40   /* spi enable                   */
#define MSTR       0x10   /* master (1) or slave (0)      */
#define CPOL       0x08   /* clock polarity               */
#define CPHA       0x04   /* clock phase (basic protocol) */
#define SPIF       0x80   /* SPI Transfer Complete        */


/************************************************************************
*                         P R O T O T Y P E S                           *
************************************************************************/
int  tolower(int ch);
int  isdigit(int ch);
int  getchar();
