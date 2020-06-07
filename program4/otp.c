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

// Declare functions
int sendAll(int, char*, int*);
int receiveAll(int, char*, int*);

int main(int argc, char *argv[])
{
	int socketFD, portNumber, charsRead, total, sizeWritten, size, index;
	struct sockaddr_in serverAddress;
	struct hostent* serverHostInfo;
	char cipherText[MAX_BUF];
	char readBuffer[MAX_BUF];

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
		fprintf(stderr, "CLIENT: Invalid type of command (post or get)\n");
		exit(1);	
	}
	
	serverAddress.sin_family = AF_INET; // Create a network-capable socket
	serverAddress.sin_port = htons(portNumber); // Store the port number
	serverHostInfo = gethostbyname("localhost"); // Convert the machine name into a special form of address
	if (serverHostInfo == NULL) 
    { 
        fprintf(stderr, "CLIENT: ERROR, no such host\n"); 
        exit(1); 
    }
	memcpy((char*)serverHostInfo->h_addr, (char*)&serverAddress.sin_addr.s_addr, serverHostInfo->h_length); // Copy in the address

	// Set up the socket
	socketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
	if (socketFD < 0) 
    {
		fprintf(stderr, "CLIENT: ERROR opening socket");
		exit(1);	
    }
	
	// Connect to server
	if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) // Connect socket to address
    {
		fprintf(stderr, "CLIENT: ERROR connecting, port %d\n", portNumber);
		exit(2);
    }

	// POST
	if (strcmp(argv[1], "post") == 0)
	{
		// Check number of arguments
		if (argc != 6)
		{
			fprintf(stderr, "CLIENT: Invalid number of arguments\n");
			exit(1);	
		}

		FILE *textFile;
		FILE *keyFile;
		char textChar, keyChar, cipherChar;

		// Open plaintext and key
		textFile = fopen(argv[3], "r");
		if (textFile == NULL)
		{
			fprintf(stderr, "CLIENT: Cannot open text file\n");
			exit(1);	
		}
		keyFile = fopen(argv[4], "r");
		if (textFile == NULL)
		{
			fprintf(stderr, "CLIENT: Cannot open key file\n");
			exit(1);
		}

		// Initialize send buffer (post:username:ciphertext)
		memset(cipherText, '\0', sizeof(cipherText));
		index = 0;

		// Put post prefix to send buffer 
		cipherText[index++] = 'p';
		cipherText[index++] = ':';

		// Put user name to send buffer
		int k = 0;
		while (k != strlen(argv[2]))
		{
			cipherText[index++] = argv[2][k++];
		}
		cipherText[index++] = ':'; // Delimit username and ciphertext

		// Encrypt plain text
		textChar = fgetc(textFile);
		keyChar = fgetc(keyFile);
		while (textChar != EOF)
		{
			// Check if key is shorter than text file
			if (keyChar == EOF)
			{
				fprintf(stderr, "CLIENT: ERROR key '%s' is too short\n", argv[4]);
				exit(1);
			}

			// Check if a file contains any bad characters
			if ((textChar < 65 || textChar > 90) && textChar != 32 && textChar != '\n')
			{
				fprintf(stderr, "CLIENT: ERROR plaintext contains bad characters\n");
				exit(1);
 			}

			// If character is space, convert it to ascii 91
			if (textChar == 32) { textChar += 59; }
			if (keyChar == 32) { keyChar += 59; }

			cipherChar = ((textChar + keyChar) - 130) % 27; // Encrypt character

			if (cipherChar == 26) { cipherChar = 32; } // Encrypt it to space
			else { cipherChar += 65; }

			cipherText[index++] = cipherChar; // Put encrypted character to buffer

			textChar = fgetc(textFile);
			keyChar = fgetc(keyFile);
		}
		fclose(textFile);
		fclose(keyFile);
		
		// Send size of text
		size = strlen(cipherText) - 1;
		char buffer[100];
		memset(buffer, '\0', sizeof(buffer));
		sprintf(buffer, "%d", size);
		sizeWritten = send(socketFD, buffer, sizeof(buffer) - 1, 0);
		if (sizeWritten < 0)
		{
			fprintf(stderr, "CLIENT: ERROR size of text");
			exit(1);
		}

		// Send data (post:username:encrypted text) to server
		if (sendAll(socketFD, cipherText, &size) == -1)
		{
			fprintf(stderr, "CLIENT: sendAll error");
			exit(1);
		}
	}
	// GET
	else if (strcmp(argv[1], "get") == 0)
	{
		// Check number of arguments
		if (argc != 5)
		{
			fprintf(stderr, "CLIENT: Invalid number of arguments\n");
			exit(1);	
		}

		// Buffer for send info (get:username)
		char sendBuffer[500];
		memset(sendBuffer, '\0', sizeof(sendBuffer));
		index = 0;
		
		// Put get prefix to send buffer 
		sendBuffer[index++] = 'g';
		sendBuffer[index++] = ':';

		// Put username to send buffer
		int k = 0;
		while (k != strlen(argv[2]))
		{
			sendBuffer[index++] = argv[2][k++];
		}

		// Send size of text to server
		size = strlen(sendBuffer);
		char buffer[100];
		memset(buffer, '\0', sizeof(buffer));
		sprintf(buffer, "%d", size);
		sizeWritten = send(socketFD, buffer, sizeof(buffer) - 1, 0);
		if (sizeWritten < 0)
		{
			fprintf(stderr, "CLIENT: ERROR empty message to server");
			exit(1);
		}

		// Send get:username
		if (sendAll(socketFD, sendBuffer, &size) == -1)
		{
			fprintf(stderr, "CLIENT: sendAll error");
			exit(1);
		}	

		// Receive size of text from server
		memset(buffer, '\0', MAX_BUF);
		charsRead = 0;
		charsRead = recv(socketFD, buffer, sizeof(buffer) - 1, 0);
		size = atoi(buffer);
		if (size <= 0)
		{
			fprintf(stderr, "CLIENT: ERROR no message");
			exit(1);
		}

		// Receive encrypted data from server
		charsRead = 0;
		total = 0;
		memset(readBuffer, '\0', MAX_BUF);
		charsRead = receiveAll(socketFD, readBuffer, &size);
		if (charsRead < 0) 
		{
			fprintf(stderr, "CLIENT: ERROR reading from socket");
			exit(1);	
		}

		// Open key file
		FILE *keyFile;
		keyFile = fopen(argv[3], "r");
		if (keyFile == NULL)
		{
			fprintf(stderr, "CLIENT: ERROR Cannot open text file\n");
			exit(1);
		}

		// Decrypt ciphertext and stdout
		char keyChar, decryptChar;
		char decryptedBuffer[size];
		memset(decryptedBuffer, '\0', sizeof(decryptedBuffer));
		index = 0;
		keyChar = fgetc(keyFile); 
		for (index = 0; index < size - 1; index++)
		{
			// Check if a file contains any bad characters
			if ((keyChar < 65 || keyChar > 90) && keyChar != 32 && keyChar != '\n')
			{
				fprintf(stderr, "CLIENT: ERROR key contains bad characters\n");
				exit(1)	;
 			}

			// Convert space to ascii 91
			if (keyChar == 32) { keyChar += 59; }
			if (readBuffer[index] == 32) { readBuffer[index] += 59; }

			// If cipher character is less than key
			decryptChar = readBuffer[index] - keyChar;
			// -26 ~ -1
			if (decryptChar < 0)
			{
				decryptChar += 27; // 1 ~ 26
				// When key is space
				if (decryptChar == 26) { decryptChar = 32; }
				else { decryptChar += 65; }
			}
			// 0 ~ 26
			else
			{
				decryptChar += 65;
				// Handle space
				if (decryptChar == 91) { decryptChar = 32; }
			}
			decryptedBuffer[index] = decryptChar;

			keyChar = fgetc(keyFile);
		}

		printf("%s\n", decryptedBuffer);
	}
	else
	{
    	fprintf(stderr, "CLIENT: Invalid type of command (post or get)\n");
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
			exit(1);	
		} 
	}

	*length = total;

	return charsWritten == -1 ? -1 : 0;
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
