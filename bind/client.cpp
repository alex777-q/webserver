#include<iostream>
#include<sys/types.h>
#include<sys/socket.h>
#include<unistd.h>
#include<arpa/inet.h>
#include<string.h>
#include<vector>

#define SEND_DATA "hello"

int main(int argc, char const *argv[])
{
    int cfd = socket(AF_INET, SOCK_STREAM, 0);
    if(cfd == -1){
        perror("socket");
        exit(-1);
    }

    struct sockaddr_in cliaddr;
    cliaddr.sin_family = AF_INET;
    cliaddr.sin_port = htons(9999);
    cliaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    int ret = connect(cfd, (struct sockaddr *)&cliaddr, sizeof(cliaddr));
    if(ret == -1){
        perror("connect");
        exit(-1);
    }

    ret = send(cfd, SEND_DATA, strlen(SEND_DATA),0);
    if(ret == -1){
        perror("send");
        exit(-1);
    }

    std::cout<< "send data successfully, data: "<< SEND_DATA << std::endl;

    char buf[128];
    ret = recv(cfd, buf, 128, 0);
    if(ret == -1){
        perror("recv");
        exit(-1);
    }
    else if(ret > 0 ){
        std::cout << "recv data successfully,data : "
        << buf << std::endl;
    }

    while(1){
        sleep(3);
    }




    // close(cfd);
    return 0;
}
