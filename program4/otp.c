#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <sys/ioctl.h>

#define MAX_BUF 100000

int sendAll(int, char*, int*);

void error(const char *msg) { perror(msg); exit(0); } // Error function used for reporting issues

int main(int argc, char *argv[])
{
	int socketFD, portNumber, charsRead;
	struct sockaddr_in serverAddress;
	struct hostent* serverHostInfo;
	char cipherText[MAX_BUF];

	// Set up the server address struct
	memset((char*)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
	if (strcmp(argv[1], "post") == 0)
	{
		portNumber = atoi(argv[5]); // Get the port number, convert to an integer from a string
	}
	else if (strcmp(argv[1], "get") == 0)
	{
		portNumber = atoi(argv[4]); // Get the port number, convert to an integer from a string	
	}
	else
	{
		fprintf(stderr, "Invalid type of command (post or get)\n");
		exit(1);	
	}
	
	serverAddress.sin_family = AF_INET; // Create a network-capable socket
	serverAddress.sin_port = htons(portNumber); // Store the port number
	serverHostInfo = gethostbyname("localhost"); // Convert the machine name into a special form of address
	if (serverHostInfo == NULL) 
    { 
        fprintf(stderr, "CLIENT: ERROR, no such host\n"); 
        exit(0); 
    }
	memcpy((char*)serverHostInfo->h_addr, (char*)&serverAddress.sin_addr.s_addr, serverHostInfo->h_length); // Copy in the address

	// Set up the socket
	socketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
	if (socketFD < 0) 
    {
        perror("CLIENT: ERROR opening socket");
    }
	
	// Connect to server
	if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) // Connect socket to address
    {
        perror("CLIENT: ERROR connecting");
    }

	// POST
	if (strcmp(argv[1], "post") == 0)
	{
		// Check number of arguments
		if (argc != 6)
		{
			fprintf(stderr, "Invalid number of arguments\n");
			exit(1);	
		}

		FILE *textFile;
		FILE *keyFile;
		char textChar, keyChar, cipherChar;

		// Open plaintext and key
		textFile = fopen(argv[3], "r");
		if (textFile == NULL)
		{
			fprintf(stderr, "Cannot open text file\n");
			exit(1);	
		}
		keyFile = fopen(argv[4], "r");
		if (textFile == NULL)
		{
			fprintf(stderr, "Cannot open key file\n");
			exit(1);
		}

		// Initialize send buffer
		memset(cipherText, '\0', sizeof(cipherText));
		int index = 0;

		// Put post prefix to send buffer
		cipherText[index++] = 'p';
		cipherText[index++] = ':';

		// Put user name to send buffer
		int k = 0;
		while (k != strlen(argv[2]))
		{
			cipherText[index++] = argv[2][k++];
		}
		cipherText[index++] = ':'; // Delimit user name and cipher text

		// Encrypt every character and put it to buffer
		textChar = fgetc(textFile);
		keyChar = fgetc(keyFile);
		while (textChar != EOF)
		{
			// Check if key is shorter than text file
			if (keyChar == EOF)
			{
				fprintf(stderr, "Error: key '%s' is too short\n", argv[4]);
				exit(1);
			}

			// Check if a file contains any bad characters
			if ((textChar < 65 || textChar > 90) && textChar != 32 && textChar != '\n')
			{
				fprintf(stderr, "otp_enc error: input contains bad characters\n");
				exit(1)	;
			}

			cipherChar = ((textChar + keyChar) % 27) + 65; // Encrypt character
			if (cipherChar == 91) // Handling space
			{
				cipherChar = 32;
			}

			cipherText[index++] = cipherChar; // Put encrypted character to buffer

			textChar = fgetc(textFile);
			keyChar = fgetc(keyFile);
		}
		fclose(textFile);
		fclose(keyFile);

		int length;

		length = strlen(cipherText) - 1;
		if (sendAll(socketFD, cipherText, &length) == -1)
		{
			perror("CLIENT: sendAll error");
		}
	}
	// GET
	else if (strcmp(argv[1], "get") == 0)
	{
		// Check number of arguments
		if (argc != 5)
		{
			fprintf(stderr, "Invalid number of arguments\n");
			exit(1);	
		}

		// Send user name
		char userName[500];
		memset(userName, '\0', sizeof(userName));
		int index = 0;
		
		// Put get prefix to send buffer
		userName[index++] = 'g';
		userName[index++] = ':';

		// Put user name to send buffer
		int k = 0;
		while (k != strlen(argv[2]))
		{
			userName[index++] = argv[2][k++];
		}
		// userName[index++] = ':'; // Delimit user name and cipher text

		printf("CLIENT: username is %s\n", userName);

		int userRead;
		userRead = send(socketFD, userName, sizeof(userName) - 1, 0);
		if (userRead < 0) 
		{
			error("ERROR reading from socket");
		}

		// Get return message from server
		// memset(buffer, '\0', sizeof(buffer)); // Clear out the buffer again for reuse
		// charsRead = recv(socketFD, buffer, sizeof(buffer) - 1, 0); // Read data from the socket, leaving \0 at end
		// if (charsRead < 0) error("CLIENT: ERROR reading from socket");
		// printf("CLIENT: I received this from the server: \"%s\"\n", buffer);
	}
	else
	{
    	fprintf(stderr, "Invalid type of command (post or get)\n");
		exit(1);
	}
	
	close(socketFD); // Close the socket

	return 0;
}

// Send all data (reference by https://beej.us/) 
int sendAll(int socketFD, char *cipherText, int *length)
{
	int charsWritten;
	int total = 0;
	int bytesleft = *length;

	while (total < *length)
	{
		charsWritten = send(socketFD, cipherText + total, bytesleft, 0);
		if (charsWritten < 0)
		{
			perror("CLIENT: ERROR writing to socket");
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
			perror("ioctl error");  // Check if we actually stopped the loop because of an error
		} 
	}

	*length = total;

	return charsWritten == -1 ? -1 : 0;
}