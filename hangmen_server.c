
#include <stdio.h>
#include <string.h> //strlen
#include <stdlib.h>
#include <errno.h>
#include <unistd.h> //close
#include <arpa/inet.h> //close
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h> //FD_SET, FD_ISSET, FD_ZERO macros

#define TRUE 1
#define FALSE 0

char controller_table[30][100];
char answer_table[30][100];
int inc_table[30];
// compile a message, clear the dest before use
void error(const char *msg)
{
    perror(msg);
    exit(1);
}
int compile_message(char* dest, char*source){
    int n = strlen(source);
    strncpy(dest+1, source, n);
    dest[0] = n;
    return n+1;
}

int controller_initialize(int client_no){
    char* sent_msg = controller_table[client_no];
    char* answer = answer_table[client_no];

    // clear
    bzero(sent_msg,100);
    bzero(answer,100);
    inc_table[client_no] = 0;
    char buffer[256];
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
  for(int i=0;i<strlen(word);i++)
  {
    answer[i]=word[i];
  }
  bzero(buffer,256);
  sent_msg[0] = '0';
  int word_len = strlen(word);
  char word_l = strlen(word)+'0';
  sent_msg[1] = word_l;
  sent_msg[2] = '0';
  int inc_len = sent_msg[2]-'0';
  for(int i=0;i<word_len;i++)
  {
    sent_msg[i+3]='_';
  }
  return word_len+3;
}

int controller_guess(int client_no, char guess_letter){

    char* sent_msg = controller_table[client_no];
    char* answer = answer_table[client_no];

    int word_len = strlen(answer);
    int guessed = 0;
    int inc_len = inc_table[client_no];
    for(int i=0;i<word_len;i++)
    {
      if(answer[i]==guess_letter)
      {
        sent_msg[i+3]=guess_letter;
        guessed = 1;
      }

    }
    int win = 1;
    int lose = 0;
    for(int i=0;i<word_len;i++)
    {
      if(sent_msg[i+3]=='_')
        win = 0;
    }

    if(!guessed)
    {
      sent_msg[3+inc_len+word_len] = guess_letter;
      inc_len = inc_len+1;
      inc_table[client_no] = inc_len;
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
      return l_msg+1;
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
      return l_msg+1;
    }
    if(!lose)
    {
      return 3+inc_len+word_len;
    }
}

int main(int argc , char *argv[])
{
    int opt = TRUE;
    int master_socket , addrlen , newsockfd , client_socket[30] ,
    max_clients = 3 , activity, i , valread , sd;
    int current_clients = 0;
    int max_sd;
    struct sockaddr_in address;

    int message_length;

    char buffer[1025]; //data buffer of 1K

    bzero(client_socket, 30);

    //set of socket descriptors
    fd_set readfds;

    //a message
    // char *message = "ECHO Daemon v1.0 \r\n";

    //initialise all client_socket[] to 0 so not checked
    for (i = 0; i < max_clients; i++)
    {
        client_socket[i] = 0;
    }

    //create a master socket
    if( (master_socket = socket(AF_INET , SOCK_STREAM , 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    //set master socket to allow multiple connections ,
    //this is just a good habit, it will work without this
    if( setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt,
                   sizeof(opt)) < 0 )
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    if(argc<2||argc>=3)
    {
  	 error("ERROR,no port provided\n");
    }
    int portno = atoi(argv[1]);
    //type of socket created
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( portno );

    //bind the socket to localhost port 8888
    if (bind(master_socket, (struct sockaddr *)&address, sizeof(address))<0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    printf("Listener on port %d \n", portno);

    //try to specify maximum of 3 pending connections for the master socket
    if (listen(master_socket, 3) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    //accept the incoming connection
    addrlen = sizeof(address);
    puts("Waiting for connections ...");

    while(TRUE)
    {
        //clear the socket set
        FD_ZERO(&readfds);

        //add master socket to set
        FD_SET(master_socket, &readfds);
        max_sd = master_socket;

        //add child sockets to set
        for ( i = 0 ; i < max_clients ; i++)
        {
            //socket descriptor
            sd = client_socket[i];

            //if valid socket descriptor then add to read list
            if(sd > 0)
            FD_SET( sd , &readfds);

            //highest file descriptor number, need it for the select function
            if(sd > max_sd)
            max_sd = sd;
        }

        //wait for an activity on one of the sockets , timeout is NULL ,
        //so wait indefinitely
        activity = select( max_sd + 1 , &readfds , NULL , NULL , NULL);

        if ((activity < 0) && (errno!=EINTR))
        {
            printf("select error");
        }

        //If something happened on the master socket ,
        //then its an incoming connection
        if (FD_ISSET(master_socket, &readfds))
        {
            if ((newsockfd = accept(master_socket,
                                     (struct sockaddr *)&address, (socklen_t*)&addrlen))<0)
            {
                perror("accept");
                exit(EXIT_FAILURE);
            }

            //inform user of socket number - used in send and receive commands
            printf("New connection , socket fd is %d , ip is : %s , port : %d\n" , newsockfd , inet_ntoa(address.sin_addr) , ntohs
                   (address.sin_port));

            // check whether there is still space
            printf("Current_Clients: %d\n", current_clients);

            // if there are not any space
            if (current_clients >=  max_clients){
                bzero(buffer,1025);
                message_length = compile_message(buffer, "server-overloaded");
                send(newsockfd, buffer, message_length, 0);
                printf("Space is full\n");
                close(newsockfd);
            }

            else{
                //add new socket to array of sockets
                current_clients += 1;
                printf("here.\n");
                for (i = 0; i < max_clients; i++)
                {
                    //if position is empty
                    if( client_socket[i] == 0 )
                    {
                        client_socket[i] = newsockfd;
                        printf("Adding to list of sockets as %d\n" , i);



                        break;

                    }
                }                
                printf("Space is not full\n");
            }
            printf("Current_Clients: %d\n", current_clients);


        }

        //else its some IO operation on some other socket
        for (i = 0; i < max_clients; i++)
        {
            sd = client_socket[i];

            if (FD_ISSET( sd , &readfds))
            {
                //Check if it was for closing , and also read the
                //incoming message
                if ((valread = read( sd , buffer, 1024)) == 0)
                {
                    //Somebody disconnected , get his details and print
                    getpeername(sd , (struct sockaddr*)&address , \
                                (socklen_t*)&addrlen);
                    printf("Host disconnected , ip %s , port %d \n" ,
                           inet_ntoa(address.sin_addr) , ntohs(address.sin_port));

                    //Close the socket and mark as 0 in list for reuse
                    close( sd );
                    client_socket[i] = 0;
                    current_clients --;
                    printf("Current_Clients: %d\n", current_clients);
                }

                //Echo back the message that came in
                else
                {
                    int l;
                    if (buffer[0] == '0'){    //initial
                        // game initialize
                        l = controller_initialize(i);
                        printf("Initialized.\n");

                    }
                    else {
                        printf("%d, %c\n", buffer[0], buffer[1]);
                        l = controller_guess(i, buffer[1]);
                    }
                    bzero(buffer,1025);
                    write(sd, controller_table[i], l);
                    printf("success\n");
                }
            }
        }
    }

    return 0;
}
