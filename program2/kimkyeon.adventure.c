#include <stdlib.h> 
#include <stdio.h> 
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>


// Define boolean type
typedef enum
{
  false,
  true
} bool;

// Structure for individual room information
struct Room
{
  char name[3];
  int numOutboundConnections;
  char outboundConnections[6][3];
  char type[11];
};

// Global array containing room info from files
struct Room sevenRooms[7];

// Function declations
char* findNewestDir(void);
struct Room createRoom();
void readDataIn(char*);
void playGame(void);
struct Room* getRoomWith(char*);
void printPossibleConnections(char*);
bool isValidConnection(char*, char*);
void printCongratulationMessage(char**, int);
char** createDynamicArray(int);
void freeDynamicArray(char**, int);
char** resizePathHistory(char**, int*);

// Entry point
int main(int argc, char *argv[])
{
   // Find the name of the newest created directory
   char *newestDirName;
   newestDirName = findNewestDir();

   printf("newest dir name is %s\n", newestDirName); // --------------------DELETE LATER----------------------------

   // Read data in every file in the directory, and save data to Room structures
   readDataIn(newestDirName);

// --------------------DELETE LATER----------------------------
   // int x;
   // for (x = 0; x < 7; x++)
   // {
   //    printf("name: %s\n", sevenRooms[x].name);
   //    int y;
   //    for (y = 0; y < sevenRooms[x].numOutboundConnections; y++)
   //    {
   //       printf("connection: %s\n", sevenRooms[x].outboundConnections[y]);
   //    }
   //    printf("type: %s\n", sevenRooms[x].type);
   // }
// --------------------DELETE LATER----------------------------

   playGame();


   return 0;
}

// Find the name of the newest created directory
// Return: char* newestDirName in stack memory
char* findNewestDir()
{
   // Find the newest directory
   // Variables for checking the newest room directory
   static char newestDirName[32];
   memset(newestDirName, '\0', sizeof(newestDirName));
   int newestDirTime = -1;
   char targetDirPrefix[] = "kimkyeon.rooms."; // Scan file with this prefix

   DIR* dirToCheck;
   struct dirent *dirRoom;
   struct stat dirRoomAttribute;

   dirToCheck = opendir("."); // Open current directory

   // Find the newest room directory and store its name to a variable
   if (dirToCheck > 0)
   {
      while ((dirRoom = readdir(dirToCheck)) != NULL) // Read a directory
      {
         if (strstr(dirRoom->d_name, targetDirPrefix) != NULL) // If the directory has a target prefix
         {
            stat(dirRoom->d_name, &dirRoomAttribute); // Save directory's information

            // Update the newest time and name
            if ((int)dirRoomAttribute.st_mtime > newestDirTime)
            {
               newestDirTime = (int)dirRoomAttribute.st_mtime;
               memset(newestDirName, '\0', sizeof(newestDirName));
               strcpy(newestDirName, dirRoom->d_name);
            }
         }
      }
   }
   closedir(dirToCheck);

   // If there's no directory to search, terminate program
   if (newestDirName[0] == '\0')
   {
      printf("There's no such directory\n");
      exit(1);
   }

   return newestDirName;
}

// Create a Room structure and initialize members
// Return: struct Room
struct Room createRoom()
{
   struct Room room;

   memset(room.name, '\0', 3);
   memset(room.type, '\0', 11);
   room.numOutboundConnections = 0;

   return room;
}

// Scan every file in the newest directory, read data line by line, and put data to structures
// Param: char* newestDirName: the name of the newest directory name
void readDataIn(char* newestDirName)
{
   DIR* dirToCheck;
   dirToCheck = opendir(newestDirName); // Open the newest directory

   if (dirToCheck > 0)
   {
      char path[32]; // Path of file that is currently scanned 
      memset(path, '\0', sizeof(path));
      char targetFilePostfix[] = "_room"; // Only scan files with this postfix
      struct dirent *fileRoom; // Pointer to individual room file in the directory
      int i = 0; // Index for Room structures

      // Loop files
      while ((fileRoom = readdir(dirToCheck)) != NULL)
      {
         if (strstr(fileRoom->d_name, targetFilePostfix) != NULL) // Read only files with _room postfix
         {
            snprintf(path, sizeof(path), "%s%s%s", newestDirName, "/", fileRoom->d_name); // Path of a file that is currently scanned

            // File pointer to a file
            FILE *fp;
            fp = fopen(path, "r");

            // Buffer for a line in a file
            char *lineBuffer = NULL; 
            size_t bufferSize = 0;

            // Buffers for strings in a line in a file 
            char categoryBuffer[16]; //  Category in a line
            memset(categoryBuffer, '\0', sizeof(categoryBuffer));
            char valueBuffer[16]; // Value in a line
            memset(valueBuffer, '\0', sizeof(valueBuffer));

            sevenRooms[i] = createRoom(); // Create a room structure and assign it to the array

            // Read a file line by line, and put data to Room structure
            while (getline(&lineBuffer, &bufferSize, fp) != -1)
            {
               sscanf(lineBuffer, "%*s %s %s", categoryBuffer, valueBuffer); // Put last two column values to each variable

               // If a line contains name info, store it to a structure
               if (strstr(categoryBuffer, "NAME")) 
               {
                  strcpy(sevenRooms[i].name, valueBuffer);
               }
               // If a line contains type info, store it to a structure
               else if (strstr(categoryBuffer, "TYPE")) 
               {
                  strcpy(sevenRooms[i].type, valueBuffer);
               }
               // If a line contains connection info, store it to a structure
               else 
               {
                  strcpy(sevenRooms[i].outboundConnections[sevenRooms[i].numOutboundConnections++], valueBuffer);
               }
            }
            free(lineBuffer);

            fclose(fp);
            i++;
         }         
      }
   }
   closedir(dirToCheck);
}

// Interface for a player
void playGame()
{
   // Find start and end rooms
   struct Room* startRoom;
   struct Room* endRoom;
   startRoom = getRoomWith("START_ROOM");
   endRoom = getRoomWith("END_ROOM");

   printf("Start room is %s, End room is %s\n", startRoom->name, endRoom->name); // --------------------DELETE LATER----------------------------

   // Input buffer
   char inputBuffer[32];
   memset(inputBuffer, '\0', sizeof(inputBuffer));
   strcpy(inputBuffer, startRoom->name);

   // Current room buffer
   char currentBuffer[32];
   memset(currentBuffer, '\0', sizeof(currentBuffer));

   // Step count
   int stepCount = 0;

   // Create a dynamic array of path history
   int numberOfAvailablePath = 8;
   char **pathHistory = createDynamicArray(numberOfAvailablePath);

   // Prompt entry point
   // Loop until user find the end room
   do
   {
      // Play message and input field
      strcpy(currentBuffer, inputBuffer); // Maintain current input room
      printf("CURRENT LOCATION: %s\n", currentBuffer);
      printPossibleConnections(currentBuffer); // Print connections
      printf("WHERE TO? >");
      scanf("%s", &inputBuffer);
      printf("\n");

      // Print error message
      // Check if the input is valid
      if (!isValidConnection(currentBuffer, inputBuffer))
      {
         printf("HUH? I DON'T UNDERSTAND THAT ROOM. TRY AGAIN.\n\n");
         strcpy(inputBuffer, currentBuffer); // Recover input value to current room
         continue;
      }

      // Save path
      strcpy(pathHistory[stepCount++], inputBuffer);

      // If the number of path exceeds the capacity of path container, resize the container to double capacity
      if (stepCount >= numberOfAvailablePath)
      {
         pathHistory = resizePathHistory(pathHistory, &numberOfAvailablePath);
      }

   } while (strcmp(inputBuffer, endRoom->name) != 0);

   // Congratulations message
   printCongratulationMessage(pathHistory, stepCount);

   // Clear dynamic array
   freeDynamicArray(pathHistory, stepCount);
}

// Get a room with the type
// Param: char* type: a type of room to be searched
// Return: struct Room* 
struct Room* getRoomWith(char* type)
{
   int i;
   for (i = 0; i < 7; i++)
   {
      if (strcmp(sevenRooms[i].type, type) == 0)
      {
         return &sevenRooms[i];
      }
   }
}

// Get possible connections of a room and print them out
// Param: char* currentBuffer: a current room
void printPossibleConnections(char* currentBuffer)
{
   int i;
   for (i = 0; i < 7; i++)
   {
      if (strcmp(sevenRooms[i].name, currentBuffer) == 0)
      {
         printf("%s", "POSSIBLE CONNECTIONS: ");
         int j;
         for (j = 0; j < sevenRooms[i].numOutboundConnections; j++)
         {
            if (j == sevenRooms[i].numOutboundConnections-1)
            {
               printf("%s.\n", sevenRooms[i].outboundConnections[j]);
               break;
            }
            else
            {
               printf("%s, ", sevenRooms[i].outboundConnections[j]);
            }
         }
         break;
      }
   }
}

// Check if the room input is valid
// Params: char* currentBuffer: current room
//         char* inputBuffer: input room an user just typed
// Return: If the input is a valid connection in a current room, return true. Otherwise, return false
bool isValidConnection(char* currentBuffer, char* inputBuffer)
{
   int i;
   for (i = 0; i < 7; i++)
   {
      if (strcmp(sevenRooms[i].name, currentBuffer) == 0)
      {
         int j;
         for (j = 0; j < sevenRooms[i].numOutboundConnections; j++)
         {
            if (strcmp(sevenRooms[i].outboundConnections[j], inputBuffer) == 0)
            {
               return true;
            }
         }
      }
   }
   return false;
}

void printCongratulationMessage(char** pathHistory, int stepCount)
{
   printf("YOU HAVE FOUND THE END ROOM. CONGRATULATIONS!\n");
   printf("YOU TOOK %d STEPS. YOUR PATH TO VICTORY WAS:\n", stepCount);
   int i;
   for (i = 0; i < stepCount; i++)
   {
      printf("%s\n", pathHistory[i]);
   }
}

// Create a 2 dimensional dynamic array
// Param: int size: a number of paths
// Return: char** temp: dynamic array
char** createDynamicArray(int size)
{
   char **temp = (char **)malloc(size * sizeof(char*));
   int i;
   for (i = 0; i < size; i++)
   {
      temp[i] = (char *)malloc(32 * sizeof(char));
   }
   return temp;
}

// Free dynamic allocated array
// Params: char** array: target array to be freed
//         int size: a number of paths
void freeDynamicArray(char** array, int size)
{
   int i;
   for (i = 0; i < size; i++)
   {
      free(array[i]);
   }
   free(array);
}

// Resize path history with double capacity
// Params: char** pathHistory: path history array to be resized
//         int* numberOfAvailablePath: capacity to be double  
// Return: New Dynamic allocated array
char** resizePathHistory(char** pathHistory, int* numberOfAvailablePath)
{
   // Create a dynamic array with double capacity
   int newCapacity = *numberOfAvailablePath * 2;
   char **temp = createDynamicArray(newCapacity);

   // Copy values from original to temp array
   int i;
   for (i = 0; i < *numberOfAvailablePath; i++)
   {
      strcpy(temp[i], pathHistory[i]);
   }

   // Free original array
   freeDynamicArray(pathHistory, *numberOfAvailablePath);

   *numberOfAvailablePath = newCapacity;

   return temp;
}
