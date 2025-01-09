#include <stdio.h>
#include <stdlib.h>

#define MAX_USERS 5 // Define maximum number of users

typedef struct {
    int id;
    char name[100];
    int age;
} UserInfo;

// Updated function prototypes
void getUserInfo(UserInfo users[], int count);
void saveUserInfo(const UserInfo users[], int count);
void displayUserInfo(const UserInfo *user);
int searchUserById(const UserInfo users[], int count, int id);

int main() {
    UserInfo users[MAX_USERS]; // Declare an array of UserInfo

    // Get information for multiple users
    getUserInfo(users, MAX_USERS);

    // Save information to a file (optional extension)
    // saveUserInfo(users, MAX_USERS);

    int searchId;
    printf("Enter ID to search: ");
    scanf("%d", &searchId);

    int index = searchUserById(users, MAX_USERS, searchId);

    if (index != -1) {
        // Display user information if found
        displayUserInfo(&users[index]);
    } else {
        printf("User not found.\n");
    }

    return 0;
}

// Update existing functions and add new functions
// Detailed comments should be provided explaining each part
