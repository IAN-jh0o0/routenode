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
#include <math.h>

struct node{int port,dist;};
struct node routingTable[16];int size=0,sock,localPort,didBC=0;struct sockaddr_in my,to;socklen_t len=sizeof(struct sockaddr_in);struct timespec ts;

void error(char *msg){
    perror(msg);
    exit(0);
}

void printStatusMessages(int mode,int a,int b){
    int i;struct node curr;
    clock_gettime(CLOCK_REALTIME, &ts);
    
    if(mode==1){
        fprintf(stderr,"[%ld] Message sent from Node <port-%d> to Node <port-%d>\n",ts.tv_nsec,a,b);
    }
    else if(mode==2){
        fprintf(stderr,"[%ld] Message received at Node <port-%d> from Node <port-%d>\n",ts.tv_nsec,a,b);
    }
    else if(mode==3){
        fprintf(stderr,"[%ld] Node <port-%d> Routing Table\n",ts.tv_nsec,a);
        for(i=0;i<16;i++){
            curr=routingTable[i];
            if(curr.port==0)break;
            fprintf(stderr,"- (%d) -> Node <port-%d>\n",curr.dist,curr.port);
        }
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

void updateRoutingTable(struct node rt[],int fromPort){
    int i,j,d=0;struct node rt_curr,curr;
    
    //find the distance from localPort -> fromPort
    for(i=0;i<16;i++){
        curr=routingTable[i];
        if(curr.port==0)break;
        if(curr.port==fromPort)d=curr.dist;
    }
    //for each toPort in rt, find toPort in routingTable and update the distance
    for(i=0;i<16;i++){
        rt_curr=rt[i];
        if(rt_curr.port==0)break;if(rt_curr.port==localPort)continue;
        for(j=0;j<16;j++){
            curr=routingTable[j];
            if(curr.port==0){routingTable[size++]=rt_curr;break;}
            if(curr.port==rt_curr.port){
                routingTable[j].dist=(int)fmin(curr.dist,d+rt_curr.dist);break;}
        }
    }
}

void init(void){
    struct hostent *hp;
    
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

void broadcast(void){
    int i;long n;
    
    //for each neighbor, send the routing information
    for(i=0;i<16;i++){
        if(routingTable[i].port==0)break;
        to.sin_port=htons(routingTable[i].port);  //neighbor's port
        n=sendto(sock,routingTable,1024,0,(struct sockaddr*)&to,len);
        if(n<0)error("sendto() failed");
        printStatusMessages(1,localPort,routingTable[i].port);
    }
    didBC=1;
}

void wait_rcv(void){
    long n;struct sockaddr_in from;struct node rt[16];
    bzero(&rt,sizeof(rt));
    
    while(1){
        n=recvfrom(sock,rt,1024,0,(struct sockaddr*)&from,&len);
        if(n<0)error("recvfrom() failed");
        printStatusMessages(2,localPort,ntohs(from.sin_port));
        updateRoutingTable(rt,ntohs(from.sin_port));
        printStatusMessages(3,localPort,0);
        if(didBC==0)broadcast();
    }
}

int main(int argc,const char **argv){
    localPort=atoi(argv[4]);
    init();
    if(strcmp(argv[argc-1],"last")==0){
        getRoutingTable(argc-1,argv);
        broadcast();
    }
    else{
        getRoutingTable(argc,argv);
    }
    wait_rcv();
    
    return 0;
}
