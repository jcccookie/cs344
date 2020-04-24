#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char* argv[])
{
  char myArray[10];
  myArray[2] = 'A';
  char* myPointer = myArray;
  char A = myPointer[2];
  printf("char A: %c\n", A);

  char my2DArray[10][20];
  my2DArray[2][4] = 'B';
  char specificChar = my2DArray[2][4];
  char* specificCharP = &my2DArray[2][4];
  printf("char specificChar: %c\n", specificChar);
  printf("char* specificCharP: %c\n", *specificCharP);

  char (*my2DPointer)[20] = my2DArray;
  printf("my2DPointer: %c\n", my2DPointer[2][4]);

  char (*noTrouble)[20] = malloc(10 * 20 * sizeof(char));
  printf("About to assign noTrouble[0][0]\n");
  noTrouble[0][0] = 'T';
  printf("About to print out noTrouble[0][0]\n");
  printf("noTrouble[0][0]: %c\n", noTrouble[0][0]);

  char one = 'C';
  char* two = &one;
  char** three = &two;
  printf("one deferenced twice: %c\n", **three);

  // Our "ls" cmd has 3 args; one more string must be NULL for execvp()
  // ls -p -l -a
  char* arr[5];
  int i;
  for (i = 0; i < 4; i++)
    arr[i] = malloc(20 * sizeof(char));
  strcpy(arr[0], "ls");
  strcpy(arr[1], "-p");
  strcpy(arr[2], "-l");
  strcpy(arr[3], "-a");
  arr[4] = NULL;

  printf("About to convert to command \'ls -p -l -a\', here are the strings:\n");
  for (i = 0; i < 5; i++)
    printf(" array member %i: %s\n", i, arr[i]);
  printf("*****Everything past this line is part of the \"ls\" command*****\n");
  fflush(stdout);
  execvp(arr[0], arr);
  return 0;
}