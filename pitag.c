/************************************************************************
 PiTag - JTAG Tool for RaspberryPi (ppcasm <ppcasm@gmail.com>)


This code is licensed to you under the terms of the GNU GPL, version 2;
see file COPYING or http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
*************************************************************************/

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <time.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include "pitag.h" //Holds most of the configurable params.

#define TRUE 1
#define FALSE 0

/* Define some properties about jtag. */
struct _jtag{
	int pTCK;
	int pTMS;
	int pTDO;
	int pTDI;
	unsigned int IRLEN;
	unsigned int DRLEN;
}JT;

unsigned int PERI_BASE_ADDR = 0;

#define BCM2708_PERI_BASE       PERI_BASE_ADDR
#define GPIO_BASE              (BCM2708_PERI_BASE + 0x200000)

int  mem_fd;
void *gpio_map;

/* I/O access */
volatile unsigned *gpio;

/* GPIO Setup macros. */
#define INP_GPIO(g) (*(gpio+((g)/10)) &= ~(7<<(((g)%10)*3)))
#define OUT_GPIO(g) *(gpio+((g)/10)) |=  (1<<(((g)%10)*3))
#define SET_GPIO_ALT(g,a) *(gpio+(((g)/10))) |= (((a)<=3?(a)+4:(a)==4?3:2)<<(((g)%10)*3))
#define GPIO_HI(g) *(gpio+7) = 1 << g
#define GPIO_LO(g) *(gpio+10) = 1 << g
#define GET_GPIO(g) (*(gpio+13)&(1<<g)) >> g // 0 if LOW, (1<<g) if HIGH

const unsigned char pinslen = sizeof(pins)/sizeof(pins[0]);

/* Check which version of RPI we are running on. */
void set_rpi_conf(){


	char buf[1024];
	unsigned long RPI_version = 0;
	char magic[]="boardrev=";
	int strsize = strlen(magic);
	int n = 0;
	int offset;
	int i;

	FILE *fp = fopen("/proc/cmdline", "r");
	if(!fp){
		printf("(ERROR) check_rpi_rev: File does not exist.\n");
		exit(-1);
	}

	n = fread(buf, 1, sizeof(buf)-1, fp);
	fclose(fp);

	for(i=0;i<n;i++){
		memcpy(magic, buf+i, strsize);
		magic[strsize]=0;
		if(!strncmp(magic, "boardrev=", strsize)){
			offset=i+strsize;
			i=i+strsize;
			while(buf[i]!=' '&&buf[i]!=0){
				i++;
			}

			strsize = i-offset;
			strncpy(buf, buf+offset, strsize);
			buf[strsize]=0;
			RPI_version = strtoul(buf, NULL, 0);
		}
	}


	switch(RPI_version){

		case 0x0002:
			printf("RaspberryPi 1 detected! (Model: B) (PCB Rev: 1.0) (Memory: 256MB)\n");
			PERI_BASE_ADDR = 0x20000000;
			//wiring = 0;
			break;

		case 0x0003:
			printf("RaspberryPi 1 detected! (Model: B(ECN001)) (PCB Rev: 1.0) (Memory: 256MB)\n");
			PERI_BASE_ADDR = 0x20000000;
			//wiring = 0;
			break;

		case 0x0004:
			printf("RaspberryPi 1 detected! (Model: B) (PCB Rev: 2.0) (Memory: 256MB)\n");
			PERI_BASE_ADDR = 0x20000000;
			//wiring = 1;
			break;

		case 0x0005:
			printf("RaspberryPi 1 detected! (Model: B) (PCB Rev: 2.0) (Memory: 256MB)\n");
			PERI_BASE_ADDR = 0x20000000;
			//wiring = 1;
			break;

		case 0x0006:
			printf("RaspberryPi 1 detected! (Model: B) (PCB Rev: 2.0) (Memory: 256MB)\n");
			PERI_BASE_ADDR = 0x20000000;
			//wiring = 1;
			break;

		case 0x0007:
			printf("RaspberryPi 1 detected! (Model: A) (PCB Rev: 2.0) (Memory: 256MB)\n");
			PERI_BASE_ADDR = 0x20000000;
			//wiring = 1;
			break;

		case 0x0008:
			printf("RaspberryPi 1 detected! (Model: A) (PCB Rev: 2.0) (Memory: 256MB)\n");
			PERI_BASE_ADDR = 0x20000000;
			//wiring = 1;
			break;

		case 0x0009:
			printf("RaspberryPi 1 detected! (Model: A) (PCB Rev: 2.0) (Memory: 256MB)\n");
			PERI_BASE_ADDR = 0x20000000;
			//wiring = 1;
			break;

		case 0x000D:
			printf("RaspberryPi 1 detected! (Model: B) (PCB Rev: 2.0) (Memory: 512MB)\n");
			PERI_BASE_ADDR = 0x20000000;
			//wiring = 1;
			break;

		case 0x000E:
			printf("RaspberryPi 1 detected! (Model: B) (PCB Rev: 2.0) (Memory: 512MB)\n");
			PERI_BASE_ADDR = 0x20000000;
			//wiring = 1;
			break;

		case 0x000F:
			printf("RaspberryPi 1 detected! (Model: B) (PCB Rev: 2.0) (Memory: 512MB)\n");
			PERI_BASE_ADDR = 0x20000000;
			//wiring = 1;
			break;

		case 0x0010:
			printf("RasbperryPi 1 detected! (Model: B+) (PCB Rev: 1.0) (Memory: 512MB)\n");
			PERI_BASE_ADDR = 0x20000000;
			//wiring = 2;
			break;

		case 0x0011:
			printf("RaspberryPi 1 detected! (Model: Computer Module) (PCB Rev: 1.0) (Memory: 512MB)\n");
			PERI_BASE_ADDR = 0x20000000;
			break;

		case 0x0012:
			printf("RaspberryPi 1 detected! (Model: A+) (PCB Rev: 1.0) (Memory: 256MB\n");
			PERI_BASE_ADDR = 0x20000000;
			//wiring = 2;
			break;

		case 0xa21041:
			printf("RaspberryPi 2 detected! (Model: B) (PCB Rev: 1.1) (Memory: 1GB)\n");
			PERI_BASE_ADDR = 0x3F000000;
			//wiring = 2;
			break;
		default:
			printf("(ERROR) main: RaspberryPi board revision not supported! (0x%x)\n", RPI_version);
			exit(-1);
			break;
	}

 return;
}

void sleepz(int time){
	int i = 0;
	for(i=0;i<=time;i++){}
}

/* Randomize JTAG scan test pattern to eliminate more possible false positives. */
void randumb_pattern(){

	int i = 0;
	srand(time(NULL));

	for(i=0;i<PATTERN_LEN;i++){
		pattern[i]=rand()%('2'-'0')+'0';
	}
}

/* Pulse TMS. */
static void pulse_tms(int tck, int tms, int s_tms)
{
	GPIO_LO(tck);
	sleepz(DELAYZ);
	if (s_tms) GPIO_HI(tms); else GPIO_LO(tms);
	sleepz(DELAYZ);
	GPIO_HI(tck);
	sleepz(DELAYZ);

 return;
}

/* Pulse TDI. */
static void pulse_tdi(int tck, int tdi, int s_tdi)
{
	GPIO_LO(tck);
	sleepz(DELAYZ);
	if (s_tdi) GPIO_HI(tdi); else GPIO_LO(tdi);
	sleepz(DELAYZ);
	GPIO_HI(tck);
	sleepz(DELAYZ);

return;
}

/* Pulse TDO. */
unsigned char pulse_tdo(int tck, int tdo)
{
	unsigned char tdo_read;
	GPIO_LO(tck); // read in TDO on falling edge
	sleepz(DELAYZ);
	tdo_read = GET_GPIO(tdo);
	sleepz(DELAYZ);
	GPIO_HI(tck);
	sleepz(DELAYZ);
	return tdo_read;
}

/*
   send pattern[] to TDI and check for output on TDO
   This is used for both loopback, and Shift-IR testing, i.e.
   the pattern may show up with some delay.
   return: 0 = no match
		   1 = match
    	           2 or greater = no pattern found but line appears active

   if retval == 1, *reglen returns the length of the register
*/
static int check_data(char pattern[], int iterations, int tck, int tdi, int tdo,
                      int *reg_len)
{
	int i;
        int w          = 0;
	int plen       = strlen(pattern);
	char tdo_read;
	char tdo_prev;
	int nr_toggle  = 0; // count how often tdo toggled
	/* we store the last plen (<=PATTERN_LEN) bits,
	 *  rcv[0] contains the oldest bit */
	char rcv[PATTERN_LEN];

	tdo_prev = '0' + (GET_GPIO(tdo));
	for(i = 0; i < iterations; i++) {

		/* output pattern and incr write index */
		pulse_tdi(tck, tdi, pattern[w++] - '0');
		if (!pattern[w])
			w = 0;

		/* read from TDO and put it into rcv[] */
		tdo_read  =  '0' + (GET_GPIO(tdo));
		nr_toggle += (tdo_read != tdo_prev);
		tdo_prev  =  tdo_read;

		if (i < plen)
			rcv[i] = tdo_read;
		else
		{
			memmove(rcv, rcv + 1, plen - 1);
			rcv[plen-1] = tdo_read;
		}

		/* check if we got the pattern in rcv[] */
		if (i >= (plen - 1) ) {
			if (!memcmp(pattern, rcv, plen)) {
				*reg_len = i + 1 - plen;
				return 1;
			}
		}
	}

	*reg_len = 0;
	return nr_toggle > 1 ? nr_toggle : 0;
}

/*
   Set the JTAG TAP state machine
*/
void tap_state(char tap_state[], int tck, int tms)
{

	while (*tap_state) { // exit when string \0 terminator encountered
		GPIO_LO(tck);
		sleepz(DELAYZ);
		if((*tap_state - '0')) GPIO_HI(tms); else GPIO_LO(tms); //conv from ascii pattern and write to TMS
		sleepz(DELAYZ);
		GPIO_HI(tck); // rising edge shifts in TMS
		sleepz(DELAYZ);
		*tap_state++;
	}

 return;
}

/*
   Initialize all pins to a default state
   default with no arguments: all pins as INPUTs
*/
void init_pins(int tck, int tms, int tdi){

	/* default all to INPUT state. */
	for (int i = 0; i < pinslen; i++) {
		INP_GPIO(pins[i]);
	}

	OUT_GPIO(tck); //TCK = output
	OUT_GPIO(tms); //TMS = output
	OUT_GPIO(tdi);; //TDI = output

 return;
}

/*
   Shift JTAG TAP to ShiftIR state. Send pattern to TDI and check
   for output on TDO
*/
static void scan(){

	int tck, tms, tdo, tdi;
	int checkdataret = 0;
	int reg_len;

	printf("Starting scan for pattern: %s\n", pattern);

		for(tck=0;tck<pinslen;tck++) {
			for(tms=0;tms<pinslen;tms++) {
				if(tms == tck  ) continue;
			     for(tdo=0;tdo<pinslen;tdo++) {
					if(tdo == tck  ) continue;
					if(tdo == tms  ) continue;
					for(tdi=0;tdi<pinslen;tdi++) {
						if(tdi == tck  ) continue;
						if(tdi == tms  ) continue;
						if(tdi == tdo  ) continue;

						init_pins(pins[tck], pins[tms], pins[tdi]);
						tap_state(TAP_SHIFTIR, pins[tck], pins[tms]);
						checkdataret = check_data(pattern, (2*PATTERN_LEN),
						                pins[tck], pins[tdi], pins[tdo], &reg_len);
						if(checkdataret == 1) {
							printf("FOUND! (TCK)%s | (TMS)%s | (TDO)%s | (TDI)%s\n", pinnames[tck], pinnames[tms], pinnames[tdo], pinnames[tdi]);
							printf("IR length: %d\n", reg_len);
							if(CUSTOM_PINS){
								JT.pTCK = JTAG_TCK;
								JT.pTMS = JTAG_TMS;
								JT.pTDO = JTAG_TDO;
								JT.pTDI = JTAG_TDI;
							}
							else{
								JT.pTCK = pins[tck];
								JT.pTMS = pins[tms];
								JT.pTDO = pins[tdo];
								JT.pTDI = pins[tdi];


							}

							JT.IRLEN = reg_len;
							if(!JT.IRLEN) printf("Likely will not work as IRLEN is 0\n");

							printf("\nPin Configuration: %s\n", (CUSTOM_PINS)? "CUSTOM" : "AUTOMATIC");
						}
						else if((checkdataret > 1) && (IGNORE_ACTIVE==FALSE)) {
							printf("active (TCK)%s | (TMS)%s | (TDO)%s | (TDI)%s", pinnames[tck], pinnames[tms], pinnames[tdo], pinnames[tdi]);
							printf(" bits toggled: %d\n", checkdataret);
						}

					}
				}
			}
		}

 return;
}

/* Set up a memory regions to access GPIO. */
void setup_io(){

   /* Setup configuration based on what RPI board you're running. */
   set_rpi_conf();

   /* open /dev/mem */
   if ((mem_fd = open("/dev/mem", O_RDWR|O_SYNC) ) < 0) {
      printf("(ERROR) setup_io: can't open /dev/mem (TRY RUNNING AS ROOT)\n");
      exit(-1);
   }

   /* mmap GPIO */
   gpio_map = mmap(
      NULL,             //Any adddress in our space will do
      4096,	        //Map length
      PROT_READ|PROT_WRITE, // Enable reading & writing to mapped memory
      MAP_SHARED,       //Shared with other processes
      mem_fd,           //File to map
      GPIO_BASE         //Offset to GPIO peripheral
   );

   close(mem_fd); //No need to keep mem_fd open after mmap

   if (gpio_map == MAP_FAILED) {
      printf("(ERROR) setup_io: mmap error %d\n", (int)gpio_map); //errno also set!
      exit(-1);
   }

   /* Notify of the peripheral base address. */
   printf("Peripheral base: 0x%08x\n\n", PERI_BASE_ADDR);

   // Always use volatile pointer!
   gpio = (volatile unsigned *)gpio_map;

 return;

}

/* Get IDCODE from SHIFTDR. */
void get_idcode(int tck, int tms, int tdo, int tdi){

      int i = 0;
      int tdo_read = 0;
      unsigned int idcode = 0;

      printf("\nTesting IDCODE...\n");

      init_pins(tck, tms, tdi);

      /* we hope that IDCODE is the default DR after reset */
      tap_state(TAP_RESET, tck, tms);
      tap_state(TAP_SHIFTDR, tck, tms);

      /* j is the number of bits we pulse into TDI and read from TDO */
      for(i = 0; i < 32; i++) {
              /* we send '0' in */
              pulse_tdi(tck, tdi, 0);
              tdo_read = GET_GPIO(tdo);
              idcode |= tdo_read << i;
     }

     /* Check if all 0's or all 1's and also if first bit of IDCODE is 1. */
     if((idcode==0xffffffff)||(idcode==0x00000000)) {printf("IDCODE FAILED! (0x%08x)\n\n", idcode); exit(-1);} else printf("IDCODE FOUND! (0x%08x)\n\n", idcode);

 return;
}

void insnDRlen(int tck, int tms, int tdo, int tdi, unsigned int insn_len){

	printf("Testing DR length for each instruction\n");
	int i = 0;
	int j = 0;
	int reg_len = 0;
	int checkdataret = 0;
	int insn = 0;
	int DR_len = 0;

	// Get all iterations. */
	for(i=0;i<insn_len;i++){
		DR_len|=(1<<i);
	}

	for(i=0;i<=DR_len;i++){
		tap_state(TAP_RESET, JT.pTCK, JT.pTMS);

		/* Shift in instruction. */
		tap_state(TAP_SHIFTIR, JT.pTCK, JT.pTMS);

		for(j=0;j<insn_len;j++){
			pulse_tdi(JT.pTCK, JT.pTDI, (insn&(1<<j)));
		}

		/* Test DR length. */
		tap_state(TAP_SHIFTDR, JT.pTCK, JT.pTMS);
		checkdataret = check_data(pattern, (2*PATTERN_LEN),
	                		  JT.pTCK, JT.pTDI, JT.pTDO, &reg_len);
		if(checkdataret == 1) {
			printf("INSN: 0x%x | DR length: %d\n", insn, reg_len);
		}
	  insn++;
	}

 return;
}

int main(int argc, char **argv){

	printf("\nPiTag - Jtag Tool for RaspberryPi (ppcasm@gmail.com)\n\n");

	/* Set up GPIO access. */
	setup_io();

	/* Randomize scan pattern to make false positives less likely. */
	randumb_pattern();

	/* Scan for possible JTAG pin configurations. */
	scan();

	/* Test IDCODE. */
	get_idcode(JT.pTCK, JT.pTMS, JT.pTDO, JT.pTDI);

	/* Now with scan/IDCODE done we should have the length of the IR. Let's check DR lengths. */
	insnDRlen(JT.pTCK, JT.pTMS, JT.pTDO, JT.pTDI, JT.IRLEN);

 return 0;
}
