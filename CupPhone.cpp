#include <iostream>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <math.h>
#include <sstream>

#define CUPPHONE_MAX_N_THREADS  	5
#define	CUPPHONE_OK			1
#define	CUPPHONE_ERROR			0
#define	CUPPHONE_FAIL			2
#define CUPPHONE_MSG_BODY_LEN		1024

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
		printf("[CupPhone] ERROR opening socket\n");
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
	printf("[CupPhone] Listener socket successfully openned at PORT %d\n",PORT);	
	//--------------------------------------------------------------
	// SOCKET STAYS LISTENING FOREVER
	//--------------------------------------------------------------
	int receivedMsgLen;
	socklen_t cliLen;	
	typedef struct receivedChonk{
		unsigned int nTotalBytes;		// Message length
		unsigned int chonkSize;			// Independent chonk max length
	}receivedChonk;
		
	unsigned int expectedNChonks, currentPosition, tmpReceivedBytes;
	receivedChonk tmpNewMsgReceived;	
	char MSG_BUFFER[CUPPHONE_MSG_BODY_LEN];
	bzero(MSG_BUFFER, sizeof(CUPPHONE_MSG_BODY_LEN));	
	while( true ){
		//------------------------------------------------------
		// Accept Socket Connection
		//------------------------------------------------------
		listen( sockfd, MAX_INCOMMING_CONNECTIONS );
		cliLen = sizeof(cli_addr);
		newsockfd	 = accept(sockfd, (struct sockaddr *) &cli_addr, &cliLen);
		if (newsockfd < 0){
			printf("[CupPhone] ERROR on accept\n");
			return false;
		}else{
			printf("[CupPhone] Accepted Socket Connection\n");
		}
		//------------------------------------------------------
		// Obtains Command (Replace, Attatch)
		//------------------------------------------------------
		bzero(MSG_BUFFER,CUPPHONE_MSG_BODY_LEN);
		receivedMsgLen = read(newsockfd,MSG_BUFFER,CUPPHONE_MSG_BODY_LEN-1);
		if( receivedMsgLen < 0 ){
			printf("[CupPhone] ERROR reading from socket\n");
			return false;
		}
		std::string strMsgReceived = static_cast<std::string>(MSG_BUFFER);
		printf("[CupPhone] Received Msg Length (%i): %s\n",receivedMsgLen,strMsgReceived.c_str());
		write(newsockfd,"1",1);
		//------------------------------------------------------
		// Obtains File Path 
		//------------------------------------------------------
		bzero(MSG_BUFFER,CUPPHONE_MSG_BODY_LEN);
		receivedMsgLen = read(newsockfd,MSG_BUFFER,CUPPHONE_MSG_BODY_LEN-1);
		if( receivedMsgLen < 0 ){
			printf("[CupPhone] ERROR reading from socket\n");
			return false;
		}
		std::string outputFilePath = static_cast<std::string>(MSG_BUFFER);
		printf("[CupPhone] Received File Path (%i): %s\n",receivedMsgLen,outputFilePath.c_str());
		write(newsockfd,"1",1);
		//------------------------------------------------------
		// Obtains File Size 
		//------------------------------------------------------
		bzero(MSG_BUFFER,CUPPHONE_MSG_BODY_LEN);
		receivedMsgLen = read(newsockfd,MSG_BUFFER,CUPPHONE_MSG_BODY_LEN-1);
		if( receivedMsgLen < 0 ){
			printf("[CupPhone] ERROR reading from socket\n");
			return false;
		}
		std::string outputFileSize = static_cast<std::string>(MSG_BUFFER);
		printf("[CupPhone] Received File Path (%i): %s\n",receivedMsgLen,outputFileSize.c_str());
		//------------------------------------------------------
		// Open File Before Stream File
		//------------------------------------------------------
		write(newsockfd,"1",1);
		//------------------------------------------------------
		// Obtains File Contain 
		//------------------------------------------------------
		bzero(MSG_BUFFER,CUPPHONE_MSG_BODY_LEN);
		receivedMsgLen = read(newsockfd,MSG_BUFFER,CUPPHONE_MSG_BODY_LEN-1);
		if( receivedMsgLen < 0 ){
			printf("[CupPhone] ERROR reading from socket\n");
			return false;
		}
		std::string outputFileChonk = static_cast<std::string>(MSG_BUFFER);
		printf("[CupPhone] Received File Path (%i): %s\n",receivedMsgLen,outputFileChonk.c_str());
		write(newsockfd,"1",1);
		
		
		/*
		//Extract the message and execute instruction identified
		memcpy( &tmpNewMsgReceived, MSG_BUFFER, receivedMsgLen );				
		printf(
				"\n[CupPhone] New Stream Message nTotalBytes(%i) chonkSize(%i)",	
				tmpNewMsgReceived.nTotalBytes,
				tmpNewMsgReceived.chonkSize
			);
		printf("\n");
		*/
		
		
		//------------------------------------------------------
		// Receive Stream of Message Payload
		//------------------------------------------------------
		//currentPosition 	= 0;
		//printf("tmpNewMsgReceived.chonkSize: %i\n",tmpNewMsgReceived.chonkSize);
		//char WHOLE_MSG[tmpNewMsgReceived.nTotalBytes];
		
		//char TMP_BUFFER[tmpNewMsgReceived.chonkSize];
		//WHOLE_MSG 	= (char*)malloc(tmpNewMsgReceived.nTotalBytes);
		//TMP_BUFFER = (char*)realloc(&TMP_BUFFER[0],tmpNewMsgReceived.chonkSize);
		//bzero(TMP_BUFFER,tmpNewMsgReceived.chonkSize);
		/*
		while( currentPosition < tmpNewMsgReceived.nTotalBytes ){			
			tmpReceivedBytes 	= read(newsockfd,TMP_BUFFER,tmpNewMsgReceived.chonkSize);			
			memcpy( &WHOLE_MSG[currentPosition], &TMP_BUFFER[0], tmpReceivedBytes );
			currentPosition		= currentPosition + tmpReceivedBytes;
			printf("tmpReceivedBytes: %i currentPosition: %i \n",tmpReceivedBytes,currentPosition);			
		}
		*/
		
		close(newsockfd);		
		
		
	}
	
	
	return true;
}

int main(int argc, char *argv[]){
	
	//PORT
	int PORT;
	std::string tmpPort;
	tmpPort = "";
	tmpPort.append(argv[1]);
	std::istringstream(tmpPort) >> PORT;	
	
	openListenerSocket(77778);
	
}


