/***************************************recvsend_clnt.c***************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUF_SIZE 30
void error_handling(char *message);

int main(int argc, char *argv[])
{
	int sd;
	FILE *fp;
	
	char buf[BUF_SIZE];
	char file_name[BUF_SIZE];
	int read_cnt;
	struct sockaddr_in serv_adr;
	if(argc!=3) {
		printf("Usage: %s <IP> <port>\n", argv[0]);
		exit(1);
	}
	
    //输入文件名
	printf("Input file name: ");
	scanf("%s", file_name);
    //打开文件名
	fp=fopen(file_name, "wb");
    
    //创建套接字
	sd=socket(PF_INET, SOCK_STREAM, 0);  
    //初始化 
	memset(&serv_adr, 0, sizeof(serv_adr));
	serv_adr.sin_family=AF_INET;
	serv_adr.sin_addr.s_addr=inet_addr(argv[1]);
	serv_adr.sin_port=htons(atoi(argv[2]));

	connect(sd, (struct sockaddr*)&serv_adr, sizeof(serv_adr));
    //写入要传输的文件
	write(sd, file_name, strlen(file_name)+1);	

	while((read_cnt=read(sd, buf, BUF_SIZE))!=0)
		fwrite((void*)buf, 1, read_cnt, fp);
	
	fclose(fp);
	close(sd);
	return 0;
}

void error_handling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}


