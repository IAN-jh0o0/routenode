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
#include <unistd.h>

struct node{int port,dist;};
struct node routingTable[16],cpy[16];int size=0,sock,localPort,didBC=0,isUpdated,isAdded,newCost,ARGC;struct sockaddr_in my,to;socklen_t len=sizeof(struct sockaddr_in);struct timespec ts;char **ARGV;

void error(char *msg){
    perror(msg);
    exit(0);
}

void printStatusMessages(int mode,int a,int b){
    int i;struct node curr;
    clock_gettime(CLOCK_REALTIME, &ts);
    
    if(mode==1)fprintf(stderr,"[%ld] Message sent from Node <port-%d> to Node <port-%d>\n",ts.tv_nsec/1000,a,b);
    else if(mode==2)fprintf(stderr,"[%ld] Message received at Node <port-%d> from Node <port-%d>\n",ts.tv_nsec/1000,a,b);
    else if(mode==3){
        fprintf(stderr,"[%ld] Node <port-%d> Routing Table\n",ts.tv_nsec/1000,a);
        for(i=0;i<16;i++){
            curr=routingTable[i];
            if(curr.port==0)break;
            fprintf(stderr,"- (%d) -> Node <port-%d>\n",curr.dist,curr.port);
        }
    }
    else if(mode==4)fprintf(stderr,"[%ld] Link value message sent from Node <port-%d> to Node <port-%d>\n",ts.tv_nsec/1000,a,b);
    else if(mode==5)fprintf(stderr,"[%ld] Link value message received at Node <port-%d> from Node <port-%d>\n",ts.tv_nsec/1000,a,b);
    else if(mode==6)fprintf(stderr,"[%ld] Cost from Node <port-%d> to Node <port-%d> is updated to %d\n",ts.tv_nsec/1000,a,b,newCost);
}

void sortRoutingTable(void){
    int i,j,low;struct node tmp;
    
    for(i=0;i<16;i++){
        low=i;
        if(routingTable[i].port==0)break;
        for(j=i+1;j<16;j++){
            if(routingTable[j].port==0)break;
            if(routingTable[j].port<routingTable[low].port)low=j;
        }
        tmp=routingTable[i];
        routingTable[i]=routingTable[low];
        routingTable[low]=tmp;
    }
}

void getRoutingTable(int argc,char **argv){
    struct node n;size=0;bzero(&routingTable,sizeof(routingTable));
    
    for(int i=5;i<argc;i++){
        n.port=atoi(argv[i]);
        n.dist=atoi(argv[++i]);
        routingTable[size++]=n;
    }
    sortRoutingTable();
}

void updateRoutingTable(struct node rt[],int fromPort){
    int i,j,d=0;struct node rt_curr,curr;isUpdated=0;isAdded=0;
    
    for(i=0;i<16;i++){
        curr=routingTable[i];
        if(curr.port==0)break;
        if(curr.port==fromPort)d=curr.dist;
    }
    for(i=0;i<16;i++){
        rt_curr=rt[i];
        if(rt_curr.port==0)break;if(rt_curr.port==localPort)continue;
        for(j=0;j<16;j++){
            curr=routingTable[j];
            if(curr.port==0){
                rt_curr.dist=d+rt_curr.dist;
                routingTable[size++]=rt_curr;
                isAdded=1;
                break;
            }
            if(curr.port==rt_curr.port){
                routingTable[j].dist=(int)fmin(curr.dist,d+rt_curr.dist);
                if(routingTable[j].dist!=curr.dist)isUpdated=1;
                break;
            }
        }
    }
    if(isAdded)sortRoutingTable();
}

void updateRoutingTableAfterSignal(struct node rt[],int fromPort){
    int i,find,curr;isUpdated=0;
    
    for(i=0;i<16;i++)if(rt[i].port==0)break;
    find=rt[i-1].dist;
    for(i=0;i<16;i++){
        curr=routingTable[i].port;
        if(curr==fromPort){
            if(routingTable[i].dist!=find){
                routingTable[i].dist=find;
                isUpdated=1;
            }
            break;
        }
    }
}

void freeARGV(void){
    int i;
    
    for(i=5;i<ARGC;i++)free(ARGV[i]);
    free(ARGV);
}

void init(void){
    struct hostent *hp;
    
    sock=socket(AF_INET,SOCK_DGRAM,0);
    if(sock<0)error("sock() failed");
    
    bzero(&my,len);
    my.sin_family=AF_INET;
    my.sin_addr.s_addr=INADDR_ANY;
    my.sin_port=htons(localPort);
    if(bind(sock,(struct sockaddr*)&my,len)<0)error("bind() failed");
    
    to.sin_family=AF_INET;
    hp=gethostbyname("localhost");
    bcopy((char*)hp->h_addr,(char*)&to.sin_addr,hp->h_length);
}

void broadcast(void){
    int i;long n;
    
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
        if(strcmp((char *)rt,"Reset routingTable")==0){
            didBC=0;
            getRoutingTable(ARGC,ARGV);
        }
        else if(strcmp((char *)rt,"Link Message")==0){
            n=recvfrom(sock,rt,1024,0,(struct sockaddr*)&from,&len);
            if(n<0)error("recvfrom() failed");
            printStatusMessages(5,localPort,ntohs(from.sin_port));
            updateRoutingTableAfterSignal(rt,ntohs(from.sin_port));
            printStatusMessages(3,localPort,0);
        }
        else{
            printStatusMessages(2,localPort,ntohs(from.sin_port));
            updateRoutingTable(rt,ntohs(from.sin_port));
            printStatusMessages(3,localPort,0);
            if(didBC==0 || isUpdated==1)broadcast();
        }
    }
}

void sigHandler(int signum){
    int i;long n;char *buf;
    
    for(i=0;i<16;i++){
        if(routingTable[i].port==0)break;
        buf="Reset routingTable";
        to.sin_port=htons(routingTable[i].port);
        n=sendto(sock,buf,1024,0,(struct sockaddr*)&to,len);
        if(n<0)error("sendto() failed");
    }
    getRoutingTable(ARGC-2,ARGV);
    routingTable[size-1].dist=newCost;
    printStatusMessages(6,localPort,routingTable[size-1].port);
    to.sin_port=htons(routingTable[size-1].port);
    n=sendto(sock,"Link Message",1024,0,(struct sockaddr*)&to,len);
    n=sendto(sock,routingTable,1024,0,(struct sockaddr*)&to,len);
    broadcast();
}

void intHandler(int signum){
    freeARGV();
    exit(1);
}

int main(int argc,char **argv){
    int i;ARGC=argc;
    ARGV=malloc((argc+1)*sizeof(*ARGV));
    for(i=0;i<argc;i++){
        size_t l=strlen(argv[i])+1;
        ARGV[i]=malloc(l);
        memcpy(ARGV[i],argv[i],l);
    }
    ARGV[argc]=NULL;
    signal(SIGINT,intHandler);
    localPort=atoi(argv[4]);
    init();
    
    if(strcmp(argv[argc-2],"last")==0){
        newCost=atoi(argv[argc-1]);
        signal(SIGALRM,sigHandler);
        alarm(30);
        getRoutingTable(argc-2,argv);
        fprintf(stderr,"%d\n",size);
        printStatusMessages(3,localPort,0);
        broadcast();
    }
    else if(strcmp(argv[argc-1],"last")==0){
        getRoutingTable(argc-1,argv);
        printStatusMessages(3,localPort,0);
        broadcast();
    }
    else{
        getRoutingTable(argc,argv);
        printStatusMessages(3,localPort,0);
    }
    wait_rcv();
    
    return 0;
}
