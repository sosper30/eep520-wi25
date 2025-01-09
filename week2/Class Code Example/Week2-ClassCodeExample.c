#include <stdio.h>
#include <stdlib.h>

//struct - is a way to group related variables undeer one name

typedef struct {
    int id;
    char name[100];
    int age;
} UserInfo;

//Function Prototypes
//These are declarations of functions that we will be using in this program. 
//Sometimes you can include Function Prototypes in a .h file.
void getUserInfo(UserInfo *user);
void saveUserInfo(const UserInfo *user);
void displayUserInfo(const UserInfo *user);

int main () {
    UserInfo user;

    getUserInfo(&user);
    saveUserInfo(&user);
    displayUserInfo(&user);


    return 0;
}


//Function to get user information
//*user is a pointer which we will use to access the Use struct. the * behind user makes it a pointer.
void getUserInfo(UserInfo *user){
    printf("Enter your ID:");
    scanf("%d", &user->id); //Use -> to access struct members when you use a pointer.
    
    printf("What is your First Name:");
    scanf("%s", &user->name);

    printf("Enter your Age:");
    scanf("%d", &user->age);

// no return is needed as this is a void function.
}

//Function to save User information which we got from getUserInfo

void saveUserInfo(const UserInfo *user){
    FILE *file = fopen("userInfo.txt", "w"); //this will open a file called userInfo in a write mode. if the file doesn't exist, it will create one for you.

    //In order to check whether the file was successfully opened:
    if (file == NULL){
        printf("Error opening userInfo.txt not successful");
        exit(1); //exits the program (or ends the program)
    }
    //Writing the user information in userInfo.txt
    fprintf(file, "ID:%d\nName:%s\nAge:%d\n", user->id, user->name, user->age);
    fclose(file);
}

//Function to display user information
void displayUserInfo(const UserInfo *user){
    printf("\nUser Information:\n");
    printf("ID: %d\n", user->id);
    printf("Name: %s\n", user->name);
    printf("Age: %d\n", user->age);
}