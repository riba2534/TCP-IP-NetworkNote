#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>

void error_handling(char *message);

int main(int argc, char* argv[])
{
	int sock;
	struct sockaddr_in serv_addr;

	char msg1[]="Hello server!";
	char msg2[]="I'm client.";
	char msg3[]="Nice to meet you too!";
	char* str_arr[]={msg1, msg2, msg3};
	char read_buf[100];

	int str_len, i;
	
	if(argc!=3){
		printf("Usage : %s <IP> <port>\n", argv[0]);
		exit(1);
	}
	
	sock=socket(PF_INET, SOCK_STREAM, 0);
	if(sock == -1)
		error_handling("socket() error");
	
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family=AF_INET;
	serv_addr.sin_addr.s_addr=inet_addr(argv[1]);
	serv_addr.sin_port=htons(atoi(argv[2]));
	
	if(connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr))==-1) 
		error_handling("connect() error!");

	for(i=0; i<3; i++)
	{
		read(sock, (char*)(&str_len), 4);
		read(sock, read_buf, str_len);
		puts(read_buf);

		str_len=strlen(str_arr[i])+1;
		write(sock, (char*)(&str_len), 4);
		write(sock, str_arr[i], str_len);
	}
	close(sock);
	return 0;
}

void error_handling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}