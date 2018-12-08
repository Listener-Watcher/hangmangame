#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <ctype.h>
// copied,error function
void error(const char *msg)
{
    perror(msg);
    exit(0);
}

int main(int argc, char *argv[])
{
//copied from 19 to 46
    int sockfd, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    char msg[256];
    char r_msg[256];
    char buffer[256];
    if (argc < 3) {
       fprintf(stderr,"usage %s hostname port\n", argv[0]);
       exit(0);
    }
    portno = atoi(argv[2]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("ERROR opening socket");
    server = gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr,
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(portno);
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
        error("ERROR connecting");
    printf("Ready to start game? (y/n):");
    char temp[256];
    bzero(temp,256);
    fgets(temp,10,stdin);
    if(temp[0]=='n')
    {
	    close(sockfd);
	    return 0;
    }
    else if(temp[0]=='y')
    {
	bzero(msg,256);
	msg[0] = '0';
	n = write(sockfd,msg,strlen(msg));//game start, send 0 to server
    	if (n < 0)
		error("ERROR writing to socket");
    	bzero(r_msg,256);
    	n = read(sockfd,r_msg,255);
    	if (n < 0)
		error("ERROR reading from socket");
        if(r_msg[0]>0&&r_msg[1]=='s'&&r_msg[2]=='e')
        {
            printf("server-overloaded\n");
            close(sockfd);
            return 0;
        }
	bzero(temp,256);
	for(int i=0;i<r_msg[1];i++)
	{
		temp[i]=r_msg[i+3];
	}
	printf("\n%s\n",temp);
	printf("Incorrect Guesses:\n\n");
	int end_game = 1;
	while(end_game)
	{
		printf("Letter to guess: ");
		bzero(msg,256);
		char msg_temp[256];
		bzero(msg_temp,256);
		fgets(msg_temp,255,stdin);
		int letter = tolower(msg_temp[0]);
		while(strlen(msg_temp)!=2||letter<97||letter>122)
		{
			printf("Error! Please guess one letter.\n");
			printf("letter to guess: ");
			bzero(msg_temp,256);
			fgets(msg_temp,255,stdin);
			letter = tolower(msg_temp[0]);
		}
		msg[0] = '1';
		msg[1] = msg_temp[0];
		n = write(sockfd,msg,strlen(msg));
		bzero(r_msg,256);
		n = read(sockfd,r_msg,255);
		if(r_msg[0]!='0')
		{
			bzero(temp,256);
			for(int i=0;i<r_msg[0];i++)
			{
				temp[i]=r_msg[i+1];
			}
			printf("%s",temp);
			end_game = 0;
		}
		else
		{
			int word_len = r_msg[1]-'0';
			int inc_len = r_msg[2]-'0';
			bzero(temp,256);
			for(int i=0;i<word_len;i++)
			{
				temp[i] = r_msg[3+i];
			}
			printf("%s\n",temp);
			bzero(temp,256);
			for(int i=0;i<inc_len;i++)
			{
				temp[i] = r_msg[i+3+word_len];
			}
			printf("Incorrect Guesses: %s\n\n",temp);
		}
	}
    }
    close(sockfd);
    return 0;
}
