

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#define PORT 9034
// port we’re listening on
#define STDIN 0

char* get_ip(char* to){
	if(strcmp(to,"shivam")==0)return "192.168.1.102";
	if(strcmp(to,"shashi")==0)return "192.168.1.104";
	if(strcmp(to,"lad")==0)return "192.168.1.105";
	
}

char* get_user(char* to){
	if(strcmp(to,"192.168.1.102")==0)return "shivam";
	if(strcmp(to,"192.168.1.104")==0)return "shashi";
	if(strcmp(to,"192.168.1.105")==0)return "lad";
}

int main(void)
{	
	
	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = 500000;
	fd_set master;
	char s[121];
	char to[20];
	char msg[100];
	fd_set read_fds; 
	fd_set write_fds;
	struct hostent *he;
	char buf[256];

	struct sockaddr_in myaddr;
	struct sockaddr_in remoteaddr; // client address
	struct sockaddr_in their_addr; // client address
	int fdmax;
	int listener;
	int sender[4];
	int sender_count=0;
	int newfd;
	
	int nbytes;
	int yes=1;
	int addrlen;
	int i, j;
	FD_ZERO(&master);
	FD_ZERO(&read_fds);
	if ((listener = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("socket");
		exit(1);
	}
	for(int i=0;i<4;i++){
		if ((sender[i] = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
			perror("socket");
			exit(1);
		}
	}
	if (setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes,
		sizeof(int)) == -1) 
	{
		perror("setsockopt");
		exit(1);
	}
	myaddr.sin_family = AF_INET;
	myaddr.sin_addr.s_addr = INADDR_ANY;
	myaddr.sin_port = htons(PORT);
	memset(&(myaddr.sin_zero), '\0', 8);
	if (bind(listener, (struct sockaddr *)&myaddr, sizeof(myaddr)) == -1) {
		perror("bind");
		exit(1);
	}
	if (listen(listener, 10) == -1) {
		perror("listen");
		exit(1);
	}
	FD_SET(listener, &master);
	fdmax = listener; 
	FD_SET(STDIN, &master);
	if(STDIN > fdmax)
		fdmax =STDIN;
	for(;;) 
	{
		read_fds = master; // copy it
		if (select(fdmax+1, &read_fds, NULL, NULL, &tv) == -1) {
			perror("select");
			exit(1);
		}
		for(i = 0; i <= fdmax; i++) {
			if (FD_ISSET(i, &read_fds)) { // we got one!!
				if (i == listener) {
					addrlen = sizeof(remoteaddr);
					if ((newfd = accept(listener, &remoteaddr, &addrlen)) == -1) {
						perror("accept");
					} else 
					{
						FD_SET(newfd, &master); // add to master set
						if (newfd > fdmax) {
							fdmax = newfd;
						}
						printf("selectserver: new connection from %s on "
							"socket %d\n", inet_ntoa(remoteaddr.sin_addr), newfd);
					}
				} 
				else if (i == STDIN){
					bzero(s,121);
					bzero(to,20);
					bzero(msg,100);

										scanf(" %[^\n]*c",s);

					int i;
					for( i=0;i<strlen(s);i++){
						if(s[i]=='/')break;
						to[i]=s[i];
					}
					int j=0;
					i++;
					while(i<strlen(s)){
						msg[j]=s[i];
						i++;
						j++;
					}
					struct sockaddr_in addr;
					int len=sizeof(addr);
					char *ip;
					int flag=1;
					for(int j=0;j<sender_count;j++){
						if(getpeername(sender[j],&addr,&len)!=-1){
							ip = inet_ntoa(addr.sin_addr);
							if(strcmp(ip,get_ip(to))==0){
								flag=0;
								int m = send(sender[j],msg,strlen(msg),0);
							}
						}	
					}
					if(flag){
						if(sender_count==4){
							perror("connection linit exceed ");
							exit(1);
						}
						if ((he=gethostbyname(get_ip(to))) == NULL) 
						{
							perror("gethostbyname");
							exit(1);
						}
						their_addr.sin_family = AF_INET; // host byte order
						their_addr.sin_port = htons(PORT);  // short, network byte order
						their_addr.sin_addr = *((struct in_addr *)he->h_addr);
						bzero(&(their_addr.sin_zero), 8);   // zero the rest of the struct
						
						if (connect(sender[sender_count], (struct sockaddr *)&their_addr, sizeof(struct sockaddr)) == -1) 
						{
							perror("connect");
							exit(1);
						}
						int m = getpeername(sender[sender_count],&addr,&len);
						int n = send(sender[sender_count],msg,strlen(msg),0);
						sender_count++;
					}


				}
				else {
					bzero(buf,256);
					if ((nbytes = recv(i, buf, sizeof(buf), 0)) <= 0) {
						if (nbytes == 0) {
							printf("selectserver: socket %d hung up\n", i);
						} else {
							perror("recv");
						}
						close(i); // bye!
						FD_CLR(i, &master); // remove from master set
					} 
					else {
						struct sockaddr_in addr;
						int len=sizeof(addr);
						char *ip;
						int flag=1;
						if(getpeername(i,&addr,&len)!=-1){
							ip = inet_ntoa(addr.sin_addr);
							printf("%s : %s\n",get_user(ip),buf);
						}	
						else {
							perror("error");
							exit(1);
						}
						bzero(buf,256);
						
						
					}
				} 
			}
		}
	}
	return 0;
}