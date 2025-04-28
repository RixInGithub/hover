/***********************************************
	hover -- Heavily OVerexaggerated Editor (Really!)
	A terminal-based code editor.
	2024-present, RixTheTyrunt + contributors.
	Licensed under the MIT License.
***********************************************/

#include "keys.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

// globals
char*file;
char**fileTxt={};
int fileLines=0;
int size[2]={0,0};
// globals
#ifdef _WIN32
#include <conio.h>
#include <windows.h>

BOOL WINAPI windows__idek1(DWORD a) {
	return(TRUE); // the functions honest reaction to that information: idgaf
}
#else
#include <sys/ioctl.h>
#include <unistd.h>
#include <termios.h>
#include <signal.h>

struct termios oldt, newt;
#endif
// non-windows requirements

// cross-compatability
void getTermSize() {
	#ifdef _WIN32
		HANDLE hConsole=CreateFile("CONOUT$",-1073741824,3,0,3,0,0);
		CONSOLE_SCREEN_BUFFER_INFO bufInf;
		if(hConsole==INVALID_HANDLE_VALUE)return;
		if(!(GetConsoleScreenBufferInfo(hConsole,&bufInf)))return;
		CloseHandle(hConsole);
		SMALL_RECT win = bufInf.srWindow;
		size[0]=win.Right-win.Left+1;
		size[1]=win.Bottom-win.Top+1;
	#else
		struct winsize win;
		if(!(ioctl(STDOUT_FILENO,TIOCGWINSZ,&win)+1))return; // -1+1=0 which is a falsy value, yuppy!
		size[0]=win.ws_col;
		size[1]=win.ws_row;
	#endif
}

void preventCtrlC() {
	#ifdef _WIN32
		SetConsoleCtrlHandler(windows__idek1,TRUE);
	#else
		signal(2,SIG_IGN);
	#endif
}

void unpreventCtrlC() {
	#ifdef _WIN32
		SetConsoleCtrlHandler(0,false);
	#else
		signal(2,SIG_DFL);
	#endif
}

char zeroEchoGetchar() {
	char res;
	#ifdef _WIN32
		res=_getch();
		return res;
	#else
		struct termios _oldt, _newt;
		tcgetattr(0,&_oldt);
		_newt = _oldt;
		_newt.c_lflag &= ~ECHO;
		tcsetattr(0,TCSANOW,&_newt);
		res = getchar();
		tcsetattr(0,TCSANOW,&_oldt);
		return res;
	#endif
}

void curVis(bool show) {
	#ifdef _WIN32
		HANDLE hConsole=CreateFile("CONOUT$",-1073741824,3,0,3,0,0);
		CONSOLE_CURSOR_INFO curInf;
		if(hConsole==INVALID_HANDLE_VALUE)return;
		GetConsoleCursorInfo(hConsole, &curInf);
		curInf.bVisible = show;
		SetConsoleCursorInfo(hConsole, &curInf);
	#else
		printf("\x1b[?25%c",104+(show*4)); // if show is false, it will be h, else l
	#endif
}
// cross-compatability

// all platforms funcs
char* getFileName(int argc, char*argv[]) {
	if(argc<2)return"[untitled]";
	FILE *fIO = fopen(argv[1], "r");
	if (fIO) {
		fclose(fIO);
		return argv[1];
	}
	return(getFileName)(0,0); // returns whatever is the default again
}

bool fileExists(char* file) {
	char**inp=malloc(16); // sizeof(char*)*2
	inp[1]=file;
	return(getFileName)(2,inp)==file;
}

char* repeat(char* str, int n) {
	if(n<=0)return"";
	char*res=malloc(strlen(str)*n+1);
	strcpy(res,str);
	// this makes res the string to append to str.
	n--;
	while(n>0){
		strcat(res,str);
		n--;
	}
	return(res);
}

char* chr2str(char c) { // also works with ints!
	char*out=malloc(2); // sizeof(char) * 2?
	out[0]=c; // i have no idea why i placed snprintf here instead of the current thing
	return(out);
}

char** getFileLines(char* f, int* len) { // supports \r, \r\n, \n
	if(f==(getFileName)(0,0))return(NULL); // a file named "[untitled]" surely cant exist, right?
	FILE* fIO = fopen(f, "r");
	if(!fIO)return(NULL);
	char**res=NULL;
	int resS=0;
	char*line=NULL;
	int lineS=0;
	int ch, lastCh = 0;
	while ((ch = fgetc(fIO)) != EOF) {
		if ((ch==10)||(ch==13)) {
			if(!((lastCh==13)&&(ch==10))){ // if we are on a \r\n, skip it lol
				if (lineS>0) {
					line=realloc(line,lineS+1);
					line[lineS]="\0"[0];
					res=realloc(res,8*(resS+1));
					res[resS++]=line;
					line=NULL;
					lineS=0;
				}
			}
			lastCh = ch;
			continue;
		}
		line = realloc(line, lineS + 2);
		line[lineS++] = ch;
		lastCh = ch;
	}
	if (lineS > 0) {
		line=realloc(line,lineS+1);
		line[lineS]="\0"[0];
		res=realloc(res,8*(resS+1));
		res[resS++]=line;
	}
	fclose(fIO);
	*len = resS;
	return res;
}
// all platforms funcs

// main functionality
void printTxt() {
	int count=0;
	while(count<(size[1]-1)){
		if(count<fileLines)printf("%s",fileTxt[count]); // otherwise just print blank line
		if(count+1<(size[1]-1))printf(" \n",count);
		count++;
	}
	fflush(stdout);
}

int main(int argc, char*argv[]) {
	curVis(false);
	bool run = true;
	bool ok = true;
	file = getFileName(argc,argv);
	fileTxt = getFileLines(file,&fileLines);
	char inp=0;
	char looped=false;
	while((inp!=CTRLC)&&(ok)){
		if(looped)printf("\x1b[%dA\x1b[1000D", size[1]);
		preventCtrlC();
		getTermSize();
		char*spaces=repeat("\x20",size[0]-strlen(file));
		printf("\x1b[7m");
		printf(file);
		printf("%s",spaces);
		printf("\x1b[0m\n");
		printTxt();
		// run=false;
		free(spaces);
		unpreventCtrlC();
		inp=zeroEchoGetchar();
		looped=true;
	}
	printf("Goodbye.\n");
	curVis(true);
	return!(ok);
}
// main functionality