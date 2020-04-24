/* readDirectory.c */
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
// The procedure is that we first use opendir() to open a directory stream so that we can step through it. Then, we loop through that directory with readdir(), which returns entries for analysis. Eventually, readdir() will return NULL when it reaches the last entry, which will terminate our loop through the directory. Each of the returned entries is in the form of a "dirent struct" that contains, among other things, the name of the entry.

// For each entry in the directory, it could be a file or a subdirectory. We use stat() to get detailed information about the entry, including the timestamp that is updated for creation and modification, for example. stat() returns information by writing its results into a "stat struct" that we pass in.

// In this sample program, we open up the current directory and then loop through each entry (file or subdirectory) inside it. We are looking for entries that begin with the pathname: "brewsteb.rooms.". If we find a match, we get the last modified timestamp of it, and record the name and timestamp. We compare these to the previously held entry, storing those that are newer.

void main()
{
  int newestDirTime = -1; // Modified timestamp of newest subdir examined
  char targetDirPrefix[32] = "brewsteb.rooms."; // Prefix we're looking for
  char newestDirName[256]; // Holds the name of the newest dir that contains prefix
  memset(newestDirName, '\0', sizeof(newestDirName));

  DIR* dirToCheck; // Holds the directory we're starting in
  struct dirent *fileInDir; // Holds the current subdir of the starting dir
  struct stat dirAttributes; // Holds information we've gained about subdir

  dirToCheck = opendir("."); // Open up the directory this program was run in

  if (dirToCheck > 0) // Make sure the current directory could be opened
  {
    while ((fileInDir = readdir(dirToCheck)) != NULL) // Check each entry in dir
    {
      if (strstr(fileInDir->d_name, targetDirPrefix) != NULL) // If entry has prefix
      {
        printf("Found the prefex: %s\n", fileInDir->d_name);
        stat(fileInDir->d_name, &dirAttributes); // Get attributes of the entry

        if ((int)dirAttributes.st_mtime > newestDirTime) // If this time is bigger
        {
          newestDirTime = (int)dirAttributes.st_mtime;
          memset(newestDirName, '\0', sizeof(newestDirName));
          strcpy(newestDirName, fileInDir->d_name);
          printf("Newer subdir: %s, new time: %d\n",
                 fileInDir->d_name, newestDirTime);
        }
      }
    }
  }

  closedir(dirToCheck); // Close the directory we opened

  printf("Newest entry found is: %s\n", newestDirName);
}