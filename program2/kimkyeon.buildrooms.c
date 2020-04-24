#include <stdlib.h> 
#include <stdio.h> 
#include <string.h>

// Define constants
#define ROOM_NUM 10
#define ROOM_NAME_NUM 2

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

// Function declarations
struct Room createRoom(int randomNumber, char* type);

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

  // Using Process ID, generate a random number to randomly choose 7 rooms
  char pidString[PID_LENGTH+1];
  sprintf(pidString, "%d", PID);
  int randomNumber = pidString[PID_LENGTH] - '0';
  
  // Create 7 Rooms
  struct Room startRoom = createRoom(randomNumber++, "START_ROOM");
  struct Room midRoomOne = createRoom(randomNumber++, "MID_ROOM");
  struct Room midRoomTwo = createRoom(randomNumber++, "MID_ROOM");
  struct Room midRoomThree = createRoom(randomNumber++, "MID_ROOM");
  struct Room midRoomFour = createRoom(randomNumber++, "MID_ROOM");
  struct Room midRoomFive = createRoom(randomNumber++, "MID_ROOM");
  struct Room endRoom = createRoom(randomNumber++, "END_ROOM");



  return 0;
}

// Create and initialize an individual room
// Params: int randomNumber: to select name of room in ROOM_NAMES 
//         char* roomType: a type of room
//         int typeStringLength: length of string of type of room
// Return: struct Room
struct Room createRoom(int randomNumber, char* roomType)
{
  struct Room room;
  
  memset(room.name, '\0', 3);
  randomNumber = randomNumber % 10; // Mod randomNumber to prevent it from being greater than 10
  strcpy(room.name, ROOM_NAMES[randomNumber]);

  memset(room.type, '\0', 11);
  strcpy(room.type, roomType);

  room.numOutboundConnections = 0;

  return room;
}

// Create all connections in graph
// while (IsGraphFull() == false)
// {
//   AddRandomConnection();
// }

// Returns true if all rooms have 3 to 6 outbound connections, false otherwise
// bool IsGraphFull()  
// {
  
// }

// Adds a random, valid outbound connection from a Room to another Room
// void AddRandomConnection()  
// {
//   Room A;  // Maybe a struct, maybe global arrays of ints
//   Room B;

//   while(true)
//   {
//     A = GetRandomRoom();

//     if (CanAddConnectionFrom(A) == true)
//       break;
//   }

//   do
//   {
//     B = GetRandomRoom();
//   }
//   while(CanAddConnectionFrom(B) == false || IsSameRoom(A, B) == true || ConnectionAlreadyExists(A, B) == true);

//   ConnectRoom(A, B);  // TODO: Add this connection to the real variables, 
//   ConnectRoom(B, A);  //  because this A and B will be destroyed when this function terminates
// }

// Returns a random Room, does NOT validate if connection can be added
// Room GetRandomRoom()
// {
  
// }

// Returns true if a connection can be added from Room x (< 6 outbound connections), false otherwise
// bool CanAddConnectionFrom(Room x) 
// {
  
// }

// Returns true if a connection from Room x to Room y already exists, false otherwise
// bool ConnectionAlreadyExists(x, y)
// {
  
// }

// Connects Rooms x and y together, does not check if this connection is valid
// void ConnectRoom(Room x, Room y) 
// {
  
// }

// Returns true if Rooms x and y are the same Room, false otherwise
// bool IsSameRoom(Room x, Room y) 
// {
  
// }
