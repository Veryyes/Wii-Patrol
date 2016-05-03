#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <gccore.h>
#include <wiiuse/wpad.h>
#include <network.h>
#include <string.h>
#include <math.h>
#include "list.h"

static void *xfb = NULL;
static GXRModeObj *rmode = NULL;

void itoa(int integer, char* buff, int buff_len)
{
        if(!integer)
        {
                buff[0]='0';
                buff[1]='\0';
                return;
        }
	if(integer<0)
	{
		char neg='-';
		strcat(buff,&neg);
		buff_len--;
		integer*=-1;
	}
        integer++;
        int digits = (int)ceil(log10(integer));
        integer--;
        for(;digits>0&&buff_len>0;digits--)
        {
                int i;
                int pow=1;
                for(i=digits;i>1;i--)
                        pow = pow *10;
		char digit[2];
                digit[0] = (char)(((integer/pow)%10)+'0');
                digit[1] = '\0';
                strcat(buff,&digit);
                buff_len--;
        }
}

void FillBox(int x1, int y1, int w, int h, int color)
{
	int i;
	int j;
	int x2 = (x1 + w)>> 1;
	x1 >>= 1;
	u32 *tmpfb = xfb; //Graphic context
	for(j=y1; j<=y1+h; j++)
	{
		int tmp = j * 320;
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
	printf("\x1b[2;2H");
	printf("Wii Patrol\n");

	//Initialize Networking
	net_init();
	s32 socket = net_socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
	if(socket>=0)
		printf("  Socket Created\n");
	else
		printf("  Error Creating Socket: %d\n", socket);

//	struct hostent* he = net_gethostbyname("bwong.me");
//	struct in_addr** addr_list = (struct in_addr**) he->h_addr_list;
	struct sockaddr_in client;
		

	client.sin_len=8;
	client.sin_family=AF_INET;
	client.sin_port=htons(1337);
	client.sin_addr.s_addr=inet_addr("192.168.43.5");//Local IP address
	//client.sin_addr = **addr_list;
	s32 connection = net_connect(socket, &client, sizeof(client));
	if(connection>=0)
		printf("  Sucessfully Connected!\n");
	else
		printf("  Could not Connect: %d\n", connection);

	//char lel[5] = "hi\n";
	//net_write(socket, lel, 5);
	Node* head = NULL;
	Node* tail = NULL;
	ir_t ir;
	
	sleep(1); //So we can see connection status

	while(1) {
		//Clears Screen
		VIDEO_ClearFrameBuffer(rmode, xfb, COLOR_BLACK);

		//Scans IR and Buttons
		WPAD_IR(0,&ir);
		WPAD_ScanPads();
		u32 pressed = WPAD_ButtonsDown(0);
		u32 held = WPAD_ButtonsHeld(0);

		printf("\x1b[2;2H");
		printf("Wii Patrol\n");
		
		//Draw Path
		Node* curr;
		for(curr=head;curr!=NULL;curr=curr->next)
			FillBox(curr->x, curr->y, 4, 4, COLOR_WHITE);
		
		//Exit on Homebutton
		if ( pressed & WPAD_BUTTON_HOME )
		{
			net_close(socket);
			exit(0);
		}
		if ((pressed & WPAD_BUTTON_PLUS)&&head!=NULL)
		{
			s32 bytes_written = 0;
			Node* curr = head->next;
			while(curr!=NULL)
			{
				//printf("%d, %d\n", (int)ceil(curr->x - curr->prev->x),(int)ceil(curr->y - curr->prev->y));
				//sleep(1);
				char dx[32];
				memset(&dx,0,32);
				itoa(ceil(curr->x - curr->prev->x), dx, 32);
				char dy[32];
				memset(&dy,0,32);
				itoa(ceil(curr->y - curr->prev->y), dy, 32);
				
				if((ceil(curr->x - curr->prev->x))!=0 || (ceil(curr->y - curr->prev->y))!=0)
				{
	//				printf("%s, ",dx);
					s32 xbytes = net_write(socket, &dx, strlen(dx));
					if(xbytes>0)
					bytes_written += xbytes;
					else
					{	
						printf("  Could not write to socket: %d\n", xbytes);
						sleep(1);
						break;
					}

					char comma = ',';
					net_write(socket,&comma,1);
					bytes_written++;
					
	//				printf("%s\n",dy);
					s32 ybytes = net_write(socket, &dy, strlen(dy));
					if(ybytes>0)
						bytes_written += ybytes;
					else
					{
						printf("  Could not write to socket: %d\n", ybytes);
						sleep(1);
						break;
					}
					
					char space =' ';
					net_write(socket, &space, 1);
					bytes_written++;

				}
				curr = curr->next;
			}
			char newline = '\n'; 
			if(net_write(socket, &newline, 1)>0)
				bytes_written++;
			printf("  Bytes Written: %d\n", bytes_written);
			clear(&head, &tail);
			sleep(3);
		}
		if ( held & WPAD_BUTTON_A)
		{
			FillBox(ir.x, ir.y, 4, 4, COLOR_WHITE);
			append(&head, &tail, ir.x, ir.y);
			//Draw Square @ cursor location
			//Add cursor location (x,y) to list
		}

		//Draw Cursor
		FillBox(ir.x, ir.y, 8, 8, COLOR_RED);

		if( pressed & WPAD_BUTTON_B)
		{
			clear(&head, &tail);
		}		
		// Wait for the next frame
		VIDEO_WaitVSync();
	}

	return 0;
}
