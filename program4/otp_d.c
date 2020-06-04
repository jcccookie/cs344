#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>

#define MAX_BUF 100000
#define MAX_CHILD 5

int parentPid = -5; // Parent proess id
int childPid = -5; // Child process id
int childExitMethod = -5;

void catchSIGCHLD(int);

void error(const char *msg) { perror(msg); exit(1); } // Error function used for reporting issues

int main(int argc, char *argv[])
{
    // Set SIGCHLD handler
    struct sigaction SIGCHLD_action = {0};
    SIGCHLD_action.sa_handler = catchSIGCHLD;
    sigfillset(&SIGCHLD_action.sa_mask);
    SIGCHLD_action.sa_flags = SA_RESTART; 
    sigaction(SIGCHLD, &SIGCHLD_action, NULL);

	int listenSocketFD, establishedConnectionFD, portNumber, charsRead, i, numChildren, total;
	socklen_t sizeOfClientInfo;
	char buffer[MAX_BUF];
	struct sockaddr_in serverAddress, clientAddress;

    pid_t spawnPid = -5; // Spawned process id

	if (argc != 2) 
    {
        fprintf(stderr, "USAGE: %s port\n", argv[0]); 
        exit(1); // Check usage & args
    }

	// Set up the address struct for this process (the server)
	memset((char *)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
	portNumber = atoi(argv[1]); // Get the port number, convert to an integer from a string
	serverAddress.sin_family = AF_INET; // Create a network-capable socket
	serverAddress.sin_port = htons(portNumber); // Store the port number
	serverAddress.sin_addr.s_addr = INADDR_ANY; // Any address is allowed for connection to this process

	// Set up the socket
	listenSocketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
	if (listenSocketFD < 0)
    {
        perror("ERROR opening socket");
    } 

	// Enable the socket to begin listening
	if (bind(listenSocketFD, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0)
    {
        perror("ERROR on binding"); // Connect socket to port
    } 
	listen(listenSocketFD, 5); // Flip the socket on - it can now receive up to 5 connections

    while (1)
    {
        // Accept a connection, blocking if one is not available until one connects
        sizeOfClientInfo = sizeof(clientAddress); // Get the size of the address for the client that will connect
        establishedConnectionFD = accept(listenSocketFD, (struct sockaddr *)&clientAddress, &sizeOfClientInfo); // Accept
        if (establishedConnectionFD < 0)
        {
            perror("ERROR on accept");
        } 
        sleep(2);

        numChildren++; // Increment number of child process
        // Limit number of child processes
        while (numChildren > MAX_CHILD)
        {
            int status;
            if (wait(&status) == 0)
            {
                numChildren--;
            }
        }

        spawnPid = fork();
         
        // Error handling
        if (spawnPid == -1)
        {
            perror("Fork Error\n"); exit(1);
        }
        // CHILD process
        else if (spawnPid == 0)
        {
            close(listenSocketFD);

            memset(buffer, '\0', MAX_BUF);

            charsRead = 0;
            total = 0;
            
            while ((charsRead = recv(establishedConnectionFD, buffer, MAX_BUF - 1, 0)) > 0)
            {
                total += charsRead;
            }
            if (charsRead < 0) 
            {
                perror("ERROR reading from socket");
            }

            int position = 0; // Position of a token
            char* tempBuffer = strdup(buffer);
            char* token = tempBuffer;

            // Parsing message from client
            while ((token = strsep(&tempBuffer, ":")) != NULL)
            {
                // POST
                if ((strcmp(token, "p") == 0) && position == 0)
                {
                    // Buffer for file name to store encrypted text
                    char fileName[100];
                    memset(fileName, '\0', strlen(fileName));

                    // Get a username and save it to a buffer as a prefix of the the file name
                    char* prefixUsername = strsep(&tempBuffer, ":");
                    strcpy(fileName, prefixUsername);
                    strcat(fileName, "_cipherfile"); // Append a postfix

                    // Print out path of the file
                    char path[300];
                    char cwd[200];
                    snprintf(path, sizeof(path), "%s%s%s", getcwd(cwd, 200), "/", fileName);
                    printf("%s\n", path);

                    // Open file descriptor
                    FILE *cipherFile;
                    cipherFile = fopen(fileName, "w+");
                    if (cipherFile == NULL)
                    {
                        fprintf(stderr, "Cannot open text file\n");
                        exit(1);
                    }

                    // Save characters to the file
                    char* encryptedMessage = strsep(&tempBuffer, ":");
                    int pos;
                    for (pos = 0; encryptedMessage[pos] != '\0'; pos++)
                    {
                        fputc(encryptedMessage[pos], cipherFile);
                    }
                    fputc('\n', cipherFile);

                    fclose(cipherFile);
                    free(tempBuffer);
                    
                    exit(0);
                }
                // GET
                else if ((strcmp(token, "g") == 0) && position == 0)
                {
                    

                    char* userName = strsep(&tempBuffer, ":");
                    printf("SERVER: Just got username: %s\n", userName);

                    free(tempBuffer);
                    exit(0);
                }
            }

        }
        // PARENT process
        else
        {
            parentPid = (int)getpid();

            childPid = spawnPid; // Save a child's pid to a global variable to catch SIGINT for only in a foreground child process

            setpgid(getpid(), spawnPid); // Set a child's pgid to parent's pid

            waitpid(spawnPid, &childExitMethod, 0);

            childPid = -5; // Reset childPid

            close(establishedConnectionFD); // Close the existing socket which is connected to the client
        }
    }


    // // Send a Success message back to the client
    // charsRead = send(establishedConnectionFD, "I am the server, and I got your message", 39, 0); // Send success back
    // if (charsRead < 0) 
    // {
    //     perror("ERROR writing to socket");
    // }

    // close(establishedConnectionFD); // Close the existing socket which is connected to the client
	
	close(listenSocketFD); // Close the listening socket

	return 0; 
}

// Signal handler for SIGCHLD 
void catchSIGCHLD(int signum)
{
   waitpid(-1, &childExitMethod, WNOHANG);
}