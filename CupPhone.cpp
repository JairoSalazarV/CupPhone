#include <iostream>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <math.h>
#include <sstream>
#include <netdb.h>
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
// GET FILE FROM DISK TO MEMORY, READY TO BE SENT BY TCP
//======================================================================
std::string file2String( const std::string& fileName ){
    std::ifstream t( fileName.c_str() );
    std::string str;
    t.seekg(0,std::ios::end);
    str.reserve( t.tellg() );
    t.seekg(0,std::ios::beg);
    str.assign(
        (std::istreambuf_iterator<char>(t)),
        std::istreambuf_iterator<char>()
    );
    return str;
}
//======================================================================
// CONNECT SELECTED TCP SOCKET
//======================================================================
int connectSocket( const int& TCP_PORT, char* IP ){
    int sockfd;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0){
        printf("[CupPhone] ERROR opening socket\n");
        return -1;
    }
    server = gethostbyname( IP );
    if (server == NULL) {
        printf("[CupPhone] ERROR No such host\n");
        return -1;
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr,
          (char *)&serv_addr.sin_addr.s_addr,
          server->h_length);
    serv_addr.sin_port = htons(TCP_PORT);
    if (::connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0){
        printf("[CupPhone] ERROR Connecting\n");
        return -1;
    }
    printf("[CupPhone] Socket is open at %i\n",TCP_PORT);
    return sockfd;
}
//======================================================================
// CREATE A SOCKET AND STAYS LISTENING FOR INCOMMING MESSAGES
//======================================================================
bool sendLocalFile( 
					const int& PORT,
					const std::string& IP,
					const std::string& localFilePath,
					const std::string& remoteFilePath
){
	char MSG_BUFFER[CUPPHONE_MSG_BODY_LEN];
    //------------------------------------------------------------------
    // CARGA ARCHIVO
    //------------------------------------------------------------------
	FILE* fileToSend;
    fileToSend = fopen( localFilePath.c_str(), "r");
    if( fileToSend == NULL ){
        printf("[CupPhone] ERROR openning file: %s\n",localFilePath.c_str());
        return false;
    }
    //------------------------------------------------------------------
    // OPEN SOCKET
    //------------------------------------------------------------------
    int sockfd                  = connectSocket( PORT, (char*)IP.c_str() );
    if(sockfd==-1){
        printf("[CupPhone] ERROR reading file: %s\n",localFilePath.c_str());
        fclose(fileToSend);
        return false;
    }
    int wrotenBytes;
    int readBytes;
    //------------------------------------------------------------------
    // SEND REMOTE FILE PATH
    //------------------------------------------------------------------
    wrotenBytes = ::write(sockfd,remoteFilePath.c_str(),remoteFilePath.length()+1);
    if(wrotenBytes<0){
		printf("[CupPhone] ERROR: Sending Remote File Path: %s\n",localFilePath.c_str());
		::close(sockfd);
		fclose(fileToSend);
        return false;
    }
    readBytes = ::read(sockfd,MSG_BUFFER,CUPPHONE_MSG_BODY_LEN);
    //------------------------------------------------------------------
    // SEND FILE LENGTH
    //------------------------------------------------------------------
    fseek(fileToSend, 0L, SEEK_END);
	unsigned int fileSize = ftell(fileToSend);
	fseek(fileToSend, 0L, SEEK_SET);

    std::string tmpPayload = std::to_string(fileSize);
    wrotenBytes = ::write(sockfd,tmpPayload.c_str(),tmpPayload.length()+1);
    if(wrotenBytes<0){
        printf("[CupPhone] ERROR: Sending Remote File Path: %s\n",tmpPayload.c_str());
        ::close(sockfd);
		fclose(fileToSend);
        return false;
    }
    readBytes = ::read(sockfd,MSG_BUFFER,CUPPHONE_MSG_BODY_LEN);
    //------------------------------------------------------------------
    // SEND CONTAIN
    //------------------------------------------------------------------
    std::string tmpFileContain = file2String( localFilePath );
    wrotenBytes = ::write(sockfd,tmpFileContain.c_str(),tmpFileContain.length());
    if( wrotenBytes == tmpFileContain.length() ){
        printf("[CupPhone] File Sent Successfully\n");
    }else{
        printf("[CupPhone] Sending File\n");
        ::close(sockfd);
		fclose(fileToSend);
        return false;
    }
    ::close(sockfd);
    fclose(fileToSend);
    return true;
}
//======================================================================
// CREATE A SOCKET AND STAYS LISTENING FOR INCOMMING MESSAGES
//======================================================================
bool openListenerSocket( const int& PORT, const int& MAX_INCOMMING_CONNECTIONS=1 ){
    //------------------------------------------------------------------
	// CREATE SOCKET
    //------------------------------------------------------------------
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
    //------------------------------------------------------------------
	// SOCKET STAYS LISTENING FOREVER
    //------------------------------------------------------------------
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
		//--------------------------------------------------------------
		// Accept Socket Connection
		//--------------------------------------------------------------
		listen( sockfd, MAX_INCOMMING_CONNECTIONS );
		cliLen = sizeof(cli_addr);
		newsockfd	 = accept(sockfd, (struct sockaddr *) &cli_addr, &cliLen);
		if (newsockfd < 0){
			printf("[CupPhone] ERROR on accept\n");
			return false;
		}else{
			printf("[CupPhone] Accepted Socket Connection\n");
		}
		//--------------------------------------------------------------
		// Obtains Command (Replace, Attatch)
		//--------------------------------------------------------------
		bzero(MSG_BUFFER,CUPPHONE_MSG_BODY_LEN);
		receivedMsgLen = read(newsockfd,MSG_BUFFER,CUPPHONE_MSG_BODY_LEN-1);
		if( receivedMsgLen < 0 ){
			printf("[CupPhone] ERROR reading from socket\n");
			return false;
		}
		std::string strMsgReceived = static_cast<std::string>(MSG_BUFFER);
		printf("[CupPhone] Received Msg Length (%i): %s\n",receivedMsgLen,strMsgReceived.c_str());
		write(newsockfd,"1",1);
		//--------------------------------------------------------------
		// Obtains File Path 
		//--------------------------------------------------------------
		bzero(MSG_BUFFER,CUPPHONE_MSG_BODY_LEN);
		receivedMsgLen = read(newsockfd,MSG_BUFFER,CUPPHONE_MSG_BODY_LEN-1);
		if( receivedMsgLen < 0 ){
			printf("[CupPhone] ERROR reading from socket\n");
			return false;
		}
		std::string outputFilePath = static_cast<std::string>(MSG_BUFFER);
		printf("[CupPhone] Received File Path (%i): %s\n",receivedMsgLen,outputFilePath.c_str());
		write(newsockfd,"1",1);
		//--------------------------------------------------------------
		// Obtains File Size 
		//--------------------------------------------------------------
		bzero(MSG_BUFFER,CUPPHONE_MSG_BODY_LEN);
		receivedMsgLen = read(newsockfd,MSG_BUFFER,CUPPHONE_MSG_BODY_LEN-1);
		if( receivedMsgLen < 0 ){
			printf("[CupPhone] ERROR reading from socket\n");
			return false;
		}
		std::string outputFileSize = static_cast<std::string>(MSG_BUFFER);
		printf("[CupPhone] Received File Path (%i): %s\n",receivedMsgLen,outputFileSize.c_str());
		//--------------------------------------------------------------
		// Open File Before Stream File
		//--------------------------------------------------------------
		write(newsockfd,"1",1);
		//--------------------------------------------------------------
		// Obtains File Contain 
		//--------------------------------------------------------------
		bzero(MSG_BUFFER,CUPPHONE_MSG_BODY_LEN);
		receivedMsgLen = read(newsockfd,MSG_BUFFER,CUPPHONE_MSG_BODY_LEN-1);
		if( receivedMsgLen < 0 ){
			printf("[CupPhone] ERROR reading from socket\n");
			return false;
		}
		std::string outputFileChonk = static_cast<std::string>(MSG_BUFFER);
		printf("[CupPhone] Received File Path (%i): %s\n",receivedMsgLen,outputFileChonk.c_str());
		write(newsockfd,"1",1);

		close(newsockfd);		
		
		
	}
	
	
	return true;
}

int main( int argc, char *argv[] ){
	
	sendLocalFile( 
					77778,
					"192.168.100.100",
					"./testImg.png",
					"./CUPPHONE_BIN/testImg.png"
	);
	
	return 0;
}


