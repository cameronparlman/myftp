//UDP SOCKETS CHAT APPLICATION (SERVER & CLIENT) USING C


//SERVER
#include <sys/time.h>
#include<stdio.h>
#include<netinet/in.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netdb.h>
#include<string.h>
#include<stdlib.h>
#define MAX 150
//#define PORT 43454
#define SA struct sockaddr
/*
void func(int sockfd)
{
    printf("in func");
    char buff[MAX];
    int n,clen;
    struct sockaddr_in cli;
    clen=sizeof(cli);

    printf("before for") ;
    for(;;)
    {
        printf("here") ;
        bzero(buff,MAX);
        recvfrom(sockfd,buff,sizeof(buff),0,(SA *)&cli,&clen);
        printf("From client: %s",buff);

        /*
        bzero(buff,MAX);
        n=0;

        while((buff[n++]=getchar())!='\n');
            sendto(sockfd,buff,sizeof(buff),0,(SA *)&cli,clen);

        if(strncmp("exit",buff,4)==0)
        {
            printf("Server Exit...\n");
            break;
        }
        
    }
}
*/

/* PACKET STRUCT */
typedef struct {
	int seqNum ;
	char * checkBits ;
	char * data ;
} Packet;

/* MAIN */
int main(int argc, char ** argv)
{
	/* INPUT CHECK */
    if(argc != 2){
        printf("please invoke program as ./server <port>") ;
        exit(0) ;
    }

	/* UDP SOCKET SETUP */
    char* portString ;
    portString = argv[1] ;
    int port = atoi(portString) ;
    int sockfd;
    struct sockaddr_in servaddr;
    sockfd=socket(AF_INET,SOCK_DGRAM,0);

    if(sockfd==-1)
    {
        printf("socket creation failed...\n");
        exit(0);
    }
    else
    {
        printf("Socket successfully created..\n");
    }
    bzero(&servaddr,sizeof(servaddr));
    servaddr.sin_family=AF_INET;
    servaddr.sin_addr.s_addr=htonl(INADDR_ANY);
    servaddr.sin_port=htons(port);
	
	/* BIND SOCKET */
    if((bind(sockfd,(SA *)&servaddr,sizeof(servaddr)))!=0)
    {
        printf("socket bind failed...\n");
        exit(0);
    }
    else
    {
        printf("Socket successfully binded..\n");
    }


    char buff[MAX];
    int clen;
    struct sockaddr_in cli;
    clen=sizeof(cli);
	int num_windows =5;

	/* RECEIVE INITIAL PACKET */
    bzero(buff,MAX);
    recvfrom(sockfd,buff,sizeof(buff),0,(SA *)&cli,&clen);
    printf("From client: %s \n",buff);

	/*Socket Timeout setup */
	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = 1000;
	if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO,&tv,sizeof(tv)) < 0) {
		perror("Error");
	}



	/* PARSE INITIAL PACKET */
    char filename[32] ;
    char checkBits[5] ;
    char seqNumString[5];
    int seqNum ,i ;
	
	/* PARSE CHECKBITS */
    for(i = 0 ; buff[i] != ':' ; i++){
        checkBits[i] = buff[i] ;
    }
   	//printf("check bits: %s \n",checkBits) ;
    //printf("%s \n",buff[i]) ;


	/* PARSE SEQUENCE NUMBER */
    i++ ;
    int j = 0;
    for( ; buff[i] != ':' ; i++){
        seqNumString[j] = buff[i] ;
        j++ ;
    }
    seqNum = atoi(seqNumString) ;
   	//printf("SeqNum: %d \n",seqNum) ;
    //printf("I: %d \n",i) ;


	/* PARSE FILENAME */
    i++ ;
    j = 0 ;
    for(i ; buff[i] != ':' ; i++){
        filename[j] = buff[i] ;
        j++ ;
    }
    filename[j] = '\0' ;
	i++;


	/* PARSE NUMBER OF WINDOWS */
/*
	char windowString[32];
	j=0;
	for(;buff[i] != ':'; i++){
		windowString[j] = buff[i];	
		j++;	
	}
	
	num_windows = atoi(windowString);	
	printf("numWindows %d\n", num_windows);
*/
	

	int windowSize = 7; 
	char windowSizeString[1] ;
	sprintf(windowSizeString,"%d",windowSize) ;
   	//printf("filename: %s \n", filename) ;


	/* ANALYZE INITIAL PACKET CHECK / SEND ACK FOR FILE TRANS */
    if(strcmp(checkBits,"check") == 0){
        //ok to begin sending
        bzero(buff,MAX);
        strcpy(buff,"1:");
        strcat(buff,windowSizeString) ;
        sendto(sockfd,buff,sizeof(buff),0,(SA *)&cli,clen);
    }
	/* INITIAL PACKET CHECK FAIL. TERMINATE */ 
    else{
        bzero(buff,MAX);
        strcpy(buff,"0");
        sendto(sockfd,buff,sizeof(buff),0,(SA *)&cli,clen);
        printf("packet was damaged durring transmission closing socket...") ;
        close(sockfd);
        exit(0) ;
    }

    //wait for data packets

	/* SETUP FOR RECEIVING DATA */
    char window[windowSize][MAX];

	char lastpacket ; 			// last packet flag 	
    Packet allPackets[10000]; 	// all packets being stored 

	/* 	INITIALIZE ACKNOWLEDGEMENTS */
    int recNums[10000] ;
    for(i = 0 ; i < 10000 ; i++ ){
        recNums[i] = -1 ;
    }
    recNums[0] = 1 ;
	

	
	/* RECEIVE DATA PACKETS */	
	
			
    int h ;
	int windowNum = 0;
	int packetCount=0;
	int lastPacketNum = 10000;
	int lastFlag = 0;
	while(lastFlag == 0 ){
		/* RECEIVE A WINDOW */
		for( h = 0 ; h < windowSize ; h ++){
			//zero/initialize values
			bzero(buff,sizeof(buff)) ;
			bzero(checkBits,sizeof(checkBits)) ;

			/* TIMEOUT ? waiting for packet */
			/* RECEIVE PACKET */
			recvfrom(sockfd,buff,sizeof(buff),0,(SA *)&cli,&clen);
			printf("Recieved: %s \n", buff) ;
			packetCount++;

			
			/* PARSE DATA PACKETS */
			/* PARSE CHECK BITS */
			for(i = 0 ; buff[i] != ':' ; i++){
				checkBits[i] = buff[i] ;
			}
			//printf("CheckBits: %s \n",checkBits ) ;
			i++;


			/* PARSE LAST BIT FLAG */
			lastpacket = buff[i];
			//printf("Last Packet: %c",lastpacket) ;


			/* PARSE SEQUENCE NUMBER */
			i+=2;
			int j = 0;
			for(i = i; buff[i] != ':' ; i++){
				//printf("in loop \n") ;
				seqNumString[j] = buff[i] ;
				j++ ;
			}
			seqNumString[j]='\0';
			seqNum = atoi(seqNumString) ;
			//printf("SeqNum: %d\n",seqNum) ;


			/*FLAG THAT PACKET WAS RECEIVED */
			recNums[seqNum] = 1 ;

		
			/* PARSE DATA FROM PACKET */
			char dataBuff[3] ;
			bzero(dataBuff,sizeof(dataBuff));
			i++ ;
			j = 0 ; 
			for(i = i; buff[i] != ':' ; i++){
				if(buff[i]=='\0'){
					dataBuff[j]=buff[i];
					break;
				}
				dataBuff[j] = buff[i] ;
				j++ ;
			}
			//printf("Data: %s\n",dataBuff) ;


			//Packet temp ;
			//temp.seqNum = seqNum ; // = {k,"check",dataBuff,"n"} ;
			//temp.checkBits = checkBits ;
			//temp.data = dataBuff ;

			
			/* STORE PACKETS */
			allPackets[seqNum].data = malloc(strlen(dataBuff));
			allPackets[seqNum].checkBits = malloc(strlen(checkBits));
			allPackets[seqNum].seqNum = seqNum;
			strcpy(allPackets[seqNum].data, dataBuff);
			strcpy(allPackets[seqNum].checkBits, checkBits);

			
			if(lastpacket == 'y'){
				printf("FLAG LAST PACKET\n");	
				lastPacketNum = seqNum;
				lastFlag=1;
			}
			//check the recNum array to find next expected packet and send ack
			/* FIND SEND ACK FOR NEXT PACKETS / MISSING */

			printf("packetCount = %d\n", lastPacketNum);
			for(i = 0 ; i < 10000 ; i++){
				if(recNums[i] == -1){
					char ack[4];
					sprintf(ack, "%d",i );
				   	printf("Sending Ack: %s \n", ack) ;
					sendto(sockfd,ack,sizeof(ack),0,(SA *)&cli,clen);
				   break ;
				}
			}





			windowNum++;
			/// check for last packet 
			if(lastpacket == 'y'){
				lastPacketNum = seqNum;
				lastFlag=1;
				break;
			}
		} //end receive windowsize of packets 
	}//end recieve windows 
	
		
	FILE *fpOut;
		fpOut = fopen("output.txt", "wb");
	//print packet data 
    for(i = 1 ; i < packetCount+1  ; i++ ){
		fprintf(fpOut, "%s", allPackets[i].data);	
        printf("ALLPACKETS: DATA: %s\t",allPackets[i].data) ;
        printf("seqNum: %d\t",allPackets[i].seqNum) ;
        printf("checkBits: %s\n",allPackets[i].checkBits) ;
    }

	
	
}//end main

/*



		for(i = 0 ; i < windowSize ; i++){
    		//receive packet store into buffer 
            //printf("looping \n") ;
    		recvfrom(sockfd,buff,sizeof(buff),0,(SA *)&cli,&clen);
    		printf("REC %s\n", buff);

            strcpy(window[i],buff) ;
            printf("Window: %s \n",window[i]) ;
    		//parse checkbits 
    		for(i = 0 ; buff[i] != ':' ; i++){
    			checkBits[i] = buff[i] ;
    		}
    		i++;
    		//parse last bit flag
    		lastpacket = buff[i];	

    		//parse sequence number 
    		i+=2;
    		int j = 0;
    		for(i = i; buff[i] != ':' ; i++){
    			//printf("in loop \n") ;
    			seqNumString[j] = buff[i] ;
    			j++ ;
    	    	}
    		seqNumString[j]='\0';
    		seqNum = atoi(seqNumString) ;
    		//printf("sequence num %s \n",seqNumString);
            recNums[seqNum] = 1 ;
    		//check off sequence numbers 
    		packetCheck[seqNum] = *seqNumString;  	
    	//	packetCheck[seqNum - numwindows * windowSize] = 't';	
    	//	int index = seqNum - numwindows * windowSize;
    		printf("recieved packets %s \n", packetCheck);

            if(lastpacket == 'y'){
               break;
            }   
           // printf("im here");

            for(i = 0 ; i < 10000 ; i++){
                if(recNums[i] == -1){
                    //send ack for i
                    char ack[4];
                    sprintf(ack, "%d",i );
                  // printf("Sending Ack: %s \n", ack) ;
                    sendto(sockfd,ack,sizeof(ack),0,(SA *)&cli,clen);
                   break ;
                }
            }

    //		printf("received packets %s  index = %d \n", packetCheck, index);
        }			

		
*/

    /*
    for(;;)
    {
        bzero(buff,MAX);
        recvfrom(sockfd,buff,sizeof(buff),0,(SA *)&cli,&clen);
        printf("From client: %s",buff);
    }
    */




   // close(sockfd);


//References 
// http://mcalabprogram.blogspot.com/2012/01/udp-sockets-chat-application-server.html
