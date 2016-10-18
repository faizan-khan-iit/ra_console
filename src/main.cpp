/**
DBMS Project II - Relational Algebra Console
    -Faizan Uddin Fahad Khan
    -14074008
    -IDD, CSE, Part III
**/

#include "header.h"
int main(){
    string db_name = start_up();
    int response = get_input();

    while(response != EXIT_PROGRAM){
        newline();
        response = get_input();
    }
    exit_program();
    return 0;
}
