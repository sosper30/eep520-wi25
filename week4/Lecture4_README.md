# Software Engineering For Embedded Applications

## Week 4: Dynamic Memory Allocation. Introduction to C++

**Last time:**

- Types in C
- Abstract data types
- Constructors and destructors
- Naming conventions
- Hiding memory allocation from the user
- Getters, setters

**Today:**

- Abstract Data Types - Cont:
- The DRY principle
- Common array operations like push, pop, and map

- How these ideas map onto C++
- How C++ makes them easier
- How C++ may be confusingly sophisticiated

# Resources

- Reference: [http://www.cplusplus.com/](http://www.cplusplus.com/)
- Books: [https://stackoverflow.com/questions/388242/the-definitive-c-book-guide-and-list](https://stackoverflow.com/questions/388242/the-definitive-c-book-guide-and-list)

## Review - Abstract Data Type (ADT)

An _Abstract Data Type_ (ADT) is a set of values and methods or operations that operate on those values. The interface to an ADT defines only how to build values and operate on them, but not how memory is allocated or how the operations are performed. ADTs are extremely useful to programmers. They allow you to

- avoid "reinventing the wheel" (and the explosion of many incompatible implementations of the "wheel")
- separate concerns: your user is concerned with other problems than how you implement array methods
- write cleaner code and
- provide a contract between you and other programmers.

## An Array API

Following many standard abstract array types in other languages, we will implement an array type with the following operations on our array ADT:

- Make a new, empty, array.
- Destroy an array.
- Set elements of the array, as in `my_array[12] = 34`. Any non-negative index should be accepted.
- Get elements of the array, as in `x = my_array[54]`. Any non-negative index should be accepted, with default values for out of bounds queries.
- Return the size of the array.
- Push new values onto the beginning and ending of the array.
- Pop values from the beginning and ending of the array.
- Concatenate two arrays.
- Map functions onto arrays to get new arrays.
- Iterate through an array.
- Print an array.

# Naming and argument conventions

In languages with build-in ADTs, the convention is to access a method like `push` of an array `a` with the notation:

```c
a.push(1)
```

In C, the `dot` notation does not allow such notation. Thus, all of our methods will be declared as stand alone functions in the global namespace. To avoid naming collisions, we will use the following convention:

- Naming methods by the data type name followed by an underscore followed by the method name;
- Putting a pointer to the datum being operated upon as the first argument of the method (except for the constructor):
  For example,

```c
DynamicArray * da = DynamicArray_new();
DynamicArray_push(da, 1);
```

# Some Getting and Setting Tests

```c

    TEST(DynamicArray, SmallIndex) {
        DynamicArray * da = DynamicArray_new();
        ASSERT_EQ(DynamicArray_size(da),0);
        DynamicArray_set(da, 0, -X);
        DynamicArray_set(da, 3, X);
        ASSERT_EQ(DynamicArray_size(da),4);
        ASSERT_EQ(DynamicArray_get(da,0), -X);
        ASSERT_EQ(DynamicArray_get(da,3), X);
        DynamicArray_destroy(da);
    }

    TEST(DynamicArray, BigIndex) {
        DynamicArray * da = DynamicArray_new();
        DynamicArray_set(da, 8, X);
        ASSERT_EQ(DynamicArray_get(da,8), X);
        DynamicArray_destroy(da);
    }

    TEST(DynamicArray, ReallyBig) {
        DynamicArray * da = DynamicArray_new();
        DynamicArray_set(da, 400, X);
        DynamicArray_set(da, 200, X/2);
        ASSERT_EQ(DynamicArray_get(da,200), X/2);
        ASSERT_EQ(DynamicArray_get(da,400), X);
        DynamicArray_destroy(da);
    }
```

# Internal Memory Allocation

We do not want the user of `DynamicArray` to worry about memory allocation. We also want array operations like getting, setting, pushing and popping to be fast, so reallocating the memory associated with a DynamicArray should happen rarely. To achieve these goals, we will use the following scheme:

![memory](https://raw.githubusercontent.com/klavins/EEP520-W20/master/week_3/images/dynamic_array_arbitrary.png)

- Start out with a buffer of given length, say 100. Call this the `capacity`.
- Keep track of two indices into the buffer, `origin` and `end`.
- Initially put `origin = end = capacity / 2`.
- As elements are added beyond `end`, change `end` to include them.
- If an element is pushed to the front, decement `origin`.
- If an element is added that is beyond the capacity, increase the buffer size by 2.

# A Dynamic Array Object

We need a place to store all of the information associated with a `DynamicArray`, which we will put in a `struct` containing the associated information:

```c
typedef struct {
    int capacity,
        origin,
        end;
    double * buffer;
} DynamicArray;
```

which appears in the header file `DynamicArray.h`.

# The Constructor

Although users could user this `typedef` to make new values, they would need to know how much memory to allocate and how to initialize the `capacity` and so on. Functions that build or construct new ADT values are called _constructors_. Here is a constructor for `DynamicArray`:

```c
DynamicArray * DynamicArray_new(void) {
    DynamicArray * da = (DynamicArray *) malloc(sizeof(DynamicArray));
    da->capacity = DYNAMIC_ARRAY_INITIAL_CAPACITY;
    da->buffer = (double *) calloc ( da->capacity, sizeof(double) );
    da->origin = da->capacity / 2;
    da->end = da->origin;
    return da;
}
```

Note that the constructor takes newno arguments, and returns a pointer to a new `DynamicArray` object. It also refers to a constant that determines the initial capacity and that is defined in the header file.

# The Destructor

Once a user is done with the array, we need to free the memory associated with it. We do not want the user to call `free(da->buffer)`, because that would imply they knew about how the memory was allocated in the first place. Thus, we provide a \*destructor` as follows:

```c
void DynamicArray_destroy(DynamicArray * da) {
    free(da->buffer);
    da->buffer = NULL;
    return;
}
```

The user can thus write

```c
DynamicArray * da = DynamicArray_new();
DynamicArray_push(da, 1);
DynamicArray_destroy(da);
free(da);
```

Note the user has to do two things. First, tell the array to free its own internal memory. Second, the user has to free the struct associated with the array. If they just call `free(da)` with, then the pointer `da->buffer` will be deleted, but not the memory it points to! That would be a serious (but not uncommon) memory leak.

# Assertions

The buffer pointer is set to `NULL`, which is usually the address `x0000000` in memory. We will use this fact to test whether a given `DynamicArray` is valid. That is, suppose the user made a new array, then destroyed it but did not free it, and then tried to push something onto it. That would result in the user pushing something to the address `NULL + end`, which would be a memory violation. To avoid this happening, we will add an assertion to our methods that states that the buffer cannot be `NULL` and if it is, to produce a runtime error. We do this with the C library `assert.h` as follows:

```c
/* in the header file */
#include <assert.h>

/* at the beginning of all method definitions */
assert(da->buffer != NULL);
```

If this assert fails, you would see a runtime error such as

```bash
test: dynamic_array.c:82: void DynamicArray_set(DynamicArray*, int, double): Assertion `da->buffer != __null' failed.
Aborted
```

# Testing for Runtime Errors

Google Test includes a method called "ASSERT_DEATH" which takes a statement and a regular expression to match against the text printed out upon the code exiting. You can use this to check that your code is exiting properly when it encounters an error.

```c
    TEST(DynamicArray, DeathTests) {
        DynamicArray * a = DynamicArray_new();
        ASSERT_DEATH(DynamicArray_pop(a), "");
        ASSERT_DEATH(DynamicArray_pop_front(a), "");
        DynamicArray_destroy(a);
        ASSERT_DEATH(DynamicArray_size(a), "");
    }
```

Later in C++ we will write "Exception Handlers" to more gracefully report errors to the user.

Without exceptions handlers, our other option is to do what we did with the RPN example, and have an error state that the user has to check after each operation.

# Size Getter

A getter is a method that "gets" information about an object. A simple bit of information about a `DynamicArray` is its size, for which we define the following method:

```c
int DynamicArray_size(const DynamicArray * da) {
    assert(da->buffer != NULL);
    return da->end - da->origin;
}
```

Note the assertion at the beginning of the method.

# Element Getter

The other obvious getter is the one that returns the value of the array at a given index. This is implemented as follows:

```c
double DynamicArray_get(const DynamicArray * da, int index) {
    assert(da->buffer != NULL);
    assert(index >= 0);
    if ( index > DynamicArray_size(da) ) {
        return 0;
    } else {
        return da->buffer[index_to_offset(da,index)]; /* TODO: Write index_to_offset method */
    }
}
```

It includes another assertion: that the index is non-negative. It also checks to see if the index is out of bounds (using the `_size` method), and if it is, then it returns zero. The alternative to the out of bounds index would be to throw an error, but since we have decided that arrays can grow without bounds, we have decided to return a default value. Zero is also the value that `calloc` uses to initialize the array buffer, so this choice seems reasonable.

# Private Methods

The element getter uses a `private` function that converts an index to a buffer offset. Because this conversion happens so often in our implementation, we've added them to the `dynamic_array.c` as follows:

```c
static int index_to_offset ( const DynamicArray * da, int index ) {
    return index + da->origin;
}

static int offset_to_index ( const DynamicArray * da, int offset ) {
    return offset - da->origin;
}
```

Sure, they are simple functions, but they clearly show in the `_get` code the relationship that the index to `da->buffer` is an offset in the buffer, and not an index in the `DynamicArray`. They are also declared statically and do not have prototypes in the header file, because we don't want our user to ever call them directly.

# Setters

A setter is a method that changes the value of an object. The main setter in the `DynamicArray` is the `_set` method, which simply changes the value at a given index. If we did not care about memory allocation, we could simply write:

```c
/* TODO: Check bounds, etc */
void DynamicArray_set(DynamicArray * da, int index, double value) {
    da->buffer[index_to_offset(da, index)] = value;
}
```

However, if the user supplies a large enough index, the computed offset will exceed the bounds of the buffer. Thus, we should check the bounds first and extend the buffer if necessary.

# Setting Carefully

```c
void DynamicArray_set(DynamicArray * da, int index, double value) {
    assert(da->buffer != NULL);
    assert ( index >= 0 );
    while ( out_of_buffer(da, index_to_offset(da, index) ) ) {
        extend_buffer(da); /* TODO: Write the extend buffer method */
    }
    da->buffer[index_to_offset(da, index)] = value;
    if ( index > DynamicArray_size(da) ) {
        da->end = index_to_offset(da,index);
    }
}
```

This method has three main steps. First, while the computed offset is out of bounds, extend the buffer. The extension is done with a help function described below. Putting the entire extension code in the `_set` method would make it very long and hard to read. Plus, there may be other opportunities to use the extend method and we do not want the same code showing up twice. Second, we set the element at the desired index to the specified value. Finally, we change the `end` index if needed.

# Extending the Buffer

To extend the buffer, we want to keep the original buffer roughly in the middle of the new buffer. Here is the scheme we will use.

The extend method makes a new buffer and then makes temporary `new_origin` and `new_end` variables. The values of these must take into account the fact that the new origin offset is not the same value as the old origin offset, since the capacity has increased. We also want to reposition the array into the middle of the buffer, as the following figure shows:

![memory](https://raw.githubusercontent.com/klavins/EEP520-W20/master/week_3/images/dynamic_array_extend.png)

# Code for Extending the Buffer

The extend method will be a private method, defined statically in the impelemntation file as follows:

```c
static void extend_buffer ( DynamicArray * da ) {

    double * temp = (double *) calloc ( 2 * da->capacity, sizeof(double) );
    int new_origin = da->capacity - (da->end - da->origin)/2,
           new_end = new_origin + (da->end - da->origin);

    for ( int i=0; i<DynamicArray_size(da); i++ ) {
        temp[new_origin+i] = DynamicArray_get(da,i);
    }

    free(da->buffer);
    da->buffer = temp;
    da->capacity = 2 * da->capacity;
    da->origin = new_origin;
    da->end = new_end;

    return;
}
```

The method allocates a temporary pointer to a new buffer, defines the new offsets, copies the existing array, frees the old buffer, and then updates the struct elements that define the array. Note that the method uses the `_get` method, which is described below. The version of this method did not use the `_get` method, but as the rest of the library was built, it seemed reasonable to substitute it in for clarity.

# The DRY Principle

In programming, DRY stands for "don't repeat yourself". We have used it above in a couple of places. First, we defined the relationship between array indices and buffer offsets in one place. This is also know as "Single source of truth". Second, we define the extend buffer method separately from the `_set` method. This is because we know that other methods, such as `_push_` and `_push_front` may need to extend the array as well.

DRY practices makes code easier to maintain. If you change how you want to extend memory in the array, you only have to change the implemenation in one place, instead of tracking down every method that extends the memory.

By the way, the opposite of DRY is WET: "write everything twice" or "we enjoy typing"!

# Visualizing an Array

It is very useful to have a method that converts an ADT into a string so you can print it. To do this in C, we define a method that dynamically allocates a string, prints into it, and then returns that string array. The method we use to print into a string is called `snprintf`. The name comes from `sprintf`, which stands for "string printf". The `n` comes from the second argument to `snprintf`, which sets a limit on how big the resulting string can be. We have reserved 20 bytes for each float (and possible comma between values in the array). But since we do not know how much space a printed double will take in the worse case, we have to be careful to prevent memory overflows. The `_to_string` method for `DynamicArray` is as follows:

```c
char * DynamicArray_to_string(const DynamicArray * da) {
    assert(da->buffer != NULL);
    char * str = (char *) calloc (20,DynamicArray_size(da)),
         temp[20];
    int j = 1;
    str[0] = '[';
    for ( int i=0; i < DynamicArray_size(da); i++ ) {
        if ( DynamicArray_get(da,i) == 0 ) {
            snprintf ( temp, 20, "0" );
        } else {
            snprintf ( temp, 20, "%.5lf", DynamicArray_get(da,i) );
        }
        if ( i < DynamicArray_size(da) - 1 ) {
            sprintf( str + j, "%s,", temp);
            j += strlen(temp) + 1;
        } else {
            sprintf( str + j, "%s", temp);
            j += strlen(temp);
        }

    }
    str[j] = ']';
    return str;
}
```

# Debug Information

To use this method, you need to make sure to free the string when you are done. Here is an example that uses the method to print out some debug information about a `DynamicArray`, showing the free method call at the end.

```c
void DynamicArray_print_debug_info(const DynamicArray * da) {
    char * s = DynamicArray_to_string(da);
    printf ( "%s\ncapacity: %d\norigin: %d\nend: %d\nsize: %d\n",
      s,
      da->capacity,
      da->origin,
      da->end,
      DynamicArray_size(da));
    free(s);
}
```

# Pushing

Pushing a value onto the end of the array is straightforward and can be implemented using other methods.

```c
void DynamicArray_push(DynamicArray * da, double value ) {
    DynamicArray_set(da, DynamicArray_size(da), value );
}
```

Note that we do not even need to assert that the buffer is not null or extend it since the `_set` method takes care of that.

Pushing to the front of the array is almost as easy. If memory allocation were not an issue, we would simply decrement `da->origin` and then set the value at index zero. However, we need to check that `da->origin` is not zero first, and if it is, extend the buffer:

```c
void DynamicArray_push_front(DynamicArray * da, double value) {
    assert(da->buffer != NULL);
    while ( da->origin == 0 ) {
        extend_buffer(da);
    }
    da->origin--;
    DynamicArray_set(da,0,value);
}
```

# Popping

Popping elements off of the end or beginning of the array is similarly straightforward. The main things to do are

- Check that the array is not empty
- Zero out values beyond the new end of the array on popping.

```c
double DynamicArray_pop(DynamicArray * da) {
    assert(DynamicArray_size(da) > 0);
    double value = DynamicArray_get(da, DynamicArray_size(da)-1);
    DynamicArray_set(da, DynamicArray_size(da)-1, 0.0);
    da->end--;
    return value;
}

double DynamicArray_pop_front(DynamicArray * da) {
    assert(DynamicArray_size(da) > 0);
    double value = DynamicArray_get(da, 0);
    da->origin++;
    return value;
}
```

# Mapping

Mapping a function onto an array returns another array. Using the above methods, this meta-function is simple to write:

```c
DynamicArray * DynamicArray_map ( DynamicArray * da, double (*f) (double) ) {
    DynamicArray * result = DynamicArray_new();
    for ( int i=0; i<DynamicArray_size(da); i++ ) {
        DynamicArray_set(result, i, f(DynamicArray_get(da, i)));
    }
    return result;
}
```

Note that in testing this method, one might write:

```c
DynamicArray * t = DynamicArray_new(), * y;
double s = 0.0;
for ( int i=0; i<628; i++ ) {
    DynamicArray_set(t, i, s);
    s = s + 0.1;
}
y = DynamicArray_map(t,sin);
for (int i=0; i<DynamicArray_size(t); i++) {
    ASSERT_NEAR(DynamicArray_get(y,i), sin(0.1*i), 0.0001);
}
DynamicArray_destroy(t);
DynamicArray_destroy(y);
```

# Rounding Errors in Tests

Due to rounding errors, you might get this somewhat odd error message:

```bash
Expected equality of these values:
  DynamicArray_get(y,i)
    Which is: 0.564642
  sin(0.1*i)
    Which is: 0.564642
```

In this case, Google Test is not showing the full precision of the values, but at some point the numbers are not the same. Google Test has another method you can use instead:

```c
ASSERT_NEAR(DynamicArray_get(y,i), sin(0.1*i), 0.00001);
```

The last argument is the precision you require for equality.

# Main Limitation of DynamicaArray

One of the main problems with our dynamic array type is that it only works for arrays of doubles. In most higher languages, arrays work with any data type. Unforutnately, C does not provide a good way to manage types beyond telling you how much memory they take up. For example, you cannot pass a type as an argument into the `DynamicArray` constructor. The best we can do is to pass the size of a type to the constructor and then remember to cast when necessary.

# Arbitrary Array Values

In this section, we will build a new ADT called `AbitraryArray` with the goal to enable code such as the following to work.

```c
typedef struct {
    double x, y, z;
} Point;

AribtraryArray * a = AribtraryArray_new(sizeof(Point));
Point p;

for ( int i=0; i<10; i++ ) {
  p = { i, 0.5*i, 0.25*i };
  AribtraryArray_set_from_ptr( a, i, &p );
}

Point * q = (Point *) AribtraryArray_get_ptr(a,5);
printf ( "{%lf, %lf, %lf}", q->x, q->y, q->z );
```

The `_set_from_ptr` method takes a pointer to an element. This is because we can declare the third argment to the method to have type `void *`, meaning a pointer to something whose type we don't know. Unfortunately in C we can't specify an non pointer argument of unknown type, so this is the best we can do.

The `_get_ptr` method similarly returns a pointer to an element, instead of the element itself. Note that in the assignment to q we had to cast the result of `_get_ptr` to a point, since the method itself does not know what type to return.

# Copies vs References

In this ADT we will _copy_ the value to the array upon a call to `_set_from_ptr`. A very good alternative would be instead to simply store the pointer itself. The main reason we do not do this now is because then the user would need to remember to `free` all the values pointed to by the array after every destructive operation. When we start working with C++, we will see that this is easy to do.

# Arbitrary Array Declarations

The declarations of these methods are as follows:

```c
AribtraryArray * AribtraryArray_new(int);
void AribtraryArray_set_from_ptr(AribtraryArray *, int, void *);
void * AribtraryArray_get_ptr(const AribtraryArray *, int);
```

As you can see, the constructor now takes an argument that specifies the size of an element. The `_set_from_ptr` method takes a `void *` pointer to an element we will _copy_ into the array. The `_get_ptr` returns a `void *`, which we will have to cast if we want to use it subsequently.

# Arbitrary Array Data

To implement these methods, we need to change a few things in our implementation of double arrays. First, we need to add the element size to the `struct` that describes an array and change the type of the buffer from `double *` to an arbitrary array of bytes.

```c
typedef struct {
    int capacity,
        origin,
        end,
        element_size;
    char * buffer;
} AribtraryArray;
```

# Arbitary Array Constructor

In the constructor, we can now use use the element size to allocate the buffer and set the `origin` an `end` offsets.

```c
AribtraryArray * AribtraryArray_new(int element_size) {
    AribtraryArray * da = (AribtraryArray *) malloc(sizeof(AribtraryArray));
    da->element_size = element_size;
    da->capacity = DYNAMIC_ARRAY_INITIAL_CAPACITY;
    da->buffer = (char *) calloc (
        da->capacity,
        element_size
    );
    da->origin = ( da->capacity / 2 ) * da->element_size;
    da->end = da->origin;
    return da;
}
```

# Pointer Arithmetic

In the original implementation, the `origin` and `end` offsets were essentially indices into the array of doubles, with C doing the pointer arithmetic for us. Here, however, we need to do the pointer arithmetic ourselves.

<img src="https://raw.githubusercontent.com/klavins/EEP520-W20/master/week_3/images/dynamic_array_arbitrary.png" width=50%>

The new helper methods now uses the element size.

```c
static int index_to_offset ( const AribtraryArray * da, int index ) {
    return da->element_size * index + da->origin;
}

static int offset_to_index ( const AribtraryArray * da, int offset ) {
    return ( offset - da->origin ) / da->element_size;
}
```

# Arbitrary Array Setting

To implement the `_set_from_ptr` method, we need to copy the bytes pointed to by the `ptr` argument into the appropriate place in the array. The C `<string.h>` library provides a useful method called `memcpy` that copies a certain number of bytes from one memory location into another, so we use it here.

```c
void ArbitraryArray_set_from_ptr(ArbitraryArray * da, int index, void * ptr) {
    assert(da->buffer != NULL);
    assert ( index >= 0 );
    while ( out_of_buffer(da, index_to_offset(da, index) ) ) {
        extend_buffer(da);
    }
    /*     destination,                             source, size            */
    memcpy(da->buffer + index_to_offset(da, index), ptr,    da->element_size);
    if ( index >= ArbitraryArray_size(da) ) {
        da->end = index_to_offset(da,index+1);
    }
}
```

Otherwise, this method is the same as the corresponding method in the `DynamicArray` ADT.

# Extending the Buffer

The new set method does call `extend_buffer`, however, which needs to be re-implemented.

```c
static void extend_buffer ( ArbitraryArray * da ) {

    char * temp = (char *) calloc ( 2 * da->capacity, da->element_size );
    int new_origin = da->element_size * (
        da->capacity - (
            offset_to_index(da,da->end) - offset_to_index(da,da->origin)
        )/2
    );
    int new_end = new_origin + (da->end - da->origin);

    for ( int i=0; i<ArbitraryArray_size(da); i++ ) {
        memcpy(temp + new_origin + i * da->element_size,
               da->buffer + da->origin + i * da->element_size,
               da->element_size);
    }

    free(da->buffer);
    da->buffer = temp;
    da->capacity = 2 * da->capacity;
    da->origin = new_origin;
    da->end = new_end;

}
```

Note that we use `memcpy` here as well. We also use the element size to compute the new centered offsets.

# Getting Elements

Finally, here is the implementation of `_get_ptr`:

```c
void * ArbitraryArray_get_ptr(const ArbitraryArray * da, int index) {
    assert(da->buffer != NULL);
    assert ( index >= 0 );
    if ( index >= ArbitraryArray_size(da) ) {
        return NULL;
    } else {
        return (void*) ( da->buffer + index_to_offset(da, index) );
    }
}
```

The main difference here is that we return a `void *` pointer instead of a `double`, similar to how `malloc` and `calloc` work. This is because the `ArbitraryArray` ADT does not know what the type of the value being pointed to is. We also use pointer arithmetic instead of array indexing to determine the address of the requested element.

## Programming Lanaguage History

<img src="https://camo.githubusercontent.com/8b66ce7a30e85901be14bcc0a20510f458f8b5e3/68747470733a2f2f7261776769746875622e636f6d2f73746572656f626f6f737465722f70726f6772616d6d696e672d6c616e6775616765732d67656e65616c6f676963616c2d747265652f67682d70616765732f696d672f6469616772616d2e737667">

# Language Types

<table style="width: 90%">
<tr>
  <td>Imperative</td>
  <td width=15%><img src="https://www.northernpublicradio.org/sites/wnij/files/styles/x_large/public/201809/hopper_edit.jpg"></td>
  <td>Grace Hopper (US Navy)</td>
  <td>Flowmatic</td>
  <td>COBOL, ...</td>
</tr>
<tr>
  <td>Functional / AI</td>
  <td><img src="https://blog.lfe.io/assets/images/posts/John-McCarthy.jpg"></td>
  <td>John McCarthy (MIT)</td>
  <td>Lisp</td>
  <td>Caml, Haskell </td>
</tr>
<tr>
  <td>Procedural</td>
  <td><img src="https://www.computerhope.com/people/pictures/john_backus.jpg"></td>
  <td>John Backus (IBM)</td>
  <td>Fortran</td>
  <td>C, C++</td> 
</tr>
<tr>
  <td>Object Oriented</td>
  <td><img src="https://history-computer.com/ModernComputer/Software/images/Dahl_and_Nygaard.jpg"></td>
  <td>Ole-Johan Dahl and Kristen Nygaard (NCC)
  </td><td>SmallTalk</td>
  <td>C++, Java </td>
</tr>
<tr>
  <td>Declarative / Logical</td>
  <td><img src="https://upload.wikimedia.org/wikipedia/commons/thumb/4/49/A-Colmerauer_web-800x423.jpg/330px-A-Colmerauer_web-800x423.jpg"></td>
  <td>Alain Colmerauer (U. Montreal)</td>
  <td>Prolog</td>
  <td>More Prolog</td>
</tr>
</table>

Most modern languages combine all of these paradigms.

# Who uses C vs C++?

<img width=98% src="http://sogrady-media.redmonk.com/sogrady/files/2019/03/lang.rank_.119.png">

# Programming Language Expressibility

<img src="http://dberkholz-media.redmonk.com/dberkholz/files/2013/03/expressiveness_weighted2.png">

- C++ is (perhaps) more expressive than C.
- The same algorithm should take fewer lines of code in C++ than it does in C, on average.
- Highly expressive languages are hard to use.
- However, data suggest that the number of bugs in a program is proportional to the number of lines of code, not to the number of ideas expressed.
- Colors show popularity.

# Procedural vs Object Oriented

<img src="https://4.bp.blogspot.com/-1ROdWNxn7v8/XBQ3W1emmmI/AAAAAAAAALU/C05s5HEb93QzB2weQXli829rK95aQpAxACLcBGAs/s1600/procedural-vs-oop-learn-java-oop.jpg" width=50%>

- A class defines an object's <u>attributes</u> and <u>methods</u>
- Inheritance shows how classes can be derived from other classes.
- The implementation is supposed to be hidden from the user.

| Good               | Bad                                    |
| ------------------ | -------------------------------------- |
| Code reuse         | Dogmatic                               |
| Scalability        | Requires training to code correctly    |
| Easy collaboration | Some things really are just procedural |

# Difficulties Scaling C projects

Avoiding naming collisions leads to very long function names:

```c++
    DynamicArray_set(a,1,2)  // dumb
```

Cannot associate methods with the objects they act upon

```c++
    DynamicArray_set(a,1,2)  // dumb
    a.set(1,2)               // better
    a[1] = 2                 // best!
```

Difficult to re-use or build upon code: `DoubleArray` vs `IntegerArray`, etc

Abstract data types are dealt with differently than built in types

- Built in types are:
  - Constructed when declared
  - Destructed when they go our of scope
  - Copied naturally with '='

# The Rise of C++

<img width=20% src="https://media-bell-labs-com.s3.amazonaws.com/pages/20150220_2231/Stroustrup2-small.jpeg"><br>
Bjarne Stroustrup (Bell Labs)

Until C++, OOP was mostly academic

Stroustrup’s goal: Combine Simula and other OOP ideas with a practical language like C.

# C++ History

1985 (while at Bell Labs and Columbia U)

- classes
- derived classes / inheritance
- strong typing
- inlining
- default arguments
- virtual functions
- function name and operator overloading
- constants
- type-safe free-store memory allocation (new/delete)
- improved type checking
- single-line comments with two forward slashes (//)

1989-1990

- multiple inheritance
- abstract classes
- static member functions
- const member functions
- and protected members

Later

- templates
- exceptions
- namespaces
- boolean type

Recently

- Anonymous functions

# C and C++ Uses

**C**:

- Operating systems, embedded systems, libraries, compilers, interpreters (e.g. Python), compilers (Java), intermediate languages (LLVM)

- Cars, planes, boats, phones, Linux, MATLAB, Mathematica, MySQL, ...

- The C compiler (so meta!)

**C++**:

- Big software: IE, Chrome, Firefox, Safari, Microsoft Office, OpenOffice, Outlook, Facebook, YouTube, Media Player, iPod software, MySQL, MongoDB, Windows UI, MacOS UI, Microsoft Visual Compiler, …

- Games: Doom III, Counter Strike, Sierra On-line: Birthright, Hellfire, Football Pro, Bullrider I & II, Trophy Bear, King’s Quest, Antara, Hoyle Card games suite, SWAT, Blizzard: StarCraft, StarCraft: Brood War, Diablo I, Diablo II: Lord of Destruction, Warcraft III, World of Warcraft, Starfleet Command, Invictus, Master of Orion III, CS-XII, MapleStory etc. Microsoft games, ...

- Languages: Javascript compiler, ...

# Linus Torvalds on C++

From the invetor of Linux (written in C)

C++ is a horrible language. It's made more horrible by the fact that a lot of substandard programmers use it, to the point where it's much much easier to generate total and utter crap with it. Quite frankly, even if the choice of C were to do _nothing_ but keep the C++ programmers out, that in itself would be a huge reason to use C.

I've come to the conclusion that any programmer that would prefer the project to be in C++ over C is likely a programmer that I really _would_ prefer to piss off, so that he doesn't come and screw up any project I'm involved with.

This says a _lot_ more about Linus than it does about C++.

# New things in C++ Covered Today

- References
- Auto declarations
- Namespaces
- iostream library
- Exceptions
- Classes and Objects
- Constructors and destructors
- Keywords: `new` instead of `malloc` and `delete` instead of `free`
- Operator overloading
- Templates

Resources

- Reference: [http://www.cplusplus.com/](http://www.cplusplus.com/)
- Books: [https://stackoverflow.com/questions/388242/the-definitive-c-book-guide-and-list](https://stackoverflow.com/questions/388242/the-definitive-c-book-guide-and-list)

# References

Essentially an alias for a variable, references allow you to refer to the same object with a different name. Somewhat confusingly, references are declared using the `&` operation, which in C (and therefore also in C++) is the _dereference_ operator. Here's an example declaration of a reference:

```c++
int x = 1;
int &y = x;
```

Note that we are defining `y` to refer to `x`. You can't just do

```c++
int &z
```

because you are not saying what `z` refers to.

You can now do anything with `y` that you can with `x`. For example,

```c++
x = 3;
y == x; // true
y = 4;
x == y; // true
```

# References are not pointers

- References are much safer!
- They are not used for memory management (like pointers are)!
- There is no "reference arithmetic"
- They are just synonyms

# References in Function Arguments

References are most useful in functions to make an argument _call by reference_ instead of _call by value_. For example, the method `magnitude` defined below takes a const reference to a point.

```c++
typedef struct {
    double x, y, z;
} Point;

double magnitude(const Point &p) {
    return sqrt(p.x*p.x + p.y*p.y + p.z*p.z);
}
```

You can use this method without caring that it is call by reference, as in:

```c++
Point p = { 0,3,4 };
ASSERT_DOUBLE_EQ(magnitude(p), 5);
```

The benefit is speed. If you call by value, then the entire point structure, with its three doubles, is copied to call the function. With call by reference, only a reference is created, which is a much smaller object. Note that almost every modern scripted language, such as Python and Javascript, use call by reference for arrays, hashes, and other objects, because these objects can get quite large and copying them every time you call a function is innefficient.

# Returning a Reference

A function can also return a reference. For example, suppose you want to compare the magnitudes of two points and return the one with a smaller magnitude. Copying all those points is a waste of time and space, so we use references instead. For example, if `smaller` is defined as

```c++
Point &smaller(Point &p, Point &q) {
    if ( magnitude(p) <= magnitude(q) ) {
        return p;
    } else {
        return q;
    }
}
```

then you can write

```c++
Point &r = smaller(p,q);
```

after which `r` will refer to the smaller of the two points. Let's say `p` is smaller. If you then do

```c++
r.x = -1;
```

you will also be setting `p.x`.

# WARNING

Do not return a reference to a locally declared variable. Local variables are deleted from memory when the function completes, so you would be returning a reference to nothing. For example:

```c++
Point &f() {
    Point p = {1,2,3};
    Point &q = p;
    return q;
}
```

will compile just fine, but is meaningless to the point of dangerous.

# Auto Declarations

Many languages allow you to declare variables without specifically saying what type they are. Somewhat late to the game, C++ added this feature in the 2011 specification. For example, if you have a function declared as follows,

```c++
Point * create_point();
```

you can use auto to avoid clutter in your code:

```c++
auto p = create_point();
```

# Namespaces

In C++ the notion of a namespace was introduced to avoid cluttering the global namespace with lots of function and type definitions. One of the main namespaces we will use is `std`, which is the namespace the standard library uses. For example, the C++ method `cout`, which is used for printing (instead of `printf`) is a method declared in the `std` namespace. To use it, you have to do use the `::` operator, which accesses the namespace.

```c++
#include <iostream>

// later ...
std::cout << "Hello world\n";
```

This may look just like added `std_` to the beginning of everything, but you can open a namespace to use its methods for the duration of a function, as in

```c++
void print_stuff() {
    using namespace std;
    cout << "First line\n";
    cout << "Second line\n";
}
```

# Creating Namespaces

Put your library of functions and datatypes in a namespace. In this example, we avoid potential name collisions with "Point" and "mag" that may be defined some other way elsewhere.

```c++
namespace my_library {
    typedef struct {
        double x, y;
    } Point;
    int mag(Point &p) {
        return sqrt(p.x*p.x + p.y*p.y);
    }
}
```

Then use the library

```c++
namespace {
    TEST(Examples, MyLibrary) {
        my_library::Point p = { x: 3, y: 4};
        ASSERT_NEAR(my_library::mag(p), 5, 0.00001);
    }
    TEST(Examples, Using) {
        using namespace my_library;
        Point p = { x: 3, y: 4};
        ASSERT_NEAR(mag(p), 5, 0.00001);
    }
}
```

# cout

By the way, `cout` is an _output stream_ that sends a stream of charaters to the terminal (other streams can do to files, for example). The `<<` operator allows you to join a number of objects together to make a strea. For example,

```c++
std::cout << "The value of x is " << x << "\n";
```

Think of `<<` like a function that converts what comes after it to a string. Thus `<<(x)` should be a string representation of whatever `x` is. the `<iostream>` library defines `<<` for all the standard types, like `int` and `double`.

If you create a new type, however, you have to define `<<` for it.

Later in README we will show how to define `<<` for our array ADT.

# Throwing Exceptions

One of the big difficulties in C is what to do when some exception occurs at run time. Usually, developers handle this by either returning a special value, such as NULL or -1, from a function in which an error occurred, or by setting up an error data structure inside an ADT to keep track of errors (like we did with the rpn calculator).

In C++, however, you can define exception handlers explicitly, as well as either choose to ignore exceptions (so they are passed to the function that calls you function), or handle them explicitly. Here is an example of how to throw an exception

```c++
#include <stdexcept>

double invert(int x, int y) {
    if ( x == 0 ) {
        throw std::invalid_argument("First argument cannot be zero");
    } else {
        return y/x;
    }
}
```

# Catching (and Testing) Exceptions

This code throws an exception in some cases. The particular exception it throws has a type `std::invalid_argument`, which is an ADT defined in the `<stdexcept>` library. There are several other exceptions in the library as well, which you can read about [here](http://www.cplusplus.com/reference/stdexcept/). We will discuss defining new types of exceptions later in the class. To catch this exception, you would do something like:

```c++
try {
    invert(0,1);
    ASSERT_EQ(true,false);
} catch (std::invalid_argument e ) {
    ASSERT_STREQ(e.what(), "First argument cannot be zero");
}
```

# Failing to Catch Exceptions

Note that if you do not catch an exception in a given function, it is passed up to the function that called it, all the way up to the operating system. For example, suppose you had another function

```c++
void show(x,y) {
    std::cout << invert(x,y) << "\n";
}
```

If whatever function calls `show` sends in a zero for the first argument, then `invert` will throw an exception. Since this exception is not handled in `show` it will be passed up the chain to whatever called show. Thus, you can do:

```c++
try {
    show(0,1);
} catch (std::invalid_argument e ) {
   // do something here
}
```

And if you do not look for any exceptions, you will end up with a runtime error.

Conveniently, Google Test will catch all exceptions and the tests that produced them as failures, print out the exception's `what` method, and proceed without crashing the whole program. To use them, just do

# Standard Exceptions

The `<exception>` library defines some standard excptions you can use in your code.

```c++
throw std::invalid_argument("First argument cannot be zero");
```

to throw. All these exceptions define a `what()` method, which you can use when catching.

```c++
catch (std::invalid_argument e ) {
    std::cout << e.what() << "\n";
}
```

**Available Exceptions:**

<pre>
logic_error                    invalid_argument              domain_error, 
length_error                   out_of_range                  future_error(C++11)
bad_optional_access(C++17)     runtime_error                 range_error
overflow_error                 underflow_error               regex_error(C++11)
system_error(C++11)            ios_base::failure(C++11)      filesystem::filesystem_error(C++17), 
tx_exception(TM TS)            nonexistent_local_time(C++20) ambiguous_local_time(C++20), 
format_error(C++20)            bad_typeid                    bad_cast
bad_any_cast(C++17)            bad_weak_ptr(C++11)           bad_function_call(C++11)
bad_alloc                      bad_array_new_length(C++11)   bad_exception
ios_base::failure(until C++11) bad_variant_access(C++17).
</pre>

**Note:** You can also define your own exception class (more on this later in the course).

# Memory Allocation in C++

Memory allocation in C++ is handled by the language, and not by a library of methods as with `malloc`. Two operators, `new` and `delete` handle everything. For example:

```c++
int * x = new int;
double * p = new double[10];

// later ...
delete x;
delete[] p;
```

Note that unlike `malloc`, `new` returns a pointer to a specific type of object.

In the first case an `int` and in the second case an array of `doubles`.

You do not need to cast anything. No `void *` pointers!

The benefit is that C++ will do some type checking for you and report compile time errors that C could not. Also note that when you delete an object, you have to use the corresponding form.

To delete a single object, use `delete` and to delete an array, use `delete[]`.

# Classes and Objects

C++ solves the problem of having objects own their methods with classes. A `class` is a description of an abstract data type's data structures and methods all within one `struct`-like entity. As an example of a class, we can write the array class we have been working on as follows:

```c++
class DoubleArray {

public:

    // Constructors
    DoubleArray();
    DoubleArray(double a, double b, double step);     // replaces the _range method
    DoubleArray(const DoubleArray& other);            // replaces the _copy method

    // Assignment operator
    DoubleArray& operator=(const DoubleArray& other); // new! lets you do array2 = array1;

    // Destructor
    ~DoubleArray();                        // Replaces destroy, deleting the buffer
                                           // and also deletes the array object itself.

    // Getters
    double get(int index) const;           // Notice all our methods now do not
    int size() const;                      // take a pointer to the array as the
                                           // first object. The object is implicit.
                                           // The trailing "const" in some methods says
                                           // the object will not be changed by the
                                           // method.
    // Setters
    void set(int index, double value);

private:

    int capacity,                          // Our data goes here. It is private, meaning
        origin,                            // the user cannot access it.
        end;

    double * buffer;

    const int INITIAL_CAPACITY = 10;       // Instead of a global #define, we use an class
                                           // defined constant, that is only available within
                                           // the class

    int index_to_offset(int index) const;  // No more static methods! These will be available
    int offset_to_index(int offset) const; // to the class, but not the user.
    bool out_of_buffer(int offset) const;
    void extend_buffer(void);

};
```

The above essentially defines the interface to our new class. It says what methods are available and what arguments they take. It also defines some private data that only the class methods will have access to.

# Using Classes to Make Objects

To use the `DoubleArray` class, we would do something like:

```c++
TEST(DoubleArray, Construction) {
    DoubleArray a,             // use the default constructor
                b(0,1,0.1);    // use the range constructor
    a.set(4, 1);               // use the dot operator to access the `set` method
    ASSERT_EQ(b.get(2), 0.2);  // use the dot operator to access the `get` method
}
```

Notice that we do not have to explicitly delete `a` and `b` when we are done with them. This is because when they go out of scope, C++ will automatically call the method `~DoubleArray`, which is specially named with the `~` to let C++ know to call it whenever it needs to delete a `DoubleArray`.

# Allcating new Objects

On the other hand, we still need to use `delete` if we happened to use `new` to allocate a `DoubleArray`. For example,

```c++
auto a = new DoubleArray();
auto b = new DooubleArray(0,1,0.25);

// later ...
delete a, b;
```

Note that when declaring pointers to objects, you can write

```c++
auto c = new DoubleArray; // Compiles, but does not call the constructor! Bad!
```

in which case you get an object of type `DoubleArray` for which the constructor has not been called. Since in our case, the constructor allocated memory, the result of the above code is a `DoubleArray` in which the buffer is uninitialized. Which is bad!

# Constructors

The constructor in a class is responsible for

- initializing variables
- allocating memory (if needed)
  If the objects constructed do not need memory allocation, then C++ provides a default constructor, so you do not need to define one. For example, if you defined class as

```c++
class Complex {
    private:
      double re, im;
};
```

then you can simple make a new Imaginary number with

```c++
Complex x;
```

# Multiple Constructors

You can add constructors that do different initializations. For example, the code

```c++
class Complex {
    public:
      Complex(double a, double b) : re(a), im(b) {};
      Complex(double a) : re(a), im(0) {};
    private:
      double re, im;
};
```

defines two new constructors. They do C++ initialization syntax to initialize the `re` and `im` fields. The first is equivalent to

```c++
Complex(double a, double b) {
    re = a;
    im = b;
}
```

so there is nothing magical about the syntax. Notice that the two constructors have the same name. C++ can tell the difference between them by what arguments are passed. In fact, there is a third constructor, the default constructor, which takes no arguments (as before). Thus, you can write:

```c++
  Complex w, x(), y(1,2), z(3);
```

and C++ will know which constructor to use.

# Memory Allocation in Constructors

If memory needs to be allocated, you must at least override the default constructor (the one that takes no arguments). In the `DoubleArray` class we defined the constructor:

```c++
DoubleArray();
```

which we implement in `dynamic_array.cc` with

```c++
DoubleArray::DoubleArray() {                     // Note the namespace :: operator
    buffer = new double[INITIAL_CAPACITY]();     // We use new instead of calloc
    capacity = INITIAL_CAPACITY;                 // Note that we can refer to member variables
    origin = capacity / 2;                       // like buffer, capacity, origin, and end
    end = origin;                                // implicitly. C++ knows what we mean.
}
```

Note that the contructor does not explictly return a value. In fact, it is called after an object is created either in a variable declaration or when using `new`.

# A Range Constructor

The other constructors can build on this code, without having to do everything again, by using the initialization syntax to initialize the object being constructed using the default constructor (the one with no arguments), and then adding more to the object in the body of the function.

```c++
DoubleArray::DoubleArray(double a, double b, double step) : DoubleArray() {
    int i = 0;
    for ( double x=a; x<=b; x += step ) {
        set(i, x);
        i+=1;
    }
}
```

# Operator Overloading

To declare a binary operator, such as `==`, we do:

```c++
bool operator==(const DoubleArray& a, const DoubleArray& b) {
    if ( a.size() != b.size() ) {
        return false;
    }
    for(int i=0; i<a.size(); i++) {
        if ( a.get(i) != b.get(i) ) {
            return false;
        }
    }
    return true;
}

bool operator!=(const DoubleArray& a, const DoubleArray& b) {
    return !(a==b);
}
```

You can now use ASSERT_EQ with arrays.

```c++
TEST(DoubleArray, Equality) {
    DoubleArray a(0,1,0.1), b(0,1,0.1), c(1,2,0.1);
    ASSERT_EQ(a, b);
    ASSERT_NE(a,c);
}
```

For more details, see [here](https://en.cppreference.com/w/cpp/language/operators).

# Assignment

To support syntax like the following

```c++
  DoubleArray x(0,1,0.1), y;
  y = x;
```

C++ needs to know what it means to assign one `DoubleArray` to have the value of another. What we want `=` to mean is

- delete the buffer in y
- allocate a new buffer in y
- copy the contents of the buffer in x to the buffer in y
- initialize origin, end, etc.

# Overloading "=", a.k.a. the Assignment Operator

```c++
DoubleArray& DoubleArray::operator=(const DoubleArray& other) {
    if ( this != &other) {
        delete[] buffer; // don't forget this or you'll get a memory leak!
        buffer = new double[other.capacity]();
        capacity = other.capacity;
        origin = other.origin;
        end = origin;
        for ( int i=0; i<other.size(); i++) {
            set(i,other.get(i));
        }
    }
    return *this;
}
```

Think of this code as defining a method called `=` that would be called with `set y to be =(x)` (psuedocode).
This method uses the keyword `this`, which refers to the object being operated on. In our C array class, we passed a pointer (usually called `da`) to the object as the first argument to every function. In C++ the pointer is implicit, but can be access if needed with `this`. In the above code, `this` refers to the assignee (the left hand side of `y=x`) and `other` refers to the object being copied. Thus, the first line checks that these two are not the same (if they are, there is nothing to do -- although you could leave this out and truely make a copy).

Also note that the method accesses the private members of `other`, which is okay because this is a class method.

# Returning Referenes

Finally, notice that the method returns a reference to the result of the copy. This is actually not needed to assign the left hand side (that's done in the body of the method), but is needed because an expression like `y=x` itself has a value, which should be a reference to `y`. This is so you can do things like

```c++
y = x = z;
```

which is equivalent to

```c++
y = (x = z);
```

Returning a reference to `*this` is called _method chaining_.

# The Copy Constructor

Using the assignment operator, we can define another constructor, called the copy constructor, as follows:

```c++
DoubleArray::DoubleArray(const DoubleArray& other) : DoubleArray() {
    *this = other;
}
```

It is quite common for these two methods to be intertwined thusly.

This would be used as in

```c++
DoubleArray x;
x.set(0,3.14); // etc
DoubleArray y(x); // sets y to be a copy of x.
```

# The Destructor

The destructor is responsible for deallocating memory.

If none of your constructors allocate memory, then _you do not need a destructor_.

However, if any constructors do allocate memory, then you have to have one. For ``DoubleArray` we just write

```c++
DoubleArray::~DoubleArray() {
    delete[] buffer;
}
```

Now, whenever a `DoubleArray` goes out of scope, its destructor is called. It is also called whenever `delete` is called on a `DoubleArray` made with `new`.

# Getters and Setters

The getters are straightforward. Notice that we can throw an exception instead of a runtime error when exceptions occur.

```c++
double DoubleArray::get(int index) const {
    if (index < 0) {
        throw std::range_error("Negative index in array");
    }
    if ( index >= size() ) {
        return 0;
    } else {
        return buffer[index_to_offset(index)];
    }
}

void DoubleArray::set(int index, double value) {
    if (index < 0) {
        throw std::range_error("Negative index in array");
    }
    while ( out_of_buffer(index_to_offset(index) ) ) {
        extend_buffer();
    }
    buffer[index_to_offset(index)] = value;
    if ( index >= size() ) {
        end = index_to_offset(index+1);
    }
}
```

# Templates

The `DoubleArray` class has the same issues as our C `DynamicArray` ADT did: it only contains doubles. However, C++ provides a powerful way to make new objects that refer to arbitrary types. The approach uses `templates`.

Here is an example of function definition that uses a template to compare two objects:

```c++
template<typename T>
int compare(const T& x, const T& y) {
    if ( x < y ) {
        return -1;
    } else if ( y < x ) {
        return 1;
    } else {
        return 0;
    }
}
```

As long as `<` is defined for the the type `T`, then this template can be used. For example,

```c++
int c = compare(1.0, 2.0)
```

works.

# Comparisons for Defined Classes

```c++
Complex x(1,2), y(3,4);
int c = compare(x,y);
```

will give the error:

```bash
error: no match for 'operator<' (operand types are 'const {anonymous}::Complex' and 'const {anonymous}::Complex')
```

However, if you defined `<` for complex numbers as follows:

```c++
bool operator<(const Complex& a, const Complex& b) {
    return a.magnitude() < b.magnitude();
}
```

Then `compare` will work just fine.

# Templates for the Array Class

To apply templates to our array class, we have to replace references to `double` with a type name:

```c++
template <typename ElementType>                      // ElementType is the type variable
class TypedArray {

public:

    TypedArray();
    TypedArray(const TypedArray& other);

    // Copy constructor
    TypedArray& operator=(const TypedArray& other);

    // Destructor
    ~TypedArray();

    // Getters
    ElementType &get(int index);
    ElementType &safe_get(int index) const;         // This is new. See below.
    int size() const;

    // Setters
    void set(int index, ElementType value);

private:

    int capacity,
        origin,
        end;

    ElementType * buffer;

    const int INITIAL_CAPACITY = 10;

    int index_to_offset(int index) const;
    int offset_to_index(int offset) const;
    bool out_of_buffer(int offset) const;
    void extend_buffer(void);

};
```

# Using Templated Classes

With this definition of array, we can make arrays of anything.

```c++
TypedArray<Point> b;
b.set(0, Point(1,2,3));
b.set(1, Point(2,3,4));
b.set(20, Point(3,4,5));
double r = b.get(20).x;
```

Or, for an even nicer example, using arrays of arrays of doubles, we can make a matrix:

```c++
TypedArray<TypedArray<double>> m;

for (int i=0; i<30; i++) {
    for (int j=0; j<30; j++) {
        m.get(i).set(j,0.5*i*j);
    }
}
```

# Where to put Template Methods

<span style="background: yellow; padding: 10px">Template methods need to appear in header files.</span>

This is because the compiler can only compile things like `TypedArray<int>` and not arbitrary typed arrays. Thus, any file that uses a templated class needs to include all the templated methods, and not just the class definition, because compiling it will require instantiating the template on the specific types used in the file.

See `week_4/typed_array/typed_array.h`.

Note that there is no `typed_array.c` file!

# Memory Allocation with Arbitrary Types

The main difference between `DoubleArray` and `TypedArray` has to do with the buffer. For example, in the constructor, we can still use `new`, but we also have to make sure to call the constructor for whatever class `ElementType` stands for, which we do with with the parentheses at the end of the statement.

```c++
template <typename ElementType>
TypedArray<ElementType>::TypedArray() {
    buffer = new ElementType[INITIAL_CAPACITY](); // Note: Calls ElementType constructor
    capacity = INITIAL_CAPACITY;
    origin = capacity / 2;
    end = origin;
}
```

The same line shows up in the assignment operator, and a similar line shows up in the private method `extend_buffer`.

# Setting calls the copy constructor (if there is one)

With DoubleArray, setting a location in the array to a value copies the value, because that's how assignment works for `double`.

But with arbitrary element types, assignment calls the copy constructor.

```c++
template <typename ElementType>
void TypedArray<ElementType>::set(int index, ElementType value) {
    if (index < 0) {
        throw std::range_error("Negative index in array");
    }
    while ( out_of_buffer(index_to_offset(index) ) ) {
        extend_buffer();
    }
    buffer[index_to_offset(index)] = value; // Should call the copy
                                            // constructor of ElementType.
    if ( index >= size() ) {
        end = index_to_offset(index+1);
    }
}
```

We can check for the right behavior in a test.

```c++
TEST(TypedArray,CopyElementsInSet2) {
    TypedArray<TypedArray<double>> m;
    TypedArray<double> x;
    x.set(0,0);
    m.set(0,x);
    x.set(0,-1);
    EXPECT_DOUBLE_EQ(m.get(0).get(0),0.0); // If set didn't make a copy
                                            // then we would expect m[0][0]
                                            // to be x[0], which we changed
                                            // to -1.
}
```

# Default Values

Another issue is this: In the `DoubleArray` class, we used zero as the default value when the user asked for an element outside the size of the array. To do that here, we need a "zero" for any object.

One idea is to use the default constructor of the supplied type to create a new element to return in such cases, and also to extend pad array with default values all the way up to the newly accessed index:

```c++
template <typename ElementType>
ElementType &TypedArray<ElementType>::get(int index) {
    if (index < 0) {
        throw std::range_error("Out of range index in array");
    }
    if ( index >= size() ) {
                          // User is access beyond the end of the array,
                          // so return the default value.
        ElementType x;    // A temporary value to use with set.
        set(index, x);    // `set` will create a copy of the default values.
    }
    return buffer[index_to_offset(index)];
}
```

# Get Returns a Reference

For the line

```c++
m.get(i).set(j,0.5*i*j);
```

to work, the call to `get` cannot copy the array at index `i` in `m`, because then we would be changing a copy of the array when we called `set` after that. Thus, `get` should return a reference. Also, note that because `get` might actually change the array, the array iteself is not guaranteed to be the same after the get.

However, we may have other methods that expect getting an element at a particular index to leave the array unchanged, for example, when copying the array. Thus, we add a new method called `safe_get`:

```c++
template <typename ElementType>
ElementType &TypedArray<ElementType>::safe_get(int index) const {
    if (index < 0 || index >= size() ) {
        throw std::range_error("Out of range index in array");
    }
    return buffer[index_to_offset(index)];
}
```

Note the extra `const` at the end of the declaration. A `const` in that position refers to `this`, that is, the object that owns the method. Since the object itself is not an argument (it is only so implicitly), C++ uses this syntax to require the method to not change the underlying array data. This is required when copying an array using the assignment operator, because the argument `other` is specified to be a const.

# Using safe_get

An example where `safe_get` is needed is in the assignment operator.

```c++
template <typename ElementType>
TypedArray<ElementType>& TypedArray<ElementType>::operator=(const TypedArray<ElementType>& other) {
    if ( this != &other) {
        delete[] buffer; // don't forget this or you'll get a memory leak!
        buffer = new ElementType[other.capacity]();
        capacity = other.capacity;
        origin = other.origin;
        end = origin;
        for ( int i=0; i<other.size(); i++) {
            set(i,other.safe_get(i));
        }
    }
    return *this;
}
```

In the above method, if we replace `safe_get` with just `get` we get the (extremely helpful) compile time error:

```bash
error: passing 'const TypedArray<double>' as 'this' argument of
'ElementType& TypedArray<ElementType>::get(int) [with ElementType = double]'
discards qualifiers [-fpermissive]
```

The term "discards qualifiers" refers to the `const` after the method declaration. The last bit is a suggestion that you compile with the `g++` flag `-fpermissive` which will let you let away with such things. However, that is a bad idea. Instead, you should try to understand what is happening and come up with a solution like we did with `safe_get`.

# Interfacing with Streams

To print arrays, we can overload the `<<` operator:

```c++
template <typename ElementType>
std::ostream &operator<<(std::ostream &os, TypedArray<ElementType> &array)
{
    os << '[';
    for (int i=0; i<array.size(); i++ ) {
        os << array.get(i);
        if ( i < array.size() - 1 ) {
            os << ",";
        }
    }
    os << ']';
    return os;
}
```

This method is not declared inside the class definition, since it is not owned by `TypedArray`. In any case, now we can write:

```c++
TypedArray<TypedArray<double>> m;

for (int i=0; i<3; i++) {
    for (int j=0; j<3; j++) {
        m.get(i).set(j,0.5*i*j);
    }
}

std::cout << m << "\n";
```

which yields

```bash
[[0,0,0],[0,0.5,1],[0,1,2]]
```

The `<<` operator is called nine times. Three times with `TypedArra<double>` arguments, and for each of those times, three more times for `double` arguments. Since the `iostream` library defines `<<` for doubles, we do not need to do that. If we had an array of array of Imaginary numbers, for example, we would need to define `<<` for them as well.

# Summary

This week, we learned to take an abstract data type written in C and rewrite it in C++. We learned to use the C++ notions of references, namespaces, exception handlers, overloading functions and operators, classes, constructors, destructors, and templates.

C++ has many other additions beyond what we have covered here. For example, a class can inherit all the methods of another class, but also add more methods. Classes can have "friends". You can overload all the operators. There is also an extensive "standard template library" which we will introduce next week. We will touch on a few other aspects of C++, in future lectures, but the subset of C++ we have defined here is fairly complete.

It is important to understand that C++ is a huge, messy language with many ways to do the same thing. Some very bad programs have been written in C++ (and in pretty much ever other language). This document is meant to introduce C++ but also to introduce conventions. For example, every class you write that does memory allocation should have a default constructor, a copy constructor, an assignment operator, and a destructor. As you delve deeper into C++, study good code examples and try to use their conventions. Although it is easy to make up a new one, doing so will confuse your collaborators and possibly even you.

A good book on C++ is the "C++ Primer" by Stanley Lippman. Consider reading as much of this book as you can, especially if you expect to be writing C++ programs for your job. There is a lot more covered there than is covered in that class.
