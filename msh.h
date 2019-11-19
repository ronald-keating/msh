/*
    Ronald Keating
    CS240
    msh v1.1
    3-29-19
    *Update*
*/

/*-----LIBRARIES-----*/
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <stdbool.h>
#define MAX_SIZE 1024                               //max string size

/*-----FUNCTIONS-----*/
int makearg(char *s, char ***args);
void parseStr(char *str);
void runCommand(char **command);

/*-----STRUCTS AND GLOBALS-----*/
int hist_count = 0;                                 //history count
struct hist{                                        //history linked list
    int hist_num;                                   //history count
    char *hist_str;                                 //user input
    struct hist *next;
};
typedef struct hist hist;
hist *head = NULL;                                  //pointer to history list

struct alias{
    char *alias_name;                               //alias string
    char *alias_command;                            //alias command
    char *alias_line;                               //to print aliases
    struct alias *next;
};
typedef struct alias alias;             
alias *start = NULL;                                //alias list pointer

struct pipeCommands{                                //attempt at multiple pipes
    char **pipes;
    struct pipeCommands *next;
};
typedef struct pipeCommands cmd;
cmd *first = NULL;

char buffer[MAX_SIZE];                              //input buffer, executes !!
int pipeFlag = 0;                                   //if is pipe
char PATH[MAX_SIZE];                                //Path variable

/*-----SECOND IMPLEMENTATION FUNCTIONS-----*/
void open_mshrc()                                   //executes from file
{
    int argc;
    char **args;
    FILE* file = fopen("mshrc", "r");
    char line[MAX_SIZE];
    if(file == NULL)
    {
        printf("No file named mshrc\n");
        return;
    }
    while(fgets(line, sizeof(line), file))
    {
        strcpy(buffer, line);
        sleep(1);
        parseStr(line);
    }
    fclose(file);
}

void changeColor(char *str)                         //changes color
{
    int color = atoi(str);
    if(color > 37 || color < 30)
        printf("Not a color.\n");
    else
        printf("\033[%dm", color);
}

void addHistory(char *str)                          //add to history
{
    int i;
    for(i = 0; i < strlen(str) - (strlen(str) - 2); i++)
    {
        if(str[i] == '!')
            return;
    }
    strcpy(buffer, str);
    hist *tmp = malloc(sizeof(hist));
    hist_count++;
    tmp->hist_num = hist_count;
    tmp->hist_str = malloc(strlen(str));
    strcpy(tmp->hist_str, str);
    tmp->next = head;
    head = tmp;
}

void printHistory(hist *head, int count)            //print history
{
    if(head == NULL)
        return;
    else if(count == 20)
        return;
    count++;
    printHistory(head->next, count);
    printf("%d. %s", head->hist_num, head->hist_str);
}

void findHistory(int num)                           //find history, for !#
{
    hist *curr = head;
    if(num > hist_count || hist_count <= 0)
        printf("History does not exist\n");
    else{
        while(curr != NULL)
        {
            if(curr->hist_num == num)
            {
                char *str = malloc(sizeof(curr->hist_str));
                strcpy(str, curr->hist_str);
                addHistory(str);
                parseStr(str);
            }
            curr = curr->next;
        }
    }
}

void parseAlias(alias *tmp, char *s)                //parses the alias
{
    char buf[MAX_SIZE];
    int i = 0;
    while(*s == ' ')
        s++;
    while(*s != '=')
    {
        buf[i] = *s;
        i++;
        s++;
    }
    buf[i] = '\0';
    tmp->alias_name = malloc(strlen(buf));
    strcpy(tmp->alias_name, buf);
    buf[0] = '\0';
    i = 0;
    while(*s != '\"')
        s++;
    s++;
    while(*s != '\"')
    {
        buf[i] = *s;
        i++;
        s++;
    }
    buf[i] = '\0';
    tmp->alias_command = malloc(strlen(buf));
    strcpy(tmp->alias_command, buf);
}

void addAlias(char *s)                              //adds alias to list
{
    char *token = strtok(s, " ");
    token = strtok(NULL, "\n");
    char *str = token;
    alias *tmp = malloc(sizeof(alias));
    tmp->alias_line = malloc(strlen(str));
    strcpy(tmp->alias_line, str);
    parseAlias(tmp, str);
    tmp->next = start;
    start = tmp;
}

void printAlias()                                   //prints aliases
{
    alias *curr = start;
    while(curr != NULL)
    {
        printf("%s\n", curr->alias_line);
        curr = curr->next;
    }
}

int isAlias(char *s)                                //if user input is an alias
{
    alias *curr = start;
    while(curr != NULL)
    {
        if(strcmp(s, curr->alias_name) == 0)
            return 1;
        curr = curr->next;
    }
    return 0;
}

void execAlias(char *s)                             //executes the alias
{
    alias *curr = start;
    char *str;
    while(curr != NULL)
    {
        if(strcmp(s, curr->alias_name) == 0)
        {
            str = malloc(sizeof(curr->alias_command));
            strcpy(str, curr->alias_command);
            parseStr(str);
        }
        curr = curr->next;
    }
}

void unAlias(char *s)                               //delete an alias
{
    alias *temp = start, *prev;
    if(temp != NULL && (strcmp(s, temp->alias_name) == 0))
    {
        start = temp->next;
        free(temp);
        return;
    }
    while(temp != NULL && (strcmp(s, temp->alias_name) == 0))
    {
        prev = temp;
        temp = temp->next;
    }
    if(temp == NULL)
    {
        printf("Alias does not exist\n");
        return;
    }
    prev->next = temp->next;
    free(temp);
}

void execPipe(char *s)
{
    char *args1, *args2, **args3, **args4;
    char *token = strtok(s, "|");                   //first arg
    args1 = token;              
    makearg(args1, &args3);             
    token = strtok(NULL, "\n");                     //second arg
    args2 = token;                                  //attempted to make multiple pipes
    makearg(args2, &args4);
    pid_t child;
    child = fork();                                 //wasnt working at first
    if(child == 0)                                  //had to fork entire process
    {
        int pd[2];
        pipe(pd);
        if(!fork())
        {
            dup2(pd[1], 1);                         //output to parent
            execvp(*args3, args3);                  //exec
        }
        else{
            wait(0);
            dup2(pd[0], 0);                         //output to input
            close(pd[1]);
            execvp(*args4, args4);                  //exec
        }
    }
    else{
        wait(0);
    }
}

void expandPath(char *s)                            //expands path
{
    char path_buf[MAX_SIZE];
    int i = 0;
    while(*s != ':')
        s++;
    s++;
    while(*s != '\0')
    {
        path_buf[i] = *s;
        i++;
        s++;
    }
    strcat(PATH, path_buf);                         //current path + new
    setenv("PATH", PATH, 1);                        //set path
}

/*-----PARSER-----*/
int words(char *s)                                  //finds total number of words (argc)
{
    int state = 0, argc = 0;                        //argc and state
    pipeFlag = 0;
    while(*s != '\n' && *s != '\0')                 //while not endline
    {
        if(*s == ' ')                               //if space
        {
            s++;                                    //traverse string
            state = 0;                              //no longer a word, set back to zero
        }
        else if(*s == '"')                          //if double quote
        {
            s++;                                    //traverse string
            state = 0;                              //no longer a word, set back to zero
        }
        else if(*s == '\'')                         //if single quote
        {
            s++;
            state = 0;
        }
        else if(*s == '\t')
        {
            s++;
            state = 0;
        }
        else if(*s == '!')
        {
            s++;
            argc++;
            state = 0;
        }
        else if(*s == '|')
        {
            s++;
            pipeFlag = 1;
            state = 0;
        }
        else if(((*s != ' ') || (*s != '"') || (*s == '\'')) && (state == 0))
        {
            argc++;                                 //increment word count
            s++;
            state = 1;                              //state to determine if still on chars
        }
        else
            s++;
    }
    return argc;
}

int strLength(char *s)                              //finds string length of each word
{
    int length = 0;                                 //increment length of words in function
    while((*s != ' ') && (*s != '"') && 
            (*s != '\'') && (*s != '|') && (*s != '!')
            && (*s != '\t'))                        //while not space and double quote
    {
        if(*s == '\n' || *s == '\0')                //if endline
            break;
        else{
            length++;                               //increment chars
            s++;
        }
    }
    if(*s == '!' && length == 0)
    {
        length++;
        return length;
    }
    else
        return length;
}

int makearg(char *s, char ***args)                  //completes 2d array to execvp
{
    int argc = words(s);
    int length, i;
    if(argc == 0)
        return -1;
    *args = malloc(argc*sizeof(char*) + 1);
    (*args)[argc] = NULL;
    argc = 0;
    while(1)
    {
        if(*s == '\n' || *s == '\0')
            break;
        else{
            while((*s == ' ') || (*s == '"') ||
                    (*s == '\'') || (*s == '|') || (*s == '\t'))
                s++;
            if(*s == '\n' || *s == '\0')
                break;
            length = strLength(s);
            if(length == 0)
                return argc;
            (*args)[argc] = malloc(length + 1);
            strncpy((*args)[argc], s, length);
            (*args)[argc][length] = '\0';
            argc++;
            s += length;
        }
    }
    return argc;
}

void parseStr(char *str)                            //made specifically for multiple commands
{
    int argc, i;
    char **args;
    char *token = strtok(str, ";\n\0");
    while(token != NULL)
    {
        argc = makearg(token, &args);
        runCommand(args);
        token = strtok(NULL, ";\n\0");
    }
}

/*-----EXECUTE COMMANDS-----*/
void runCommand(char **command)                     //moved from main to own function
{                                                   //makes executing from other functions easier
        pid_t child_pid;
        if(strcmp(*command, "exit") == 0)
        {
            printf("\033[0m");
            free(command);
            free(head);
            free(start);
            exit(0);
        }
        else if(strcmp(*command, "history") == 0)
        {
            int i = 0;
            printHistory(head, i);
            free(command);
        }
        else if(strcmp(command[0], "!") == 0)
        {
            if(strcmp(command[0], "!") == 0 &&
                    command[1] == NULL)
                    printf("msh: unexpected token endline.\n");
            else if(strcmp(command[1], "!") != 0)
            {
                int history_num = atoi(command[1]);
                findHistory(history_num);
            }
            else
                parseStr(buffer);
            free(command);
        }
        else if(strcmp(*command, "alias") == 0)
        {
            if(command[1] == NULL)
            {
                printAlias();
            }
            else
                addAlias(buffer);
            free(command);
        }
        else if(strcmp(command[0], "unalias") == 0)
        {
            if(command[1] == NULL)
                printf("msh: expected token\n");
            else
                unAlias(command[1]);
            free(command);
        }
        else if(strcmp(*command, "mshrc") == 0)
        {
            if(command[1] == NULL)
                open_mshrc();
            else
                printf("msh: unexpected token\n");
            free(command);
        }
        else if(strcmp(*command, "color") == 0)
        {
            if(strcmp(command[1], "0") == 0)
                printf("\033[0m");
            else
                changeColor(command[1]);
            free(command);
        }
        else if(strcmp(*command, "cd") == 0)
        {
            char *dir, *temp, path[MAX_SIZE];
            char slash[MAX_SIZE] = "/\0";
            temp = strcat(slash, command[1]);
            getcwd(path, sizeof(path));
            dir = strcat(path, temp);
            chdir(dir);
        }
        else if(isAlias(*command) == 1)
        {
            execAlias(*command);
            free(command);
        }
        else if(pipeFlag == 1)
        {
            execPipe(buffer);
            free(command);
        }
        else if(strcmp(command[0], "export") == 0)
        {
            expandPath(command[1]);
            free(command);
        }
        else{
            child_pid = fork();
            if(child_pid == 0)
            {
                execvp(*command, command);
                perror("msh");
                exit(0);
            }
            else if(child_pid > 0)
            {
                wait(0);
                free(command); 
            }
        }
}
