#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

// Define constants
#define ROOM_NUM 10
#define ROOM_NAME_NUM 2
#define MIN_OUT_NUM 3
#define MAX_OUT_NUM 6

// Define boolean type
typedef enum
{
  false,
  true
} bool;

// 10 room names in array
char ROOM_NAMES[ROOM_NUM][ROOM_NAME_NUM+1] = {"ca", "or", "wa", "hi", "nv", "ut", "ny", "nj", "ma", "pa"};

// struct for individual room information
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
struct Room createRoom(int randomNumber, char* type);
bool isGraphFull(void);
void addRandomConnection(void);
struct Room getRandomRoom(void);
bool canAddConnectionFrom(struct Room*);
bool connectionAlreadyExists(struct Room*, struct Room*);
void connectRoom(struct Room*, struct Room*);
bool isSameRoom(struct Room*, struct Room*);


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
//  int result = mkdir(dirName, 0755);

  // If an error occurs, exit
//   if (result)
//   {
//     printf("%s", "Directory has not been made!!");
//     exit(1);
//   }

  // Generate a random number
  srand(time(NULL));
  int randomNumber = rand() % ROOM_NUM;
  
  // Create 7 Rooms and put rooms into array
  selectedRooms[0] = createRoom(randomNumber++, "START_ROOM");
  selectedRooms[1] = createRoom(randomNumber++, "END_ROOM");
  int i;
  for (i = 2; i < 7; i++)
  {
    selectedRooms[i] = createRoom(randomNumber++, "MID_ROOM");
  }

  printf("name: %s, type: %s, num: %d\n", selectedRooms[0].name, selectedRooms[0].type, selectedRooms[0].numOutboundConnections);
  printf("name: %s, type: %s, num: %d\n", selectedRooms[1].name, selectedRooms[1].type, selectedRooms[1].numOutboundConnections);
  printf("name: %s, type: %s, num: %d\n", selectedRooms[2].name, selectedRooms[2].type, selectedRooms[2].numOutboundConnections);
  printf("name: %s, type: %s, num: %d\n", selectedRooms[3].name, selectedRooms[3].type, selectedRooms[3].numOutboundConnections);
  printf("name: %s, type: %s, num: %d\n", selectedRooms[4].name, selectedRooms[4].type, selectedRooms[4].numOutboundConnections);
  printf("name: %s, type: %s, num: %d\n", selectedRooms[5].name, selectedRooms[5].type, selectedRooms[5].numOutboundConnections);
  printf("name: %s, type: %s, num: %d\n", selectedRooms[6].name, selectedRooms[6].type, selectedRooms[6].numOutboundConnections);

  // Create all connections in graph
  while (isGraphFull() == false)
  {
    addRandomConnection();
  }

  printf("The rooms connected to (%s) are:\n", selectedRooms[0].name);

  int j;
  for (j = 0; j < selectedRooms[0].numOutboundConnections; j++)
    printf("  (%s)\n", selectedRooms[0].outboundConnections[j]->name);

  printf("name: %s, type: %s, num: %d\n", selectedRooms[0].name, selectedRooms[0].type, selectedRooms[0].numOutboundConnections);
  printf("name: %s, type: %s, num: %d\n", selectedRooms[1].name, selectedRooms[1].type, selectedRooms[1].numOutboundConnections);
  printf("name: %s, type: %s, num: %d\n", selectedRooms[2].name, selectedRooms[2].type, selectedRooms[2].numOutboundConnections);
  printf("name: %s, type: %s, num: %d\n", selectedRooms[3].name, selectedRooms[3].type, selectedRooms[3].numOutboundConnections);

  return 0;
}

// Create a room and initialize data in the room
// Params: int randomNumber: to select name of room in ROOM_NAMES
//         char* roomType: a type of room
// Return: struct Room
struct Room createRoom(int randomNumber, char* roomType)
{
  struct Room room;
  
  memset(room.name, '\0', ROOM_NAME_NUM+1);
  randomNumber = randomNumber % ROOM_NUM; // Mod randomNumber to prevent it from being greater than 9
  strcpy(room.name, ROOM_NAMES[randomNumber]);

  memset(room.type, '\0', 11);
  strcpy(room.type, roomType);

  room.numOutboundConnections = 0;

  return room;
}


// Returns true if all rooms have 3 to 6 outbound connections, false otherwise
bool isGraphFull()
{
  bool isInRange;
  int i;
  for (i = 0; i < 7; i++)
  {
    if (selectedRooms[i].numOutboundConnections >= MIN_OUT_NUM && selectedRooms[i].numOutboundConnections <= MAX_OUT_NUM)
    {
      isInRange = true;
    }
    else
    {
      isInRange = false;
      break;
    }
  }
  return isInRange;
}

// Adds a random, valid outbound connection from a Room to another Room
void addRandomConnection()
{
  struct Room A;
  struct Room B;

  do
  {
    A = getRandomRoom();
  } while (canAddConnectionFrom(&A) == false);
  
  do
  {
    B = getRandomRoom();
  }
  while (canAddConnectionFrom(&B) == false || isSameRoom(&A, &B) == true || connectionAlreadyExists(&A, &B) == true);
  
  connectRoom(&A, &B);
}

// Returns a random Room, does NOT validate if connection can be added
struct Room getRandomRoom()
{
  int randomIndex = rand() % 7;

  return selectedRooms[randomIndex];
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
