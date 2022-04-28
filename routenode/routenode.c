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
struct node routingTable[16];int size=0,sock;struct sockaddr_in my,to;socklen_t len=sizeof(struct sockaddr_in);

void error(char *msg){
    perror(msg);
    exit(0);
}

void init(int port){
    struct hostent *hp;
    //get socket
    sock=socket(AF_INET,SOCK_DGRAM,0);
    if(sock<0)error("sock() failed");
    
    //my sockaddr_in
    bzero(&my,len);
    my.sin_family=AF_INET;
    my.sin_addr.s_addr=INADDR_ANY;
    my.sin_port=htons(port);
    if(bind(sock,(struct sockaddr*)&my,len)<0)error("bind() failed");
    
    //to sockaddr_in (only handle IP address)
    to.sin_family=AF_INET;
    hp=gethostbyname("localhost");
    bcopy((char*)hp->h_addr,(char*)&to.sin_addr,hp->h_length);
}

//broadcast the routing information
void broadcast(struct node *rt){
    char buf[1024];int i,j;long n;
    
    //for each neighbor, send the information about other neightbors
    for(i=0;i<size;i++){
        to.sin_port=htons(rt[i].port);  //neighbor's port
        for(j=0;j<size;j++){
            if(i==j)continue;
            sprintf(buf,"%d/%d",rt[j].port,rt[j].dist);  //neighbor/dist
            n=sendto(sock,buf,strlen(buf),0,(struct sockaddr*)&to,len);  //send
            if(n<0)error("sendto() failed");
        }
    }
}

void wait_rcv(void){
    struct sockaddr_in from;char buf[1024];
    while(1){
        long n=recvfrom(sock,buf,1024,0,(struct sockaddr*)&from,&len);
        if(n<0)error("recvfrom() failed");
        printf("%s\n",buf);
    }
}

void getRoutingTable(int argc,const char **argv){
    for(int i=5;i<argc;i++){
        struct node n;
        n.port=atoi(argv[i]);
        n.dist=atoi(argv[++i]);
        routingTable[size++]=n;
    }
}

int main(int argc,const char **argv){
    int localPort=atoi(argv[4]);
    init(localPort);
    if(strcmp(argv[argc-1],"last")==0){
        getRoutingTable(argc-1,argv);
        broadcast(routingTable);
    }
    else{
        getRoutingTable(argc,argv);
    }
    wait_rcv();
    
    return 0;
}
