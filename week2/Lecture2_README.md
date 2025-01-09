# Software Engineering For Embedded Applications

## Lecture 2 - C Syntax and Data Types

---

Last week we focused on the installation and use of several development tools. The aims were to give you a solid environment in which to develop code in C and C++, as well as to make sure the class as a whole are using the same tools.

This week we review (or introduce) the C language and use our environment to make a simple application.

- Build environment revisited
- C data types
- C control structures
- Example: RPN Calculator
- Homework

### VS Code

- Please use this editor to edit your source files, header files, and Makefile.
- VS Code also includes a terminal for entering command line statements.

### Command line / Linux

Get away from using Finder and Explorer. Use the command line.

Make sure you understand these basic linux commands:

- `pwd`, `cd`, `ls`, `mkdir`, `cp`, `mv`, `rm`

Make sure you know these file system abbreviations

- `.`, `..`, `/`

When we say to run a particular command from "within" a directory X, we mean that the current directory of your command line terminal should be X. That is, `pwd` should return `X`. If not, `cd` to the directory X before running the command.

More information about command line and bash: http://linuxcommand.org/index.php. This is not merely a suggestion!

### Git

Use `git` to keep track of versions of your code, to save your code to `github`, and to retrieve example code and lecture notes from the course repository.

You will put your code in a repository called `520-Assignments` under your username. When you make changes, and to submit your homework, run the following from within `520-Assigments` (or any subdirectory of `520-assignments`).

```bash
git add .
git commit -m "an informative message"
git push origin master
```

### Docker

You should have Docker running on your computer by now. For the next several weeks we will continue to use the `cppenv` image to start a container that has the gnu compiler tools, the Google test library, and the Doxygen API documentation tool.

Within your host environment you will execute a command like

```bash
docker run -v /path/to/EEP520_code_on_your_local_machine:/source -it klavins/ecep520:cppenv bash
```

to get a `bash` prompt that allows you to run commands within the "container environment".

You will initially be placed to the `/source` directory onto which the `-v` option to docker will have mounted the `/path/to/EE P590` directory in your host environment. This directory and its children are the only host directories available within the container environment.

### Compiling and Running C Code

===

Commands gcc and g++ are available in the container environment, and are used to compile C and C++ source files into object files and executables.

**Note:** Some of you may also have these commands available in your host environments, but will not have libraries like `gtest` installed on your host environment. So you should only use these commands in the container environment.

To compile the hello world example, do the following from within in the `cppenv` bash shell:

```bash
cd hello_c                      # change directory
ls                              # see what's in the directory
gcc -c hello.c                  # compile hello.c into hello.o
gcc -c main.c                   # compile main.c into main.o
gcc -o hello *.o                # link the two .o files to make an executable
ls -l                           # show all the new files
./hello                         # run the new execuable
```

### Compiling C++ Code

===

From within in the `cppenv` bash shell,

```bash
cd hello_cpp
g++ -c hello.cc
g++ -c main.cc
g++ -o hello *.o
./hello
```

### Compiling Tests and Linking Libraries

===

In the fractions directory is a simple Fraction api, and some unit tests for it. These tests use [Google Test](https://github.com/google/googletest), a library that the `cppenv` Docker container already has installed in it. To build the executable for the fractions tests, do

```bash
cd fractions
g++ -c fraction.c -std=c++17
g++ -c unit_tests.c -std=c++17
g++ -c main.c -std=c++17
g++ -o test *.o fraction.h -lgtest -lpthread
```

Note that although the Fraction api is written in C, the test library is written in C++, so we use the `g++` compiler and linker to build the code. Also, Google Test uses the C++11 standard, which is not the default for C++, so we have to tell g++ top use it. Finally, the linker needs to know to link the shared object library for Google Test, called `gtest`, and also the PThread library, called `pthread`, which Google Test users.

To run the tests in `unit_tests.c`, simply do

```bash
test
```

### Using a Makefile

Having to type `g++` commands over and over is tedious. To manage the build process, we will use [make](https://www.gnu.org/software/make/). The fractions directory has a Makefile in it with generic definitions for making a new project. To run make, just do

```bash
make
```

from within the fractions directory. This will make two new directories, `bin` and `build`. The former will have the executable, test, in it. The latter will have all the `*.o` files. Then it builds and links everything.

To documentation, which turns comments in the code into an html browsable form, you can do

```bash
make docs
```

Don't worry about documentation for now.

You can clean up by doing

```bash
make clean
```

or

```bash
make spotless
```

to really clean up.

You will normally just use the course Makefile and edit a few lines to tell make what files are part of your source code. We have made changes to the Makefile, by the way. You no longer need to add your source and header files, because we have changed the `HEADERS` and `SOURCES` definitions to:

```make
HEADERS     := $(wildcard *.h)
SOURCES     := $(wildcard *.c)
```

### gTest

The gTest is a C library installed in the cppenv image and available to `gcc` and `g++` within the container environment. Specifically, the cppenv conatiner has the gtest include files installed in

```bash
/usr/local/include/gtest
```

which you get by putting

```c
#include "gtest/gtest.h"
```

in your source files. The gtest shared object library (which is like a unix DLL) is located at

```bash
/usr/local/lib/libgtest.a
```

which you link when you compile with the `-lgest` option to `gcc` or `g++`.

**Note:** Gtest is a C++ library. To use it with our C examples, we will need to compile everyting with `g++`. Thus, although C++ syntax is available to us, we will only use C for now in our code.

## The C Programming Language

===

Most of you have encountered C at some point. We will not review all of the details of C syntax. Many guides exist online and book by Kerninghan and Ritchie is an excellent source. If you are not familiar with the following concepts in C, you should review them before attempting the homework (for example [here](http://www.cplusplus.com/doc/tutorial/control/)).

- `if` statements
- `for` loops
- `while` loops
- `do` ... `while` loops
- `switch` ... `case` statements
- operators such as `*`, `+`, `-`, `%`, `||`, `&&`, `>`, `<`, `>=`, `<=`, `!`, `++`, and `--`
- the `a ? b : c` syntax for inline if statments

In this lecture, we will assume these concepts are straightforward and mainly talk about C's type system, which is the most complex aspect of C programming.

# printf

One of the most useful functions in C is the `printf`. You can use it to format strings together with variables to print information to the shell. The declaration of `printf` is

```c
int printf ( const char * format, ... );
```

which means it takes a pointer to a (null terminated) string of characters and an optional set of arguments. The optional arguments are values that will be interpoloated into the string based on 'format specifiers', one for each type. For example,

```c
int x = 1;
double y = 2.3;
char z[] = "uw";
printf("x = %d, y = %lf, z = '%s'\n", x, y, z );
```

prints out the values of `x`, `y`, and `z` in a nicely formatted way. See the [documentation](http://www.cplusplus.com/reference/cstdio/printf) for `printf` for a list of the other format specifiers and modifiers available to `printf`.

# Types in C

In C, every variable, function argument, and function return value needs to have a declared type. One of the basic types in C is `int`, which refers to a positive, zero, or negative integer. The following bit of code uses the `int` type to define a function that takes an `int` argument, has two local `int` variables, and returns an `int` value.

```c
int sum(int n) {
  int i,
      s = 0;
  for (i=0; i<=n; i++) {
    s = s + i;
  }
  return s;
}
```

# Scope

Note that when `s` is declared it is also assigned an initial value. Initializating a variable when it is declared is optional, but often a good idea. You can also write the above function like this:

```c
int sum(int n) {
  int s = 0;
  for (int i=0; i<=n; i++) {
    s = s + i;
  }
  return s;
}
```

In this case, the variable `i` is delcared within the scope of the for loop and not in the broader scope of the function. In general, you can declare local variables in any block, whether it is a function block, a `for` loop or an `if` statement. In particular, you can declare variables in google test `TEST` blocks.

```c++
TEST(MyTest, LocalOne) {
  int x = 5;
}

TEST(MyTest, LocalTwo) {
  double x = 6.28; /* different variable than in the previous block */
}
```

# The void type

In C, an object with type `void` has no value. Usually we use `void` to refer to functions that do not return anything, such as

```c
void say_hello() {
  printf("hello\n");
  return;
}
```

Note that you cannot declare a variable of type `void`.

# Basic Types

The following code demonstrates most of the basic integer types in C along with some initializations showing what sorts of values they take. Note that character values such as `'a'`, `'b'` and `'c'` are shorthand for ASCII integer values, in this case `97`, `98`, and `99`.

```c
char a = 97;             /* one byte */
unsigned char b = 'b';
signed char c = -99;
short d;                 /* two bytes */
unsigned short e;
int f;                   /* two or four bytes */
unsigned int g;
long h;                  /* four bytes */
unsigned long i;
```

To see how many bytes a type has, you can use the `sizeof` function, as in

```c
printf("The size of an int on this machine is %d\n", sizeof(int));
```

# Float Types

In addition , the following floating point types are available:

```c
float x;                 /* four bytes, 6 decimal places     */
double y;                /* eight bytes, 15 decmial places   */
long double z;           /* sixteen bytes, 19 decimal places */
```

You trade storage space and speed as you increase the size of these types.

# Limits

The minimum and maximum values of variables with these types for the particular C implemtation you are working with are noted in the `<limits.h>` header file. If you include this header in a source file, open the file with Visial Studio Code, and follow its definition, you'll see all sorts of C pre-processor macros defining limits. To use the definitions, for example, do:

```c
#include <limits.h>
#include <stdio.h>
...
printf("The minimum value of INT = %d\n", INT_MIN);
printf("The maximum value of INT = %d\n", INT_MAX);
...
```

which on an Intel machine will print out

```bash
The minimum value of INT = -2147483648
The maximum value of INT = 2147483647
```

