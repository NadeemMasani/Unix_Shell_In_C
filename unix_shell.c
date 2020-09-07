/*Program2 created for CS570 class taken by Professor John Carroll at SDSU written by Nadeem Masani*/
/*Due date for Program 2 : 10/04/2019 or 10/09/2019 11 PM */
#include "p2.h"
/*Variables for big buffer and newargv*/
char big_buffer[INPUT];
char *newargv[100];
char prompt[]="%1% ";
int next_place=0;
int newargv_point=0;
/*Variables for input, output and error redirection*/
int output_redirect_count=0;
int input_redirect_count=0;
int output_error_redirect_count=0;
char input_file [255];
char output_file[255];
char output_error_file[255];
/*Flag for handling meta character "&" */
char background_process='F';
/*struct for handling !!, it will save previous command executed along with its redirection flags and file names*/
struct previous_command{
        char *previousnewargsv[100];
        char previous_output_redirect_flag;
        char previous_input_redirect_flag;
        char previous_output_error_redirect_flag;
        int  previous_output_redirect_count;
        int  previous_input_redirect_count;
        int  previous_output_error_redirect_count;
        char previous_input_file [255];
        char previous_output_file[255];
        char previous_output_error_file[255];
};
/*Signal handler function to handle SIGTERM signal sent from killpg by p2, it is just an empty routine to catch signal
 *  *  *  * so p2 does not kill itself while executing killpg to kill all running children in same process group*/
void sig_handler(int alrm){}
/*Parse fuction will keep calling getword routine to get inputcommands from stdin, It places all the words sent by getword in a big buffer and also places them in newargv buffer so they can be executed using execvp.
 *  *  * Meta characters (>,<,>& and &)  and their subsequent file names will not be added in newargv buffer but they will be used to set appropriate flags,counts for further use in main.
 *   *   * 1.parse returns -1 if EOF file is encountered and no commands are present in newargv to execute.
 *    *    * 2.parse returns 0 if new line is encountered and no commands are present in newargv buffer to execute or there is confusing syntax from stdin(No file name after redirection, multiple redirection characters) after sending appropriate error message to stderr.
 *     *     * 3.parse returns 1 if new line is encountered and there is command in newargv buffer to execute after setting up all required redirection flags
 *      *      * If the output redirect file or output and error redirect file is present it will issue error cannot overrite and return 0 to re-issue the prompt
 *       *       * If input file from which command in newargv has to take input is not present it will issue eroor and return 0 to re-issue the prompt*/
int parse(char *v){

                int getword_length,i;
        char file_error='F';
                char output_redirect_flag='F';
                char input_redirect_flag='F';
                char output_error_redirect_flag='F';
        for (;;) {
                getword_length = getword(v);
                if (getword_length > 0){
                         if (strcmp(v,">")== 0){ /* ">" meta character returned from getword so set the flag and increase count and continue..(">" is not added in big buffer or newargv)*/
                                output_redirect_count++;
                                output_redirect_flag='T';
                                continue;
                         }
                         if (strcmp(v,"<")== 0){ /* "<" meta character returned from getword so set the flag and increase count and continue..("<" is not added in big buffer or newargv)*/
                                input_redirect_count++;
                                input_redirect_flag='T';
                                continue;
                        }
                        if (strcmp(v,">&") == 0){ /* ">&" meta character returned from getword so set the flag and increase count and continue..( ">&" is not added in big buffer or newargv)*/
                                output_error_redirect_flag='T';
                                output_error_redirect_count++;
                                continue;
                        }
                         if (output_redirect_flag == 'T' && output_redirect_count == 1 && strcmp(v,"&") != 0){ /*store output file name which comes after metacharacter ">",ignore if word is "&" in this case output file will be the word after "&" in stdin and if "&" is the last word the filname will be null*/
                                for ( i=0; i< (int)strlen(v); i++){output_file[i]=v[i];}
                                output_file[i]='\0';
                                output_redirect_flag='F';
                                        if(output_file[0] !='\0' && access( output_file, F_OK ) == 0){ /* if output file is already present give error to stderr and set error flag so parse returns 0 to re-issue the prompt*/
                                                  fprintf(stderr,"%s :File exists cannot override\n",output_file);
                                              file_error='T';
                                                }
                                continue;
                        }
                        if (input_redirect_flag == 'T' && input_redirect_count == 1 && strcmp(v,"&") != 0){ /*store input file name which comes after metacharacter "<",ignore if word is "&" in this case file name will be the word after "&"in stdin and if "&" is the last word the filname will be null*/
                                for ( i=0; i< (int)strlen(v); i++){
                                        input_file[i]=v[i];
                                }
                                input_file[i]='\0';
                                input_redirect_flag='F';
                                                if( input_file[0] !='\0' && access( input_file, F_OK ) != 0 ){  /* if input file is not present give error to stderr and set file_error flag so parse returns 0 to re-issue the prompt*/
                                            fprintf(stderr,"%s :File does not exist\n",input_file);
                                            file_error='T';
                            }
                                continue;
                        }
                        if (output_error_redirect_flag == 'T' && output_error_redirect_count == 1 && strcmp(v,"&") != 0){ /*store file name in which output and error will be redirected,the word which comes after metacharacter ">&",ignore if word is "&" in this case  file name will be the word after "&" in stdin and if "&" is the last word the filname will be null*/
                                for ( i=0; i< (int)strlen(v); i++){output_error_file[i]=v[i];}
                            output_error_file[i]='\0';
                            output_error_redirect_flag='F';
                                                   if( output_error_file[0] !='\0' && access( output_error_file, F_OK ) != -1 ) { /* if output file is already present give error cannot overwrite and set file_error so parse returns 0 to re-issue prompt*/
                                           fprintf(stderr,"%s :File exists cannot override\n",output_error_file);
                                                   file_error='T';
                                    }
                            continue;
                        }
                        /*store words from getword other than meta charatcers in big buffer and point start of every word in big buffer to the newargv pointer array*/
                        newargv[newargv_point]=big_buffer+next_place;
                        for (i =0; i < getword_length; i++){
                                big_buffer[next_place]=v[i];
                                next_place++;
                        }
                        big_buffer[next_place]='\0';
                        next_place++;
                        newargv_point++;
                }
        else if ( getword_length == 0 && newargv_point !=0 && file_error == 'T'  ) { /* if file_error flag is set(output file already exists or input file does not exist) when new line is encountered with commands in the newargv reset the flag and newargv for next command and return 0 to re-issue the promt*/
            file_error='F';
            newargv_point=0;
            *newargv= NULL;
            return 0;
        }
                else if (getword_length == 0 && newargv_point != 0 ){  /*new line returned from getword and commands are present in newargv so end newargv and return 1, if found counfusing syntax return 0,also check for "&" to set up for background process*/
                                if (strcmp(newargv[newargv_point-1],"&") == 0){ /*check if the process needs to be run in background by checking the last command in newargv and set flag if found*/
                                        background_process='T';
                                        newargv[newargv_point-1]='\0';   /*remove the "&" meta-character from newargv buffer*/
                                }
                               /*check for confusing syntax in newargv (multiple metacharacters of same type,no redirection file after the redirection metacharacters), if found send appropriate errors to stderr and reset all flags and return 0 to re-issue the prompt*/
                                if (input_redirect_count > 1 || output_redirect_count > 1 || output_error_redirect_count > 1){
                                        fprintf (stderr,"Ambiguous redirection\n");
                                        input_redirect_flag='F';
                                        output_redirect_flag='F';
                                        output_error_redirect_flag='F';
                                        input_redirect_count=0;
                                        output_redirect_count=0;
                                        output_error_redirect_count=0;
                                        input_file[0]='\0';
                                        output_file[0]='\0';
                                        output_error_file[0]='\0';
                                        background_process='F';
                                        return 0;
                                }
                                if (input_redirect_count == 1 && input_file[0] == '\0'){
                                        fprintf(stderr,"No input file Name found after < redirect\n");
                                        input_redirect_flag='F';
                                        input_redirect_count=0;
                                        background_process='F';
                                        return 0;
                                }
                                if (output_redirect_count == 1 && output_file[0] == '\0'){
                                        fprintf(stderr,"No outputfile file Name found after > redirect\n");
                                        output_redirect_flag='F';
                                        output_redirect_count=0;
                                        background_process='F';
                                        return 0;
                                }
                                if (output_error_redirect_count == 1 && output_error_file[0] == '\0'){
                                        fprintf(stderr,"No outputfile and error file Name found after >& redirect ");
                                        output_error_redirect_flag='F';
                                        output_error_redirect_count=0;
                                        background_process='F';
                                        return 0;
                                }
                        /*end newargv buffer with null string,re-set newargv_point for next command and return 1*/
                        newargv[newargv_point]='\0';
                        newargv_point=0;
                        return 1;
                }
                else if (getword_length == -1 && strcmp(v,"done") == 0 && newargv_point > 0 ){ /*when getword returns done and it is not the first word,add it in the big buffer and newargv buffer*/
                        newargv[newargv_point]=big_buffer+next_place;
                        for (i =0; i < (int)strlen(v); i++){
                                        big_buffer[next_place]=v[i];
                                        next_place++;
                        }
                        big_buffer[next_place]='\0';
                        newargv_point++;
                        next_place++;
                        newargv[newargv_point]='\0';
                        continue;
                }
                else if (getword_length == -1 && newargv_point > 0){ /*premature EOF signal encountered when word is present in newargv buffer, add word in newargv, terminate newargv with '\0' and return 1 for execvp*/
                        newargv[newargv_point]='\0';
                        newargv_point=0;
                        return 1;
                }
                else if (getword_length == -1 && newargv_point==0) return -1; /*EOF signal with no words in buffer or word "done" encountered at start of buffer, return -1 to terminate p2*/
                else{  /* encountered new line while no words in newargv buffer check for confusing sytax(send error to stderr if found) and return 0 to reissie prompt*/
                if((input_redirect_count == 1 || output_redirect_count == 1 || output_error_redirect_count == 1) && newargv[0] == '\0'){ /*no command found but redirection metacharacter found, send error to stderr and return 0 after resetting redirection flags for next input*/
                     fprintf(stderr,"No command found after redirection\n");
                         input_redirect_flag='F';
                         output_redirect_flag='F';
                         output_error_redirect_flag='F';
                         background_process='F';
                         input_redirect_count=0;
                                                 output_redirect_count=0;
                                                 output_error_redirect_count=0;
                                                 input_file[0]='\0';
                         output_file[0]='\0';
                         output_error_file[0]='\0';
                        }
                        return 0;
                }
        }
}
/*p2 main function implements a interactive unix shell which prompts user to enter commands to be executed. It handles metacharacters ">","<",">&" and "&" like normal unix shells.
 *  *  *  * Any confusing syntaxt entered by user will give appropriate error to stderr and re-issue the prompt. It will keep running unitll EOF signal is found with no commands in stdin or the command "done"
 *   *   *   * is present as the first command on stdin, if an input file is given as command line argument it will take commands from the file and execute them and terminate on encountering EOF*/
int main(int argc, char *argv[]){
        int newargv_status; /*return status of parse fuction*/
        char s[STORAGE];
        pid_t child;
        pid_t ci;
        int flags,y;
                /*Redirection File descriptors*/
        int output_file_descriptor;
        int input_file_descriptor;
        int argv_file_descriptor;
        int output_error_file_descriptor;
        int dev_null_file_descriptor;
        /*pointer to $HOME env variable*/
                char *home;
        struct previous_command p1 ={0};
        flags = O_CREAT | O_WRONLY;
        (void)signal(SIGTERM,sig_handler); /*signal catcher to catch SIGTERM signal sent by killpg() so p2 does not kill itself*/
        if (setpgid(0,0) != 0) perror("setpgid() error"); /*set pid of parent as process group id of parent and all its children*/
/*If command line arguments are found redirect sdtdin to filename in the argv so parse takes input from the file instead of stdin*/
        if ( argc > 1 ){
                argv_file_descriptor = open(argv[1], O_RDONLY );
                if( argv_file_descriptor == -1 ){
                        perror( "Trouble opening file newargv file \n" );
                        exit(3);
                }
                if((dup2(argv_file_descriptor,STDIN_FILENO)) == -1){
                        perror( "Trouble with dup2 to newargv file \n" );
                        close( argv_file_descriptor );
                        exit(2);
                }
                else close( argv_file_descriptor );
        }
        for(;;){
                                (void)printf("%s",prompt);
                newargv_status = parse(s);
                /*EOF file returned with no commands in the newargv buffer or done as first word of buffer so break out of for loop to terminate p2*/
                if (newargv_status == -1) break;
                else if (newargv_status == 0){ /*Empty line or some confusing syntax in the buffer so reset newargv and re-issue prompt(appropriate error is sent to stderr for confusing syntax in parse function)*/
                        newargv_point=0;
                        *newargv=NULL;
                        continue;
                }
                else{  /*parse sent a command in newargv buffer so handle !!,builtins(cd) first then fork child to execute command in newargv*/
                                                if (strcmp(newargv[0],"!!") == 0){ /* !! found at begining of current newargv so replace it with newargv of previous command and set redirection flags from previous command*/
                                for (y=0;p1.previousnewargsv[y] != '\0';y++){
                                        newargv[y]=p1.previousnewargsv[y];
                                }
                                newargv[y]='\0';
                                output_redirect_count=p1.previous_output_redirect_count;
                                input_redirect_count=p1.previous_input_redirect_count;
                                output_error_redirect_count=p1.previous_output_error_redirect_count;
                                strcpy(input_file,p1.previous_input_file);
                                strcpy(output_file,p1.previous_output_file);
                                strcpy(output_error_file,p1.previous_output_error_file);
                          }
                        else{ /* !! not found at begining of current newargv, copy the current command and redirection flags in previous command struct*/
                                  for (y=0;newargv[y] != '\0';y++){
                                        p1.previousnewargsv[y]=newargv[y];
                                  }
                                        p1.previousnewargsv[y]='\0';
                                        p1.previous_output_redirect_count=output_redirect_count;
                                        p1.previous_input_redirect_count=input_redirect_count;
                                        p1.previous_output_error_redirect_count=output_error_redirect_count;
                                        strcpy(p1.previous_input_file,input_file);
                                        strcpy(p1.previous_output_file,output_file);
                                        strcpy(p1.previous_output_error_file,output_error_file);
                        }
                        if (strcmp(newargv[0],"cd") == 0){ /*handle cd command as built-in*/
                                                                if ( newargv[2] == '\0'){
                                                                                if ( newargv[1] != '\0'){
                                                if ( (chdir(newargv[1])) != 0 )
                                                        perror("chdir() to failed");
                                                if (background_process == 'T'){background_process='F';}
                                                continue;
                                        }
                                        else{ /*cd command does not have any arguments in newargv so cd to $HOME dir*/
                                                home=getenv("HOME");
                                                if ( (chdir(home)) != 0 )
                                                        perror("chdir() to failed");
                                                if (background_process == 'T'){background_process='F';}
                                                continue;
                                        }
                                }
                        else {  /*give error to stderr if multiple arguments given to cd and reset newargv buffer and re-issue prompt by continue*/
                                        fprintf(stderr,"Multiple arguments to cd found ");
                                        newargv_point=0;
                                        *newargv = NULL;
                                        continue;
                                }
                        }
                                                /* command in newargv is not a built-in or !! fork a child to execute newargv*/
                        if ((child = fork()) == 0){ /*Inside child*/
                                        if (output_redirect_count > 0){ /*output redirection count is set so replace stdout of child with output_file*/

                                                if((output_file_descriptor=open(output_file,flags, S_IRUSR | S_IWUSR)) < 0){ /*open output file once confirmed it does not already exist*/
                                                        perror("Error while opening output file\n");
                                                        exit(3); /*child will exit with code 3 if file open fails*/
                                                }
                                                else{ /*replace stdout with the opened output file using dup2*/
                                                    if((dup2(output_file_descriptor,STDOUT_FILENO)) == -1){
                                                          perror( "Trouble with dup2 to stdin\n");
                                                          close(output_file_descriptor);
                                                          exit(2);/*child will exit with code 2 if dup2 fails*/
                                                        }
                                                        else close(output_file_descriptor);
                                                }
                                        }
                                        if (input_redirect_count > 0){/*input redirection count is set so replace stdin of child with input file*/

                                            input_file_descriptor = open(input_file, O_RDONLY );/*open input file once confirmed it exists*/
                                                if(input_file_descriptor == -1){
                                                    perror( "Trouble opening file \n" );
                                                    exit(3);/*child will exit with code 3 if file open fails*/
                                                }
                                                if((dup2(input_file_descriptor,STDIN_FILENO)) == -1){
                                                        perror( "Trouble with dup2 to stdin\n" );
                                                        close( input_file_descriptor );
                                                        exit(2);/*child will exit with code 2 if dup2 fails*/
                                                }
                                                else close(input_file_descriptor);
                                        }
                                        else{ /*input redirection count is not set so replace stdin of child with /dev/null so it does not compete with parent for input from keyboard*/
                                                dev_null_file_descriptor = open("/dev/null", O_RDONLY);
                                                if( dev_null_file_descriptor == -1 ){
                                                    perror( "Trouble opening file /dev/null \n");
                                                    exit(3);/*child will exit with code 3 if file open /dev/null fails*/
                                                }
                                                if((dup2(dev_null_file_descriptor, STDIN_FILENO)) == -1){
                                                        perror( "Trouble with dup2 to /dev/null\n" );
                                                        close(dev_null_file_descriptor);
                                                        exit(2);/*child will exit with code 2 if dup2 fails*/
                                                }
                                                else close(dev_null_file_descriptor);
                                        }
                                        if (output_error_redirect_count > 0){ /*output and stderr redirection count is set so replace stdout and stderr of child with output file*/
                                                if((output_error_file_descriptor=open(output_error_file,flags, S_IRUSR | S_IWUSR)) < 0){
                                                        perror("Error while creating file\n");
                                                        exit(3);/*child will exit with code 3 if file open /dev/null fails*/
                                                }
                                                if((dup2(output_error_file_descriptor,STDERR_FILENO)) == -1){
                                                        perror( "Trouble with dup2 to stderr \n" );
                                                        close(output_error_file_descriptor);
                                                        exit(2);/*child will exit with code 2 if dup2 fails*/
                                                }
                                                if( (dup2(output_error_file_descriptor,STDOUT_FILENO)) == -1)
                                                {
                                                        perror( "Trouble with dup2 to stdout \n" );
                                                        close(output_error_file_descriptor);
                                                        exit(2);/*child will exit with code 2 if dup2 fails*/
                                                }
                                                else close(output_error_file_descriptor);
                                        }
                                        /*execute the command in newargv using execvp*/
                                        execvp(newargv[0], newargv);
                                        perror("Child process could not do execvp.\n");/* child will reach here only if execvp fails,send error to stdree and exit child with code 1*/
                                        exit(1);
                                }
                                else{/*Inside Parent*/
                                        if (child == (pid_t)(-1)){ /*if fork by parent fails, send error to stderr and exit parent with code 1*/
                                                fprintf(stderr, "Fork failed.\n");
                                                exit(1);
                                        }
                                        else{ /*check if parent needs to wait for child to complete or not*/
                                                if (background_process != 'T'){ /* & not present at the end of command so wait for child to complete*/
                                                while ((ci = wait(NULL)) > 0);
                                                /*child completed so reset all flags for next command*/
                                                        input_redirect_count=0;
                                                        output_redirect_count=0;
                                                        output_error_redirect_count=0;
                                                        input_file[0]='\0';
                                                        output_file[0]='\0';
                                                        output_error_file[0]='\0';
                                                        newargv_point=0;
                                                        continue;
                                                }
                                                else{  /* &  present at the end of command so do not wait for child to complete, just print the process name and pid and reset all flags for next command and continue*/
                                                        printf("%s ",newargv[0]);
                                                        printf("[%d] \n",child);
                                                        input_redirect_count=0;
                                                        output_redirect_count=0;
                                                        output_error_redirect_count=0;
                                                        input_file[0]='\0';
                                                        output_file[0]='\0';
                                                        output_error_file[0]='\0';
                                                        background_process='F';
                                                        newargv_point=0;
                                                        continue;
                                                }
                                        }
                                }
                }
        }
        /*P2 is completed, all background child processes in the same process group are terminated by sending kill pg singal to their process group so we do not have zombies*/
        killpg(getpgrp(), SIGTERM);
        printf("p2 terminated.\n");
        exit(0);
}