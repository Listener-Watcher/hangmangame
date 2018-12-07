#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/shm.h>
#include<time.h> 
void error(const char *msg)
{
    perror(msg);
    exit(1);
} 
//传进来的sockfd，就是互相建立好连接之后的socket文件描述符
//通过这个sockfd，可以完成 [服务端]<--->[客户端] 互相收发数据
void clientProcess(int newsockfd)
{
    char buffer[256];
    char sent_msg[256];
    int status;
    static const char filename[] = "hangman_words.txt";
	FILE *file = fopen(filename,"r");
	int count = 0;
	if(file!=NULL)
	{
		char line[128];
		while(fgets(line,sizeof line,file)!=NULL)
		{
			count++;
		}
	fclose(file);
	}
	else
	{
		error("file not exist");
	}
	srand(time(0));
	count = rand()%count+1;
	file = fopen(filename,"r");
	int t_count = 0;
	char word[256];
	bzero(word,256);
	if(file!=NULL)
	{
		char line[128];
		while(fgets(line,sizeof line,file)!=NULL)
		{
			t_count++;
			if(t_count == count)
			{
				for(int i=0;i<strlen(line)-1;i++)
				{
					word[i] = line[i];
				}
			}
		}
	fclose(file);
	}
	bzero(sent_msg,256);
	bzero(buffer,256);
	sent_msg[0] = '0';
	int word_len = strlen(word);
	char word_l = strlen(word)+'0';
	sent_msg[1] = word_l;
	sent_msg[2] = '0';
	int inc_len = sent_msg[2]-'0';
     	status = read(newsockfd,buffer,255);
     	if (status < 0) error("ERROR reading from socket");
	for(int i=0;i<word_len;i++)
	{
		sent_msg[i+3]='_';
	}
	status = write(newsockfd,sent_msg,strlen(sent_msg));
	if(status<0) error("ERROR writing to socket");
	int win = 0;
	int lose = 0;
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
			sent_msg[3+inc_len+word_len] = buffer[1];
			inc_len = inc_len+1;
			sent_msg[2] = inc_len+'0';
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
}
 
int main(int argc, char **argv)
{
    //定义IPV4的TCP连接的套接字描述符
    int server_sockfd = socket(AF_INET,SOCK_STREAM, 0);
   int num_client = 0;
  if(argc<2||argc>=3)
  {
	 error("ERROR,no port provided\n");
  } 
  int portno = atoi(argv[1]);
    //定义sockaddr_in
    struct sockaddr_in server_sockaddr;
    server_sockaddr.sin_family = AF_INET;
    server_sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_sockaddr.sin_port = htons(portno);
 
    //bind成功返回0，出错返回-1
    if(bind(server_sockfd,(struct sockaddr *)&server_sockaddr,sizeof(server_sockaddr))==-1)
    {
        perror("bind");
        exit(1);//1为异常退出
    }
    printf("bind success.\n");
 
    //listen成功返回0，出错返回-1，允许同时帧听的连接数为QUEUE_SIZE
    if(listen(server_sockfd,QUEUE_SIZE) == -1)
    {
        perror("listen");
        exit(1);
    }
    printf("listen success.\n");
 
    for(;;)
    {
        struct sockaddr_in client_addr;
        socklen_t length = sizeof(client_addr);
        //进程阻塞在accept上，成功返回非负描述字，出错返回-1
        int conn = accept(server_sockfd, (struct sockaddr*)&client_addr,&length);
	if(num_client<3){
        if(conn<0)
        {
            perror("connect");
            exit(1);
        }
        printf("new client accepted.\n");
	num_client++;
	printf("number of clients:%d\n",num_client);
        pid_t childid;
        if(childid=fork()==0)//子进程
        {
            printf("child process: %d created.\n", getpid());
            close(server_sockfd);//在子进程中关闭监听
            clientProcess(conn);
            num_client--;
            exit(1);

        }
	}
	else
	{
		close(conn);
	}
    }
 
    printf("closed.\n");
    close(server_sockfd);
    return 0;
}
