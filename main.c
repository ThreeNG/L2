#pragma interrupt INTIT r_it_interrupt
#include "r_macro.h"  /* System macro and standard type definition */
#include "showLCD.h"
#include "SW.h"
#include "api.h"
	
/******************************************************************************
Macro definitions
******************************************************************************/
/******************************************************************************
Typedef definitions
******************************************************************************/
typedef struct
	{
	int minute;
	int second;
	int centi;
	} time_t;

/******************************************************************************
Imported global variables and functions (from other files)
******************************************************************************/
extern char status;
//extern int time_change_stt;  // flag for paused and counting no record
extern int matchtime;
extern char* stringtime[10];
volatile int G_elapsedTime;   // Timer Counter
time_t curr, prev[19];
char* stringrecord[12];
int timesave;
int lineNo;
int recordNo;
int recordIndex;
int recordflag;
int recordcount;
int show2scount;
int up;
int down;
int upcount;
int downcount;
int recordshowcount;


/******************************************************************************
Exported global variables and functions (to be accessed by other files)
******************************************************************************/


/******************************************************************************
Private global variables and functions
******************************************************************************/
/******************************************************************************
* Function Name: scrollfunction
* Description  : scroll
* Arguments    : none
* Return Value : none
******************************************************************************/
void scrollfunction() 
{	
	if (pollingSW1()) {
					/*print record from [upcount] to [upcount+6] */
					if (upcount < recordcount) {
						upcount++;	
						for (up = 1; up <= 6; up++) {
							recordNo = up + upcount;
							recordIndex = up + upcount - 1;
							lineNo = up*8+ LCD_LINE2;
							sprintf(stringtime, "#%d: %d:%d:%d  ",recordNo, prev[recordIndex].minute, prev[recordIndex].second, prev[recordIndex].centi);
							show_LCD(lineNo, stringtime);
						} else {
							show2scount = 0;
							DisplayLCD(LCD_LINE1,(unsigned char *)"First record");
						}
					}
					
			if (!recordflag) {
				DisplayLCD(LCD_LINE1,(unsigned char *)"No record");
			} else {
				/*show2scount = 0;
				DisplayLCD(LCD_LINE1,(unsigned char *)"First record");*/
			}
		}			
	if (pollingSW2()) {
			if (!recordflag) {
				DisplayLCD(LCD_LINE1,(unsigned char *)"No record");
			} else {
				/*show2scount = 0;
				DisplayLCD(LCD_LINE1,(unsigned char *)"Last record");*/
			}
	}
}

/******************************************************************************
* Function Name: delay_10CentiS
* Description  : delay
* Arguments    : none
* Return Value : none
******************************************************************************/
void delay_10CentiS() 
{
	timesave = G_elapsedTime;
	while (timesave >= G_elapsedTime - 10 );
}

/******************************************************************************
* Function Name: checksw1orsw2
* Description  : check switch 1 switch 2
* Arguments    : none
* Return Value : none
******************************************************************************/
void sw1sw2function()
{	
	if (pollingSW1()){
		timesave = G_elapsedTime;
		while (timesave >= G_elapsedTime - 35 ){
			if (pollingSW2()){
				if (status == 'p') {
					LCD_reset();
					recordflag = 0;
					show2scount = 0;
					recordshowcount = 0;
					recordcount = 0;
					curr.minute = 0;
					curr.second = 0;
					curr.centi = 0;
				} else {
					status = 'p';
					DisplayLCD(LCD_LINE1,(unsigned char *)"Pausing...");
					delay_10CentiS();
				}
				
			} else {
				if ((status != 'p')||(!recordflag)) {
					scrollfunction();
				} else {
					if (recordflag) {
						scrollfunction();
					} else {
						/* do nothing */
					}	
				}
			}
		}
	}

	if (pollingSW2()){
		timesave = G_elapsedTime;
		while (timesave >= G_elapsedTime - 35){
			if (pollingSW1()){
				if (status == 'p') {
					LCD_reset();
					recordflag = 0;
					show2scount = 0;
					recordshowcount = 0;
					recordcount = 0;
					curr.minute = 0;
					curr.second = 0;
					curr.centi = 0;
				} else {
					status = 'p';
					DisplayLCD(LCD_LINE1,(unsigned char *)"Pausing...");
					delay_10CentiS();
					
				}
			} else {
				if ((status != 'p')||(!recordflag)) {
					scrollfunction();
				} else {
					if (recordflag) {
						scrollfunction();
					} else {
						/* do nothing */
					}
				}
			}
		}
	}
}

/******************************************************************************
* Function Name: main
* Description  : Main program
* Arguments    : none
* Return Value : none
******************************************************************************/
void main() {
	R_IT_Create();
	R_IT_Start();
	r_main_userinit();
	LCD_reset();

	recordflag = 0;
	status = 'p';
	//rollupcount = 0;
	//rolldowncount = 0;
	
	/* Infinit loop */
	while (1U){
		switch (status)	{
		case 'p' :
				/* show Pausing... again afer 2 second */
				if (G_elapsedTime >= 200) {
					DisplayLCD(LCD_LINE1,(unsigned char *)"Pausing...");
					G_elapsedTime = 0;
				}
				/* polling switchs */	
				sw1sw2function();
				timesave = G_elapsedTime;
				if (pollingSW3())
					{
					DisplayLCD(LCD_LINE1,(unsigned char *)"Running...");
					delay_10CentiS();
					status = 'r';
					}
				break;
		case 'r' :
				delay_10CentiS();
				/* show Running... again afer 2 second */
				if (show2scount >= 2) {
					DisplayLCD(LCD_LINE1,(unsigned char *)"Running...");
					show2scount = 0;
				}
				/* couting time and print on the first line */
				if (G_elapsedTime >= (curr.centi + 1)) {
					curr.centi = G_elapsedTime;
					if (G_elapsedTime >= 99) {
						curr.second ++;							
						show2scount++;
						G_elapsedTime = 0;
						curr.centi = 0;
					}
					if (curr.second >= 59) {
						curr.minute++;
						curr.second = 0;
					}
					if (curr.minute >= 99) {
						curr.minute = 0;
					}
					sprintf(stringtime, "%d:%d:%d      ",curr.minute, curr.second, curr.centi);
					show_LCD(LCD_LINE2, stringtime);
				}
					
				/* polling switchs */		
				sw1sw2function();
				if (pollingSW3()) {
					if (recordcount <= 20)
						recordcount++;
						
					/*set record from No20 to No2  */
					for (recordNo = recordcount; recordNo >= 1; recordNo--){
						recordIndex = recordNo - 1;
						prev[recordNo].centi = prev[recordIndex].centi;
						prev[recordNo].second = prev[recordIndex].second;
						prev[recordNo].minute = prev[recordIndex].minute;
					}
						
					/* set record No1 */
					prev[0].centi = curr.centi;
					prev[0].second = curr.second;
					prev[0].minute = curr.minute;
					
					/*print record from No1 to recordcount */
					if (recordshowcount < 6)
						recordshowcount++;
					for (recordNo = 1; recordNo <= recordshowcount; recordNo++) {
						recordIndex = recordNo - 1;
						lineNo = recordNo*8+ LCD_LINE2;
						sprintf(stringtime, "#%d: %d:%d:%d  ",recordNo, prev[recordIndex].minute, prev[recordIndex].second, prev[recordIndex].centi);
						show_LCD(lineNo, stringtime);
					}
					delay_10CentiS();
				}
				if (recordcount > 0) { 
					recordflag = 1;
				} else {
					recordflag = 0;
				}
				break;
		default :
				break;
		}
	}
}

__interrupt static void r_it_interrupt(void)
{
    /* Start user code. Do not edit comment generated here */
    G_elapsedTime++;
    /* End user code. Do not edit comment generated here */
}
/******************************************************************************
End of file
******************************************************************************/
