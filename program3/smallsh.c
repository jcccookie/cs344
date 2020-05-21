// Oregon State University - CS 344 (Operating Systems)
// Program 3
// Name: smallsh
// Author: Kyeong-nam Kim


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>

#define ARGS_LIMIT 513

// Boolean enumerator
typedef enum
{
  false,
  true
} bool;

int parentPid = -5; // Parent proess id
int childPid = -5; // Child process id
bool isForeground = false; // Foreground-only toggle
bool duringProcess = false; // To track a process status to properly print ':' on prompt when a user type ctrl-z during the process or block SIGCHLD while parent process is running
int childExitMethod = -5;

// Declare functions
char* getCommandLine(void);
void getStatus(int);
char* expandPID(char*, char*);

// Signal handlers
void catchSIGCHLD(int);
void catchSIGINT(int);
void catchSIGTSTP(int);
void catchSIGTERM(int);

// Entry point
int main(int argc, char *argv[])
{
   char* line = NULL; // A line input from an user
   char* word = NULL; // A buffer to store a token of a string
   char* convertedLine = NULL; // An expanded line 

   char* sourceFile = NULL; // Pointer to source file
   char* targetFile = NULL; // Pointer to target file
   int sourceFD, targetFD, fileResult; // File descriptors
   bool isInput = false; // Boolean for checking if a source file exists
   bool isOutput = false; // Boolean for checking if a target file exists
   int stdIn, stdOut; // To restore stdin or stdout after redirection
   
   char* arguments[ARGS_LIMIT]; // An array for arguments
  
   pid_t spawnPid = -5; // Spawned process id

   // Set SIGCHLD handler
   struct sigaction SIGCHLD_action = {0};
   SIGCHLD_action.sa_handler = catchSIGCHLD;
   sigfillset(&SIGCHLD_action.sa_mask);
   SIGCHLD_action.sa_flags = SA_RESTART; 
   sigaction(SIGCHLD, &SIGCHLD_action, NULL);

   // Set SIGINT handler
   struct sigaction SIGINT_action = {0};
   SIGINT_action.sa_handler = catchSIGINT;
   sigfillset(&SIGINT_action.sa_mask);
   SIGINT_action.sa_flags = SA_RESTART;
   sigaction(SIGINT, &SIGINT_action, NULL);

   // Set SIGTSTP handler
   struct sigaction SIGTSTP_action = {0};
   SIGTSTP_action.sa_handler = catchSIGTSTP;
   sigfillset(&SIGTSTP_action.sa_mask);
   SIGTSTP_action.sa_flags = SA_RESTART;
   sigaction(SIGTSTP, &SIGTSTP_action, NULL);

   // To block SIGTSTP, SIGCHLD from occuring during the process execution
   sigset_t new_stop, old_stop, new_child, old_child;
   sigemptyset(&new_stop);
   sigaddset(&new_stop, SIGTSTP);
   sigemptyset(&new_child);
   sigaddset(&new_child, SIGCHLD);

   // Set SIGTERM handler
   struct sigaction SIGTERM_action = {0};
   SIGTERM_action.sa_handler = catchSIGTERM;
   sigfillset(&SIGTERM_action.sa_mask);
   SIGTERM_action.sa_flags = 0;
   sigaction(SIGTERM, &SIGTERM_action, NULL);

   // Convert PID to a string
   int pid = getpid();
   char strPid[100];
   sprintf(strPid, "%d", pid);

   while (1)
   {
      int index = 0; // Index of arguments
      line = getCommandLine();
      bool isBackground = false;
   
      // Handle commenting out and blank
      if (line[0] == '#' || line[0] == '\0')
      {
         continue;
      }
      
      // Expand $$ to PID
      convertedLine = expandPID(line, strPid);
      
      // Split a string by space and put it into arguments array
      while ((word = strsep(&convertedLine, " ")) != NULL)
      { 
         // If output file exists, save it to a targetFile
         if (strcmp(word, ">") == 0)
         {
            word = strsep(&convertedLine, " ");
            targetFile = strdup(word);
            isOutput = true;
            continue;
         }
         // If input file exists, save it to a sourceFile
         else if (strcmp(word, "<") == 0)
         {
            word = strsep(&convertedLine, " "); 
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
         // Terminate all child process forked from the parent process before exit
         if (parentPid != -5)
         {
            killpg(getpid(), SIGTERM); // Terminate every child with pgid which is the same with parent's pid

            exit(0);
         }

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
               perror("Wrong home path!"); fflush(stdout);
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
                  perror("Wrong path!"); fflush(stdout);
               }
            }
            // Relative path
            else
            {
               snprintf(relativePath, sizeof(relativePath), "%s%s%s", getcwd(cwd, 200), "/", path);

               if (chdir(relativePath) != 0)
               {
                  perror("Wrong path!"); fflush(stdout);
               }
            }
         }
      }
      // Built-in command: status
      else if (strcmp(arguments[0], "status") == 0)
      {
         // Print out status before running any process
         if (childExitMethod == -5)
         {
            printf("exit value %d\n", 0); fflush(stdout);
            continue;
         }

         getStatus(childExitMethod);
      } 
      // Non built-in commands
      else
      {
         spawnPid = fork();
         
         duringProcess = true;

         // Error handling
         if (spawnPid == -1)
         {
            perror("Fork Error\n"); exit(1);
         }
         // CHILD process
         else if (spawnPid == 0)
         {
            sigprocmask(SIG_SETMASK, &old_stop, NULL); // Unblock SIGTSTP

            // Redirect stdin and stdout to /dev/null in background process 
            if (isBackground && !isForeground)
            {
               if (sourceFile == NULL)
               {
                  stdIn = dup(0); // Save stdin descriptor

                  sourceFD = open("/dev/null", O_RDONLY);
                  if (sourceFD == -1) 
                  {
                     perror("error: source open()"); exit(1);
                  }

                  fileResult = dup2(sourceFD, 0);
                  if (fileResult == -1) 
                  { 
                     perror("error: source dup2()"); exit(1); 
                  }
                  // close(sourceFD);
                  fcntl(sourceFD, F_SETFD, FD_CLOEXEC);
               }
               if (targetFile == NULL)
               {
                  stdOut = dup(1); // Save stdout descriptor

                  targetFD = open("/dev/null", O_WRONLY, 0644);
                  if (targetFD == -1)
                  {
                     perror("error: target open()"); exit(1);
                  }

                  fileResult = dup2(targetFD, 1);
                  if (fileResult == -1) 
                  { 
                     perror("error: target dup2()"); exit(1); 
                  }
                  // close(targetFD);
                  fcntl(targetFD, F_SETFD, FD_CLOEXEC);
               }
            }

            // Redirect stdin if source file exists
            if (isInput)
            {
               stdIn = dup(0); // Save stdin descriptor

               // Read input file
               sourceFD = open(sourceFile, O_RDONLY);
               if (sourceFD == -1) 
               {
                  perror("error: source open()"); exit(1);
               }

               // Redirects stdin to a file
               fileResult = dup2(sourceFD, 0);
               if (fileResult == -1) 
               { 
                  perror("error: source dup2()"); exit(1); 
               }
               // close(sourceFD);
               fcntl(sourceFD, F_SETFD, FD_CLOEXEC);
               free(sourceFile);
               sourceFile = NULL;
            }

            // Redirect stdout if target file exists
            if (isOutput)
            {
               stdOut = dup(1); // Save stdout descriptor

               // Create or write output file
               targetFD = open(targetFile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
               if (targetFD == -1)
               {
                  perror("error: target open()"); exit(1);
               }

               // Redirects stdout to a file
               fileResult = dup2(targetFD, 1);
               if (fileResult == -1) 
               { 
                  perror("error: target dup2()"); exit(1); 
               }

               // close(targetFD);
               fcntl(targetFD, F_SETFD, FD_CLOEXEC);
               free(targetFile);
               targetFile = NULL;
            }

            // In background and not foreground only mode, ignore SIGINT(ctrl + c)
            if (isBackground && !isForeground)
            {
               struct sigaction ignore_action = {0};
               ignore_action.sa_handler = SIG_IGN;
               sigaction(SIGINT, &ignore_action, NULL);
            }

            sigprocmask(SIG_BLOCK, &new_stop, &old_stop); // Block SIGTSTP before exec()
            
            // Execute exec()
            if (execvp(arguments[0], arguments) == -1)
            {
               perror("exec() failed"); exit(1);
            }
         }
         // PARENT process
         else
         {
            parentPid = (int)getpid();

            // Let user know background has started
            if (isBackground && !isForeground)
            {
               printf("background pid is %d\n", spawnPid);
               isBackground = false;
            }
            else
            {
               sigprocmask(SIG_BLOCK, &new_child, &old_child); // Block SIGCHLD during the parent's process execution

               childPid = spawnPid; // Save a child's pid to a global variable to catch SIGINT for only in a foreground child process

               setpgid(getpid(), spawnPid); // Set a child's pgid to parent's pid

               waitpid(spawnPid, &childExitMethod, 0);

               childPid = -5; // Reset childPid 

               sigprocmask(SIG_SETMASK, &old_child, NULL); // Unblock SIGCHLD
            }
         }
      }
      
      duringProcess = false; // reset duringProcess
            
      // Restore stdin and stdout descriptors to original path
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

      // Free line and convertedLine memory 
      free(line);
      line = NULL;
      free(convertedLine);
      convertedLine = NULL;
   }

   return 0;
}

// Get a command line from a user
// Return: a string entered by a user with newline removed
char* getCommandLine()
{
   int numCharEntered = -5;
   size_t bufferSize = 0;
   char* lineEntered = NULL;

   printf(": "); fflush(stdout);

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

// Get a status of exit or signal termination
// Params: an exit method a child process
void getStatus(int exitMethod)
{
   int status;

   // Print out exit status
   if (WIFEXITED(exitMethod))
   {
      status = WEXITSTATUS(exitMethod);
      printf("exit value %d\n", status); fflush(stdout);
   }
   // Print out signal value when terminated by signal
   else if (WIFSIGNALED(exitMethod))
   {
      status = WTERMSIG(exitMethod);
      printf("terminated by signal %d\n", status); fflush(stdout);
   }
}

// Expand $$ to PID
// Params: string command line to be scanned
//       : PID number in string form
// Return: expanded command line
char* expandPID(char* line, char* strPid) 
{ 
   int pidLength = strlen(strPid); // Length of PID

   // Count the number of $$ in command line
   int countSymbol = 0; 
   int i;
   for (i = 0; line[i] != '\0'; i++) 
   { 
      if (strstr(&line[i], "$$") == &line[i]) 
      { 
         countSymbol++; 
         i++;
      } 
   } 

   int lineLength = strlen(line);
   // Size of buffer: Length of original command line + (number of $$ * (length of pid - length of $$)) + 1
   int bufferSize = lineLength + (countSymbol * (pidLength - 2)) + 1; 
   char* convertedLine = malloc(bufferSize); // Allocate a string with sufficient spaces

   int j;
   int k = 0; // index of converted line
   for (j = 0; j < lineLength; j++)
   {
      // If find $$, copy PID
      if (strstr(line, "$$") == line)
      {
         strcpy(&convertedLine[k], strPid);
         k += pidLength;
         line += 2;
         j++;
      }
      // Otherwise, copy a character from original command line
      else
      {
         strcpy(&convertedLine[k], line);
         line++;
         k++;
      }
   }

   convertedLine[k] = '\0'; // Set null terminator

   return convertedLine;
}

// Signal handler for SIGCHLD 
void catchSIGCHLD(int signum)
{
   int pid;
   
   while ((pid = waitpid(-1, &childExitMethod, WNOHANG)) > 0)
   {
      int pidLength = sizeof(pid);
      char pidStr[pidLength+1];
      sprintf(pidStr, "%d", pid);

      write(STDOUT_FILENO, "\33[2K\r", 6); fflush(stdout); // Remove ":" in prompt
      write(STDOUT_FILENO, "background pid ", 15); fflush(stdout);
      write(STDOUT_FILENO, pidStr, pidLength+2); fflush(stdout);
      write(STDOUT_FILENO, " is done: ", 10); fflush(stdout);
      
      getStatus(childExitMethod);

      // Print ":" only when the process normally exit not terminated by signal and not occuring during process
      if (WIFEXITED(childExitMethod) && !duringProcess)
      {
         write(STDOUT_FILENO, ": ", 3); fflush(stdout);
      }
   }
}

// Signal handler for SIGINT
void catchSIGINT(int signum)
{
   if (childPid != -5)
   {
      waitpid(childPid, &childExitMethod, 0);
      getStatus(childExitMethod);
   }
}

// Signal handler for SIGTSTP 
void catchSIGTSTP(int signum)
{
   int exitMethod;

   if (childPid != -5)
   {
      waitpid(childPid, &exitMethod, 0);
   }

   // Print out an informative message
   if (!isForeground)
   {
      write(STDOUT_FILENO, "\nEntering foreground-only mode (& is now ignored)\n", 50); fflush(stdout);
   }
   else
   {
      write(STDOUT_FILENO, "\nExiting foreground-only mode\n", 30); fflush(stdout);
   }

   // Toggle foreground only mode
   if (isForeground)
   {
      isForeground = false;
   }
   else
   {
      isForeground = true;
   }

   // If not SIGTSTP occurs during the process printf ":" on prompt
   if (!duringProcess)
   {
      write(STDOUT_FILENO, ": ", 3); fflush(stdout);
   }
}

// Print nothing(not to print 'Killed') in prompt when parent process is terminated by command 'exit' with children process running in background
void catchSIGTERM(int signum)
{
   return;
}
