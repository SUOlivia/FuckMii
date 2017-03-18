#include <3ds.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

#include "menu.h"

//Initiate the variables
int  p, r, q, i, PrintBanks=0, i2 = 0, MenuIndex, step=0;
char a[5000], b, o, s[5000], FilePath[262] , header[262] = "Please choose the script to run";
const char *files[255];
PrintConsole topScreen, bottomScreen;
DIR *dp;
struct dirent *ep;
FILE *z;

const char *mode[] =
	{
		"Normal",
		"Step",
		"Update banks",
		"Step + Update banks"
	};

void Wait4key(u32 key)
{
	while(aptMainLoop())
		{
			hidScanInput();
			u32 kDown = hidKeysDown();
			if(key==KEY_A)
				if(kDown & KEY_A)
					break;
			if(key==KEY_START)
				if(kDown & KEY_START)
					break;
			
			gspWaitForVBlank();
		}
}
	
//Function used for listing the files in a folder
void ListDir(char* path, const char* list[])
{
	dp = opendir(path);
	if(dp != NULL)
	{
		while ((ep = readdir(dp)))
		{
			list[i2] = malloc(strlen(ep->d_name) + 1);
			strcpy(list[i2], ep->d_name);
			i2++;
		}
		(void) closedir (dp);
	}
}

//Get character input from software keyboard
int getcharinput(char *out) //Needs to be fixed
{
	char mybuf[2];
	static SwkbdState swkbd;
	SwkbdButton button = SWKBD_BUTTON_NONE;
	swkbdInit(&swkbd, SWKBD_TYPE_NORMAL, 1, -1);
	swkbdSetValidation(&swkbd, 0, 0, 0);
	button = swkbdInputText(&swkbd, mybuf, sizeof(mybuf));
	return 0;
}

void Step()
{
	if(step)
	{
		consoleSelect(&bottomScreen);
		printf("Press A to continue\n");
		Wait4key(KEY_A);
		consoleSelect(&topScreen);
	}
}

//Shows the value of the banks on the bottom screen (Will probably be replaced by current action later)
void Update_banks(char *in)
{
	if(PrintBanks)
	{
		consoleSelect(&bottomScreen);
		for(i=0;i<5000;i++)
			if(in[i]!=0)
				printf("Bank %u: %u\n", i, in[i]);
		consoleSelect(&bottomScreen);
		for(i=0;i<30;i++)printf("-");
		printf("\n");
		//delay(1000);
		consoleSelect(&topScreen);
	}	
}



//This is all where the magic is done :D
int interpret(char *c)
{
	char *d;

	r++;
	while( *c ) {
		//if(strchr("<>+-,.[]\n",*c))printf("%c",*c);
		switch(o=1,*c++) {
		
		case '<': p--; Step(); break; 
		case '>': p++; Step(); break;
		case '+': a[p]++; Update_banks(a); Step(); break; 
		case '-': a[p]--; Update_banks(a); Step(); break; 
		case '.': putchar(a[p]); fflush(stdout); Step(); break;
		case ',': getcharinput(&a[p]);fflush(stdout); Update_banks(a); Step(); break;
		case '[':
			for( b=1,d=c; b && *c; c++ )
				b+=*c=='[', b-=*c==']';
			if(!b) {
				c[-1]=0;
				while( a[p] )
					interpret(d);
				c[-1]=']';
				break;
			}
		case ']':
			puts("UNBALANCED BRACKETS"); return 0;
		case '#':
			if(q>2)
				printf("%2d %2d %2d %2d %2d %2d %2d %2d %2d %2d\n%*s\n",
				       *a,a[1],a[2],a[3],a[4],a[5],a[6],a[7],a[8],a[9],3*p+2,"^");
			break;
		default: o=0;
		}
		if( p<0 || p>100)
			puts("RANGE ERROR"); return 0;
	}
	r--;
	return 0;
}

int main()
{
	//Basic init stuff
	gfxInitDefault();
	consoleInit(GFX_TOP, &topScreen);
	consoleInit(GFX_BOTTOM, &bottomScreen);
	consoleSelect(&topScreen);
	hidInit();
	
	MenuIndex = display_menu(mode, 4, "With which mode do you want to run your script");
	if(MenuIndex==1)
		step=1;
	else if(MenuIndex==2)
			PrintBanks=1;
		else if(MenuIndex==3)
			{
				step=1;
				PrintBanks=1;
			}
	//Find all files in the FuckMii folder in the root of the sd card
	ListDir("/FuckMii", files);
	//Ask the user which file to run (might need to be made better, due for some people to have tons of files in their folder)
	MenuIndex = display_menu(files, i2, header); 
	//Getting ready for the file to be ran	
	snprintf(FilePath, 255, "/FuckMii/%s", files[MenuIndex]);
	consoleClear();
	z=fopen(FilePath,"rb");
	//Read and run file
	if(z)
	{
		fseek(z, 0L, SEEK_END);
		size_t size = ftell(z);
		fseek(z, 0L, SEEK_SET);
		fread(s, 1, size, z);
		interpret(s);
	}
	//Prompt the user to exit the app
	consoleSelect(&bottomScreen);
	printf("Press START to exit");
	Wait4key(KEY_START);
	
	gfxExit();
	return 0;
}