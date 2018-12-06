#include <stdio.h>
#include <ctype.h>
#include <stdlib.h> // atoi
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[])
{
     int sockfd, newsockfd, portno;
     socklen_t cli_len;
     char buffer[256];
     char sent_msg[256];
     struct sockaddr_in serv_addr, cli_addr;
     int status;
     if (argc < 2||argc>=3) {
         error("ERROR, no port provided\n");
     }
     sockfd = socket(AF_INET, SOCK_STREAM, 0);
     if (sockfd < 0) 
        error("ERROR opening socket");
     bzero((char *) &serv_addr, sizeof(serv_addr));
     portno = atoi(argv[1]); // convert string to integer, get port number
     serv_addr.sin_family = AF_INET;
     serv_addr.sin_addr.s_addr = INADDR_ANY;
     serv_addr.sin_port = htons(portno);
     if (bind(sockfd, (struct sockaddr *) &serv_addr,
              sizeof(serv_addr)) < 0) 
              error("ERROR on binding");
     listen(sockfd,5);
     cli_len = sizeof(cli_addr);

     while(newsockfd = accept(sockfd, 
                 (struct sockaddr *) &cli_addr, 
                 &cli_len))
     {
	bzero(sent_msg,256);
	bzero(buffer,256);
	sent_msg[0] = '0';
	sent_msg[1] = '5';
	sent_msg[2] = '0';
     	status = read(newsockfd,buffer,255);
     	if (status < 0) error("ERROR reading from socket");
	int word_len = sent_msg[1]-'0';
	int inc_len = 0;
	for(int i=0;i<word_len;i++)
	{
		sent_msg[i+3]='_';
	}
	status = write(newsockfd,sent_msg,strlen(sent_msg));
	if(status<0) error("ERROR writing to socket");
	int win = 0;
	int lose = 0;
	char word[256] = "mongo";
	while((!win)&&(!lose))
	{	
     		status = read(newsockfd,buffer,255);
     		if (status < 0) error("ERROR reading from socket");
		int guessed = 0;
		for(int i=0;i<word_len;i++)
		{
			if(word[i]==buffer[1])
			{
				sent_msg[i+3]=buffer[1];
				guessed = 1;
			}
		}
		win = 1;
		lose = 0;
		for(int i=0;i<word_len;i++)
		{
			if(sent_msg[i+3]=='_')
				win = 0;
		}

		if(!guessed)
		{
			sent_msg[3+sent_msg[1]+sent_msg[2]] = buffer[1];
			inc_len = inc_len+1;
		}
		if(inc_len>=6)
		{
			bzero(sent_msg,256);
			char sent_temp[256]="You Lose!\nGame Over!\n";
			int l_msg = strlen(sent_temp);
			for(int i=l_msg-1;i>=0;i--)
			{
				sent_msg[i+1] = sent_temp[i];
			}
			sent_msg[0] = l_msg;
			lose = 1;
		}
		if(win)
		{
			bzero(sent_msg,256);
			char sent_temp[256] = "You Win!\nGame Over!\n";
			int l_msg = strlen(sent_temp);
			for(int i=l_msg-1;i>=0;i--)
			{
				sent_msg[i+1] = sent_temp[i];
			}
			sent_msg[0] = l_msg;
		}
		status = write(newsockfd,sent_msg,strlen(sent_msg));
		if(status<0) error("ERROR writing to socket");
		printf("%d%d",win,lose);
	}
	close(newsockfd);
	bzero(buffer,256);
     }
     error("ERROR on accept");
     close(newsockfd);
     close(sockfd);
     return 0; 
}
