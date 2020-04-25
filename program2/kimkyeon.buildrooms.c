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
void shuffleStates(void);
struct Room createRoom(int randomNumber, char* type);
bool isGraphFull(void);
void addRandomConnection(void);
struct Room* getRandomRoom(void);
bool canAddConnectionFrom(struct Room*);
bool connectionAlreadyExists(struct Room*, struct Room*);
void connectRoom(struct Room*, struct Room*);
bool isSameRoom(struct Room*, struct Room*);

void PrintRoomOutboundConnections(struct Room*);


// Program starts
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
    printf("%s", "Directory has not been made!!");
    exit(1);
  }

  srand(time(NULL)); // Generate a seed for random numbers
  shuffleStates(); // Shuffle list of states
  
  // Create 7 Rooms and put rooms into array
  selectedRooms[0] = createRoom(0, "START_ROOM");
  selectedRooms[1] = createRoom(1, "END_ROOM");
  int i;
  for (i = 2; i < MAX_ROOM_NUM; i++)
  {
    selectedRooms[i] = createRoom(i, "MID_ROOM"); 
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

  int j;
  for (j = 0; j < 7; j++)
  {
    PrintRoomOutboundConnections(&selectedRooms[j]);
  }


  printf("name: %s, type: %s, num: %d\n", selectedRooms[0].name, selectedRooms[0].type, selectedRooms[0].numOutboundConnections);
  printf("name: %s, type: %s, num: %d\n", selectedRooms[1].name, selectedRooms[1].type, selectedRooms[1].numOutboundConnections);
  printf("name: %s, type: %s, num: %d\n", selectedRooms[2].name, selectedRooms[2].type, selectedRooms[2].numOutboundConnections);
  printf("name: %s, type: %s, num: %d\n", selectedRooms[3].name, selectedRooms[3].type, selectedRooms[3].numOutboundConnections);
  printf("name: %s, type: %s, num: %d\n", selectedRooms[4].name, selectedRooms[4].type, selectedRooms[4].numOutboundConnections);
  printf("name: %s, type: %s, num: %d\n", selectedRooms[5].name, selectedRooms[5].type, selectedRooms[5].numOutboundConnections);
  printf("name: %s, type: %s, num: %d\n", selectedRooms[6].name, selectedRooms[6].type, selectedRooms[6].numOutboundConnections);

  return 0;
}

void PrintRoomOutboundConnections(struct Room* input)
{
  printf("The rooms connected to (%s/%d) are:\n",
         input->name);

  int i;
  for (i = 0; i < input->numOutboundConnections; i++)
    printf("  (%s/%d)\n", input->outboundConnections[i]->name);
  return;
}

// Shuffle states to be able to randomly pick states
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

// Create a room and initialize data in the room
// Params: int randomNumber: to select name of room in ROOM_NAMES 
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

// Returns true if all rooms have 3 to 6 outbound connections, false otherwise
bool isGraphFull()  
{
  bool isInRange;
  int i;
  for (i = 0; i < MAX_ROOM_NUM; i++)
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
