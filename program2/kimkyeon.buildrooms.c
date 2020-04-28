#include <stdlib.h> 
#include <stdio.h> 
#include <string.h>
#include <time.h>

// Define constants
#define STATES_NUM 10
#define STATE_NAME_NUM 2
#define MAX_ROOM_NUM 7
#define MIN_OUT_NUM 3
#define MAX_OUT_NUM 6

// Define boolean type
typedef enum
{
  false,
  true
} bool;

// 10 room names in array 
char STATE_NAMES[STATES_NUM][STATE_NAME_NUM+1] = {"ca", "or", "wa", "hi", "nv", "ut", "ny", "nj", "ma", "pa"};

// Structure for individual room information
struct Room
{
  char name[3];
  int numOutboundConnections;
  struct Room* outboundConnections[6];
  char type[11];
};

// Global array of randomly selected rooms
struct Room selectedRooms[7];

// Function declarations
void shuffleStates(void);
struct Room createRoom(int, char*);
void assignRooms(void);
void createRoomFiles(char*);
bool isGraphFull(void);
void addRandomConnection(void);
struct Room* getRandomRoom(void);
bool canAddConnectionFrom(struct Room*);
bool connectionAlreadyExists(struct Room*, struct Room*);
void connectRoom(struct Room*, struct Room*);
bool isSameRoom(struct Room*, struct Room*);



void PrintRoomOutboundConnections(struct Room* input); //------------------------DELETE LATER------------------------------



// Entry point
int main(int argc, char *argv[])
{
  // Get PID and its length
  const int PID = getpid();
  const unsigned int PID_LENGTH = sizeof(PID);
  
  // Set a name of directory
  const char PREFIX[] = "kimkyeon.rooms.";
  const int PREFIX_LENGTH = strlen(PREFIX); 

  // Set size of string for directory name
  const int BUFFER_SIZE = PID_LENGTH + PREFIX_LENGTH;
  char dirName[BUFFER_SIZE+1];

  // Concatenate prefix and pid to directory name
  sprintf(dirName, "%s%d", PREFIX, PID);

  // Create a directory
  int result = mkdir(dirName, 0755);

  // If an error occurs, exit 
  if (result)
  {
    printf("Directory has not been made!!\n");
    exit(1);
  }

  srand(time(NULL)); // Generate a seed for random numbers

  shuffleStates(); // Shuffle states in array 
  
  assignRooms(); // Assign rooms to structures

  // Create all connections in graph
  while (isGraphFull() == false)
  {
    addRandomConnection();
  }

  // Create 7 files of rooms
  createRoomFiles(dirName);


  //------------------------DELETE LATER------------------------------
  int j;
  for (j = 0; j < 7; j++)
  {
    PrintRoomOutboundConnections(&selectedRooms[j]);
  }
  //------------------------DELETE LATER------------------------------
  
  return 0;
}

// --------------------------------DELETE LATER-------------------------------------
void PrintRoomOutboundConnections(struct Room* input)
{
  printf("The rooms connected to (%s/%d) are:\n",
         input->name);

  int i;
  for (i = 0; i < input->numOutboundConnections; i++)
    printf("  (%s/%d)\n", input->outboundConnections[i]->name);
  return;
}
// --------------------------------DELETE LATER-------------------------------------

// Shuffle states in place to be able to randomly pick states
void shuffleStates()
{
  int i;
  for (i = 0; i < STATES_NUM-1; i++)
  {
    int j = rand() % STATES_NUM;
    char temp[3];
    strcpy(temp, STATE_NAMES[j]);
    strcpy(STATE_NAMES[j], STATE_NAMES[i]);
    strcpy(STATE_NAMES[i], temp);
  }
}

// Assign rooms to structure array
void assignRooms()
{
  // Create 7 Rooms structures and put the Rooms into array
  selectedRooms[0] = createRoom(0, "START_ROOM");
  selectedRooms[1] = createRoom(1, "END_ROOM");
  int i;
  for (i = 2; i < MAX_ROOM_NUM; i++)
  {
    selectedRooms[i] = createRoom(i, "MID_ROOM"); 
  }
}

// Create a room and initialize members 
// Params: int index: an index of STATE_NAMES 
//         char* roomType: a type of room
// Return: struct Room
struct Room createRoom(int index, char* roomType)
{
  struct Room room;
  
  memset(room.name, '\0', STATE_NAME_NUM+1);
  strcpy(room.name, STATE_NAMES[index]);

  memset(room.type, '\0', 11);
  strcpy(room.type, roomType);

  room.numOutboundConnections = 0;

  return room;
}

// Create 7 files for rooms
// Params: String dirName: A variable of directory title
void createRoomFiles(char* dirName)
{
  char filePostFix[6] = "_room"; 

  int k;
  for (k = 0; k < MAX_ROOM_NUM; k++)
  {
    // Variable for a name of a room file
    char fileName[strlen(dirName) + STATE_NAME_NUM + strlen(filePostFix) + 2];

    // Create a path with directory location and name of file
    snprintf(fileName, sizeof(fileName), "%s%s%s%s", dirName, "/", selectedRooms[k].name, filePostFix);

    // Create a file in the directory
    FILE *fp;
    fp = fopen(fileName, "w");

    // Create a ROOM NAME line and write it to the file
    char roomName[14];
    snprintf(roomName, sizeof(roomName), "%s%s", "ROOM NAME: ", selectedRooms[k].name);
    fputs(roomName, fp);
    fputs("\n", fp);

    // Create CONNECTION lines and write them to the file
    int m;
    for (m = 0; m < selectedRooms[k].numOutboundConnections; m++)
    {
      char connection[17];
      snprintf(connection, sizeof(connection), "%s%d%s%s", "CONNECTION ", m+1, ": ", selectedRooms[k].outboundConnections[m]);
      fputs(connection, fp);
      fputs("\n", fp);
    }

    // Create a ROOM TYPE line and write it to the file
    char roomType[22];
    snprintf(roomType, sizeof(roomType), "%s%s", "ROOM TYPE: ", selectedRooms[k].type);
    fputs(roomType, fp);
    fputs("\n", fp);

    fclose(fp);
  }
}

// Returns true if all rooms have 3 to 6 outbound connections, false otherwise
bool isGraphFull() 
{
  bool isInRange;
  int i;
  for (i = 0; i < MAX_ROOM_NUM; i++)
  {
    if (selectedRooms[i].numOutboundConnections >= MIN_OUT_NUM && selectedRooms[i].numOutboundConnections <= MAX_OUT_NUM)
    {
      isInRange = true; // If the number of connections is between 3 to 6
    }
    else
    {
      isInRange = false; // Otherwise, return false
      break;
    }
  }
  return isInRange;
}

// Adds a random, valid outbound connection from a Room to another Room
void addRandomConnection()  
{
  struct Room* A;
  struct Room* B;

  do
  {
    A = getRandomRoom();
  } while (canAddConnectionFrom(A) == false);
  
  do
  {
    B = getRandomRoom();
  }
  while (canAddConnectionFrom(B) == false || isSameRoom(A, B) == true || connectionAlreadyExists(A, B) == true);
  
  connectRoom(A, B);  
}

// Returns a random Room, does NOT validate if connection can be added
struct Room* getRandomRoom()
{
  int randomIndex = rand() % MAX_ROOM_NUM;

  return &selectedRooms[randomIndex];
}

// Returns true if a connection can be added from Room x (< 6 outbound connections), false otherwise
bool canAddConnectionFrom(struct Room* x)
{
  if (x->numOutboundConnections >= MAX_OUT_NUM)
  {
    return false;
  }
  return true;
}

// Returns true if a connection from Room x to Room y already exists, false otherwise
bool connectionAlreadyExists(struct Room* x, struct Room* y)
{
  int i;
  for (i = 0; i < x->numOutboundConnections; i++)
  {
    if (strcmp(x->outboundConnections[i]->name, y->name) == 0)
    {
      return true;
    }
  }
  return false;
}

// Connects Rooms x and y together, does not check if this connection is valid
void connectRoom(struct Room* x, struct Room* y)
{
  x->outboundConnections[x->numOutboundConnections++] = y;
  y->outboundConnections[y->numOutboundConnections++] = x;
}

// Returns true if Rooms x and y are the same Room, false otherwise
bool isSameRoom(struct Room* x, struct Room* y) 
{
  if (strcmp(x->name, y->name) == 0)
  {
    return true;
  }
  return false;
}
