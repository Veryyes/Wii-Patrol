#include <stdio.h>
#include <stdlib.h>
#include <gccore.h>
#include <wiiuse/wpad.h>
#include <network.h>
#include <string.h>
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
	// e.g. printf ("\x1b[%d;%dH", row, column );
	printf("\x1b[2;0H");
	printf("Wii Patrol\n");

	//Initialize Networking
	net_init();
	s32 socket = net_socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
	printf("s32 Socket: %d\n", socket);

	struct sockaddr_in client;
	
	client.sin_len=8;
	client.sin_family=AF_INET;
	client.sin_port=htons(1337);
	client.sin_addr.s_addr=inet_addr("127.0.0.1");

/*	struct sockaddr sock_addr;
	sock_addr.sa_len = 8;
	sock_addr.sa_family=AF_INET;*/
	
	//char buff[32];
	char message[32]="Dogs\n";	

	s32 connection = net_connect(socket, &client, sizeof(client));
	printf("net_connect: %d\n",connection);
	s32 w=net_write(socket,message,strlen(message));
//	s32 n=net_sendto(socket, buff,32,0,&client,sizeof(client));
	printf("net_write: %d\n", w);
	//s32 n=net_read(socket,buff,32);
	//printf("net_read: %d %s\n", n, buff);
	//printf(buff);
	Node* head = NULL;
	Node* tail = NULL;
	ir_t ir;
	
	
	while(1) {
		//Clears Screen
	//	VIDEO_ClearFrameBuffer(rmode, xfb, COLOR_BLACK);

		//Scans IR and Buttons
		WPAD_IR(0,&ir);
		WPAD_ScanPads();
		u32 pressed = WPAD_ButtonsDown(0);
		u32 held = WPAD_ButtonsHeld(0);

	//	printf("\x1b[2;0H");
	//	printf("Wii Patrol\n");
		
		//Draw Path
		Node* curr;
		for(curr=head;curr!=NULL;curr=curr->next)
			FillBox(curr->x, curr->y, 4, 4, COLOR_WHITE);
		
		//Exit on Homebutton
		if ( pressed & WPAD_BUTTON_HOME ) 
			exit(0);

		if ( held & WPAD_BUTTON_A)
		{
			FillBox(ir.x, ir.y, 4, 4, COLOR_WHITE);
			append(&head, &tail, ir.x, ir.y);
			//Draw Square @ cursor location
			//Add cursor location (x,y) to list
		}

		//Draw Cursor
		FillBox(ir.x, ir.y, 8, 8, COLOR_RED);

		if( pressed & WPAD_BUTTON_B){
			/*VIDEO_ClearFrameBuffer(rmode, xfb, COLOR_BLACK);
			printf("\x1b[2;0H");
			printf("Wii Patrol");
			printlist(head);*/
				
			clear(&head, &tail);
		}		
		// Wait for the next frame
		VIDEO_WaitVSync();
	}

	return 0;
}
