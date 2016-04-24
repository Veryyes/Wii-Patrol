#include <stdio.h>
#include <stdlib.h>
#include <gccore.h>
#include <wiiuse/wpad.h>
#include "list.h"

static void *xfb = NULL;
static GXRModeObj *rmode = NULL;

void FillBox(int x1, int y1, int w, int h, int color)
{
	int i;
	int j;
	int x2 = (x1 + w)>> 1; //Not sure why we need to divide by 2
	x1 >>= 1;
	u32 *tmpfb = xfb; //Graphic context
	for(j=y1; j<=y1+h; j++)
	{
		int tmp = j * 320; //why * 320?
		for(i=x1; i<= x2; i++)
		{
			tmpfb[tmp+i] = color;
		}
	}
}
//---------------------------------------------------------------------------------
int main(int argc, char **argv) {
//---------------------------------------------------------------------------------

	// Initialise the video system
	VIDEO_Init();
	
	// This function initialises the attached controllers
	WPAD_Init();
	WPAD_SetVRes(0,640,480);
	WPAD_SetDataFormat(WPAD_CHAN_0, WPAD_FMT_BTNS_ACC_IR);
	// Obtain the preferred video mode from the system
	// This will correspond to the settings in the Wii menu
	rmode = VIDEO_GetPreferredMode(NULL);

	// Allocate memory for the display in the uncached region
	xfb = MEM_K0_TO_K1(SYS_AllocateFramebuffer(rmode));
	
	// Initialise the console, required for printf
	console_init(xfb,20,20,rmode->fbWidth,rmode->xfbHeight,rmode->fbWidth*VI_DISPLAY_PIX_SZ);
	
	// Set up the video registers with the chosen mode
	VIDEO_Configure(rmode);
	
	// Tell the video hardware where our display memory is
	VIDEO_SetNextFramebuffer(xfb);
	
	// Make the display visible
	VIDEO_SetBlack(FALSE);

	// Flush the video register changes to the hardware
	VIDEO_Flush();

	// Wait for Video setup to complete
	VIDEO_WaitVSync();
	if(rmode->viTVMode&VI_NON_INTERLACE) VIDEO_WaitVSync();

	
	// The console understands VT terminal escape codes
	// This positions the cursor on row 2, column 0
	// we can use variables for this with format codes too
	// e.g. printf ("\x1b[%d;%dH", row, column );
	printf("\x1b[2;0H");
	

	printf("Wii Patrol");
	Node* head = NULL;
	Node* tail = NULL;
	ir_t ir;
	while(1) {

		// Call WPAD_ScanPads each loop, this reads the latest controller states
		WPAD_ScanPads();

		// WPAD_ButtonsDown tells us which buttons were held in this loop
		u32 pressed = WPAD_ButtonsDown(0);
		u32 held = WPAD_ButtonsHeld(0);

		// We return to the launcher application via exit
		if ( held & WPAD_BUTTON_HOME ) exit(0);
		if ( held & WPAD_BUTTON_A)
		{
			WPAD_IR(0,&ir);
			FillBox(ir.x, ir.y, 4, 4, COLOR_WHITE);
			append(&head, &tail, ir.x, ir.y);
			//printf("(%f, %f)\n",ir.x, ir.y);
			//Draw Square @ cursor location
			//Add cursor location (x,y) to array
		}
		if( pressed & WPAD_BUTTON_B){
			VIDEO_ClearFrameBuffer(rmode, xfb, COLOR_BLACK);
			printf("\x1b[2;0H");
			printf("Wii Patrol");
			printlist(head);
	//		clear(&head, &tail);
		}
		// Wait for the next frame
		VIDEO_WaitVSync();
	}

	return 0;
}
