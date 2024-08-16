#include <iostream>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>

#define CUPPHONE_MAX_N_THREADS  	5
#define	CUPPHONE_OK			1
#define	CUPPHONE_ERROR			0
#define	CUPPHONE_FAIL			2
#define	CUPPHONE_MSG_BODY_LEN		1024

//#include <pthread.h>
//#include <stdlib.h>
//#include <sys/types.h> 
//#include <sys/socket.h>
//#include <ifaddrs.h>
//#include <netdb.h>
//#include <string>
//#include <math.h>
//#include <fstream>
//#include <iostream>
//#include <sys/stat.h>
//#include <sstream>
//#include <streambuf>
//#include <ctime>

using namespace std;

//======================================================================
// CREATE A SOCKET AND STAYS LISTENING FOR INCOMMING MESSAGES
//======================================================================
bool openListenerSocket( const int& PORT, const int& MAX_INCOMMING_CONNECTIONS=1 ){
	//--------------------------------------------------------------
	// CREATE SOCKET
	//--------------------------------------------------------------
	int sockfd, newsockfd;
	struct sockaddr_in serv_addr, cli_addr;	
	sockfd 	= socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0){
		printf("[CupPhone] ERROR opening socket");
		return false;
	}
	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family 		= AF_INET;
	serv_addr.sin_addr.s_addr 	= INADDR_ANY;
	serv_addr.sin_port 		= htons(PORT);
	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0){
		printf("[CupPhone] ERROR on binding %d",PORT);
		return false;
	}
	//Obtiene IP local
	printf("[CupPhone] Listener socket successfully openned at PORT %d\n",PORT);	
	//--------------------------------------------------------------
	// SOCKET STAYS LISTENING FOREVER
	//--------------------------------------------------------------
	int receivedMsgLen;
	socklen_t clilen;
	char BUFFER[CUPPHONE_MSG_BODY_LEN];
	struct receivedChonk{
		unsigned char idMsg;			// Id instruction
		unsigned int consecutive;		// Consecutive
		unsigned int numTotMsg;			// Total number of message to send
		unsigned int bodyLen;			// Message lenght
		int trigeredTime;           		// Time before to apply console command
		char payload[CUPPHONE_MSG_BODY_LEN];	// message's payload
	}receivedChonk;
	while( true ){
		listen(sockfd,MAX_INCOMMING_CONNECTIONS);
		clilen = sizeof(cli_addr);
		newsockfd	 = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
		if (newsockfd < 0){
			printf("[CupPhone] ERROR on accept");
			return false;
		}
		bzero(BUFFER,CUPPHONE_MSG_BODY_LEN);
		receivedMsgLen = read(newsockfd,BUFFER,CUPPHONE_MSG_BODY_LEN-1);
		if( receivedMsgLen < 0 ){
			printf("[CupPhone] ERROR reading from socket");
			return false;
		}

		//Ordering frame received
		//msgCurrentPosition = receivedMsgLen-headerLen;       
		memcpy( &receivedChonk, BUFFER, receivedMsgLen );

		//Extract the message and execute instruction identified
		printf("\n[CupPhone] idMessage(%i) n(%i)",receivedChonk.idMsg,receivedMsgLen);
		printf("\n");
	}
	
	
	return true;
}

int main(int argc, char *argv[]){
	openListenerSocket(77778);
	
}


