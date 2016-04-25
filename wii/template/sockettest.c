#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <ogcsys.h>
#include <gccore.h>
#include <fcntl.h>
#include <network.h>
#include <debug.h>
#include <errno.h>
#include <unistd.h>
#include <wiiuse/wpad.h>


#if	0
#   define      DEBUG_PRINT(x) \
        net_print_string x
#   define      DEBUG_BINARY(x) \
        net_print_binary x
#else
#   define      DEBUG_PRINT(x)
#   define      DEBUG_BINARY(x)
#endif

#define	OneSecond (1000000)

static void *xfb = NULL;
static GXRModeObj *rmode = NULL;

void *initialise();
void *httpd (void *arg);

static	lwp_t httd_handle = (lwp_t)NULL;
static	lwp_t debug_handle = (lwp_t)NULL;

static int my_func1( int p)
{
	p++;

	return p+1;
}

static int my_func2( int p)
{
	p++;

	return p+1;
}

static void *debug_thread(void *arg) 
{
	int	i = 0;

	DEBUG_PRINT((NULL,0, "After _break(%d) is called.\n", i));
	while (1) {
		i++;
		//_break();
		usleep( 10 * OneSecond);
		my_func1(i);
	}
	return NULL;
}

int main(int argc, char **argv) {
//---------------------------------------------------------------------------------
	s32 ret;

	char localip[16] = {0};
	char gateway[16] = {0};
	char netmask[16] = {0};
	
	xfb = initialise();

	printf ("\nlibogc network demo\n");
	printf("Configuring network ...\n");

	// Configure the network interface
	ret = if_config ( localip, netmask, gateway, TRUE);
	if (ret>=0) {
		printf ("network configured, ip: %s, gw: %s, mask %s\n", localip, gateway, netmask);

		LWP_CreateThread(	&httd_handle,	/* thread handle */ 
							httpd,			/* code */ 
							localip,		/* arg pointer for thread */
							NULL,			/* stack base */ 
							16*1024,		/* stack size */
							50				/* thread priority */ );
	} else {
	}

	DEBUG_PRINT(("before net_print_init()\n"));
	//net_print_init(NULL,0);

	LWP_CreateThread( &debug_handle,	/* thread handle */ 
			debug_thread,		/* code */ 
			NULL,		/* arg pointer for thread */
			NULL,		/* stack base */ 
			16*1024,	/* stack size */
			50		/* thread priority */ );

	
	DEBUG_PRINT((__FILE__,__LINE__, "entering while loop.\n"));


	while(1) {

		VIDEO_WaitVSync();
		WPAD_ScanPads();

		int buttonsDown = WPAD_ButtonsDown(0);
		
		if (buttonsDown & WPAD_BUTTON_HOME) {
			exit(0);
		}
	}

	return 0;
}

const static char http_200[] = "HTTP/1.1 200 OK\r\n";

const static char indexdata[] = "<html> \
                               <head><title>A test page</title></head> \
                               <body> \
                               This small test page has had %d hits. \
                               </body> \
                               </html>";

const static char http_html_hdr[] = "Content-type: text/html\r\n\r\n";
const static char http_get_index[] = "GET / HTTP/1.1\r\n";

//---------------------------------------------------------------------------------
void *httpd (void *arg) {
//---------------------------------------------------------------------------------

	s32 sock, csock;
	int ret;
	u32	clientlen;
	struct sockaddr_in client;
	struct sockaddr_in server;
	char temp[1026];
	static int hits=0;
	
	DEBUG_Init(100,5656); /* unused second parameter*/
	printf("after DEBUG_Init()...\n");

	printf("Before _break() is called.\n");
	_break();
	printf("After _break() is called.\n");

	clientlen = sizeof(client);

	printf( "httpd is up and running...\n");
	sock = net_socket (AF_INET, SOCK_STREAM, IPPROTO_IP);

	if (sock == INVALID_SOCKET) {
      printf ("Cannot create a socket!\n");
    } else {

		memset (&server, 0, sizeof (server));
		memset (&client, 0, sizeof (client));

		server.sin_family = AF_INET;
		server.sin_port = htons (80);
		server.sin_addr.s_addr = INADDR_ANY;
		ret = net_bind (sock, (struct sockaddr *) &server, sizeof (server));
		
		if ( ret ) {

			printf("Error %d binding socket!\n", ret);

		} else {

			if ( (ret = net_listen( sock, 5)) ) {

				printf("Error %d listening!\n", ret);

			} else {
			
				while(1) {
	
					csock = net_accept (sock, (struct sockaddr *) &client, &clientlen);
					my_func2( csock);
				
					if ( csock < 0 ) {
						printf("Error connecting socket %d!\n", csock);
						while(1);
					}

					printf("Connecting port %d from %s\n", client.sin_port, inet_ntoa(client.sin_addr));
					memset (temp, 0, 1026);
					ret = net_recv (csock, temp, 1024, 0);
					printf("Received %d bytes\n", ret);

					if ( !strncmp( temp, http_get_index, strlen(http_get_index) ) ) {
						hits++;
						net_send(csock, http_200, strlen(http_200), 0);
						net_send(csock, http_html_hdr, strlen(http_html_hdr), 0);
						sprintf(temp, indexdata, hits);
						net_send(csock, temp, strlen(temp), 0);
					}

					net_close (csock);
				}
			}
		}
	}
	return NULL;
}

//---------------------------------------------------------------------------------
void *initialise() {
//---------------------------------------------------------------------------------

	void *framebuffer;

	VIDEO_Init();
	WPAD_Init();
	
	rmode = VIDEO_GetPreferredMode(NULL);
	framebuffer = MEM_K0_TO_K1(SYS_AllocateFramebuffer(rmode));
	console_init(framebuffer,20,20,rmode->fbWidth,rmode->xfbHeight,rmode->fbWidth*VI_DISPLAY_PIX_SZ);
	
	VIDEO_Configure(rmode);
	VIDEO_SetNextFramebuffer(framebuffer);
	VIDEO_SetBlack(FALSE);
	VIDEO_Flush();
	VIDEO_WaitVSync();
	if(rmode->viTVMode&VI_NON_INTERLACE) VIDEO_WaitVSync();

	return framebuffer;

}