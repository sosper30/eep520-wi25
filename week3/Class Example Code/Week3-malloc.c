#include <stdio.h>  
#include <stdlib.h> 

int main() {  
    int *arr; // Declare a pointer 'arr' of type integer. This will be used to point to an array of integers.
    int size, i; // Declare two integers: 'size' to store the size of the array and 'i' for loop iterations.

    printf("Enter the size of the array you want: "); // Prompt the user to input the size of the array
    scanf("%d", &size); // Read the size inputted by the user and store it in the variable 'size'

    // Dynamically allocate memory for the array
    arr = (int*) malloc(size * sizeof(int)); // Allocate memory block of 'size' integers and assign its address to 'arr'.
   

    // Check if memory allocation was successful
    if(arr == NULL) { // If 'arr' is NULL, memory allocation failed.
        printf("Memory not allocated, Try again.\n"); // Inform the user that memory allocation failed.
        return 0; // Exit the program
    } 

    // If the program reaches here, memory allocation was successful
    printf("Memory successfully allocated using malloc, Good job.\n");

    // Initialize the array with values
    for(i = 0; i < size; i++) { // Loop over each element in the array
        arr[i] = i + 1; // Assign the value 'i + 1' to the ith element of the array
                        // This will fill the array with numbers 1, 2, 3, ..., size
    }

    // Print the values of the array
    for(i = 0; i < size; i++) { // Loop over each element in the array
        printf("%d ", arr[i]); // Print the ith element of the array followed by a space
    }
    printf("\n"); // After printing all elements, print a newline character

    // Free the dynamically allocated memory
    free(arr); // Release the memory pointed to by 'arr'
               // Always remember to free dynamically allocated memory to avoid memory leaks
    printf("Memory successfully freed.\n"); 

    return 0;
}
