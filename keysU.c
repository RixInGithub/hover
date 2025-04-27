// compile this if ur on linux/unix
#include <stdio.h>
#include <unistd.h>
#include <termios.h>

int main() {
	int inp;
	struct termios _oldt, _newt;
	tcgetattr(0,&_oldt);
	_newt = _oldt;
	_newt.c_lflag &= ~(ECHO|ISIG);
	tcsetattr(0,TCSANOW,&_newt);
	inp = getchar();
	tcsetattr(0,TCSANOW,&_oldt);
	printf("%d\n",inp);
	return 0;
}