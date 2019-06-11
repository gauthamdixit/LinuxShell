 /*
    * Gautham Dixit
    * Carroll
    * CS570
    * Due: 12/6/18
    */
   #include "getword.h"
   #include <stdlib.h>
   #include <pwd.h>
   #include <sys/types.h>
   
   
   int getword(char *w) {
      int ch;
      int ionext;
      int size = 0;
      int istild = 0;
      int subLen = 0;
      char *homepath;
      int multiplier = 1;
      int increment = 1;
      int tilda = 0;
      int iterate = 0;
      homepath = getenv("HOME");
      subLen = strlen(homepath);
      struct passwd *pw;
      char tf_file[255];
      char *t_ret;
      
   
      /*
      * Initial check of newline, EOF, or # (special characters)
      */
   
      ch = getchar();
   
      /* Skip leading white-space to get to a valid character*/
      while (ch == ' ')
              ch = getchar();
   
      /* Put null character in array to be able to print empty string. Return */
      if (ch == '\n') {
              *w = '\0';
              return 101;
      }
   
      /* Put null character in array to be able to print empty string. Return */
      if (ch == EOF) {
              *w = '\0';
              return 0;
      }
      
   
      /* Check for metacharacters to  return -1 or -2  for each character*/
      if (ch == '<' || ch == '>' || ch == '|' || ch == '&') {
              if (ch == '<')
              {
                      *w = ch;
                      w++;
                      ch = getchar();
                      *w = '\0';
                      if (ch == '<')
                      {
                              return 300;
                      }
                      ch = ungetc(ch, stdin);
                      return 510;
   
              }
   
              /* Specifically check for |& case */
              if (ch == '|') {
                      *w = ch;
                      w++;
                      ch = getchar();
                      if (ch == '&') {
                              *w = ch;
                              w++;
                              *w = '\0';
                              return 500;
                      }
                      else {
                              ch = ungetc(ch, stdin);
                             *w = '\0';
                              return 510;
                      }
              }
              *w = ch;
              w++;
              *w = '\0';
              return 510;
      }
      if(ch == '$')
      {
              multiplier *= -1;
              ch = getchar();
      }
      /*
     * Process any other characters to be stored in array
     */
     
     while (1) {
             if(ch == '~')
             {
                     ch = getchar();
                     if(ch == '\n' || ch == '<' || ch == '>' || ch == '|' || ch == '&' || ch == EOF || ch == '/' || ch == ' ')
                     {
                             char *char_home;
                             for(char_home = homepath; *char_home != '\0'; char_home++)
                             {
                                     size += increment;
                                     *w = *char_home;
                                     w++;
                             }
                             ch = ungetc(ch,stdin);                  
                     }
                     else
                     {
                             int k = 0;
                              while(ch != '\n' && ch != '<' && ch != '>' && ch != '|' && ch != '&' && ch != EOF && ch != '/' && ch != ' ')
                             {
                                     tf_file[k] = ch;
                                     ch = getchar();
                                     k++;
                             }
                             tf_file[k] = '\0';
                             ch = ungetc(ch,stdin);
                             pw = getpwnam(tf_file);
                             if(pw == NULL)
                             {
                                    fprintf(stderr,"Couldn't find user");
                                    fflush(stdout);
                                     fflush(stderr);
                            }
                            else{
                             t_ret = pw->pw_dir;
                             int t_size = strlen(t_ret);
                             k = 0;
                             while(k < t_size)
                             {
                                     *w = t_ret[k++];
                                     w++;
                                     size += increment;
                             }
                     }
                             
                     }
                     ch = getchar();
                     continue;
             }
 
             /* Check to see if word exceeds STORAGE (255) */
             if (size == STORAGE - 1) {
                    ch = ungetc(ch, stdin);
                     *w = '\0';
                     return size * multiplier;
             }
  
            /* Check for EOF and space to return word size before it */
            if (ch == EOF || ch == ' ') {
                             *w = '\0';
                             return size * multiplier;       
             }
  
             /* Check for metacharacters to return word size before it */
             if (ch == '\n' || ch == '<' || ch == '>' || ch == '|' || ch == '&') {
  
                     /* Go one character back in stdin in order to process on next call */
                     ch = ungetc(ch, stdin);
                     *w = '\0';
                     return size * multiplier;
             }
  
             /* Checks backslash character, skips to process next character */
             if (ch == '\\') {
                     ch = getchar();
 
                     /* Stops if newline of EOF is found */
                     if (ch == '\n' || ch == EOF) {
                             ch = ungetc(ch, stdin);
                             *w = '\0';
                             return size * multiplier;
                     }
             }
  
             *w = ch;
             w++;
             size++;
             ch = getchar();
     }
  }
