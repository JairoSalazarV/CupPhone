#include <iostream>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <math.h>
#include <sstream>
#include <fstream>

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
		
	unsigned int expectedNChonks, currentPosition;
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
		// Obtains File Path 
		//------------------------------------------------------
		bzero(MSG_BUFFER,CUPPHONE_MSG_BODY_LEN);
		receivedMsgLen = read(newsockfd,MSG_BUFFER,CUPPHONE_MSG_BODY_LEN);
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
		receivedMsgLen = read(newsockfd,MSG_BUFFER,CUPPHONE_MSG_BODY_LEN);
		if( receivedMsgLen < 0 ){
			printf("[CupPhone] ERROR reading from socket\n");
			return false;
		}
		std::string tmpOutputFileSize 	= static_cast<std::string>(MSG_BUFFER);
		unsigned int outputFileSize	= atoi(tmpOutputFileSize.c_str());
		printf("[CupPhone] Received File Size (%i): %i\n",receivedMsgLen,outputFileSize);
		write(newsockfd,"1",1);	
		//------------------------------------------------------
		// Obtains File Contain 
		//------------------------------------------------------
		currentPosition = 0;
		std::ofstream fp;
		fp.open( outputFilePath.c_str(), std::ios::out | std::ios::binary );	
		bool primeraVez = true;	
		while( currentPosition < outputFileSize ){
			//----------------------------------------------
			// Receive chonk and append it into received file
			//----------------------------------------------
			bzero(MSG_BUFFER,CUPPHONE_MSG_BODY_LEN);			
			receivedMsgLen 	= read(newsockfd,MSG_BUFFER,CUPPHONE_MSG_BODY_LEN+1);			
			fp.write(MSG_BUFFER,receivedMsgLen-1);
			currentPosition += receivedMsgLen-1;
			//----------------------------------------------
			// Show Status to Users
			//----------------------------------------------
			//currentPosition	= currentPosition + receivedMsgLen - 1;			
			printf("receivedMsgLen: %i currentPosition: %i (of %i)\n",receivedMsgLen,currentPosition,outputFileSize);
			printf( "\nCONTENIDO: " );
			for(int i=0; i<receivedMsgLen; i++ ){
				printf( "%c", MSG_BUFFER[i] );
			}
			printf( "\n" );
			//----------------------------------------------
			// Append into file
			//----------------------------------------------					
			if( currentPosition >= outputFileSize ){
				printf("Successfully Transmited\n");
				fflush(stdout);
			}else{
				//printf("currentPosition: %i outputFileSize: %i \n",currentPosition,outputFileSize);
				//receivedFile << MSG_BUFFER;
			}			
			write(newsockfd,"1",1);
		}
		fflush(stdout);
		fp.close();
		close(newsockfd);
		
		
		
		
		/*
		//------------------------------------------------------
		// Obtains File Contain 
		//------------------------------------------------------
		std::remove(outputFilePath.c_str());
		FILE* outputFile;
		outputFile = fopen(outputFilePath.c_str(),"w");
		currentPosition=0;		
		while( currentPosition < outputFileSize ){
			//----------------------------------------------
			// Receive chonk and append it into received file
			//----------------------------------------------
			bzero(MSG_BUFFER,CUPPHONE_MSG_BODY_LEN);			
			receivedMsgLen 	= read(newsockfd,MSG_BUFFER,CUPPHONE_MSG_BODY_LEN+1);
			//----------------------------------------------
			// Append into file
			//----------------------------------------------
			for(int i=0; i<receivedMsgLen-1; i++ ){
				fprintf(outputFile,(char*)&MSG_BUFFER[i]);
				currentPosition++;
			}
			//----------------------------------------------
			// Show Status to Users
			//----------------------------------------------		
			printf("receivedMsgLen: %i currentPosition: %i (of %i)\n",receivedMsgLen,currentPosition,outputFileSize);			
			if( currentPosition >= outputFileSize ){
				printf("Successfully Transmited\n");
			}
			write(newsockfd,"1",2);
		}
		fflush(stdout);
		fclose(outputFile);
		close(newsockfd);
				*/
		
		
	}
	
	
	return true;
}

std::string file2String( const std::string &fileName )
{
	std::ifstream t(fileName.c_str());
	std::string str;
	t.seekg(0,std::ios::end);
	str.reserve(t.tellg());
	t.seekg(0,std::ios::beg);
	str.assign(
					(std::istreambuf_iterator<char>(t)),
					std::istreambuf_iterator<char>()
			  );
	return str;
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


