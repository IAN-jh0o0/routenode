//
//  routenode.c
//  routenode
//
//  Created by Ian Choi on 4/28/22.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>

struct node{
    int port,dist;
};

void error(char *msg){
    perror(msg);
    exit(0);
}

void broadCast(struct node *rt,int localPort){
    int sock,i;
    socklen_t length;
    long n;
    struct sockaddr_in to;
    struct hostent *hp;
    char buf[1024];
    
    sock=socket(AF_INET,SOCK_DGRAM,0);
    if(sock<0)error("sock() failed");
    to.sin_family=AF_INET;
    hp=gethostbyname("localhost");
    bcopy((char*)hp->h_addr,(char*)&to.sin_addr,hp->h_length);
    for(i=0;i<sizeof(rt);i++){
        to.sin_port=htons(rt[i].port);
        sprintf(buf,"%d/%d",localPort,rt[i].dist);
        length=sizeof(struct sockaddr_in);
        n=sendto(sock,buf,strlen(buf),0,(struct sockaddr*)&to,length);
        if(n<0)error("sendto() failed");
    }
}

struct node routingTable[16];
void getRoutingTable(int argc,const char **argv){
    int tmp=0;
    for(int i=5;i<argc;i++){
        struct node n;
        n.port=atoi(argv[i]);
        n.dist=atoi(argv[++i]);
        routingTable[tmp++]=n;
    }
}

int main(int argc,const char **argv){
    int localPort=atoi(argv[4]);
    if(strcmp(argv[argc-1],"last")==0){
        getRoutingTable(argc-1,argv);
        broadCast(routingTable,localPort);
    }
    else{
        getRoutingTable(argc,argv);
    }
    return 0;
}
