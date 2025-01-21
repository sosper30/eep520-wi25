# Software Engineering For Embedded Applications


---



# The C Programming Language

## Review: Types in C

### Integer Types in C
In C, every variable, function argument, and function return value needs to have a declared type. 

One of the basic types in C is `int`, which refers to a positive, zero, or negative integer. The following bit of code uses the `int` type to define a function that takes an `int` argument, has two local `int` variables, and returns an `int` value.

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

### Float Types in C

In addition , the following floating point types are available:

```c
float x;                 /* four bytes, 6 decimal places     */
double y;                /* eight bytes, 15 decmial places   */
long double z;           /* sixteen bytes, 19 decimal places */
```

You trade storage space and speed as you increase the size of these types.

### The void Type

In C, an object with type `void` has no value. Usually we use `void` to refer to functions that do not return anything, such as

```c
void say_hello() {
  printf("hello\n");
  return;
}
```

Note that you cannot declare a variable of type `void`.

## Review: Modifiers

### Modifiers: `register`

This keyword is used to strongly suggest to the compiler that the variable should be stored in a register instead of in RAM. You would do something like this:

```c
void f(int x) {
  register int i;
  for(i=0; i<10; i++) {
    /* do something */
  }
}
```

However, most compilers know how to figure out when to use a register for a counter without the `register` keyword, so you will almost never need to use this modifer.

### Modifiers: `volatile`

This keyword is used to refer to locations in memory that might change do to something happening outside of the code. It prevents the compiler from assuming that a volatile variable that is assigned only once in the code will never change and subsequently optimizing it out of existence. For example, on an embedded device, if you know that location 0x5555 was a globally available register being written to by, for example, a sensor or interrupt handler, then you could do

```c
voif f() {
  volatile int * x = 0x5555; /* x is a pointer to the
                              * location 0x5555 (see pointers
                              * below) */
  while ( *x == 0 ) {
    /* wait */
  }
  /* do something because *x changed,
    * presumably due to some outside event */
}
```

### Modifiers: `static`

A static variable is one that preserves its value even after it has gone out of scope. For example, compare the following two functions

```c
int f() {
  int x = 0;
  x++;
  return x;
}

int g() {
  static int x = 0; /* note: must be a literal constant,
                      *  not a computed value */
  x++;
  return x;
}
```

If you call `f` twice in a row, you will get the value `1` each time. If you call `g` twice in a row, you will get `1` the first time and `2` the second time. Thus, the function `g` is using `x` as local storage. It initializes the variable the first time it is called, but does not reinitialize it upon subsequent calls to the function.

### Modifiers: `static` within a File

The `static` keyword is also used in a _totally different way_ to declare variables and functions as local to the file in which they are defined. If a function is defined without static, as in

```c
int f(int x) {
  return x+1;
}
```

then it is globally available to your code (assuming your code includes its declaration). If a function is defined as static, then it is only available in that file:

```c
static int f(int x) {
  return x+1;
}
```

If you put this in a file called `my_source.c`, then only codw within `my_source.c` can use the function `f`. This is a way to make private functions that you do not want users of an API to access.

### Modifiers: `const`

This keyword is used in variable declarations to make symbols that refer to constants. For example

```c
int f() {
  const double PI = "3.14159";
  /* etc */
}
```

The compiler will complain if you attempt to reassign `PI` later in the code for `f`.

The use of `const` gets complicated when combined with pointers (see below). In short, you can define a constant pointer with

```c
int * const ptr;
```

and a pointer to a constant value with

```c
const int * ptr;
```

### Modifiers: `const` Arguments

If you have a function that takes a pointer to a value and want to enforce that the function does not modify the value pointed to, then you would define the argument to the function as follows:

```c
void f(const int * p) {
  /* do things like print *p but don't
    * modify it */
}
```

The following example will produce a compile error because the function attempts to change the value pointed to by `x`.

```c
int f ( const int * x ) {
    *x = *x + 1;
    return *x;
}
```

Compiling this code gives the error

```bash
  error: assignment of read-only location '* x'
  *x = *x + 1;
    ^
```

### Modifiers: `extern`

This keyword is used in certain circumstances to declare functions without defining them, and to tell the compiler to go ahead and link your code expecting that the function will be defined somewhere else. We will likely not need it, although you will see it a lot in header files we include.

## Structures

A structure in C is like a record or dictionary in other languages. It is a way to define a new type of object to store information that belongs together. For example, you might use the following structure to keep track of the information associated with a new data type you are defining,called point, and then declare some points.

```c
struct point {
  double x, y, z;
};
struct point p, q;
```

You can then refer to the components of a point with the `.` notation as in `p.x` or `q.z`. If you do not name the `struct` then you can declare `p` and `q` directly, but then cannot declare more structs of the same type:

```c
struct {
  double x, y, z;
} p, q;
```

## Typedef with `struct`

If you would like to avoid having to write `struct point` over and over, you can also make a type definition as in the following example:

```c
typedef struct point {
  double x, y, z;
} Point;
Point p, q;
```

which also delcared `p` and `q` to be of type `struct point`.

You can initialize a struct using either of the following notations:

```c
Point p = { .x = 1.0, .y = 2.0, .z = 3.0 };
Point q = { 1.0, 2.0, 3.0 };
```

The order in which the members of the struct were declared determines the order in which the initializers should appear.

## Unions

A `union` is like a `struct`, but with all members using the same memory location. A `union` allows you to use only **one** of the members of the union at the same time. For example,

```c
typedef union thing {
  int x;
  double y;
  float z;
} Thing;
Thing t;
```

In this case, the addresses in memory of `t.x`, `t.y` and `t.z` are all the same. If we replaced the above with a `struct`, they would all be different.

## Enums

An `enum` is a way to enumerate the possible values a variable might have. For example

```c
typedef enum status {
  IDLE, RUNNING, ERROR
} Status;
Status x = IDLE;
```

defines a variable of type `Status` whose values are either `IDLE`, `RUNNING` or `ERROR`. These values are _not_ strings. They are symbols. However, in C (but not in C++), the compiler actually just uses the integers 0, 1, 2, ... internally to represent the values in an enum. Thus, you will notice that you can treat them like integers, but you should make every effort not to do so, since other compilers may use different numbers to represent an enum's values.

# Pointers

The most difficult aspect of C is its use of pointers, which most other higher level languages like Python, Java or Javascript do not have. When you declare a variable, the C compiler has to store it in memory somewhere. The location in memory of the value of a variable is called its _address_. So a pointer variable is a variable whose value is an address.

There are two operators in C that you use to change back and forth between a variable's value and its address. The first is the `&` operator, which finds a variable's address. For example,

```c
int p = 1;
printf("The value of p is %d and the address of p is %x.\n", p, &p);
```

which would print out something like:

```bash
The value of p is 1 and the address of p is e5788eac.
```

The hexadecimal number e5788eac is the physical address in memory of the first byte of the integer `p`.

# Dereferencing

The second operator is the `*` operator, which dereferences a pointer and is also used to declare a pointer. For example, suppose `p` and `ptr` were declared as follows:

```c
int p = 1;
int * ptr = &p;
```

then `p` is an integer and `ptr` is the address of `p` in memory. If you would like to get the value pointed to by `ptr`, then you would use the syntax `*p`. For example,

```c
int q = *ptr; /* q is 1 */
*ptr = 2;     /* indirectly changes p to 2 */
```

# Pointers to `structs`

One of the places pointers are used extensively in C is with pointers to structs. This is because a struct can be quite large, and passing structs around by copying everything in them is a waste of time and space. When you have a variable that is a pointer to a struct, then you use the `->` operator instead of the `.` operator, as in the following example:

```c
typedef struct {
  double x, y, z;
} Point;
Point * ptr;
ptr->x = 3;
```

Actually, `ptr->x` is really just shorthand for

```c
(*ptr).x
```

but is more preferred.

# Pointers to functions

You can also define pointers to functions. The syntax is tricky. For example, the following defines a function `add` and a pointer `f_ptr` to it.

```c
int add(int n, int m) {
  return n+m;
}
int (* f_ptr) (int,int);
f_ptr = add;
```

You can use this syntax to send functions to other functions as arguments. For example, the following function applies a function to a value and returns the value.

```c
int apply(int (*f) (int), int x) {
  return f(x);
}
```

# Arrays

Arrays in C are contiguous regions of memory that contain strings of values.
Arrays can be declared with the `[]` operator as follows:

```c
int x[10];                             /* an array of 10 integers */
Point plist[20];                       /* An array of 20 Point structures */
double y[] = { 2.1, 3.2, 4.3, 5.4, };  /* Array of four doubles with initial values */
```

Arrays are zero indexed. Elements can be assigned and retrieved using the `[]` operator as well. For example,

```c
x[0] = 1;
x[1] = x[0];
plist[5] = (Point) { 3.1, 4.1, 5.9 };
y[3] = plist[5].y;
```

In the above cases, `x`, `plist` and `y` are just pointers to the beginning of the memory location for the arrays they represent. The `[]` operator is just shorthand for pointer arithmetic. Thus, the above code is equivalent to the following:

```c
*x = 1;
*(x+1) = *(x);
*(plist + 5) = (Point) { 3.1, 4.1, 5.9 };
*(y+3) = (plist+5)->y;
```

# Bound Checking

**Warning**: Arrays in C are not bounds checked. For example, the following code may compile just fine, but in fact contains a serious error.

```c
int a[10];
a[12] = 5;
ASSERT_EQ(a[12], 5);
ASSERT_EQ(a[13], 0);
```

This compiles and runs without error in the `cppenv` container, but it is just luck. The memory locations at `a+12` and `a+13` just happen to be unused. If you do try to write to `a[1000]` or larger, you will almost certainly encounter either

- a segmentation fault, meaning that you wrote a value outside the memory reserved by the OS for your application;
- strange behavior, meaning you wrote a value to some other data structure's portion of memory.
  To catch errors like this, you can use a tool called 'Address Sanitizer'. To use it, we modify the Makefile as follows

```c
CFLAGS      := -fsanitize=address -ggdb
LIB         := -lgtest -lpthread -lasan
```

Now, the code still compiles, but when you run it you get all sorts of useful error information from `asan`.

# Memory Allocation

When you declare arrays as with the above, you know at compile time how big they should be. However, often you do not know this, and may also need to write functions that return arrays. To dynamically allocate memory in C, you use the functions `malloc` and `calloc`, which are available in `stdlib`. For example, to dynamically allocate memory for 10 doubles, you can do:

```c
double * a = (double *) malloc(10*sizeof(double));
int * b = (int *) malloc(15*sizeof(int));
char * c = (char *) malloc(5*sizeof(char));
```

or

```c
double * a = (double *) calloc(10,sizeof(double)); /* also inits array to zeros */
```

Now `a` can be used just like a normal array, with elements `a[0]`, `a[1]` and so on. Note that 'malloc' and 'calloc' return `void *` pointers, because they do not have any way of knowing what type of array you want. Thus, we have to _type cast_ or _coerce_ the value returned into the correct type. That is what the `(double *)` notation does.

# Deallocation

It is important to note that if you declare a pointer and allocate memory for it in a function, then the pointer disappears when the function is complete, but the memory allocated does not. Thus, when you are done using the memory, your code must give the memory back to the operating sytem using the `free` function, also in `stdlib.h`. For example,

```c
void f() {
  int * a = (int *) calloc(1000,sizeof(int));
  /* do stuff with a */
  free(a);
}
```

This issue becomes particularly important when you use functions that allocate memory for you. For example, here is a function that joins two arrays together into a new array:

```c
int * join(const int * a, int length_a, const int * b, int length_b) {
    int * c = (int *) calloc(length_a+length_b, sizeof(int));
    for (int i=0; i<length_a; i++ ) {
        c[i] = a[i];
    }
    for (int i=0; i<length_b; i++ ) {
        c[i+length_a] = b[i];
    }
    return c;
}
```

# The Allocation / Free Pattern

To use this function and then free the result, you might do

```c
TEST(Examples,AlocateAndFree) {
    int a[] = { 0, 1, 2 };
    int b[] = { 3, 4, 5 };
    int * c = join(a, 3, b, 3);
    for (int i=0; i<6; i++ ) {
        ASSERT_EQ(c[i], i);
    }
    free(c);
}
```

Repeated failure to free the result of `join` would result in a _memory leak_, which will eventually use up all the RAM on your computer, causing a crash. These are hard to catch. Memory leaks are in fact one of the biggest issues plaguing modern software. Modern languages have _garbage collectors_ to clean up unused memory, but (a) they don't work in every case and (b) they are written in C or C++ by humans who make mistakes.

# Strings are arrays of chars

Strings in C are contiguous regions of one byte memory addresses whose values are usually interpreted as characters. To declare and initialize strings, you do something like:

```c
char x[] = "Hi";
char y[3] = { 'H', 'i', '\0' };
char z[3];
char z[0] = 'H';
char z[1] = 'i';
char z[2] = '\0';
```

The special character `\0` is called the **null** character, and it ris used to terminate strings so that functions that manipulate them know when to stop.

When you declare a string within a scope, its memory is allocated for the duration of that scope. If you want a string that lasts a long time, you might have to allocate it yourself, in which case you would just treat it as an allocated array of characters.

# Abstract Data Type (ADT)

Next Week
