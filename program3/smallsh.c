#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>

#define ARGS_LIMIT 513

char* getCommandLine(void);

int main(int argc, char *argv[])
{
   char* line;
   char* word;
   
   char* arguments[ARGS_LIMIT];

   int index = 0; // Index of arguments
   while (1)
   {
      line = getCommandLine();

      // Split a string by space and put it into arguments array
      while ((word = strsep(&line, " ")) != NULL)
      {
         arguments[index++] = strdup(word);
      }
      arguments[index] = NULL; // Set Null terminator for exec

      if (strcmp(arguments[0], "exit") == 0)
      {
         // TO DO:
         // Need to clear out all processes and background processes before exit


         exit(0);

      }
      else if (strcmp(arguments[0], "cd") == 0)
      {

         // No argument exists. Directs to HOME directory
         if (arguments[1] == NULL)
         {
            char* home;
            home = getenv("HOME");
            chdir(home);
            
            if (chdir(home) != 0)
            {
               perror("Wrong home path!\n");
            }
         }
         // An argument exists. Directs to the path
         else
         {
            // Path an user entered in
            char* path;
            path = arguments[1];
            char relativePath[100];
            char cwd[200];

            // Absolute path
            if (path[0] == '/')
            {
               if (chdir(path) != 0)
               {
                  perror("Wrong path!\n");
               }
            }
            // Relative path
            else
            {
               snprintf(relativePath, sizeof(relativePath), "%s%s%s", getcwd(cwd, 200), "/", path);
               if (chdir(relativePath) != 0)
               {
                  perror("Wrong path!\n");
               }
            }
         }
      }
      else if (strcmp(arguments[0], "status") == 0)
      {
         // TODO:
         // Exit status
         // Terminating signal of the last foreground process
         // If this is running before any foreground process, return the exit status 0.




      }
      else
      {
         execvp(arguments[0], arguments); 
      }
      
      index = 0;

      free(line);
      line = NULL;
   }


   return 0;
}

// Get a command line from a user
// Return: a string entered by a user with a newline detached
char* getCommandLine()
{
   int numCharEntered = -5;
   size_t bufferSize = 0;
   char* lineEntered = NULL;

   printf(": ");
   fflush(stdout);

   numCharEntered = getline(&lineEntered, &bufferSize, stdin);

   // If a prompt interrupted by a signal, clear the stdin buffer
   if (numCharEntered == -1)
   {
      clearerr(stdin);
   }

   // Remove newline 
   if (lineEntered[numCharEntered - 1] == '\n') 
   {
      lineEntered[numCharEntered - 1] = '\0';
   }

   return lineEntered;
}