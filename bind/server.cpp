#include<iostream>
#include<sys/types.h>
#include<sys/socket.h>
#include<unistd.h>
#include<arpa/inet.h>
#include<string.h>
#include<vector>

int main(int argc, char const *argv[])
{
    int lfd = socket(AF_INET, SOCK_STREAM, 0);
    if(lfd == -1){
        perror("socket");
        exit(-1);
    }

    struct sockaddr_in saddr;
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(9999);
    saddr.sin_addr.s_addr = htonl(INADDR_ANY);

    int ret = bind(lfd, (struct sockaddr *)&saddr, sizeof(saddr));
    if(ret == -1){
        perror("bind");
        exit(-1);
    }

    ret = listen(lfd, 100);
    if(ret == -1){
        perror("listen");
        exit(-1);
    }

    std::vector<int> clifds;
    while(1){
        struct sockaddr_in cliaddr;
        int cliaddr_len = sizeof(cliaddr);
        int clifd = accept(lfd, (struct sockaddr *)&cliaddr, 0);
        if(clifd == -1){
            perror("accept");
            exit(-1);
        }

        char buf[128];
        int len = recv(clifd, buf, 128, 0);
        if(len == -1){
            perror("recv");
            exit(-1);
        }
        else if(len == 0){
            std::cout<<"client close..."<<std::endl;
            break;
        }
        else if(len > 0){
            std::cout<<"client say: "<< buf << std::endl;
            int ret = send(clifd, buf, strlen(buf), 0);
            if(ret == -1){
                perror("send");
                exit(-1);
            }
            std::cout << "send data to client successfully, data: "
                << buf << std::end;
            break;
        }

        clifds.push_back(clifd);



    }


    close(lfd);
    return 0;
}
