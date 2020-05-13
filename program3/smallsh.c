#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>

#define ARGS_LIMIT 513

typedef enum
{
  false,
  true
} bool;

char* getCommandLine(void);

int main(int argc, char *argv[])
{
   char* line; // A line input from an user
   char* word; // A buffer to store a token of a string
   char* sourceFile, targetFile;
   int sourceFD, targetFD, fileResult;
   bool isInput = false;
   bool isOutput = false;
   int stdIn, stdOut;
   
   char* arguments[ARGS_LIMIT]; // An array for arguments
  
   pid_t spawnPid = -5;
   int childExitMethod = -5;
   int status = 0; // Exit or Signal status

   while (1)
   {
      int index = 0; // Index of arguments
      line = getCommandLine();
      bool isBackground = false;

      // Split a string by space and put it into arguments array
      while ((word = strsep(&line, " ")) != NULL)
      { 
         // If output file exists, save it to a targetFile
         if (strcmp(word, ">") == 0)
         {
            word = strsep(&line, " ");
            targetFile = strdup(word);
            isOutput = true;
            continue;
         }
         // If input file exists, save it to a sourceFile
         else if (strcmp(word, "<") == 0)
         {
            word = strsep(&line, " "); 
            sourceFile = strdup(word);
            isInput = true;
            continue;
         }
         // Save a token to arguments array
         arguments[index++] = strdup(word);
      }

      // Check ampersand in the command line
      if (strcmp(arguments[--index], "&") == 0)
      {
         isBackground = true;
      }
      else
      {
         index++;
      }
      
      arguments[index] = NULL; // Set Null terminator for exec

      // Built-in command: exit
      if (strcmp(arguments[0], "exit") == 0)
      {
         // TO DO:
         // Need to clear out all processes and background processes before exit


         exit(0);
      }
      // Built-in command: cd
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
            char cwd[200]; // Buffer for current working directory

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
      // Built-in command: status
      else if (strcmp(arguments[0], "status") == 0)
      {
         // TODO:
         // Exit status
         // Terminating signal of the last foreground process
         // If this is running before any foreground process, return the exit status 0.
         if (childExitMethod == -5)
         {
            printf("exit value %d\n", status); 
            continue;
         }
         
         // Print out exit status
         if (WIFEXITED(childExitMethod))
         {
            status = WEXITSTATUS(childExitMethod);
            printf("exit value %d\n", status);
         }
         // Print out signal value when terminated by signal
         else if (WIFSIGNALED(childExitMethod))
         {
            status = WTERMSIG(childExitMethod);
            printf("terminated by signal %d\n", status);
         }
      } 
      // Non built-in commands
      else
      {
         spawnPid = fork();

         if (spawnPid == -1)
         {
            perror("Fork Error\n"); exit(1);
         }
         else if (spawnPid == 0)
         {
            // Redirect stdin
            if (isInput)
            {
               stdIn = dup(0); // Save stdin descriptor
               
               // Read a input file
               sourceFD = open(sourceFile, O_RDONLY);
               if (sourceFD == -1) 
               {
                  perror("error: source open()\n"); exit(1);
               }

               // Redirects stdin to a file
               fileResult = dup2(sourceFD, 0);
               if (fileResult == -1) 
               { 
                  perror("error: source dup2()"); exit(1); 
               }

               close(sourceFD);
               free(sourceFile);
               sourceFile = NULL;
            }

            // Redirect stdout
            if (isOutput)
            {
               stdOut = dup(1); // Save stdout descriptor

               // Create or write output file
               targetFD = open(targetFile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
               if (targetFD == -1)
               {
                  perror("error: target open()\n"); exit(1);
               }
            
               // Redirects stdout to a file
               fileResult = dup2(targetFD, 1);
               if (fileResult == -1) 
               { 
                  perror("error: target dup2()"); exit(1); 
               }

               close(targetFD);
               free(targetFile);
               targetFile = NULL;
            }

            // Execute exec()
            if (execvp(arguments[0], arguments) == -1)
            {
               perror("exec() failed"); exit(1);
            }
            
         }
         
         waitpid(spawnPid, &childExitMethod, 0);
      }

      fflush(stdout);

      // Restore stdin and stdout descriptors
      if (isInput)
      {
         dup2(stdIn, 0);
         close(stdIn);
         isInput = false;
      }
      if (isOutput)
      {
         dup2(stdOut, 1);
         close(stdOut);
         isOutput = false;
      }

      // Free allocated memory for arguments
      int j;
      for (j = 0; j < index; j++)
      {
         free(arguments[j]);
      }

      // Free getline memory 
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

   // Replace newline with NULL terminator 
   if (lineEntered[numCharEntered - 1] == '\n') 
   {
      lineEntered[numCharEntered - 1] = '\0';
   }

   return lineEntered;
}