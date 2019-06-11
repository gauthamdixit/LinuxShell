  #include <libgen.h>
  #include <stdio.h>
  #include <unistd.h>
  #include <stdlib.h>
  #include <sys/types.h>
  #include <sys/stat.h>
  #include <sys/wait.h>
  #include <fcntl.h>
  #include <dirent.h>
  #include <signal.h>
  #include "/home/cs/faculty/cs570/CHK.h"
  #include "getword.h"
  
  #define MAXITEM 100                /* max number of words per line */
  #define MAXSTORAGE (STORAGE*MAXITEM)       /* max amount of characters per command (STORAGE * MAXITEM) */
  #define MAXPIPES 10                /* Max amount of pipes one can use 10 */
  #define MAXPIPEARGS 100            /* Max amount of arguments with use of 10 pipes */
  
  int parse();
  void do_Pipe();
  void sighandler();
  /*
  * Gautham Dixit
  * Carroll
  * CS570
  * Due: 12/6/18
  */
  #include "p2.h"
  
  
  int chars;                                         // Number of characters returneoutputd by getword()
  int ind_pipe[MAXPIPEARGS];         // Index of the address of the word after a pipe (|)
  int pipe_err[MAXPIPEARGS];         // Index of the left of (|&)
  char charray[MAXSTORAGE];          // Array of the characters entered (to stdin) by user
  char *words[MAXITEM];                      // Addresses of the start of each word from charray
  char UIstr[255];
  int input; 
  int output;
  int devnull;
  int argv_get;      
  char *home;        
  char *IN_FILE;
  char *OUT_FILE;                    
  char *ARR_DELIM;
  //flags to be used in parse()
  int BG_FLAG;
  int UI_FLAG = 0;
  int L_ARROW = 0;
  int R_ARROW = 0;
  int PIPE_T = 0;
  int PIPEAMP_T = 0;
  int H_FLAG = 0;
  int S_FLAG = 0;
  int MONEY_FLAG = 0;
  int D_ARROW_FLAG = 0;
  char *file_loc = "tempfile.txt";
  
  
  
  int parse() 
  {
     int i = 0;              
     int word_start = 0;     
     IN_FILE = NULL, OUT_FILE = NULL;
     UI_FLAG = 0;            //flags for each metacharacter to be handled by parse()
     BG_FLAG = 0;
     L_ARROW = 0;
     R_ARROW = 0;
     PIPE_T = 0;
     PIPEAMP_T = 0; 
     H_FLAG = 0;     
     MONEY_FLAG = 0;
     D_ARROW_FLAG = 0;
     
     
     for(;;) {
             
             chars = getword(charray + word_start);
             
             //Setting metacharacter flags
             if(chars == 0) {
                     break;
             }
             
             else if(chars == 101) {
                     break;
             }
             
             
             else if(H_FLAG != 0) {
                     word_start += abs(chars) + 1;
             }
             
             
             
             else if(*(charray + word_start) == '&' && chars == 510) {
                     BG_FLAG++;
                     break;
             }
             //if just a '<' was found set IN_FILE to point to charray
             else if((*(charray + word_start) == '<' && chars == 510) || L_ARROW == 1) {
                     if(L_ARROW == 1)
                     {
                             if(chars < 0)
                             {
                                     if(getenv(charray + word_start) == NULL)
                                     {
                                             fprintf(stderr,"variable not found");
                                     }
                                     IN_FILE = getenv(charray + word_start); 
                             }               
                             else
                             {
                                     IN_FILE = charray + word_start;
                             }
                     }
                     L_ARROW++;
             }
             
             else if((*(charray + word_start) == '>' && chars == 510) || R_ARROW == 1) {
                     if(R_ARROW == 1)
                     {
                             if(chars < 0)
                             {
                                     if(getenv(charray + word_start) == NULL)
                                     {
                                             fprintf(stderr,"variable not found");
                                     }
                                     OUT_FILE = getenv(charray + word_start);    
                             }               
                             else
                             {
                                     OUT_FILE = charray + word_start;
                             }
                     }
                     R_ARROW++;
                     
             }
             //if '<<' was found ie. 300 was returned, set the delimiter to the next word captured by getword.
             else if ((*(charray + word_start) == '<' && chars == 300)|| UI_FLAG == 1) {
                     if (UI_FLAG == 1)
                             ARR_DELIM = charray + word_start;
                     UI_FLAG++;
             }
             
             else if(*(charray + word_start) == '|' && (chars == 510 || chars == 500)) {
                     if(chars == -2) {               
                             pipe_err[PIPE_T] = 1;
                     }
                             
                     PIPE_T++;
                     words[i] = NULL;        
                     i++;
                     ind_pipe[PIPE_T-1] = i;         // Save position of word after pipe
             }
  
             else if((*(charray + word_start) == '#' && chars == 510)) {
                     if(S_FLAG != 0) {
                             H_FLAG++;
                     }
                     else {
                             words[i] = charray + word_start;
                             i++;
                     }
             }
             else if(chars < 0)
             {
                     if(getenv(charray + word_start) == NULL)
                     {
                             fprintf(stderr,"variable not found");
                     }
                     else{
                             words[i] = getenv(charray + word_start);
                             i++;
                     }
             }
             
             //Process the rest of the regular words from the command line
             else {
                     words[i] = charray + word_start;
                     i++;
             }
             word_start += abs(chars) + 1;
     }
     words[i] = NULL;
     return i;
  }
  
  void do_Pipe() 
  {
     //array to hold file descriptors from stdin and stdout
     int fildes[PIPE_T*2];
     //pid for the middle children
     pid_t middle, last;             
     
     // '<' input redirection command found 
  
     if(L_ARROW != 0) 
             {
                     if(IN_FILE == NULL)
                     {
                             fprintf(stderr, "Missing redirect name\n");
                             return;
                     }
             
                     if(L_ARROW > 2) 
                     {
                             fprintf(stderr, "Uknown redirect for input\n");
                             return;
                     }
                     
                     //Opens file outside of fork in read only mode
                     if((input = open(IN_FILE, O_RDONLY)) == -1)
                     {
                             perror(IN_FILE);
                             return;
                     }
     }
     
     // '>' found 
     if(R_ARROW != 0) 
     {
             /* 
                What each of the flags mean:
                O_RDWR: Read/write mode
                O_APPEND: add to end of file
                O_CREAT: able to create a file if one with same name doesn't already exist when opening
                O_EXCL: Ensures file is created, if file exists error is sent 
             */
             int flags = O_RDWR | O_APPEND | O_CREAT | O_EXCL;
             
             if(OUT_FILE == NULL) 
             {
                     fprintf(stderr, "Missing name for redirect\n");
                     return;
             }
             
             if(R_ARROW > 2) 
             {
                     fprintf(stderr, "unknown output redirect\n");
                     return;
             }
             
             /* Open file outside of fork if fails. Allow for read/write permission */
             if((output = open(OUT_FILE, flags, S_IRUSR | S_IWUSR)) == -1)
             {
                     perror(OUT_FILE);
                     return;
             }
     }               
     
     fflush(stdout);
     fflush(stderr);
     
     //creates child with fork()
     CHK(last = fork());
     if(last == 0) 
     {
             //Variables for 'for' loops to close fildes
             int p;
             int n;
             int i;
             
             
             //pipe within child so only child and grandchildren know the file des
             for(p = 0; p < PIPE_T; p++) 
             {
                     CHK(pipe(&fildes[2*p]));
             }
             
             //Redirect input file to STDIN
             if(L_ARROW != 0) 
             {
                     CHK(dup2(input, STDIN_FILENO));
                     CHK(close(input));
             }
             
             //Redirect output file to STDOUT 
             if(R_ARROW != 0)
             {
                     CHK(dup2(output, STDOUT_FILENO));
                     CHK(close(output));
             }
             
             
             for(i = 0; i < PIPE_T; i++) 
             {
                     // For 'for' loop to close fildes
                     int j;                  
                     fflush(stdout);
                     fflush(stderr);
                     CHK(middle = fork());
                     if(middle == 0) 
                     {
                             //if theres only one pipe
                             //printf("child %d\n", i);
                             if(PIPE_T == 1) 
                             {
                                     if(pipe_err[(PIPE_T-1)-i] == 1)
                                             CHK(dup2(fildes[(i*2)+1], STDERR_FILENO));
                                     CHK(dup2(fildes[1], STDOUT_FILENO));
                                     CHK(close(fildes[0]));
                                     CHK(close(fildes[1]));
                                     CHK(execvp(words[0], words));
                             }
                     }
                     else 
                     {
                             //printf("parent %d\n", i);
                             if(PIPE_T != 1)
                             {
                                     
                                     if(i != PIPE_T-1) 
                                     {
                                             CHK(dup2(fildes[(i*2)+2], STDIN_FILENO));
                                     }
                                     
                                     if(pipe_err[(PIPE_T-1)-i] == 1)
                                     {
                                             CHK(dup2(fildes[(i*2)+1], STDERR_FILENO));
                                     }
                     
                                     CHK(dup2(fildes[(i*2)+1], STDOUT_FILENO));
                                     for(j = 0; j < PIPE_T * 2; j++) 
                                     {
                                             CHK(close(fildes[j]));
                                     }
                             
                                     if(i == PIPE_T-1) 
                                     {
                                             CHK(execvp(words[0], words));
                                     }
                                     else
                                     {
                                             CHK(execvp(words[ind_pipe[(PIPE_T-2)-i]], words + ind_pipe[(PIPE_T-2)-i]));
                                     }
                             }
                     }
             }
             
             CHK(dup2(fildes[0], STDIN_FILENO));
             for(n = 0; n < PIPE_T * 2; n++)
             {
                     CHK(close(fildes[n]));
             }
             CHK(execvp(words[ind_pipe[PIPE_T-1]], words + (ind_pipe[PIPE_T-1])));
     }
  
     for(;;) 
     {
             pid_t pid;
             CHK(pid = wait(NULL));
             if(pid == last)
                     break;
     }
  }
  
                                                                     
  int main(int argc, char *argv[])
  {
     
  
     char cwd[1024];
     int g = 0;
     char prompt[20] = ":570: ";
     int word_count = 0;
     int flags = O_RDWR | O_APPEND | O_CREAT | O_EXCL;
     pid_t pid, kidpid;      
     
     // Set group PID for p2.c. 
     //Catches the SIGTERM singal 
     //sends it to handler function 
     setpgid(0,0);
     (void)signal(SIGTERM, sighandler);
     
     for(;;) 
     { 
             unlink(file_loc);
             //checks for invalid commands and too many arguments
             if(argc > 2)
             {
                     fprintf(stderr, "Too many command line arguments\n");
                     exit(EXIT_FAILURE);
             }
             
             else if (argv[2] != NULL && (strcmp(argv[2], "LANG=C") != 0)) 
             {
                     fprintf(stderr, "Invalid command\n");
                     exit(EXIT_FAILURE);
             }
             
             if(S_FLAG == 0)
             {
                     //p2 prompt
                     if(g == 0)
                             printf(prompt);
                     //g = 1 means that cd was found with no arguments
                     else if(g == 1)
                     {       
                             home = getenv("HOME");
                             char *getBase = strdup(home);
                             char *base = basename(getBase);
                             printf("%s%s",base,prompt);
                             
                     }
                     //g = 2 means that cd was found with one arguments
                     //gets current working directory using getcwd()
                     else if (g == 2)
                     {
                             home = getcwd(cwd, sizeof(cwd));
                             char *getBase = strdup(home);
                             char *base = basename(getBase);
                             printf("%s%s", base, prompt);   
                     }
                             
             }
             
             word_count = parse();
             if(chars == 0 && word_count == 0)
             {       
                     break;
             }
     
             //a '<<' was found 
             
             if(word_count == 0)
             {
                     continue;
             }
             //handles if environ is the first argument form stdin
             else if (UI_FLAG == 2)
             {
                     if(L_ARROW != 0)
                     {
                             fprintf(stderr,"bad redirect");
                             continue;
                     }
                     if(ARR_DELIM == NULL)
                     {
                             fprintf(stderr,"no delmiter found");
                             continue;
                     }
                     //open temporary file to store lines from stdin
                     int dup = 0;
                     FILE *fp = fopen("tempfile.txt", "w+");
                     //prompt user fgets
                     printf("? ");
                     fgets(UIstr, 255, stdin);
                     strcat(ARR_DELIM, "\n");
                     //if the user input matches the delimiter, break out of while loop
                     while(strcmp(UIstr, ARR_DELIM) != 0)
                     {
                             fseek(fp,0,SEEK_CUR);
                             fputs(UIstr, fp);
                             printf("? ");
                             fgets(UIstr, 255, stdin);
                             
                     }
                     
                     //close file and open as a file descriptor
                     fclose(fp);
                     if((dup = open("tempfile.txt", O_RDWR)) == -1)
                     {
                             perror("tempfile.txt");
                             continue;
                     }
                     //point output file to tempfile created.
                     else
                     {
                             dup2(dup,output);
                     }
                     
                     
             }
  
             else if(strcmp(words[0],"environ") == 0)
             {
                     //if there are over 3 arguments, there are too many
                     if(word_count > 3 ) 
                     {
                             fprintf(stderr, "Too many arugments \n");
                     }
                     //word_count == 2 means that only one argument came after 'environ'
                     else if(word_count == 2)
                     {
                             //checks if words[1] is a valid directory name
                             if((home = getenv(words[1])) == NULL)
                             {
                                     fprintf(stderr, "Directory doesn't exist \n");
                             }
                             //if directory exists print the path
                             else
                                     printf("%s\n",home);
                     }
                     //word_count == 3 means that there are 2 arguments after 'environ'
                     else if(word_count == 3)
                     {
                             if((home = getenv(words[1])) == NULL)
                             {
                                     fprintf(stderr, "Directory doesn't exist \n");
                             }
                             //if the first argument is valid directory, set environment value to the words[2]
                             //words[2] is the path name of which you want to change value to
                             else if (setenv(words[1],words[2],2) == -1)
                             {
                                     fprintf(stderr, "setting failed \n");
                             }
                             
                     }
                     continue;
  
             }
             else if(strcmp(words[0],"cd") == 0) 
             {
                     //catches if there are more than 2 argurments, then illegal
                     if(word_count > 2) 
                     {
                             fprintf(stderr, "Too many arugments \n");
                     }
                     
                     //If input was only cd with no args
                     //gets HOME environment variable
                     else if (word_count == 1) 
                     {
                             
                             if((home = getenv("HOME")) == NULL) 
                                     fprintf(stderr, "\'HOME\' environment variable doesn't exist\n");
                             else
                             {
                                     //g = 1 for prompt change
                                     g = 1;
                                     chdir(home);
                             }
                     }
                     //if cd has one argument, attempt to change into the directory specified by the argument after 'cd'
                     else 
                     {
                             
                             if ((chdir(words[1])) == -1)
                             {
                                     perror(words[1]);
                             }
                             //set g = 2 for prompt change flag
                             else
                             {
                                     g = 2;
                             }
                     }
                     continue;
             }
             
             // '|' encountered. Pipe command
             if(PIPE_T != 0)
             {
                     int i;
                     int error = 0;
                     if(PIPE_T > MAXPIPES) 
                     {
                             fprintf(stderr, "Too many pipe characters. Max \n");
                             continue;
                     }
                     for(i = 0; i < PIPE_T; i++)
                     {
                             if(words[ind_pipe[i]] == NULL) 
                             {
                                     fprintf(stderr, "Invalid null command\n");
                                     error = 1;
                             }
                     }
                     if(error == 1)
                             continue;
                     do_Pipe();
                     continue;
             }
             
             // '<' encountered. input redirection command
             if(L_ARROW != 0) 
             {
                     if(IN_FILE == NULL)
                     {
                             fprintf(stderr, "Missing name for redirect\n");
                             continue;
                     }
                     
                     if(L_ARROW > 2) 
                     {
                             fprintf(stderr, "Ambiguous input redirect\n");
                             continue;
                     }
                     
                     // opens file in read only if fork fails
                     if((input = open(IN_FILE, O_RDONLY)) == -1)
                     {
                             perror(IN_FILE);
                             continue;
                     }
             }
             
             // '>' encountered. output redirection command
             if(R_ARROW != 0) 
             {
                     int flags = O_RDWR | O_APPEND | O_CREAT | O_EXCL;
                     
                     if(OUT_FILE == NULL) 
                     {
                             fprintf(stderr, "Missing name for redirect\n");
                             continue;
                     }
                     
                     if(R_ARROW > 2)
                     {
                             fprintf(stderr, "Ambiguous output redirect\n");
                             continue;
                     }
                     
                     /* Open file outside of fork if fails. Allow for read/write permission */
                     if((output = open(OUT_FILE, flags, S_IRUSR | S_IWUSR)) == -1) 
                     {
                             perror(OUT_FILE);
                             continue;
                     }
             }
                             /* '|' Pipe command found */    
             
             /* '&' background character command found */
             if(BG_FLAG != 0 && L_ARROW == 0)
             {
                     /* Open /dev/null so background job doesn't read from stdin */
                     if((devnull = open("/dev/null", O_RDONLY)) == -1)
                     {
                             perror("/dev/null");
                             continue;
                     }
             }
             
             //clears out stdout and stderr buffer from input
             fflush(stdout);
             fflush(stderr);
             
             //Forking process to create a child
             if((kidpid = fork()) == -1)
             {
                     perror("Fork failed");
                     exit(1);
             }
             
             //fork suceeded
             //child process:
             else if(kidpid == 0)
             {
             
                     /* Redirect input file to STDIN. Close input file descriptor. */
                     if(L_ARROW != 0) 
                     {
                             if((dup2(input,STDIN_FILENO)) == -1) 
                             {
                                     perror("Failed to redirect file descriptor to STDIN");
                                     exit(1);
                             }
                             if((close(input)) == -1) 
                             {
                                     perror("Unable to close file descriptor\n");
                                     exit(1);
                             }
                     }
  
                     /* Redirect output file to STDOUT. Close output file descriptor. */
                     if(R_ARROW != 0) 
                     {
                             if((dup2(output,STDOUT_FILENO)) == -1) 
                             {
                                     perror("Failed to redirect file descriptor to STDOUT");
                                     exit(1);
                             }
                             if((close(output)) == -1)
                             {
                                     perror("Unable to close file descriptor\n");
                                     exit(1);
                             }
                     }
                     
                     /* Redirect /dev/null to STDIN */
                     if(BG_FLAG != 0 && L_ARROW == 0) 
                     {
                             if((dup2(devnull,STDIN_FILENO)) == -1)
                             {
                                     perror("Failed to redirect file descriptor to STDIN");
                                     exit(1);
                             }
                             if((close(devnull)) == -1) 
                             {
                                     perror("Unable to close file descriptor\n");
                                     exit(1);
                             }
                     }
                     
                     /* Execute command */
                     if((execvp(words[0],words)) == -1 && UI_FLAG != 2) 
                     {
                             perror("Command not found");
                             exit(9);
                     }
             }
             
             /* Parent process */
             else
             {
                     /* No background process found */
                     if(BG_FLAG == 0)
                     {
                             /* Waits for child process to finish */
                             for(;;) 
                             {
                                     pid = wait(NULL);
                                     if(pid == kidpid)
                                             break;
                             }
                     }
                     
                     /* Print out background process */
                     else
                     {
                             printf("%s [%d]\n", words[0], kidpid);
                             continue;
                     }
                     
             }
             
     }
     
     killpg(getpgrp(), SIGTERM);
     printf("p2 terminated.\n");
     exit(0);
  }
  
  void sighandler() {
  
  }
