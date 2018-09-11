/*
*author: Saar Weitzman
*ID: 204175137
*Description: This program is an extension of ex1 (simulate shell)
*In this program we also supporting SIGINT (ctrl+c) ,background (& sign), pipe and rediections
*/

#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <pwd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>


#define MAX_LENGTH 512
#define END_PROGRAM "done\n"
#define ERR "ERR\n" 


//functions declerations:
void printShell();
int count_cmd_words(char input_copy[]);
char** make_argv_array(int wordsCounter ,char input[]);
void childProcess(char **argv, int wordsCounter ,char **left_array , char **right_array);
void fatherProcess(char **argv, char **left_array, char **right_array);
void check_spaces(char input[]);
void done_print();

void sig_handler(int signum);
int check_pipe(char **argv, int wordsCounter, char *pipe);
char **make_left_array_argv(char **argv, int wordsCounter, int sign_index);
char **make_right_array_argv(char **argv, int wordsCounter, int sign_index);
void func_pipe(char **argv, int wordsCounter, int pipe_index,char **left_array, char **right_array);
void free_array(char **array, int length);
void func_redirect_in_out_err(char **argv, int wordsCounter, int redirect_sign_index, char **left_array, char **right_array);
int is_redirection( char *ptrInput);



//static varaiables: 
static int flag_ampersand;      // to check if there is & in the command
static int numOfCmd = 0;
static int cmdLength = 0;
static int left_array_size;
static int right_array_size;
static int pipe_index;
static int redirection_index;


int main()
{
    char input[MAX_LENGTH];
	 
    do
    {
        signal (SIGINT,sig_handler);   //when you get SIGINT, go to sig_handler function (to catch the ctrl+c)
        signal (SIGCHLD,SIG_DFL);
        
        pipe_index = -1;               // pipe char flag and the place
    	redirection_index = -1;       // redirection char flag and place
        flag_ampersand = 0;		      //ampersand flag to know if need to do exec on background
        
        char **left_array = NULL;
        char **right_array = NULL;
        left_array_size = 0;
        right_array_size = 0;
        
        
		printShell();                       //print path
        fgets(input, MAX_LENGTH, stdin);    //get the input of the user

        if (strcmp(input, END_PROGRAM) != 0)
        {
            char input_copy[MAX_LENGTH];
            strcpy(input_copy, input);

            int wordsCounter = count_cmd_words(input_copy);

            if(wordsCounter != 0)    //there is a command in the input
            {
                char** argv = make_argv_array(wordsCounter, input);
                
                if (strcmp(argv[0], "cd") == 0)
  					{
       		 			int ret;
       		 			if (argv[1] != NULL)
	        				ret = chdir(argv[1]);         //changes the directory (cd command)
        				free_array(argv, wordsCounter+1);
        				continue;				     //go straight to the condition of the do-while loop
    				}
                
				pipe_index  = check_pipe(argv,wordsCounter + 1,"|");
				
				
                pid_t d;
                d = fork();

                if(d < 0)    // fork() didnt work
                {
                    printf(ERR);
                    exit(0);
                }
                if (d == 0)   //in child process
                    childProcess(argv, wordsCounter, left_array, right_array);

                else       // in father process
                {
                	
                    fatherProcess(argv, left_array, right_array);
                    free_array(argv , wordsCounter+1);   
                }
            }
            else
                check_spaces(input);
        }
      
    }
    while (strcmp(input, END_PROGRAM) != 0);
    done_print();
    return 0;
}


/*prints the command line*/
void printShell()
{
    char path[MAX_LENGTH];
    printf("%s@%s>", getpwuid(0)->pw_name,getcwd(path, MAX_LENGTH)); // if you want to get the real id put getpwuid(geteuid());
}


/* counts how many words we have in the command we get from the shell*/
int count_cmd_words(char input_copy[])   
{

    int wordsCounter = 0;
    char* ptrInput;


    ptrInput = strtok (input_copy," \n");
    while (ptrInput != NULL)        // count the number of words in the input
    {
        if (strcmp(ptrInput,"&") == 0 && strtok (NULL, " \n") == NULL)
        {
            wordsCounter--;         //we don't want to count the "&" sign as a word in the array
            flag_ampersand = 1;		//there is an ampersand in the last place of the input
        }
        wordsCounter++;
        ptrInput = strtok (NULL, " \n");
    }
    return wordsCounter;
}


/*makes the argv array from the input we get from the user*/
char **make_argv_array(int wordsCounter ,char input[])
{
    char **argv = (char**)malloc((wordsCounter + 1) * sizeof(char*)); // make the argv array
    if (argv == NULL)
    {
        printf(ERR);
        exit(1);
    }

    int i = 0;
    char* ptrInput;
    ptrInput = strtok (input," \n");        // skip only when there is " " or \n in the input

    while (i < wordsCounter)
    {
        argv[i] = (char*)malloc(strlen(ptrInput) + 1 * sizeof(char));   //put the strings of the command in the argv array's indexes
        if(argv[i] == NULL)
        {
            printf(ERR);
            exit(1);
        }
        strcpy(argv[i], ptrInput);
        if (is_redirection(ptrInput) == 1)
        	redirection_index = i;                 // save the place of redirection_index in the argv array we make
        i++;
        ptrInput = strtok (NULL, " \n");
    }
    argv[i] = NULL;       //put NULL in the last index of the argv array
    return argv;
}


/*executes the child process*/
void childProcess(char **argv, int wordsCounter, char **left_array, char **right_array)    
{
    signal (SIGINT, sig_handler);

	if ( redirection_index != -1 )
	{
		left_array_size = redirection_index + 1;
    	right_array_size = wordsCounter - redirection_index;
		left_array =  make_left_array_argv(argv, wordsCounter, redirection_index);	
		right_array = make_right_array_argv(argv, wordsCounter, redirection_index);
    	 
		func_redirect_in_out_err(argv, wordsCounter, redirection_index, left_array, right_array);
	}
	
    if (pipe_index != -1 && redirection_index == -1)	//only pipe without redirection in the command
    {
    	left_array_size = pipe_index + 1;
    	right_array_size = wordsCounter - pipe_index;
    	
    	left_array =  make_left_array_argv(argv, wordsCounter, pipe_index);
		right_array = make_right_array_argv(argv, wordsCounter, pipe_index);
		
        func_pipe(argv, wordsCounter, pipe_index, left_array, right_array);
	}
	
    if (redirection_index == -1 && pipe_index == -1) // there are no special commands in the input
    {
        int checkExec;
        checkExec = execvp(argv[0],argv);         //executes the command (besides cd)
	}
    exit(0);
}


/*executes the father process*/
void fatherProcess(char **argv, char **left_array, char **right_array)  
{
        cmdLength += strlen(argv[0]); // to count the string's length of the cmd
        numOfCmd++;                   //to count the command
    
    if (pipe_index != -1 || redirection_index != -1)
    {
    	free_array(left_array , left_array_size);
    	free_array(right_array , right_array_size);
   	}
   	
   	 if ( flag_ampersand == 0 )
		wait(0);
}


/*check case that the input cmd is space/s*/
void check_spaces(char input[])       
{
    if (strcmp(input, "\n") != 0)     //the input is spaces
    {
        numOfCmd++;         //we count spaces ,but don't count enters
    }
}


/*finish the program*/
void done_print()
{
    printf("Num of cmd: %d\nCmd length: %d\nBye !\n",numOfCmd,cmdLength);
    exit(0);
}


/*The new SIGINT function when we get ctrl+c*/
void sig_handler(int signum)
{
    if( signum == SIGINT )
        signal(SIGCHLD, sig_handler);

    if( signum == SIGCHLD )
    {
		int status;
		waitpid(-1, &status, WNOHANG);
    }
}


/*check if there is pipe in the argv array*/
int check_pipe(char **argv, int wordsCounter, char *pipe)
{
    for ( int i = 0; i < wordsCounter-1 ; i ++)
        if (strcmp(argv[i], pipe) == 0 && i != 0 )
            return i;              // the place of the pipe left_array | right_array
    return -1;	               //there is no pipe
}


/*makes the left array from the argv array we get as input to the function*/
char **make_left_array_argv(char **argv, int wordsCounter ,int sign_index)
{
    char** left_array = (char**)malloc((sign_index + 1) * sizeof(char*));
    if (left_array == NULL)
    {
        printf(ERR);
        exit(1);
    }

    int i = 0;                          //make left_array
    while (i < sign_index)
    {
        left_array[i] = (char*)malloc(strlen(argv[i]) +1* sizeof(char));   // put the commands in the arr
        if(left_array[i] == NULL)
        {
            printf(ERR);
            exit(1);
        }
        strcpy(left_array[i], argv[i]);
        i++;
    }
    left_array[i] = NULL;  // the last index
    return left_array;
}


/*makes the right array from the argv array we get as input to the function*/
char** make_right_array_argv(char** argv,int wordsCounter,int sign_index)
{
    char** right_array = (char**)malloc((wordsCounter - sign_index) * sizeof(char*));
    if (argv == NULL)
    {
        printf(ERR);
        exit(1);
    }

    int j = 0;
    int i = sign_index + 1;
    while (j < wordsCounter - sign_index - 1 )
    {
        right_array[j] = (char*)malloc(strlen(argv[i])+1 * sizeof(char));
        if(right_array[j] == NULL)
        {
            printf(ERR);
            exit(1);
        }
        strcpy(right_array[j], argv[i]);
        j++;
        i++;
    }
    right_array[j] = NULL;  // the last index
    return right_array;
}


/*executes the pipe in the command we get as input*/
void func_pipe(char **argv, int wordsCounter, int pipe_index, char **left_array, char **right_array)
{
    pid_t d1;

    int pipe_fd[2];

    if ((pipe(pipe_fd)) == -1)
    {
        perror(ERR);
        exit(0);
    }

    d1 = fork();

    if(d1 < 0)
    {
        perror(ERR);
        exit(0);
    }

    if(d1 == 0)
    {
        close(pipe_fd[0]);
        int value = dup2(pipe_fd[1],STDOUT_FILENO);
        close(pipe_fd[1]);
        if(value == -1)
        {
			perror(ERR);
			exit(0);
        }
        int checkExec;
        checkExec = execvp(left_array[0],left_array);    // executes the command
        if (checkExec == -1 )
        {
            printf(ERR);
            exit(1);
        }
        exit(0);
    }

    else
    {
        wait(0);
        close(pipe_fd[1]);
        int value = dup2(pipe_fd[0],STDIN_FILENO);
        close(pipe_fd[0]);

        if(value == -1)
        {
            printf(ERR);
            exit(1);
        }

        int checkExec;
        checkExec = execvp(right_array[0],right_array);    // executes the command
        if (checkExec == -1 )
        {
            printf(ERR);
            exit(1);
        }
    }
}


/*does free to the input array*/
void free_array(char **array, int length)
{

    for (int i = 0; i < length; i++)      // +1 NULL in the end
        free(array[i]);
    free(array);
}


/*redirects the input/output by the sign we get as input*/
void func_redirect_in_out_err(char **argv, int wordsCounter, int redirect_sign_index, char **left_array, char **right_array)
{
    int fd = -1;

    if ( strcmp(argv[redirect_sign_index],">>") == 0 )
    {
        fd = open(right_array[0], O_CREAT | O_APPEND | O_WRONLY, S_IRWXU | S_IRWXG |S_IRWXO);
        dup2(fd, STDOUT_FILENO);
        close(fd);
    }
    
    else if ( strcmp(argv[redirect_sign_index],">") == 0 )
    {
        fd = open(right_array[0], O_CREAT | O_TRUNC | O_WRONLY, S_IRWXU | S_IRWXG |S_IRWXO);
        dup2(fd, STDOUT_FILENO);
        close(fd);
    }
    
    else if ( strcmp(argv[redirect_sign_index],"2>") == 0 )
    {
        fd = open(right_array[0], O_WRONLY|O_CREAT|O_APPEND, S_IRWXU);
        dup2(fd, STDERR_FILENO);
        close(fd);
    }

    else              // the char "<"
    {
        fd = open(right_array[0], O_RDONLY, 0);
        dup2(fd, STDIN_FILENO);
        close(fd) ;
    }

    if( pipe_index != -1 )
    	{
    	char **temp_left_array =  make_left_array_argv(left_array, redirect_sign_index, pipe_index);      // replace the left_array for doing the pipe func
    	char **temp_right_array = make_right_array_argv(left_array, redirect_sign_index, pipe_index); 	 // replace the right_array for doing the pipe func
    	
    	free_array(left_array, left_array_size);
    	free_array(right_array, right_array_size);
    	
    	left_array = temp_left_array;
    	right_array = temp_right_array;
    	
    	left_array_size = pipe_index + 1;
    	right_array_size = redirect_sign_index - pipe_index;
    	
        func_pipe(left_array, redirect_sign_index, pipe_index, left_array, right_array);
        }
        
    else
    {
        int checkExec;
        checkExec = execvp(left_array[0],left_array);       // executing the command
	}
    wait(0);
}


/*checks if there is a redirection sign in the input we get*/
int is_redirection(char *ptrInput)
{
	if ( strcmp(ptrInput,">>") == 0 )
		return 1;

	if ( strcmp(ptrInput,">") == 0 )
		return 1;
	
	if ( strcmp(ptrInput,"2>") == 0 )
		return 1;

	if ( strcmp(ptrInput,"<") == 0 )
		return 1;			
	return 0;
}
