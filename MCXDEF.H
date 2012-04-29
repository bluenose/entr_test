/************
 * mcxdef.h *
 ************/

#define NTASKS  12
#define NQUEUES 1
#define NNAMSEM 9
#define NTIMERS 13

#define TASK_clkdrv        1
#define TASK_dev           2
#define TASK_maintenance   3
#define TASK_keypad        4
#define TASK_mdb           5
#define TASK_main          6
#define TASK_initmdb       7
#define TASK_pollmdb       8
#define TASK_display       9
#define TASK_periodic      10
#define TASK_vend1         11
#define TASK_vend2         12

#define STACKSIZE_clkdrv        18
#define STACKSIZE_dev           96
#define STACKSIZE_maintenance   72
#define STACKSIZE_keypad        100
#define STACKSIZE_mdb           72
#define STACKSIZE_main          224
#define STACKSIZE_initmdb       100
#define STACKSIZE_pollmdb       136
#define STACKSIZE_display       64
#define STACKSIZE_periodic      104
#define STACKSIZE_vend1         120
#define STACKSIZE_vend2         120

#define QUEUE_char         1

#define SEMA_mdb           1
#define SEMA_display       2
#define SEMA_card          3
#define SEMA_cardvend      4
#define SEMA_cardconfig    5
#define SEMA_keypad        6
#define SEMA_cardsession   7
#define SEMA_delivered     8
#define SEMA_periodic      9

extern void task_dev (void);
extern void task_maintenance (void);
extern void task_keypad (void);
extern void task_mdb (void);
extern void task_main (void);
extern void task_initmdb (void);
extern void task_pollmdb (void);
extern void task_display (void);
extern void task_periodic (void);
extern void task_vend1 (void);
extern void task_vend2 (void);
