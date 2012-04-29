/************
 * mcxgen.c *
 ************/

/*****************************************************************
*
* This program is designed to maintain for the MCX11 real-time
* kernel environment the following files:
*
*  mcxdef.s    - header of defines for mcxinit, mcx, & mcxdata
*  mcxdata.s   - table for application and variable definitions
*  mcxdef.h    - definition of mcx variables and constants
*  mcxdbug.h   - definition of mcx variables and constants
*
******************************************************************
*  Ver    Date    ID      Description
*  ---  --------  ---  ------------------------------------
*  1.0  03/08/95  bc   First coded by Bill Colias.
*  1.1  08/03/95  bc   Format changes.
*  1.2  08/07/95  bc   Format changes.
*  1.3  10/30/95  bc   Document output added.
*  1.4  04/12/96  bc   System stack increased to 64 bytes.
*
******************************************************************/

#include <stdio.h>
#include <string.h>
#include <ctype.h>


/*****************************************************************
*  	D e f i n e s
*****************************************************************/
#define TITLE    "MCXGEN  version 1.4\nMCX11 real-time system generator\n"
#define DATAFILE "mcxgen.dat"
#define DOCFILE  "mcxgen.txt"
#define FOREVER 1
#define MaxTasks  126
#define MaxSema   126
#define MaxTimers 126
#define MaxQueues 126
#define RUN  0
#define IDLE 1
#define NameSize 20


/*****************************************************************
*  	P r o t o t y p e s
*****************************************************************/
void prn_define(FILE *fp, char *s1, int n, char *s2);
int entervalue(char *t, int def, int min, int max);
void entertext(char *t, char *s);
void initvars();
int loadvars();
void savevars();
int main();


/*****************************************************************
*    S t r u c t u r e s
*****************************************************************/
typedef struct {
	char name[NameSize];
	int stacksize;
	int initial;
  int memtype;
} TASK;
typedef struct {
	char name[NameSize];
	int width, depth;
} QUEUE;


/*****************************************************************
*    G l o b a l s
*****************************************************************/
char  gProject[80];
int   gNTASKS, gNQUEUES, gNNAMSEM, gNTIMERS;
TASK  task[MaxTasks];
char  sema_name[MaxSema][NameSize];
QUEUE queue[MaxQueues];




/*****************************************************************
*  Enter a value.  *t=prompt, def=default value, min & max.
*****************************************************************/
int entervalue(char *t, int def, int min, int max)
	{
	int i;
	char msg[80];
	do {
		i = -1;
		printf ("%s <%d> ", t, def);
		gets(msg);
		if (*msg)
			sscanf(msg, "%d", &i);
		else
			i = def;
		if (i==-1)
			printf("You must enter a numeric value.\n");
		else if (i<min)
			printf("Value too small.\n");
		else if (i>max)
			printf("Value too big.\n");
		else
			break;
		} while (FOREVER);
	return i;
	}


/*****************************************************************
*  Enter a hex value.  *t=prompt, def=default value, min & max.
*****************************************************************/
int enterhex(char *t, int def, int min, int max)
	{
	int i;
	char msg[80];
	do {
		i = -1;
    printf ("%s <%X> ", t, def);
		gets(msg);
		if (*msg)
      sscanf(msg, "%X", &i);
		else
			i = def;
		if (i==-1)
			printf("You must enter a numeric value.\n");
		else if (i<min)
			printf("Value too small.\n");
		else if (i>max)
			printf("Value too big.\n");
		else
			break;
		} while (FOREVER);
	return i;
	}


/*****************************************************************
*  Enter a line of text
*****************************************************************/
void entertext(char *t, char *s)
	{
	char msg[80];

	printf ("%s <%s> ", t, s);
	gets(msg);
	if (*msg)
		strcpy(s,msg);
	}




/*****************************************************************
*  Set the MCX11 configuration variables to default values.
*****************************************************************/
void initvars()
	{
	int i;
	char num[10];

	gNTASKS  = 2;
	gNQUEUES = 1;
	gNNAMSEM = 1;
	gNTIMERS = 1;

  strcpy(gProject,"Run-Time application");
	for (i=0; i<MaxTasks; i++)
		{
		sprintf(num,"%d",i+1);
		strcpy(task[i].name,num);
		task[i].stacksize = 18;
		task[i].initial = IDLE;
		}

	for (i=0; i<MaxSema; i++)
		{
		sprintf(num,"%d",i+1);
		strcpy(sema_name[i], num);
		}

	for (i=0; i<MaxQueues; i++)
		{
		sprintf(num,"%d",i+1);
		strcpy(queue[i].name,num);
		queue[i].width = 1;
		queue[i].depth = 16;
		}

	strcpy(task[0].name,"clkdrv");
	task[0].stacksize = 18;
	task[0].initial = RUN;
	}



/**********************************************************
*  Load the MCX11 configuration variables from DATAFILE
**********************************************************/
int loadvars()
	{
	FILE *fp;
	int i;
  char msg[80];

  if (!(fp = fopen(DATAFILE,"r")))
		return 0;

  fgets(gProject,80,fp);
  for (i=0; i<79; i++)
    if (gProject[i]=='\n')
      break;
  gProject[i] = '\0';
  fgets(msg,80,fp);
  sscanf(msg, "%d %d %d %d\n", &gNTASKS, &gNQUEUES, &gNNAMSEM, &gNTIMERS);
	for (i=0; i<gNTASKS; i++)
    {
    fgets(msg,80,fp);
    sscanf(msg, "%s %d %d %d\n", task[i].name, &task[i].stacksize, &task[i].initial,
      &task[i].memtype);
    }
	for (i=0; i<gNQUEUES; i++)
    {
    fgets(msg,80,fp);
		sscanf(msg, "%s %d %d\n", queue[i].name, &queue[i].width, &queue[i].depth);
    }
	for (i=0; i<gNNAMSEM; i++)
    {
    fgets(msg,80,fp);
    sscanf(msg, "%s\n", sema_name[i]);
    }
	fclose(fp);

	return 1;
	}



/**********************************************************
*  Save the MCX11 configuration variables in DATAFILE
**********************************************************/
void savevars()
	{
	FILE *fp;
	int i;

  if (!(fp = fopen(DATAFILE,"w")))
		return;
  fprintf(fp, "%s\n", gProject);
	fprintf(fp, "%d %d %d %d\n", gNTASKS, gNQUEUES, gNNAMSEM, gNTIMERS);
	for (i=0; i<gNTASKS; i++)
    fprintf(fp, "%s %d %d %d\n", task[i].name, task[i].stacksize, task[i].initial,
      task[i].memtype);
	for (i=0; i<gNQUEUES; i++)
		fprintf(fp, "%s %d %d\n", queue[i].name, queue[i].width, queue[i].depth);
	for (i=0; i<gNNAMSEM; i++)
		fprintf(fp, "%s\n", sema_name[i]);
	fclose(fp);
	}



/***************************************************
*  Enter the configuration of the MCX11 system
***************************************************/
void enterconfig()
	{
	int i;
	char msg[NameSize];

	printf("\n\nEnter parameters for your MCX11 application:\n\n");

  entertext("Project name?", gProject);
	gNTASKS = entervalue ("Number of tasks (including timer)?", gNTASKS, 1, MaxTasks);
	gNQUEUES = entervalue ("Number of queues?", gNQUEUES, 1, MaxQueues);
	gNNAMSEM = entervalue ("Number of semaphores?", gNNAMSEM, 1, MaxSema);
	gNTIMERS = entervalue ("Number of timers?", gNTIMERS, 1, MaxTimers);

  for (i=0; i<gNTASKS; i++)
		{
		printf("\n");
		printf("Task #%d:\n", i+1);
    entertext(" Task name?", task[i].name);
    task[i].stacksize = enterhex(" Stack size?", task[i].stacksize, 18, 32767);
    do {
      strcpy(msg,task[i].memtype?"bss":"data");
      entertext(" Memory type?", msg);
			*msg = tolower(*msg);
      } while (*msg!='b' && *msg!='d');
    task[i].memtype = *msg=='b' ? 1: 0;
    do {
      strcpy(msg,task[i].initial?"idle":"run");
			entertext(" Initial state?", msg);
			*msg = tolower(*msg);
			} while (*msg!='i' && *msg!='r');
		task[i].initial = *msg=='i' ? 1: 0;
		}

	for (i=0; i<gNQUEUES; i++)
		{
		printf("\n");
		printf("Queue #%d:\n", i+1);
		entertext(" Name?", queue[i].name);
		queue[i].width = entervalue(" Width?", queue[i].width, 1, 128);
		queue[i].depth = entervalue(" Depth?", queue[i].depth, 1, 128);
		}
	printf("\n");

	for (i=0; i<gNNAMSEM; i++)
		{
		sprintf(msg, "Semaphore #%d name?", i+1);
		entertext(msg, sema_name[i]);
		}
	printf("\n\n");
	}


/***************************************************
*   Show the configuration of the MCX11 system
***************************************************/
void showconfig()
	{
  int i,j,i2, membss, memdata;
  char *fmt = " %2d %3X  %-4s   %-4s   %-16.16s";
  char *hd  = "    STK  TYPE   STATE  NAME";

  printf("Tasks for %s:\n%s  %10s%s", gProject, hd, "", hd);
	i2 = (gNTASKS+1)/2;
	for (i=0; i<i2; i++)
		{
    printf("\n");
    for (j=i; j<gNTASKS; j+=i2)
      printf(fmt, j+1,
      task[j].stacksize,
      task[j].memtype ? "bss": "data",
      task[j].initial ? "idle": "run",
      task[j].name);
		}
  membss = memdata = 0;
  for (i=0; i<gNTASKS; i++)
    {
    if (task[i].memtype)
      membss += task[i].stacksize;
    else
			memdata += task[i].stacksize;
    }
  printf("\n\nTimer use: %d\nStack use: (%Xh data) + (%Xh bss) = (%Xh total)\n\n",
    gNTIMERS, memdata, membss, membss+memdata);

	for (i=0; i<gNQUEUES; i++)
		printf("Queue #%-2d %2dx%-3d %s\n", i+1,
			queue[i].width, queue[i].depth, queue[i].name);
	printf("\n");

	printf("Semaphores:");
	for (i=0; i<gNNAMSEM; i++)
		{
		if (!(i%5))
			printf("\n");
		printf("  %-12s", sema_name[i]);
		}
	printf("\n\n\n");
	}




/***************************************************
*   Print the configuration to a document file
***************************************************/
void configdoc()
	{
  int i,j,i2, membss, memdata;
  char *fmt = " %2d %3X  %-4s   %-4s   %-16.16s";
  char *hd  = "    STK  TYPE   STATE  NAME";
  FILE *fp;

  if (!(fp = fopen(DOCFILE,"w")))
    return ;
  fprintf(fp,TITLE);
  fprintf(fp,"\nTasks for %s:\n%s  %10s%s", gProject, hd, "", hd);
	i2 = (gNTASKS+1)/2;
	for (i=0; i<i2; i++)
		{
		fprintf(fp,"\n");
    for (j=i; j<gNTASKS; j+=i2)
      fprintf(fp,fmt, j+1,
      task[j].stacksize,
      task[j].memtype ? "bss": "data",
      task[j].initial ? "idle": "run",
      task[j].name);
		}
  membss = memdata = 0;
  for (i=0; i<gNTASKS; i++)
    {
    if (task[i].memtype)
      membss += task[i].stacksize;
    else
      memdata += task[i].stacksize;
    }
  fprintf(fp,"\n\nTimer use: %d\nStack use: (%Xh data) + (%Xh bss) = (%Xh total)\n\n",
    gNTIMERS, memdata, membss, membss+memdata);

	for (i=0; i<gNQUEUES; i++)
    fprintf(fp,"Queue #%-2d %2dx%-3d %s\n", i+1,
			queue[i].width, queue[i].depth, queue[i].name);
  fprintf(fp,"\n");

  fprintf(fp,"Semaphores:");
	for (i=0; i<gNNAMSEM; i++)
		{
		if (!(i%5))
      fprintf(fp,"\n");
    fprintf(fp,"  %-12s", sema_name[i]);
		}
  fprintf(fp,"\n");
  fclose(fp);
	}




/*********************************************************
*           M A I N
*********************************************************/
int main()
	{
	FILE *fp;
	char msg[80], num[10], *fmt1, *fmt2;
	int i;

  printf(TITLE);

	initvars();           /* initialize config variables */
	if (loadvars())       /* load config variables       */
		{
		printf("\nLast MCX11 configuration:\n\n");
    showconfig();
		do {
			printf("Do you wish to change the configuration? ");
			gets(msg);
			*msg = tolower(*msg);
			} while (*msg!='y' && *msg!='n');
		if (*msg=='n')
			return 0;
		}


	do {
		enterconfig();
		printf("\nNew MCX11 configuration:\n\n");
    showconfig();
		do {
			printf("Are the configuration parameters correct? ");
			gets(msg);
			*msg = tolower(*msg);
			} while (*msg!='y' && *msg!='n');
		} while (*msg=='n');


	do {
		printf("Save configuration? ");
		gets(msg);
		*msg = tolower(*msg);
		} while (*msg!='y' && *msg!='n');

	if (*msg=='n')
		{
		printf("Not saved.\n");
		return 0;
		}
	savevars();
  configdoc();


	/***************************
	 * Write the MCXDEF.S file *
	 ***************************/
	fp = fopen("mcxdef.s", "w");
	fprintf(fp,"; mcxdef.s\n\n");
	fmt1 = "        .DEFINE   %-10s = %3d      ; %s\n";
	fprintf(fp,fmt1, "NTASKS",  gNTASKS,  "Number of tasks (counting timer)");
	fprintf(fp,fmt1, "NQUEUES", gNQUEUES, "Number of queues");
	fprintf(fp,fmt1, "NNAMSEM", gNNAMSEM, "Number of named semaphores");
	fprintf(fp,fmt1, "NTIMERS", gNTIMERS, "Number of timers");
	fclose(fp);


	/***************************
	 * Write the MCXDEF.H file *
	 ***************************/
	fp = fopen("mcxdef.h", "w");
	fprintf(fp,"/************\n * mcxdef.h *\n ************/\n\n");
	fprintf(fp,"#define NTASKS  %d\n", gNTASKS);
	fprintf(fp,"#define NQUEUES %d\n", gNQUEUES);
	fprintf(fp,"#define NNAMSEM %d\n", gNNAMSEM);
	fprintf(fp,"#define NTIMERS %d\n", gNTIMERS);
	fprintf(fp,"\n");
	for (i=0; i<gNTASKS; i++)
		fprintf(fp,"#define TASK_%-12s  %d\n", task[i].name, i+1);
	fprintf(fp,"\n");
	for (i=0; i<gNTASKS; i++)
		fprintf(fp,"#define STACKSIZE_%-12s  %d\n", task[i].name, task[i].stacksize);
	fprintf(fp,"\n");
	for (i=0; i<gNQUEUES; i++)
		fprintf(fp,"#define QUEUE_%-12s %d\n", queue[i].name, i+1);
	fprintf(fp,"\n");
	for (i=0; i<gNNAMSEM; i++)
		fprintf(fp,"#define SEMA_%-12s  %d\n", sema_name[i], i+1);
	fprintf(fp,"\n");
	for (i=1; i<gNTASKS; i++)
		fprintf(fp,"extern void task_%s (void);\n", task[i].name);
	fclose(fp);


	/****************************
	 * Write the MCXDBUG.H file *
	 ****************************/
	fp = fopen("mcxdbug.h", "w");
	fprintf(fp,"/*************\n * mcxdbug.h *\n *************/\n\n");
	fprintf(fp,"\n");
	fprintf(fp,"typedef struct {\n");
	fprintf(fp,"    char *name;\n");
	fprintf(fp,"    int  stacksize;\n");
	fprintf(fp,"} MCXTASK;\n");
	fprintf(fp,"#ifdef _DBUG_\n");
  fprintf(fp,"const MCXTASK mcxtask[%d]={\n", gNTASKS);
	for (i=0; i<gNTASKS; i++)
		fprintf(fp,"    \"%s\", %d%s\n", task[i].name, task[i].stacksize, (i==gNTASKS-1)? "};": ",");
	fprintf(fp,"#else\n");
	fprintf(fp,"extern MCXTASK mcxtask[];\n");
	fprintf(fp,"#endif\n");
	fclose(fp);


	/****************************
	 * Write the MCXDATA.S file *
	 ****************************/
	fp = fopen("mcxdata.s", "w");
	fprintf(fp,"; mcxdata.s\n\n");
	fprintf(fp,"        .INCLUDE \"DEFINE.S\"\n");
	fprintf(fp,"        .INCLUDE \"MCXDEF.S\"\n\n");
	fprintf(fp,"        .PUBLIC   FLGTBL, QHDRTBL, STATLS, TIMERS\n");
	fprintf(fp,"        .PUBLIC   tickcnt, FREE, ACTIVE, curtsk, curtcb, hipri\n");
	fprintf(fp,"        .PUBLIC   pritcb, intlvl, temp, _width, _depth, notmt\n");
	fprintf(fp,"        .PUBLIC   SYSTACK, TCBDATA, QUEDATA, STKBASE\n");
	fprintf(fp,"        .PUBLIC   _TCBDATA\n\n");
	for (i=0; i<gNTASKS; i++)
		fprintf(fp,"        .EXTERNAL %s%s\n", i?"_task_":"", task[i].name);
	fprintf(fp,"\n\n");
	fprintf(fp,";************************\n");
	fprintf(fp,";  Stack Areas\n");
	fprintf(fp,";************************\n");
   fprintf(fp,"        .PSECT _%s\n", task[0].memtype ? "bss" : "data");
	fprintf(fp,"        .BYTE [64]\n");
	fprintf(fp,"SYSTACK: .BYTE [19]\n");
	for (i=0; i<gNTASKS; i++)
		{
      fprintf(fp,"        .PSECT _%s\n", task[i].memtype ? "bss" : "data");
		sprintf(msg,"STAK%d:", i+1);
		fprintf(fp,"        .BYTE [%d]\n", task[i].stacksize);
		fprintf(fp,"%-7s\n", msg);
		}
	fprintf(fp,"STKBASE:\n");
   fprintf(fp,"\n        .PSECT _%s\n", task[0].memtype ? "bss" : "data");
	fprintf(fp,"\n");
	fprintf(fp,";************************\n");
	fprintf(fp,";  Queue Headers\n");
	fprintf(fp,";************************\n");
	fprintf(fp,"QHDRTBL:\n");
	for (i=0; i<gNQUEUES; i++)
		{
		sprintf(msg,"Q%dHDR:",i+1);
		fprintf(fp,"%-7s .BYTE [QHDRLEN]\n", msg);
		}
	fprintf(fp,"\n");
	fprintf(fp,";************************\n");
	fprintf(fp,";  Timers\n");
	fprintf(fp,";************************\n");
	fprintf(fp,"TIMERS: .BYTE [NTIMERS*TIMRLEN]\n\n");
	fprintf(fp,"\n");
	fprintf(fp,";************************\n");
	fprintf(fp,";  System Variables\n");
	fprintf(fp,";************************\n");
	fmt1 = "%-7s .BYTE [1]       ; %s\n";
	fmt2 = "%-7s .WORD [1]       ; %s\n";
	fprintf(fp,"MCXVAR:\n");
	fprintf(fp,"tickcnt: .BYTE [1]      ; Tick counter\n");
	fprintf(fp,fmt2,"FREE:","Address of first free timer block");
	fprintf(fp,fmt2,"ACTIVE:","Address of first active timer in list");
	fprintf(fp,fmt1,"curtsk:","Current task (i.e. the active task)");
	fprintf(fp,fmt2,"curtcb:","Address of current task's TCB");
	fprintf(fp,fmt1,"hipri:","Highest priority task ready to run");
	fprintf(fp,fmt2,"pritcb:","Address of TCB of highest priority task");
	fprintf(fp,fmt1,"intlvl:","Depth of nested interrupts");
	fprintf(fp,fmt2,"temp:","Temporary area");
	fprintf(fp,fmt1,"_width:","Work area for queue width");
	fprintf(fp,fmt1,"_depth:","Work area for queue depth");
	fprintf(fp,fmt1,"notmt:","Work area for queue not empty semaphore");
	fprintf(fp,"\n");
	fprintf(fp,";************************\n");
	fprintf(fp,";  Semaphore table\n");
	fprintf(fp,";************************\n");
	fprintf(fp,"FLGTBL: .BYTE [NQUEUES*2 + NTASKS + NNAMSEM]\n\n");
	fprintf(fp,";************************\n");
	fprintf(fp,";  Task Control Block\n");
	fprintf(fp,";************************\n");
	fprintf(fp,"STATLS:\n");
	for (i=0; i<gNTASKS; i++)
		{
		sprintf(msg,"TASK%d:",i+1);
		fprintf(fp,"%-7s .BYTE [TCBLEN]\n", msg);
		}
	fprintf(fp,"\n");
	fprintf(fp,";************************\n");
	fprintf(fp,";  Queue Bodies\n");
	fprintf(fp,";************************\n");
	for (i=0; i<gNQUEUES; i++)
		{
		sprintf(msg,"Q%dBODY:",i+1);
		fprintf(fp,"%-7s .BYTE [%d*%d]\n", msg, queue[i].width, queue[i].depth);
		}
	fprintf(fp,"\n");
	fprintf(fp,"\n");
	fprintf(fp,"        .PSECT _text\n");
	fprintf(fp,"\n");
	fprintf(fp,";************************\n");
	fprintf(fp,";  Queue data in ROM\n");
	fprintf(fp,";************************\n");
	fprintf(fp,"QUEDATA:\n");
	for (i=0; i<gNQUEUES; i++)
		{
		fprintf(fp,"        .BYTE   %d,%d\n", queue[i].width, queue[i].depth);
		fprintf(fp,"        .WORD   Q%dHDR,Q%dBODY\n", i+1, i+1);
		}
	fprintf(fp,"\n");
	fprintf(fp,"\n");
	fprintf(fp,";************************\n");
	fprintf(fp,";  TCB data in ROM\n");
	fprintf(fp,";************************\n");
	fprintf(fp,"TCBDATA:\n");
	fprintf(fp,"_TCBDATA:\n");
	i = 0;
	for (i=0; i<gNTASKS; i++)
		{
		fprintf(fp,"        .BYTE   %s\n", task[i].initial ? "_IDLE":"_RUN");
		fprintf(fp,"        .WORD   %s%s, STAK%d-1, TASK%d\n\n",
			i?"_task_":"", task[i].name, i+1, i+1);
		}
	fprintf(fp,"\n");
	fprintf(fp,"        .END\n");
	fclose(fp);

	printf("Saved.\n");
	return 0;
	}
