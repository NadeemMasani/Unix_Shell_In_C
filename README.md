# Unix_Shell_In_C
main function implements a interactive unix shell which prompts user to enter commands to be executed
It handles metacharacters ">","<",">&" and "&" like normal unix shells.
Any confusing syntaxt entered by user will give appropriate error to stderr and re-issue the prompt. It will keep running unitll EOF signal is found with no commands in stdin or the command "done" is present as the first command on stdin, if an input file is given as command line argument it will take commands from the file and execute them and terminate on encountering EOF
