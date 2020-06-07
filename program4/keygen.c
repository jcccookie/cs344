#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// Global constant variables
const char* CHAR_SEEDS = "ABCDEFGHIJKLMNOPQRSTUVWXYZ ";
const int CHAR_NUM = 27;

int main(int argc, char *argv[])
{
   // Check if the number of arguments is 2
   if (argc != 2)
   {
      fprintf(stderr, "Number of arguments is invalid\n");
      exit(1);
   }

   // Number of keys
   int num = atoi(argv[1]);
   char keys[num+1];

   // Seed for random numbers
   srand(time(NULL));

   // Get random keys
   int i;
   for (i = 0; i < num; i++)
   {
      keys[i] = CHAR_SEEDS[rand() % CHAR_NUM];
   }

   // Null terminator
   keys[num] = '\0';

   // Put a newline
   printf("%s\n", keys);

   return 0;
}
