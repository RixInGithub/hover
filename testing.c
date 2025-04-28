#include <stdio.h>
#include <signal.h>

int main() {
	printf("sigint=%d, sig_ign=%d", SIGINT, SIG_IGN);
}