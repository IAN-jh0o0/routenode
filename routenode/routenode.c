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
#include <sys/time.h>

struct node{int port,dist};
struct node routingTable[16];int size=0,sock,localPort;struct sockaddr_in my,to;socklen_t len=sizeof(struct sockaddr_in);struct timespec ts;

void error(char *msg){
    perror(msg);
    exit(0);
}

void printStatusMessages(int mode,int a,int b){
    clock_gettime(CLOCK_REALTIME, &ts);
    if(mode==1){
        fprintf(stderr,"[%ld] Message sent from Node <port-%d> to Node <port-%d>\n",ts.tv_nsec,a,b);
    }
    else if(mode==2){
        fprintf(stderr,"[%ld] Message received at Node <port-%d> from Node <port-%d>\n",ts.tv_nsec,a,b);
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

void updateRoutingTable(struct node rt[]){
    
}

void init(void){
    struct hostent *hp;
    //get socket
    sock=socket(AF_INET,SOCK_DGRAM,0);
    if(sock<0)error("sock() failed");
    
    //my sockaddr_in
    bzero(&my,len);
    my.sin_family=AF_INET;
    my.sin_addr.s_addr=INADDR_ANY;
    my.sin_port=htons(localPort);
    if(bind(sock,(struct sockaddr*)&my,len)<0)error("bind() failed");
    
    //to sockaddr_in (only handle IP address)
    to.sin_family=AF_INET;
    hp=gethostbyname("localhost");
    bcopy((char*)hp->h_addr,(char*)&to.sin_addr,hp->h_length);
}

void broadcast(struct node *rt){
    int i;long n;
    
    //for each neighbor, send the routing information
    for(i=0;i<size;i++){
        to.sin_port=htons(rt[i].port);  //neighbor's port
        n=sendto(sock,routingTable,1024,0,(struct sockaddr*)&to,len);
        if(n<0)error("sendto() failed");
        printStatusMessages(1,localPort,rt[i].port);
    }
}

void wait_rcv(void){
    struct sockaddr_in from;struct node rt[16];
    bzero(&rt,sizeof(rt));
    
    while(1){
        long n=recvfrom(sock,rt,1024,0,(struct sockaddr*)&from,&len);
        if(n<0)error("recvfrom() failed");
        printStatusMessages(2,localPort,ntohs(from.sin_port));
        updateRoutingTable(rt);
    }
}

int main(int argc,const char **argv){
    localPort=atoi(argv[4]);
    init();
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
