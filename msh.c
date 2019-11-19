/*
    Ronald Keating
    CS240
    msh v1.1
    3-29-19
    *Update*
*/
#include "msh.h"

int main()
{
    int argc, i;
    char *temp = getenv("PATH");            //sets current path to PATH
    strcat(PATH, temp);
    char **command, str[MAX_SIZE];          //command 2d array and string input
    while(1)                                //breaks on exit input
    {
        printf(">: ");                      //prompt
        fgets(str, sizeof(str), stdin);     //get input from user
        addHistory(str);                    //adds to history
        if(strcmp(str, "\n") == 0)          //if just enter
            continue;                       //continue
        else
            parseStr(str);                  //parse string and execute
    }
    return 0;                               //only ever exits(0);
}