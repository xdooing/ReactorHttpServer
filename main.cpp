#include<iostream>
#include<unistd.h>
#include"TcpServer.h"
#include<signal.h>
#include<assert.h>
#include<string.h>
using namespace std;

void addsig(int sig, void(handler)(int)) {
	struct sigaction sa;
	memset(&sa, '\0', sizeof(sa));
	sa.sa_handler = handler;
	sigfillset(&sa.sa_mask);
	assert(sigaction(sig, &sa, NULL) != -1);
}



int main(int argc,char**argv){
    unsigned short port = 10000;
    chdir("/home/xdoo/MyReactorServer/web-resource");

    // addsig(SIGPIPE, SIG_IGN);


    TcpServer* server = new TcpServer(port,4);

    server->run();
    
    return 0;
}