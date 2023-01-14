/*************************************************************************
    > File Name: main.cpp
    > Author: lls
    > Mail: lls840308420@163.com
    > Created Time: 2023年01月12日 星期四 13时17分41秒
 ************************************************************************/

#include<iostream>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<errno.h>
#include<fcntl.h>
#include<sys/epoll.h>
#include "locker.h"
#include "threadpool.h"
#include "http_conn.h"
using namespace std;
#define MAX_FD 65535 // 最大的文件描述符个数
#define MAX_EVENT_NUMBER 10000 // 监听的最大的事件数量

// 添加信号捕捉
void addsig(int sig, void(handler)(int)){
	struct sigaction sa;
	memset(&sa, '\0', sizeof(sa));
	sa.sa_handler = handler;
	sigfillset(&sa.sa_mask);
	sigaction(sig, &sa, NULL); 
	// assert( sigaction( sig, &sa, NULL) != -1);
}

// 添加文件描述符到epoll中
extern void addfd(int epollfd, int fd, bool one_shot);

// 从epoll 中删除文件描述符
extern void removefd(int epollfd, int fd);

// 修改文件描述符
extern void modfd(int epollfd, int fd, int ev);
int main(int argc, char **argv){

	if(argc <= 1){
		printf("按照如下格式运行：%s port_number\n", basename(argv[0]));
		exit(-1);
	}

	// 获取端口号
	int port = atoi(argv[1]);

	// 对SIGPIE 信号进行处理
	addsig(SIGPIPE, SIG_IGN);

	// 创建线程池，初始化线程池
	threadpool<http_conn> *pool = NULL;
	try{
		pool = new threadpool<http_conn>;
	}catch(...){
		exit(-1);
	}
	
	// 创建一个数组用于保存所有的客户端信息 
	http_conn *users = new http_conn[MAX_FD];
	
	// 创建监听的套接字
	int lfd = socket(AF_INET, SOCK_STREAM, 0);

	// 设置端口复用
	int reuse = 1;
	setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

	// 绑定
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = INADDR_ANY;	
	int ret = bind(lfd, (struct sockaddr *)&addr, sizeof(addr));
	if(ret == -1){
		perror("bind");
		exit(-1);
	} 

	// 监听
	ret = listen(lfd, 5);
	if(ret == -1){
		perror("listen");
		exit(-1);
	}

	// 创建epoll对象，事件数组，添加
	epoll_event events[MAX_EVENT_NUMBER];
	int epollfd = epoll_create(5);

	// 将监听的文件描述符添加到epoll对象中
	addfd(epollfd, lfd, false);
	http_conn::m_epollfd = epollfd;

	while(true){
		int num = epoll_wait(epollfd, events, MAX_EVENT_NUMBER,-1);
		if((num < 0) && (errno != EINTR)){
			printf("epoll failure!\n");
			break;
		}

		// 循环遍历
		for(int i = 0; i < num; i++){
			int sockfd = events[i].data.fd;
			if(sockfd == lfd){
				// 有客户端连接
				struct sockaddr_in caddr;
				socklen_t caddr_len = sizeof(caddr);
				int cfd = accept(lfd, (struct sockaddr*)&caddr, &caddr_len);
				if(cfd == -1){
					perror("accept");
					exit(-1);
				}

				if(http_conn::m_user_count >= MAX_FD){
					// 目前连接数满了
					// 给客户端写信息，服务器内部正忙
					close(cfd);
					continue;
				}
				// 将新的客户的数据初始化，放到数组中
				users[cfd].init(cfd, caddr);
			}else if(events[i].events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)){
				// 对方异常断开或错误等事件
				users[sockfd].close_conn();

			}else if(events[i].events & EPOLLIN){
				if(users[sockfd].read()){
					// 一次性把所有的数据都读完
					pool->append(users + sockfd);
				}else{
					users[sockfd].close_conn();
				}
			
			}else if(events[i].events & EPOLLOUT){
				if(!users[sockfd].write()){
					// 一次性写完所有数据
					users[sockfd].close_conn();
				}
			}
		}
	}


	close(epollfd);
	close(lfd);
	delete []users;
	delete pool;


	return 0;
}
