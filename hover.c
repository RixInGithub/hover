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

#ifdef _WIN32
#include <conio.h>
#include <windows.h>

void onWindowsStart() {
	
}

void onWindowsEnd() {
	
}
#else
#include <sys/ioctl.h>
#include <unistd.h>
#include <termios.h>
#include <signal.h>

struct termios oldt, newt;
void onWindowsStart(){}
void onWindowsEnd(){}
#endif
// non-windows requirements

// cross-compatability
int* getTermSize() {
	int*out=malloc(8); // sizeof(int) * 2
	#ifdef _WIN32
		HANDLE hConsole=CreateFile("CONOUT$",-1073741824,3,0,3,0,0);
		CONSOLE_SCREEN_BUFFER_INFO bufInf;
		if(hConsole==INVALID_HANDLE_VALUE)return(out);
		if(!(GetConsoleScreenBufferInfo(hConsole,&bufInf)))return(out);
		CloseHandle(hConsole);
		SMALL_RECT win = bufInf.srWindow;
		out[0]=win.Right-win.Left+1;
		out[1]=win.Bottom-win.Top+1;
		return out; // this should NEVER jump out of the windows if block.
	#else
		struct winsize win;
		if(!(ioctl(STDOUT_FILENO,TIOCGWINSZ,&win)+1))return(out); // -1+1=0 which is a falsy value, yuppy!
		out[0]=win.ws_col;
		out[1]=win.ws_row;
		return out;
	#endif
}

void preventCtrlC() {
	#ifdef _WIN32
		SetConsoleCtrlHandler(0,true);
	#else
		tcgetattr(0,&oldt);
		newt=oldt;
		newt.c_lflag&=-393;
		tcsetattr(0,0,&newt);
		signal(2,SIG_IGN);
	#endif
}

void unpreventCtrlC() {
	#ifdef _WIN32
		SetConsoleCtrlHandler(0,false);
	#else
		tcsetattr(0,TCSANOW,&oldt);
		signal(2,SIG_DFL);
	#endif
}

char zeroEchoGetchar() {
	char res;
	#ifdef _WIN32
		res=_getch();
	#else
		struct termios _oldt, _newt;
		tcgetattr(0,&_oldt);
		_newt = _oldt;
		_newt.c_lflag &= ~(ECHO|ISIG);
		tcsetattr(0,TCSANOW,&_newt);
		res = getchar();
		tcsetattr(0,TCSANOW,&_oldt);
	#endif
	return res;
}
// cross-compatability

// all platforms funcs/vars
char** fileTxt;

char* getFileName(int argc, char*argv[]) {
	if(argc<2)return"[untitled]";
	FILE *fIO = fopen(argv[1], "r");
	if (fIO) {
		fclose(fIO);
		return argv[1];
	}
	return(getFileName)(0,0); // returns whatever is the default again
}

char** safeLines(char**lines,int*oLen,int len) {
	while(*oLen<len){
		lines[*oLen]=NULL;
		(*oLen)++;
	}
	return(lines);
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
	snprintf(out, strlen(out), "%c", c);
	return(out);
}

char** getFileLines(char*f, int*oLen) { // supports \r, \r\n, \n
	char** res;
	if(f==(getFileName)(0,0))return(res); // a file named "[untitled]" surely cant exist, right?
	FILE* fIO = fopen(f, "r");
	int resS;
	char*line;
	int lineS=0;
	int ch,lastCh=0;
	while((ch=fgetc(fIO))!=EOF){
		if((ch==10)||(ch==13)){
			if(!((lastCh==13)&&(ch==10))){ // if we are on a \r\n, skip it lol
				line=realloc(line,lineS+1);
				line[lineS]=0;
				res=realloc(res,sizeof(char*)*(resS++));
				res[resS]=line;
				line=0;
				lineS=0;
			}
			lastCh=ch;
			continue;
		}
		line=realloc(line,lineS++);
		line[lineS]=ch;
		lastCh=ch;
	}
	if(lineS){
		line=realloc(line,lineS+1);
		line[lineS]=0;
		res=realloc(res,sizeof(char*)*(resS++));
		res[resS]=line;
	}
	fclose(fIO);
	*oLen=resS;
	return(res);
}
// all platforms funcs

// main functionality
void printTxt() {
	
}

int main(int argc, char*argv[]) {
	onWindowsStart();
	bool run = true;
	bool ok = true;
	char*file = getFileName(argc,argv);
	int*size;
	char*spaces;
	char inp;
	while((run)&&(ok)){
		preventCtrlC();
		size = getTermSize();
		spaces = repeat("\x20",size[0]-strlen(file));
		printf("\x1b[7m");
		printf("%s",file);
		printf("%s",spaces);
		printf("\x1b[0m\n");
		free(spaces);
		free(size);
		printTxt();
		// run=false;
		printf("\x1b[1A");
		// printf("\x1b[%dA", size[1]);
		unpreventCtrlC();
		inp=zeroEchoGetchar();
		run=inp!=CTRLC;
	}
	onWindowsEnd();
	return!(ok);
}
// main functionality