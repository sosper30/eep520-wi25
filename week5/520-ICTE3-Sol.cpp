/*!
 * \brief Main function demonstrating dynamic array allocation in C++.
 *
 * This program prompts the user to enter the size of an array and then
 * dynamically allocates memory for that array. It fills the array with numbers
 * from 1 to the specified size, prints each element, and then deallocates the memory.
 *
 * \return int Returns 0 upon successful execution.
 */
#include <iostream>

int main() {
    int size;

    // Prompting the user to enter the size of the array
    std::cout << "Enter the size of the array: ";
    std::cin >> size;

    // Dynamically allocating memory for an array of integers
    int* arr = new int[size];

    // Filling the array and printing each element
    for (int i = 0; i < size; ++i) {
        arr[i] = i + 1;
        std::cout << "Array[" << i << "] = " << arr[i] << std::endl;
    }

    // Deallocating the dynamically allocated memory
    delete[] arr;

    return 0;
}


//-----------------------------------------------------------------------------------------------
/*!
 * \brief Main function demonstrating the use of vectors and structs to store student information.
 *
 * This program creates a vector of `Student` structs, prompts the user for the
 * number of students, and then allows entry of each student's name and grade.
 * It then calculates and displays the average grade.
 *
 * \return int Returns 0 upon successful execution.
 */
#include <iostream>
#include <vector>
#include <string>

// Define a structure to store student information
struct Student {
    std::string name; // Student's name
    double grade;     // Student's grade
};

int main() {
    std::vector<Student> students;
    int numStudents;

    // Asking user for the number of students
    std::cout << "Enter the number of students: ";
    std::cin >> numStudents;

    // Inputting each student's information
    for (int i = 0; i < numStudents; ++i) {
        Student student;
        std::cout << "Enter name and grade for student " << i + 1 << ": ";
        std::cin >> student.name >> student.grade;
        students.push_back(student);
    }

    // Calculating and displaying the average grade
    double sum = 0;
    for (const auto& student : students) {
        sum += student.grade;
    }
    double average = sum / students.size();
    std::cout << "Average grade: " << average << std::endl;

    return 0;
}



//-----------------------------------------------------------------------------------------------
/*!
 * \brief Main function demonstrating the creation and use of a dynamically allocated 2D array.
 *
 * This program prompts the user for the dimensions of a 2D array, dynamically
 * creates it, fills it with sequential values, calculates the sum of each row,
 * and then deallocates the memory.
 *
 * \return int Returns 0 upon successful execution.
 */
#include <iostream>
#include <vector>

int main() {
    int rows, cols;

    // Prompting user for the dimensions of the 2D array
    std::cout << "Enter the number of rows and columns: ";
    std::cin >> rows >> cols;

    // Dynamically creating a 2D array
    int** array = new int*[rows];
    for (int i = 0; i < rows; ++i) {
        array[i] = new int[cols];
    }

    // Vector for storing the sum of each row
    std::vector<int> rowSums;

    // Filling the array and calculating row sums
    for (int i = 0; i < rows; ++i) {
        int sum = 0;
        for (int j = 0; j < cols; ++j) {
            array[i][j] = i * cols + j + 1;
            sum += array[i][j];
        }
        rowSums.push_back(sum);
    }

    // Printing the sum of each row
    for (int i = 0; i < rows; ++i) {
        std::cout << "Sum of row " << i << " = " << rowSums[i] << std::endl;
    }

    // Deallocating the dynamically allocated 2D array
    for (int i = 0; i < rows; ++i) {
        delete[] array[i];
    }
    delete[] array;

    return 0;
}
