
This software is a terminal (shell/bash) simulator with linux command

support (without folder control) "|" , "<" , ">" , ">>"
and "2>" argumenet support.

There can be up to 2 processes running per client (one for command, one for shell/bash CLI).
there is a set limit of an input up to 511 characters and a limit of hostname display of 511 characters

how to install:
open linux terminal, navigate to the folder containing ex2.c
using the "cd" command (confirm it by using ls command)
in that case, we have a makefile, so just type make (make sure you are in the folder where the makefile is)
and the program will automaticily be compiled
case you don't want to you the makefile, type gcc -Wall ex2.c -o ex2
and your program will automaticily be compiled

to activate:
open linux terminal, navigate to ex2 executeable file
location using "cd" command (confirm it using ls command) and type
./ex2

to operate:
once you are inside ex2 type any command you want

*incase a command execution was successfull, return code will be
updated

*incase the command "done" was sent, the terminal simulator will
print the wanted info and exit the program.

*incase of empty command input(space enter, enter...) or any
failure to fetch input from user,
last return code will be retained and a new line will be made for
the simulator command

*the program has 6 static variables - cmdLength, numOfCmd, flag_ampersand, left_array_size, right_array_size, pipe_index, redirection_index 


------------------ functions -------------------------------

void printShell():
input: none
output: prints the command line (the directory).


char** make_argv_array(int wordCounter, char input[]):
input: int - the number of words, char[] - user input
output: a dynamic memory allocated string array of the input
string divided into words, last element is NULL


int count_cmd_words(char input_copy[]):
input: char[] - user input
output: integer describing how many words (words meaning paragraphs
divided by ' ' and '\n') there are in a string


void childProcess(char **argv, int wordsCounter ,char **left_array , char **right_array):
input: char** - the array of words (argv), int- the num of words, char** - left array, char** - right array 
output: executes the command


void fatherProcess(char **argv, char **left_array, char **right_array):
input: char** - the array of words (argv), char** - the left array, char** the right array
output: executes the command


void check_spaces(char input[]):
input: char[] - user input, int* - num of cmd (by address)
output: checks if the command is actually 'space/s'


void sig_handler(int signum):
input: int - signal number
output: handles child signal / interrupt signal


int check_pipe(char **argv, int wordsCounter, char *pipe):
input: char** - the array of words, int - num of words, char* -the sign "|"
output: if the sign "|" exists in the array then it returns it's index in the argv array, else, returns -1


char **make_left_array_argv(char **argv, int wordsCounter, int sign_index):
input: char** - the array of words, int - num of words, int - index that decides until where to seperate the input array
output: the left array of words until the index


char **make_right_array_argv(char **argv, int wordsCounter, int sign_index):
input: char** - the array of words, int - num of words, int - index that decides until where to seperate the array
output: the right array of words until the index


void func_pipe(char **argv, int wordsCounter, int pipe_index,char **left_array, char **right_array):
input: char** - the array of words, int - num of words, int - the pipe index, char**- the left array from the pipe, char**- the right array from thr pipe
output: executes pipe command


void func_redirect_in_out_err(char **argv, int wordsCounter, int redirect_sign_index, char **left_array, char **right_array):
input: char** - the array of words, int -num of words, int - the redirection index
output: executes redirection command


int is_redirection( char *ptrInput):
input: char* - one of the signs "<" , ">" , ">>"
output: if one of the signs exists in the array then it returns it's index in the argv array, else, returns -1 

void done_print():
input: none
output: prints cmd length and num of cmd

void free_array(char** array, int length):
input: char** - array of words, int - size
output: frees the words allocated in the memory and the array itself


int main() command:
input: standart main input
output: the shell CLI simulatorProgram has been fully tested with valgrind to test for memory
leak, no leak was found.
