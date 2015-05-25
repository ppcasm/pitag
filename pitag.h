/************************************************************************
 PiTag - JTAG Tool for RaspberryPi (ppcasm <ppcasm@gmail.com>)


This code is licensed to you under the terms of the GNU GPL, version 2;
see file COPYING or http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
*************************************************************************/

/* USE THIS FILE TO CONFIGURE SPECIFIC SETTINGS. */

/* Set signal pulse timing delay. */
#define DELAYZ 200

/* You can custom define PINS here, once found.*/
#define CUSTOM_PINS FALSE
#define JTAG_TCK 17
#define JTAG_TMS 27
#define JTAG_TDO 3
#define JTAG_TDI 22

/* Include all possible JTAG pins here. (Remember to use the GPIO listing and not pin number.)*/
unsigned char pins[] = {2, 3, 4, 17, 27, 22};

/* Give the pins pretty names. */
char * pinnames[] = { "GPIO2", "GPIO3", "GPIO4", "GPIO17",
                      "GPIO27", "GPIO22"};

/* Ignore active pins (sometimes used to clean list up during scanning.) */
#define IGNORE_ACTIVE TRUE

/* Scan pattern length for testing JTAG config. */
#define PATTERN_LEN     64

/* Scan pattern for testing JTAG configuration. (Overwritten if using randumb function.)*/
char pattern[PATTERN_LEN]= "0110011101001101101000010111001001";

/* Add some useful TAP TMS states. */
#define TAP_RESET                "11111"
#define TAP_SHIFTDR              "111110100"
#define TAP_SHIFTIR              "1111101100"

/* Prototypes */
static void randumb_pattern();
static void pulse_tms(int tck, int tms, int s_tms);
static void pulse_tdi(int tck, int tdi, int s_tdi);
unsigned char pulse_tdo(int tck, int tdo);
static int check_data(char pattern[], int iterations, int tck, int tdi, int tdo, int *reg_len);
void tap_state(char tap_state[], int tck, int tms);
void init_pins(int tck, int tms, int tdi);
static void scan(void);
void setup_io(void);
void get_idcode(int tck, int tms, int tdo, int tdi);
void insnDRlen(int tck, int tms, int tdo, int tdi, unsigned int insn_len);
void set_rpi_conf(void);


