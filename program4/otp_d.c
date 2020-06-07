#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <dirent.h>
#include <time.h>
#include <limits.h>

#define MAX_BUF 100000
#define MAX_CHILD 5

int parentPid = -5; // Parent proess id
int childPid = -5; // Child process id
int childExitMethod = -5;
int numChildren = 0;

// Declare functions
int sendAll(int, char*, int*);
char* findOldestFile(char*);
int receiveAll(int, char*, int*);

// Signal handlers
void catchSIGCHLD(int);

int main(int argc, char *argv[])
{
    // Set SIGCHLD handler
    struct sigaction SIGCHLD_action = {0};
    SIGCHLD_action.sa_handler = catchSIGCHLD;
    sigfillset(&SIGCHLD_action.sa_mask);
    SIGCHLD_action.sa_flags = SA_RESTART;
    sigaction(SIGCHLD, &SIGCHLD_action, NULL);

	int listenSocketFD, establishedConnectionFD, portNumber, charsRead, sizeWritten, total, index, size;
	socklen_t sizeOfClientInfo;
	char buffer[MAX_BUF];
    char cipherText[MAX_BUF];
	struct sockaddr_in serverAddress, clientAddress;

    pid_t spawnPid = -5; // Spawned process id

	if (argc != 2) 
    {
        fprintf(stderr, "SERVER: USAGE: %s port\n", argv[0]); 
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
        fprintf(stderr, "SERVER: ERROR opening socket");
    }

	// Enable the socket to begin listening
	if (bind(listenSocketFD, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0)
    {
        fprintf(stderr, "SERVER: ERROR on binding");
    } 

    while (1)
    {
	    listen(listenSocketFD, 5); // Flip the socket on - it can now receive up to 5 connections

        // Accept a connection, blocking if one is not available until one connects
        sizeOfClientInfo = sizeof(clientAddress); // Get the size of the address for the client that will connect
        establishedConnectionFD = accept(listenSocketFD, (struct sockaddr *)&clientAddress, &sizeOfClientInfo); // Accept
        if (establishedConnectionFD < 0)
        {
            fprintf(stderr, "SERVER: ERROR on accept");
        } 

        sleep(2);

        // Limit the number of child process to 5
        numChildren++;
        if (numChildren >= MAX_CHILD)
        {
            continue;
        }

        spawnPid = fork();
         
        // Error handling
        if (spawnPid == -1)
        {
            close(establishedConnectionFD);
            fprintf(stderr, "SERVER: Fork Error\n");
            exit(1);
        }
        // CHILD process
        else if (spawnPid == 0)
        {
            int pid = getpid();

            // Receive size of text from client
            memset(buffer, '\0', MAX_BUF);
            charsRead = 0;
            charsRead = recv(establishedConnectionFD, buffer, 99, 0);
            int size = atoi(buffer);
            
            // Receive data from client
            memset(buffer, '\0', MAX_BUF);
            charsRead = 0;
            total = 0;
            charsRead = receiveAll(establishedConnectionFD, buffer, &size);
            if (charsRead < 0) 
            {
                fprintf(stderr, "SERVER: ERROR reading from socket");
            }
            
            char* tempBuffer = strdup(buffer);
            char* ptrBuffer = tempBuffer;
            char* token = tempBuffer;

            token = strsep(&tempBuffer, ":");
            
            // POST
            if (strcmp(token, "p") == 0)
            {
                // Buffer for file name to store encrypted text
                char fileName[100];
                memset(fileName, '\0', strlen(fileName));

                // Get a username and save it to a buffer as a prefix of the the file name
                char* prefixUsername = strsep(&tempBuffer, ":");
                sprintf(fileName, "%s%s%d", prefixUsername, "_cipherfile", pid);

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
                    fprintf(stderr, "SERVER: Cannot open text file\n");
                }

                // Save characters to the file
                char* encryptedMessage = strsep(&tempBuffer, ":");
                for (index = 0; encryptedMessage[index] != '\0'; index++)
                {
                    fputc(encryptedMessage[index], cipherFile);
                }
                fputc('\n', cipherFile);

                fclose(cipherFile);
                free(ptrBuffer);
                exit(0);
            }
            // GET
            else if (strcmp(token, "g") == 0)
            {
                char* userName = strsep(&tempBuffer, ":"); // To search a file name with the username
                char* oldestFileName = findOldestFile(userName); // Find the oldest file

                FILE *cipherFile;
                char cipherChar;

                // Open the oldest ciphertext file
                cipherFile = fopen(oldestFileName, "r");
                if (cipherFile == NULL)
                {
                    fprintf(stderr, "SERVER: Cannot open text file\n");
                }

                // Retrieve data from the oldest file
                memset(cipherText, '\0', sizeof(cipherText));
                index = 0;
                cipherChar = fgetc(cipherFile);
                while (cipherChar != EOF)
                {
                    cipherText[index++] = cipherChar;

                    cipherChar = fgetc(cipherFile);
                }
                fclose(cipherFile);

                // Send size of text to client
                size = strlen(cipherText);
                char sizeBuffer[100];
                memset(sizeBuffer, '\0', sizeof(sizeBuffer));
                sprintf(sizeBuffer, "%d", size);
                sizeWritten = send(establishedConnectionFD, sizeBuffer, sizeof(sizeBuffer) - 1, 0);
                if (sizeWritten < 0)
                {
                    fprintf(stderr, "SERVER: ERROR size of text");
                }

                // Send encrypted data to client
                size--;
                if (sendAll(establishedConnectionFD, cipherText, &size) == -1)
                {
                    fprintf(stderr, "SERVER: sendAll error");
                }

                // Remove ciphertext file
                if (remove(oldestFileName) != 0)
                { 
                    fprintf(stderr, "SERVER: File doesn't exist");
                }
                
                free(ptrBuffer);
                exit(0);
            }
        }
        // PARENT process
        else
        {
            close(establishedConnectionFD); // Close the existing socket which is connected to the client
        }
    }
	
	close(listenSocketFD); // Close the listening socket

	return 0; 
}

// Send all data (reference by https://beej.us/) 
int sendAll(int socketFD, char *cipherText, int *length)
{
	int charsWritten;
	int total = 0;
	int bytesleft = *length;

    // Send data until total sent data is less than length of text
	while (total < *length)
	{
		charsWritten = send(socketFD, cipherText + total, bytesleft, 0);
		if (charsWritten < 0)
		{
            fprintf(stderr, "SERVER: ERROR writing to socket");
			break;
		}
		total += charsWritten;
		bytesleft -= charsWritten;

		int checkSend = -5;  // Holds amount of bytes remaining in send buffer
		do
		{
			ioctl(socketFD, TIOCOUTQ, &checkSend);  // Check the send buffer for this socket
		}
		while (checkSend > 0);  // Loop forever until send buffer for this socket is empty
		if (checkSend < 0)
		{
            fprintf(stderr, "SERVER: ioctl error");
		} 
	}

	*length = total;

	return charsWritten == -1 ? -1 : 0;
}

// Find the oldest file in the current directory
char* findOldestFile(char* userName)
{
   // Find the oldest file
   // Variables for checking the oldest ciphertext files
   static char oldestFileName[32]; // Save the directory name to data segment
   memset(oldestFileName, '\0', sizeof(oldestFileName));
   int oldestFileTime = INT_MAX;

   DIR* dirToCheck;
   struct dirent *filePtr;
   struct stat fileAttribute;

   dirToCheck = opendir("."); // Open current directory

   // Find the oldest ciphertext file and store its name to a variable
   if (dirToCheck > 0)
   {
      while ((filePtr = readdir(dirToCheck)) != NULL) // Read a directory
      {
         if (strstr(filePtr->d_name, userName) != NULL) // If the directory has a target prefix
         {
            stat(filePtr->d_name, &fileAttribute); // Save file's information

            // Update the oldest time and name
            if ((int)fileAttribute.st_mtime < oldestFileTime)
            {
               oldestFileTime = (int)fileAttribute.st_mtime;
               memset(oldestFileName, '\0', sizeof(oldestFileName));
               strcpy(oldestFileName, filePtr->d_name);
            }
         }
      }
   }
   closedir(dirToCheck);

   // If there's no directory to search
   if (oldestFileName[0] == '\0')
   {
      fprintf(stderr, "SERVER: There's no such file with the user\n");
   }

   return oldestFileName;
}

// Receive all data 
int receiveAll(int socketFD, char* buffer, int* size)
{
	int bytesReceived;
	int charsRead;
    int bytesLeft = *size;

	bytesReceived = 0;

    // Loop until buffer receive all data 
	while (bytesLeft)
	{
		charsRead = recv(socketFD, buffer, bytesLeft, 0);

		if (charsRead <= 0)
        {
            break;
        }

		bytesReceived += charsRead;
		bytesLeft -= charsRead;

		buffer += charsRead;
	}

    return charsRead == -1 ? -1 : 0;
}

// Signal handler for SIGCHLD 
void catchSIGCHLD(int signum)
{
    numChildren--; // Decrement the number of child process after terminated
    waitpid(-1, &childExitMethod, WNOHANG);
}
