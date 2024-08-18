#include "CupPhone.h"

int main( int argc, char *argv[] ){
	
	sendLocalFile( 
					77778,
					"127.0.0.1",
					"./testImg.png",
					"./CUPPHONE_BIN/testImg.png"
	);
	
	return 0;
}


