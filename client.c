#include<sys/socket.h>
#include<netdb.h>
#include<string.h>
#include<stdlib.h>
#include<stdio.h>
#include <unistd.h>
#include <sys/time.h>

#define MAX 150
//#define port 43454
#define SA struct sockaddr



/* PACKET STRUCT */
typedef struct Packet {
            int seqNum ;
            char * checkBits ;
            char * data ;
            char * isEnd ;
} Packet;


int main(int argc, char ** argv)
{
	//input check
    if(argc != 4){
        printf("please invoke program as ./client <host> <port> <filename>") ;
        exit(0) ;
    }
	

	//  BUILD UDP SOCKET
    char* hostString ;
    char* portString ;
    hostString = argv[1] ;
    portString = argv[2] ;
    char* filename = argv[3] ;
    int port = atoi(portString) ;

    char buff[MAX];
    char initialPacket[MAX] ;
    int sockfd,len,n;
    struct sockaddr_in servaddr;
    sockfd=socket(AF_INET,SOCK_DGRAM,0);
    if(sockfd==-1)
    {
        printf("socket creation failed...\n");
        exit(0);
    }
    else{
        printf("Socket successfully created..\n");
    }
	
	/*Socket Timeout setup */
	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = 1000;
	if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO,&tv,sizeof(tv)) < 0) {
		perror("Error");
	} 


    bzero(&servaddr,sizeof(len));
    servaddr.sin_family=AF_INET;
    servaddr.sin_addr.s_addr=inet_addr(hostString);
    servaddr.sin_port=htons(port);
    len=sizeof(servaddr);

 
	//OPEN FILE TO TRANSFER 
    FILE *fileptr;
    char *fileInBytes;
    int filelen;

    fileptr = fopen(filename, "rb");  // Open the file in binary mode
   // printf("%s \n",filename) ;
    fseek(fileptr, 0L, SEEK_END);          // Jump to the end of the file
    filelen = ftell(fileptr);             // Get the current byte offset in the file
    rewind(fileptr);                      // Jump back to the beginning of the file

    fileInBytes = (char *)malloc((filelen+1)*sizeof(char)); // Enough memory for file + \0
    fread(fileInBytes, filelen, 1, fileptr); // Read in the entire file
    fclose(fileptr); // Close the file


		

    //File is now read into buffer. Now we must transmit the file
    /* CREATE INITIAL PACKET */
	int numWindows= filelen/20;
    char* seqNum = "0" ;
    bzero(initialPacket,sizeof(initialPacket));
    strcpy(initialPacket,"check");
    strcat(initialPacket,":");
    //cast seq num to char*
   // char * seqNumString = (char*)&seqNum;
    strcat(initialPacket,seqNum);
    strcat(initialPacket,":");
    strcat(initialPacket,filename);
    strcat(initialPacket,":");
	//strcat(initialPacket,numWindows);
	//strcat(initialPacket,":");
    //cast file length to char*
    /*
    char lengthString[8] ;
    sprintf(lengthString, "%d", filelen);

    strcat(initialPacket,lengthString);
    strcat(initialPacket,":") ;

    printf("packet: %s \n",initialPacket) ;
    */
   // printf("Sending to server--- %s \n",initialPacket) ;


	/* SEND THE INITIAL PACKET */
    sendto(sockfd,initialPacket,sizeof(initialPacket),0,(SA *)&servaddr,len);

	
	/* WAIT ON SERVER RESPONSE */
    //packet has been sent to server wait for servers response before data transmission begins:
    printf("Waiting on Servers response: \n") ;
     recvfrom(sockfd,buff,sizeof(buff),0,(SA *)&servaddr,&len);
    printf("Server says: %s \n",buff) ;

	int windowSize;
    if(buff[0] == '1'){
        printf("begining sending...  \n\n") ;

        int i = 0 ;
        while(buff[i] != ':'){
            i++ ;
        }
        i++ ;

        char windowSizeChar[1] ;
        windowSizeChar[0] = buff[i] ;
        windowSizeChar[1] = '\0' ;
        //printf("windowSizeChar: %s \n", windowSizeChar) ;
        windowSize = atoi(windowSizeChar);
        printf("window Size: %d \n" , windowSize) ;
    }
    else{
        printf("error occured") ;
        close(sockfd);
        exit(0) ;
    }
	


	/*SERVER ACCEPTED CONNECTION CONTINUING */
	//Server has given us approval to send so package up the data and prepare to send it
    

	int packetSize = 5;
    int seqNumInt = 1 ;
	int lastPacket = filelen/packetSize;
    char allPackets[10000][MAX] ;
    int j = 0 ;
    int k = 0 ;
	/* CREATE PACKETS LOOP */
    for(j = 0; j < filelen/packetSize+1; j++){
        char packet[MAX] ;
        bzero(packet,MAX) ;

		/* APPLY CHECK */
        strcpy(packet,"check");
        strcat(packet,":");

		/* LAST PACKET CHECK / FLAG FOR LAST PACKET*/ 
		if(j==lastPacket) 
			strcat(packet,"y");
		else 
			strcat(packet,"n");
			strcat(packet,":");

		/* APPLY SEQUENCE NUMBER  */	
        char seqNumString[4] ;
        sprintf(seqNumString, "%d",seqNumInt );
        strcat(packet,seqNumString);
        strcat(packet,":");
	
		/* APPLY DATA */
        int i = 0 ;
        char dataBuff[3] ;   //? packet data size ? 
		bzero(dataBuff,sizeof(dataBuff));
        for (i ; i < 5 ; i ++){  // 5?  packet data size? 
			if(fileInBytes[k] == EOF){
				dataBuff[i]='\0';
				break;//
			}
            dataBuff[i] = fileInBytes[k] ;
            k++ ;
        }
        //dataBuff[5] = "\0" ;
        strcat(packet,dataBuff) ;
        strcat(packet,":") ;
       	//packet[i] = "\0" ;
        //printf("packet: %s \n",packet) ;
        strcpy(allPackets[j], packet);
        //printf("%s \n",allPackets[j]) ;
       	//bzero(packet,MAX) ;
        seqNumInt++ ;
    }


    /* TEST PRINT ALL PACKETS 
    int i = 0 ; 
    for(i ; i < filelen/5 +1 ; i++){
        printf("%s \n",allPackets[i]) ;
    }
    */



	/* SEND PACKETS */

	// initialize acknowledgements 
    int recNums[10000] ;
    int i ;
    int lastRecAck ;
    for(i = 0 ; i < 10000 ; i++ ){
        recNums[i] = -1 ;
    }
    recNums[0] = 1 ;



	/*	SEND WINDOWS */ 
    int windowNum = 0 ;
	int lastWindow =(lastPacket / windowSize)+1;
	int window_count = 0;
	int sentLastPacket=0;

	printf("lastWindow %d\n",lastWindow);
	for(; windowNum < lastWindow ; windowNum++){
		printf("sending window %d\n",windowNum);

		/*SEND WINDOWSIZE OF PACKETS */
		for(i = 0 ; i < windowSize ; i++){
			if(i+windowNum*windowSize == lastPacket+1){
				printf("sending last packet\n");	
				sentLastPacket=1;
				break;
			}			

			//if(i != 4) 
			{
				printf("Sending: %s \n",allPackets[i+windowNum*windowSize]) ;
				sendto(sockfd,allPackets[i+windowNum*windowSize],sizeof(allPackets[i+windowNum*windowSize]),0,(SA *)&servaddr,len);
			}
		}

		/* RECEIVE ACKNOWLEDGEMENTS */
		int windowComplete = 0;
		while(windowComplete == 0){
		for(i = 0 ; i < windowSize ; i++){
			char ack[4] ;
			recvfrom(sockfd,ack,sizeof(ack),0,(SA *)&servaddr,&len);
			printf("recieved ack: %s \n",ack) ;
			int ackInt = atoi(ack) ;
			recNums[ackInt] = 1;
			//printf("ackInt: %d \n", ackInt);
		}

		/* CHECK FOR MISSING ACKNOWLEDGEMENTS */	
		for(i = 2 ; i < 10000 ; i++){
			if(recNums[i] == -1){
				 lastRecAck = i ;
				 break ;
			}
		}

		/* HANDLE MISSING ACKNOWLEDGEMENTS */
//		if(lastRecAck 

		printf("lastRec-1 %d < %d\n", lastRecAck-1, ((windowNum+1)*windowSize));	
		if(lastRecAck > (windowNum+1)*windowSize){
			printf("REC ACK for next window\n");
			windowComplete =1;
		}	
		else{
			if(sentLastPacket == 1){
				break;
			}
			printf("Need to resend %d\n", lastRecAck-2);	
			printf("RESending %d\n",lastRecAck-2);	
			sendto(sockfd,allPackets[lastRecAck-2],sizeof(allPackets[lastRecAck-2]),0,(SA *)&servaddr,len);
			break;	

			}
		}
		
/*
		if(lastRecAck != (windowNum * windowSize)){
			if(lastRecAck > windowNum*windowSize){
				printf("missing ack past window good to send next?\n");
				
			}
			printf("missing packet %d\n", lastRecAck);

			//handle missing frame
		}
 */   

	} //window send loop






} //end main

    /*
    Packet * allPackets = (Packet*)malloc(1000*sizeof(Packet)) ;
    int i = 0 ;
    int k = 0 ;
    for(;;){
        printf("i: %d \n", i) ;
    // Packet temp ;
     //   temp->seqNum = seqNumBuff ;
      //  temp->checkBits = "check" ;
        char dataBuff[5] ;
        int j = 0 ;
        for(i ; (j < 5) && (fileInBytes[i] != EOF) ; i++){
            dataBuff[j] = fileInBytes[i] ;
            j++ ;
        }
        //printf("databuff: %s\n",dataBuff) ;
      //  temp->data = dataBuff ;
        allPackets[k].seqNum = k ;    // = {k,"check",dataBuff,"n"} ;
        allPackets[k].checkBits = "check" ;
        allPackets[k].data = dataBuff ;
        allPackets[k].isEnd = "n" ;
        //printf("Tempdatabuff: %s\n",temp.data) ;
       // allPackets[k] = temp ;
        if(fileInBytes[i] == EOF){
            break ;
        }
        k++ ;
        //bzero(dataBuff,5);
    }
    printf("here") ;
    int h ;
    for(h = 0 ; h < 10 ; h++){
        printf("%s",allPackets[h].data) ;
    }

    */

        
    // Now that initial packet has been formatted and sent wait for response from server to begin transmission:
   // for(;;)
   // {
  //  }


/*      
        char * response ;
        printf("Do you want to send another file? enter Y or N: \n");
        scanf("%s", response) ;
        if(strncmp(response,"N",1)){
            exit(0) ;
        }
*/
           /*


        bzero(buff,sizeof(buff));

        while((buff[n++]=getchar())!='\n');
        
            sendto(sockfd,buff,sizeof(buff),0,(SA *)&servaddr,len);
        bzero(buff,sizeof(buff));
        recvfrom(sockfd,buff,sizeof(buff),0,(SA *)&servaddr,&len);
        printf("From Server : %s\n",buff);

        if(strncmp("exit",buff,4)==0)
        {
            printf("Client Exit...\n");
            break;
        }
        */
//close(sockfd);
