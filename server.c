/*
Author: Cameron Parlman
Description: 
	ftp client/server using a udp connection 

*/

//SERVER
#include <sys/time.h>
#include<stdio.h>
#include<netinet/in.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netdb.h>
#include<string.h>
#include<stdlib.h>

#define MAX 300
//#define PORT 43454
#define SA struct sockaddr
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
    printf("Initial Packet From client: %s \n",buff);


	/*Socket Timeout setup */
	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = 1000;
	if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO,&tv,sizeof(tv)) < 0) {
		perror("Error");
	}



	/* PARSE INITIAL PACKET */
    //char checkBits[checkbitsSize] ;
	int checkbitsSize = 6;
    int seqNum ,i ;

	char* token = strtok(buff, ":");
	//checkbits
	char * checkBits = (char*)malloc(sizeof(char)*(sizeof(token)+1));
	memcpy(checkBits, token, sizeof(char) * (sizeof(token)+1) );
	//int checkbitsSize = sizeof(checkBits);

	//sequence number
	token = strtok(NULL, ":");
	char * seqNumString = (char*)malloc(sizeof(char)*(sizeof(token)+1));
	memcpy(seqNumString, token, sizeof(char) * (sizeof(token)+1) );
	seqNum = atoi(seqNumString);	
	free(seqNumString);

	//filename
	token = strtok(NULL, ":");
	char * filename = (char*)malloc(sizeof(char)*(sizeof(token)+1));
	memcpy(filename, token, sizeof(char) * (sizeof(token)+1) );
	
	//filesize 
	token = strtok(NULL, ":");
	char * fileLengthString = (char*)malloc(sizeof(char)*(sizeof(token)+1));
	memcpy(fileLengthString, token, sizeof(char) * (sizeof(token)+1) );
	int filesize = atoi(fileLengthString) ;
	free(fileLengthString);
	token = strtok(NULL, ":");

	printf("check size:\t%d\n", checkbitsSize);
    printf("check bits:\t%s\n",checkBits) ;
    printf("SeqNum:\t\t%d\n", seqNum) ;
   	printf("Filename:\t%s\n", filename) ;
    printf("FileSize:\t%d\n", filesize) ;
	printf("\n");

	//return 0;
	


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

	char * lastpacket = (char*)malloc(sizeof(char)); 	// last packet flag 	
	while(lastFlag == 0 ){
		/* RECEIVE A WINDOW */
		for( h = 0 ; h < windowSize ; h ++){
			//zero/initialize values
			bzero(buff,sizeof(buff)) ;
			//bzero(checkBits,sizeof(checkBits)) ;

			/* TIMEOUT ? waiting for packet */
			/* RECEIVE PACKET */
			//TODO set up an if packet received,.. catch errors. resend.
			recvfrom(sockfd,buff,sizeof(buff),0,(SA *)&cli,&clen);
			printf("Recieved: %s \n", buff) ;
			packetCount++;

			
			/* PARSE DATA PACKETS */
			/* PARSE CHECK BITS */
			char* tmpBuff = (char*)malloc(sizeof(char)*(sizeof(buff)));
			memcpy(tmpBuff, buff, sizeof(char) * (sizeof(buff)));
			char* Data;
			//check

			//memcpy(checkBits, token, sizeof(char) * (sizeof(token)+1) );
			
			token = strtok(buff, ":"); 
			if(token){
				//checkbits
				memcpy(checkBits, token, strlen(token)+1);	
				printf("CheckB: %s\n", checkBits);
				printf("CheckTok: %s\n", token);
				printf("Checksize: %ld\n", sizeof(token)+1);

				/* PARSE LAST BIT FLAG */
				token = strtok(NULL, ":"); 
				memcpy(lastpacket, token, strlen(token));
				printf("lastp: %s\n", lastpacket);
				
				/* PARSE SEQUENCE NUMBER */
				token = strtok(NULL, ":"); 
				seqNum = atoi(token) ;
				printf("seqNu: %d\n", seqNum);

				/*PARSE DATA SIZE */ 
				token = strtok(NULL, ":"); 
				int datasize = atoi(token);
				printf("dataS: %d\n", datasize);
			
				/* PARSE DATA FROM PACKET */
				token = strtok(NULL, ":"); 
				Data = (char*)malloc(sizeof(char)*sizeof(token)+1);
				memcpy(Data, token, strlen(token));	
				printf("Data : %s\n\n", Data);
				
				//int dataTokenSize = sizeof(char)*(sizeof(token)+1);
				int dataTokenSize = strlen(token);//sizeof(char)*(sizeof(token)+1);
				if(dataTokenSize != datasize){
					printf("UNEQUAL, tokensize:%d\tdatasize:%d\n\n",dataTokenSize, datasize);
				}

			}
						/*FLAG THAT PACKET WAS RECEIVED */
			/* and correct */
			recNums[seqNum] = 1 ;

			/* STORE PACKETS */
			allPackets[seqNum].data = malloc(strlen(Data));
			allPackets[seqNum].checkBits = malloc(strlen(checkBits));
			allPackets[seqNum].seqNum = seqNum;
			strcpy(allPackets[seqNum].data, Data);
			strcpy(allPackets[seqNum].checkBits, checkBits);

			//free memory 
			if(tmpBuff)
				free(tmpBuff);
			if(Data)
				free(Data);
		

			
			if(*lastpacket == 'y'){
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
			if(*lastpacket == 'y'){
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








