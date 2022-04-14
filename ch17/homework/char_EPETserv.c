//边缘触发
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/epoll.h>

#define BUF_SIZE 100
#define MAX_CLNT 256
#define EPOLL_SIZE 50

void setnonblockingmode(int fd);
void error_handling(char *buf);
void send_msg(char * msg, int len);

int clnt_cnt = 0;
int clnt_socks[MAX_CLNT];

int main(int argc, char *argv[])
{
	int serv_sock, clnt_sock;
	struct sockaddr_in serv_adr, clnt_adr;
	socklen_t adr_sz;
	int str_len, i;
	char buf[BUF_SIZE];

	struct epoll_event *ep_events;
	struct epoll_event event;
	int epfd, event_cnt;

	if(argc!=2) {
		printf("Usage : %s <port>\n", argv[0]);
		exit(1);
	}

	serv_sock = socket(PF_INET, SOCK_STREAM, 0);
	memset(&serv_adr, 0, sizeof(serv_adr));
	serv_adr.sin_family = AF_INET;
	serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_adr.sin_port = htons(atoi(argv[1]));
	
	if(bind(serv_sock, (struct sockaddr*) &serv_adr, sizeof(serv_adr)) == -1)
		error_handling("bind() error");
	if(listen(serv_sock, 5) == -1)
		error_handling("listen() error");

	epfd = epoll_create(EPOLL_SIZE);
	ep_events = malloc(sizeof(struct epoll_event)*EPOLL_SIZE);

	setnonblockingmode(serv_sock);
	event.events = EPOLLIN;
	event.data.fd = serv_sock;	
	epoll_ctl(epfd, EPOLL_CTL_ADD, serv_sock, &event);

	while(1)
	{
		event_cnt = epoll_wait(epfd, ep_events, EPOLL_SIZE, -1);
		if(event_cnt == -1)
		{
			puts("epoll_wait() error");
			break;
		}

		puts("return epoll_wait");
		for(i = 0; i < event_cnt; ++i)
		{
			if(ep_events[i].data.fd == serv_sock)
			{
				adr_sz = sizeof(clnt_adr);
				clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_adr, &adr_sz);
				setnonblockingmode(clnt_sock);
				event.events = EPOLLIN|EPOLLET;
				event.data.fd = clnt_sock;
				epoll_ctl(epfd, EPOLL_CTL_ADD, clnt_sock, &event);
				clnt_socks[clnt_cnt++] = clnt_sock;
				printf("connected client: %d \n", clnt_sock);
			}
			else
			{
				while(1)
				{
					str_len = read(ep_events[i].data.fd, buf, BUF_SIZE);
					if(str_len == 0)    // close request!
					{
						epoll_ctl(epfd, EPOLL_CTL_DEL, ep_events[i].data.fd, NULL);
						close(ep_events[i].data.fd);

						for(i = 0; i < clnt_cnt; ++i) 
						{
							if(ep_events[i].data.fd == clnt_socks[i])
							{
								while(i++ < clnt_cnt-1)
									clnt_socks[i] = clnt_socks[i+1];
								break;
							}
						}
						--clnt_cnt;
						printf("closed client: %d \n", ep_events[i].data.fd);
						break;
					}
					else if(str_len < 0)
					{
						if(errno == EAGAIN)
							break;
					}
					else
					{
						send_msg(buf, str_len);
					}
				}
			}
		}
	}
	close(serv_sock);
	close(epfd);
	return 0;
}


void send_msg(char * msg, int len)   // send to all
{
	int i;
	for(i = 0; i < clnt_cnt; ++i)
		write(clnt_socks[i], msg, len);
}

//设置为非阻塞模式
void setnonblockingmode(int fd)
{
	int flag = fcntl(fd, F_GETFL, 0);
	fcntl(fd, F_SETFL, flag|O_NONBLOCK);
}

void error_handling(char *buf)
{
	fputs(buf, stderr);
	fputc('\n', stderr);
	exit(1);
}
