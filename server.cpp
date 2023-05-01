/*
Author:			Nate Dunlap
Date: 			4/29/2023
Description:	a program that create a server that accepts request to format source code
				based upon the following protocol specification 


<request> ::= <header><options><nl><text_or_code>
<header>  ::= ASTYLE<nl>
<options> ::= (style|mode)=<avalue><nl>|SIZE=<digits><nl>
<avalue>  ::= [a-zA-Z]
<digits>  ::= [0-9]+
<text_or_code>    ::= .*

<reply>   ::= <rephdr><sizeopt><text_or_code>
<rephdr>  ::= ok<nl>
<sizeopt> ::= SIZE=<digits><nl>

<error>   ::= <ehdr><sizeopt><text_or_code>
<ehdr>    ::= ERROR<nl>
*/




#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <iostream>
#include <cstring>
#include <map>
#include <fstream>
#include <stdexcept>


//define global variables

#define PORT 8007
#define MAX_FILE_SIZE 20*1024
bool Req_Error = false;
std::string Req_Estr = "";


//define function prototoypes
void  ASErrorHandler(int errorNumber, const char* errorMessage);
char*  ASMemoryAlloc(unsigned long memoryNeeded);
extern "C" char*  AStyleMain(const char* sourceIn,const char* optionsIn, void (* fpError)(int, const char*), char* (* fpAlloc)(unsigned long));
void handle_connect(int newsockfd);



void parse(FILE* fp, std::map<std::string,std::string> &options, std::string &doc){


	//initialize local buffer and variables
	char buf[2048];
	std::string header = "";
	int size = 0;


	//if fgets fails to read then throw exception
	if(!fgets(buf, sizeof(buf), fp)){

		throw std::runtime_error("Unexpected end of file when reading header");
	}


	// move buf into a string and trim the newline
	header = buf;

	header = header.substr(0,header.find("\n"));


	// if the header is not "ASTYLE"
	if(header != "ASTYLE"){

		throw std::runtime_error("Expected header ASTYLE but got " + header + "\n");
	}	

	
	// read in parameters
	while(fgets(buf, sizeof(buf), fp )){


		// in the case of only a newline then beak
		if(buf[0] == '\n'){
			break;
		}

		std::string trim_line = buf;
		std::string key;
		std::string value;


		// determine where "=" is withing the string
		int pos = trim_line.find("="); 

		// if there exists an "=" present in the string
		if(pos != -1){

			// split string into key and value
			key = trim_line.substr(0,pos);
			value = trim_line.substr(pos+1);


			// throw bad option if not matching specification
			if(key != "SIZE" && key != "mode" && key != "style"){

				throw "Bad option";
			}
		}
		else{

			throw "Bad option";
		}

		// convert value to integer and store in size
		if(key == "SIZE"){

			size = stoi(value);
		}

		//insert (key,value) into options map
		options["SIZE"] = value;
	}

	// if expected input size is beyond the working bounds
	if(size <= 0){
		throw "Bad code size";
	}
	if(size > MAX_FILE_SIZE){

		throw "Bad code size";
	}


	// for as long as there are bytes to read, read into doc
	while(fgets(buf, size+1, fp) && size > 1){
		
		std::string str_line = buf;
		doc += str_line;
		size -= strlen(buf);

	}
}



int main(int argc, char* argv[]) {
	int sockfd, newsockfd;
	socklen_t clilen;
	struct sockaddr_in cli_addr, serv_addr;

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {

		printf("Failed to create socket\n");
	}

	bzero((void *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(PORT);


	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) { 
		
		printf("Failed to bind\n");
	}
	
	
	listen(sockfd, 5);

	for (;;) {
		clilen = sizeof(cli_addr);
		newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
		if (newsockfd < 0) {
			printf("Failed to accept socket");
		}
		handle_connect(newsockfd);
	}

}
void handle_connect(int newsockfd) {

	// declare file pointer, map, and doc variables

	FILE *fp = fdopen(newsockfd, "r+");
	std::map<std::string, std::string> options;
	std::string doc = "";



	// attempt to parse a user request and throw exceptions as needed=
	try {
		parse(fp, options, doc);
	}
	catch(const char* err){
		
		Req_Estr = err;
		std::string ret_message("ERR\nSIZE=" + std::to_string(Req_Estr.size()) + "\n\n" + Req_Estr.c_str() + "\n");
        fprintf(fp, "%s", ret_message.c_str());

		fclose(fp);
		return;
	}


	// delcare empty string to store for response
	std::string parameter = "";


	// iterate through map and populate the string
	std::map<std::string, std::string>::iterator it;
	for(auto it=options.begin(); it!=options.end(); it++){

		if(it->first != "SIZE"){

			std::string value = it->first + "=" + it->second + "\n";
			parameter.append(value);

		}
	}


	// handle errors and format code 
	Req_Error = false;
    Req_Estr = "";
    char* textOut = AStyleMain(doc.c_str(), parameter.c_str(), ASErrorHandler, ASMemoryAlloc);
    if (Req_Error) {
        std::string ret_message("ERR\nSIZE=" + std::to_string(Req_Estr.size()) + "\n\n" + Req_Estr.c_str());
        fprintf(fp, "%s", ret_message.c_str());
    } else {
        std::string ret_message("OK\nSIZE=" + std::to_string(strlen(textOut)) + "\n\n" + textOut);
        fprintf(fp, "%s", ret_message.c_str());
    }



	// clsose file pointer
	fclose(fp);
}


void ASErrorHandler(int errorNumber, const char* errorMessage) {   
    //std::cout << "astyle error " << errorNumber << "\n"
    //     << errorMessage << std::endl;
    Req_Error = true;
    Req_Estr += errorMessage + std::string("\n");

}

char* ASMemoryAlloc(unsigned long memoryNeeded) {   

	// error condition is checked after return from AStyleMain
    char* buffer = new (std::nothrow) char [memoryNeeded];
    return buffer;
}


