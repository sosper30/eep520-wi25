# Software Engineering For Embedded Applications

## Week 6 (February 06, 2023): Standard Template Libary

**Last time:**

- Classes and objects in C++
- Constructors in C++
- Operator overloading
- Copy constructors
- Destructors
- Templates

**Today:**

- Standard Template Library (STL)
  - Sequential containers
  - Vector
  - String
  - Foreach loop
- Iterators
- Generic algorithms
- Lambda expressions
- Callbacks
- Associative containers

This week, we continue our investigation of the C++ Standard Template Library (STL). This library includes varitions on templated, dynamic, sequential containers such as queues, lists, and arrays. It also includes maps, sets, and a set of generic algorithms we can use to operate on these objects. Most C++ programs make heavy use of these templates and algorithms, avoiding dynamic memory management as much as possible. These week we will introduce these templates at a high level. The complete documentation for the STL can be found at [http://www.cplusplus.com/reference/stl/](http://www.cplusplus.com/reference/stl/).

Next week, we will begin to focus on the types and classes needed to represent common tasks in embedded systems. We will introduce classes for sensors, processes, a process manager, and a scheduler. Together, these classes will allow us to write complex, event driven, reactive systems that are at the heart of embedded systems and robotics control systems. We will build the classes and modules we need to implement these ideas almost exclusively using the STL.

# Resources

- Reference: [http://www.cplusplus.com/](http://www.cplusplus.com/)
- Books: [https://stackoverflow.com/questions/388242/the-definitive-c-book-guide-and-list](https://stackoverflow.com/questions/388242/the-definitive-c-book-guide-and-list)

# Review: Classes and Objects

A `class` is a description of an abstract data type's data structures and methods all within one `struct`-like entity. As an example of a class, we can write the array class we have been working on as follows:

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

    const int INITIAL_CAPACITY = 10;       // Instead of a global #define, we use a class
                                           // defined constant, that is only available within
                                           // the class

    int index_to_offset(int index) const;  // No more static methods! These will be available
    int offset_to_index(int offset) const; // to the class, but not the user.
    bool out_of_buffer(int offset) const;
    void extend_buffer(void);

};
```

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

# Constructors

The constructor in a class is responsible for

- initializing variables
- allocating memory (if needed)
  If the objects constructed do not need memory allocation, then C++ provides a default constructor, so you do not need to define one. For example, if you defined a class as

```c++
class Complex {
    private:
      double re, im;
};
```

then you can simply make a new complex number with

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

# Range Constructor

The other constructors can build on this code, without having to do everything again, by using the initialization syntax to initialize the object being constructed using the default constructor (the one with no arguments), and then adding more to the object in the body of the function.

```c++
DoubleArray::DoubleArray(double a, double b, double step) : DoubleArray() {
    int i = 0;
    for (double x = a; x <= b; x += step, ++i) {
        set(i, x);
    }
}
```

# Operator Overloading

To declare a binary operator, such as `==`, we do:

```c++
bool operator==(const DoubleArray& a, const DoubleArray& b) {
    if (a.size() != b.size()) {
        return false;
    }
    for(int i = 0; i < a.size(); i++) {
        if (a.get(i) != b.get(i)) {
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
    ASSERT_NE(a, c);
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
    if (this != &other) {
        delete[] buffer; // don't forget this or you'll get a memory leak!
        buffer = new double[other.capacity]();
        capacity = other.capacity;
        origin = other.origin;
        end = origin;
        for (int i = 0; i < other.size(); i++) {
            set(i, other.get(i));
        }
    }
    return *this;
}
```

Think of this code as defining a method called `=` that would be called with `set y to be =(x)` (pseudocode).
This method uses the keyword `this`, which refers to the object being operated on. In our C array class, we passed a pointer (usually called `da`) to the object as the first argument to every function. In C++ the pointer is implicit, but can be access if needed with `this`. In the above code, `this` refers to the assignee (the left hand side of `y=x`) and `other` refers to the object being copied. Thus, the first line checks that these two are not the same (if they are, there is nothing to do -- although you could leave this out and truely make a copy).

Also note that the method accesses the private members of `other`, which is okay because this is a class method.

# Returning Referenes

Finally, notice that the method returns a reference to the result of the copy. This is actually not needed to assign the left hand side (that's done in the body of the method), but is needed because an expression like `y = x` itself has a value, which should be a reference to `y`. This is so you can do things like

```c++
y = x = z;
```

which is equivalent to

```c++
y = (x = z);
```

Returning a reference to `*this` is called _method chaining_.

## Without Method Chaining

Here is an example.

```c++
class Arith {

    public:

    Arith() : _value(0) {}

    void &zero()          { _value  = 0; }
    void &add(int x)      { _value += x; }
    void &subtract(int x) { _value -= x; }
    void &multiply(int x) { _value *= x; }
    void &divide(int x)   { _value /= x; }

    int value() { return _value; }

    private:

    int _value;

};
```

Without method chaining you would have to write

```c++
Arith a;
a.add(1);
a.subtract(2);
ASSERT(a.value(), -1);
```

## With Method Chaining

Here is an example.

```c++
class Arith {

    public:

    Arith() : _value(0) {}

    Arith &zero()          { _value  = 0; return *this; }
    Arith &add(int x)      { _value += x; return *this; }
    Arith &subtract(int x) { _value -= x; return *this; }
    Arith &multiply(int x) { _value *= x; return *this; }
    Arith &divide(int x)   { _value /= x; return *this; }

    int value() { return _value; }

    private:

    int _value;

};
```

With method chaining you can do

```c++
Arith a;
ASSERT(a.add(1).subtract(2).value(),-1);
```

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

# Standard Template Library

## Sequential Containers

===

A _container class_ is one that contains some other class. Our `TypedArray` class is an example. A _sequential_ class is one in which the elements are stored and access sequentially, with indices 1, 2, ... and so on. The STL includes several templated sequential containers:

- `vector`: Like our `TypedArray` class, except you can't `get` elements at an index until something has been put at that index.
- `string`: A vector of characters with some special operations for manipulating strings.
- `array`: A fixed-size array whose size must be declared at compile time. No dynamic memory allocation, so possibly faster.
- `deque`: A double ended queue. Like a `vector` but optimized for adding elements to both ends (like our `TypedArray`).
- `list`: A doubly linked list.
- `forward_list`: A singly linked list.
  The most commonly used containers are `vector` and `string`, which we will cover here. The rest of the containers have fairly specialized uses, and many guides and textbooks advise that you simply don't use them, or that you prototype with 'vector' and 'string' and only switch to one of the more specialized classes if you can show it improves performance.

In the following notes, we will use `vector` and `string` for all of our examples. Many of the methods that act on these objects also act on the other ones.

# Constructors

To use the `vector` container, you need to first include it:

```c++
#include <vector>
using std::vector;
```

To make a new `vector`, you use the template sytnax we've already learned. For example

```c++
vector<double> t;          // vector of doubles
vector<Imaginary> u;       // vector of Imaginaries
vector<vector<double>> v;  // essentially a matrix of doubles
```

You can also initialize a `vector` when constructing it.

```c++
vector<int> t = { 1, 2, 3 };  // vector of three ints
vector<double> u(10,3.14);    // vector of 10 3.14s
vector<double> v = u;         // copy constructor with assignment operator
```

# Strings

In the STL, a `string` is a bit more than just a pointer to a `char` as with C. In fact, a `string` is a first class objects that is quite similar to a `vector`. Strings can be constructed and initialized similarly.

```c++
string s;
string t = "hello";
string u = t;
```

Note that the following code

```c++
vector<string> v = { "embedded", "systems", "are", "cool" };
```

involves the constructor for the `string` class, which takes a `char *` as an argument. Thus, although it may look like this snippet makes an array of `char *` values, it actually builds a `vector` of strings.

# Common Methods

Vectors and strings have many methods in common, since they are based on essentially the same code. You can test if they are empty, find their sizes, compare them, and get their individual elements. So for vectors, you can write:

```c++
vector<int> v = { 1, 2, 3 };
v.push_back(4);
v[0];      // 1
v.back();  // 3
v.front(); // 1
v.empty(); // false
v.size();  // 4
v == v;    // true
v > v;     // false
v.pop_back();   // changes the array to { 1, 2 } (returns void)
```

and for strings, you can write

```c++
vector<string> s = {"a", "b", "c"};
s.push_back("d");
s[0];      // 'a'
s.back();  // 'c'
s.front(); // 'a'
s.empty(); // false
s.size();  // 4
s == s;    // true
s > s;     // false
v.pop_back();   // changes the array to { 1, 2 } (returns void)
```

The STL also defines the methods

```c++
s.clear();                    // clears the string
s.insert(s.begin()+1, 'x');   // insert an 'x' into position 1
q.erase(5,2);                 // remove two elements starting at position 5, reindexing subsequent elements
```

# Emplace

One method that can be useful is called `emplace`. It is used to construct new elements of the base type of, for example, a `vector`. This is needed because constructing elements and then pushing them into the vector can be cumbersome. Thus, if you have a class `Imaginary` with a constructor that takes two arguments, the real part and the imaginary part, you can do

```c++
vector<Imaginary> r;
r.emplace(r.begin(),2.0,3.0);
```

which creates a `vector` whose first element is the imaginary numer `2+3i`.

# Additional For Loop Form

A new for loop form is available in C++ as well, allowing you to iterate over the elements in a sequential container easily. For example, to iterate over the characters in a string, you can do:

```c++
for ( char c : s ) {
    cout << c << "\n";
}
```

Or for a vector, you can write

```c++
vector<string> v = { "i", "am", "on", "a", "very", "bumpy", "flight" };
for (string s : v ) {
    cout << s << "!\n";
}
```

Note that the `for` pattern can request either a copy of the element, or a reference to it. You might want a copy if you are going to change the elements as you iterate through them, as in the following code:

```c++
vector<string> v = { "i", "am", "on", "a", "very", "bumpy", "flight" };
for (string &s : v ) { //&s - reference to s
    s.push_back('!');
}
```

The new `for` pattern uses the `:` operator. The sequential containers overload this operator to make this possible. If you define a new class, you can [overload this operator](https://stackoverflow.com/questions/8164567/how-to-make-my-custom-type-to-work-with-range-based-for-loops) for your new class as well.

# String Methods

Because strings are made of characters, it is often helpful to know the methods that are available for manipulating them.

```c++
int isalnum(int c);
int isalpha(int c);
int isblank(int c);
int iscntrl(int c);
int isdigit(int c);
int isgraph(int c);
int islower(int c);
int isprint(int c);
int ispunct(int c);
int isspace(int c);
int isupper(int c);
int isxdigit(int c);
int tolower(int c);
int toupper(int c);
```

Their names should be self-explanatory. These are in the library `<cctype>`.

For example, you can use the `toupper` method to convert a string to all caps:

```c++
for ( char &c : q ) {
    c = toupper(c);
}
```

# More String Methods

The string container also defines a number of string specific methods, such as `assert`, `append`, `replace`, `substr`, and `find` whose definitions are mainly self-explanatory. There are also a number of methods for converting between strings and numbers, such as `to_string` and `atoi`.

Interestingly, since vectors and strings are dynamic classes much like the `TypedArray` class we built earily, they have a capacity, which you can find with a simple method

```c++
v.capacity(); // some number bigger than v.size();
```

Also, if you need to keep the amount of space a sequential container takes up to a minimum, you can do:

```c++
v.shrink_to_fit();
```

# Iterators

Before the `for(x:v)` form was introduced, the way to iterate through containers like `vector` and `string` was to use an `iterator`. For example, the following code also changes all the characters in a string to caps:

```c++
for ( string::iterator i = q.begin(); i != q.end(); i++ ) {
    *i = tolower(*i);
}
```

Here, `q.begin()` returns an iterator that refers to the element at the beginning of the array, while `q.end()` refers to a non-element that is "one past" the end of the array. The notation `i++` is overloaded for iterators to mean "increment the iterator". Finally, the '\*' operator is overloaded to return a reference to the element the iterator refers to.

# Iterators with auto

Iterators are a good place to use the `auto` keyword. For example, an iterator to a vector of vector of doubles would be declared and initialized by

```c++
vector<vector<double>>::iterator i = v.begin();
```

and it is a lot easier (and clearer) to write

```c++
auto i = v.begin();
```

I suspect that usages similar to this one were the reason the `auto` keyword was introduced in the first place.

# Manipulating Indices

Iterators can be manipulated like indices into normal C arrays, via overloaded operators for `+`, `-`, `<`, and so on. For example

```c++
auto i = v.begin();
auto j = i + v.size() / 2;
i < j; // true
```

It should be noted that iterators are _not_ numbers. For example, if you do `cout << i`, you will get a compiler error because `i` is not some kind of number for which `<<` is defined, and furthermore, `<<` is not defined for iterators.

**Note:** If you would like to access the elements of a `vector` or `string` in reverse, you can use `v.rbegin()` and `v.rend()` methods. If a `vector` or `string` was declared `const`, you can use `v.cbegin()` and `v.cend()`, which return `const_iterators` instead of `iterators` (just use `auto` if you are unsure).

# Tuples

Sometimes you just need to hold a few values in a tuple. For example, you might want two integers, or a string and a double. Here is how you do that in C++ with a [tuple](https://en.cppreference.com/w/cpp/utility/tuple).

```c++
#include <tuple>
#include <string>

...

using std;
tuple<int, int> x = make_tuple(123,234);
int a = get<0>(x); // 123
int b = get<1>(x): // 234
```

Notice the interesting use of templates in defining the type of `get`. In fact, `get` has something like this signature:

```c++
template< int I, class... Types >
typename std::tuple_element<I, tuple<Types...> >::type const&&
    get( const tuple<Types...>&& t );
```

It uses a size a plain old integer `I` in the template so that it can declare getters for tuples of any size with only one definition.

Please don't ask me what the `&&` operator does. Unless you would like to go on an hour long tangent about move semantics.

# Generic Algorithms

The STL uses function polymorphism to let you apply simple algorithms, such as finding and sorting, to pretty much any sequential container. One such method is called `find`. Here are a few examples:

```c++
vector<string> v = { "The", "quick", "brown", "fox" };
string s = "this is a string";
int a[10] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

auto f1 = find(v.begin(), v.end(), "quick");
cout << *f1; // "quick"

auto f2 = find(s.begin(), s.end(), 'i');
cout << *f2; // `i`

auto f3 = find(begin(a), end(a), 6);
cout << *f3; // 6
```

In the first two cases, `find` is using the `begin` and `end` methods of the `vector` and `string` containers. In the last case, we use the global methods `begin` and `end` to construct new iterators for the C style array `a`, which doesn't have any methods of its own.

The `find` algorithm works on _any_ object that defines `==`. Other algorithms, such as `sort` work on any object that defines `<`. That's whjy they are called _generic_.

# More Read Only Algorithms

Here are a few more _read only_ algorithms (ones that do not change their arguments) like find:

```c++
vector<int> v = { 1, 2, 3, 4, 5 },
            w = { 2,3,4,5,6,6,7,8 };
accumulate(v.begin(), v.end(), 0);    // sum of elements in v
count(v.begin(), v.end(), 3);         // number of occurences of 3 in v
equal(v.begin(), v.end(), w.begin()); // true if first v.size() elements are equal to the
                                      // first v.size() elements of w, which must have equal
                                      // or greater length
```

These work equally well on any sequential container of values for which operations like `==` and `+` are defined.

Algorithms that change the underlying array, include things like `sort` and `fill`:

```c++
sort(v.begin(), v.end());                 // sort the elements
fill(v.begin(), v.end(), 1);              // replace element with third argument
fill_n(v.begin(), 3, 1);                  // replace three elements at the beginning with 1
copy(v.begin(), v.end(), w.begin() + 3);  // copy v into w starting at position 3
replace(v.begin(), v.end(), 3,-1);        // replace element at position 3 with -1
```

Further documentation on generic algorithms can be found [here](http://www.cplusplus.com/reference/algorithm/).

# Lambda Expressions

You can define functions as in two ways in C++. The first is the standard way, as in this example which squares its argument

```c++
double square(double x) {
    return x*x;
}
```

The other is with what are called `lambda` expressions, named after the _lambda calculus_ invented by Alonzo Church in the 1930s as a way to capture the essence of computation mathematically. A lambda expression defining the same function as above is:

```c++
[](double x) { return x*x; }
```

It is the same function, but it doesn't have a name. You can use evaluate lambda expressions by applying them like functions, as in:

```c++
([](double x) { return x*x; })(3.0); // 9.0
```

or you can set a variable equal to a lambda expression, and apply it later, as in

```c++
auto square = [](double x) { return x*x; }
square(3.0); // 9.0
```

# Using Lambdas with Generic Algorithms

Lambdas becomes useful when using generic algorithms, such as `transform`, which take functions as arguments:

```c++
vector<int> v = { 1, 2, 3, 4, 5 };
transform(
    v.begin(),  // start of input elements to be transformed
    v.end(),    // end of input elements to be transformed
    v.begin(),  // output location where transformed inputs are written
    [](double x) { return x*x; }  // function that transforms inputs and write into output locations
);
```

Note that lambda expressions do not return function pointers. Rather, they return function _objects_, which are instantiations of a _function_ class which overrides the `()` application operator.

# Capturing Variables

The square brackets of a lambda expression can list variables to _capture_ from the surrounding scope. For example, if we define a function like this:

```c++
void add_to_all(vector<int>& v, int x) {
    transform(
        v.begin(),
        v.end(),
        v.begin(),
        [](int y) { return x + y; } // WRONG!
    );
}
```

the C++ compiler will complain that the lambda expression does not have access to the variable `x` in its scope. We can fix this by _capturing_ `x` as follows:

```c++
void add_to_all(vector<int>& v, int x) {
    transform(
        v.begin(),
        v.end(),
        v.begin(),
        [x](int y) { return x + y; } // RIGHT!
    );
}
```

# Callbacks

Sometimes you want to pass a function as a callback function to another function. For example, you might do

```c++
sleep_then(10, []() { cout << "Ten seconds have passed\n"; });
cout << "I am waiting\n";
```

If the sleep_then() function is executed apart from the main program flow in another thread, i.e. waiting for ten seconds and then running its callback, then the main program flow continues after calling sleep_then() and you would get the output:

```
I am waiting
Ten seconds have passed
```

We will get to code like a `sleep_then()` function in a few weeks.

# Example Lambda as Argument

```c++
std::vector<int> map(std::vector<int> v, std::function< int(int) > f) {
    std::vector<int> result;
    for ( auto x : v ) {
        result.push_back(f(x));
    }
    return result;
}

TEST(Lambda, Argument) {
    std::vector<int> v = { 1,2,3,4,5 };
    auto f = [](int x) { return x*x; };
    std::vector<int> u = map(v, f);
    for ( auto x : u ) {
        std::cout << x << " ";
    }
    std::cout << "\n";
}
```

# Using Templates for Function Arguments

template<typename FunctionType>
...

# More Info

Further reading on lambda expressions can be found [here](https://docs.microsoft.com/en-us/cpp/cpp/lambda-expressions-in-cpp?view=vs-2017).

# Associative Containers

Associative containers are different from sequential containers in that they index the container with keys, kind of like a `struct`, dictionary, or database. The associative containers defined in the STL are 'map', 'set', 'multimap', and 'multiset'. We will focus here on 'map', which can be used to illustrate the main features.

In a `map`, a set of keys are used to index a set of values. For example, you might define a `map` as follows:

```c++
   map<string,string> person;
   person["First"] = "Alan";
   person["Last"] = "Turing";
   person["job"] = "Code breaker";
```

Note that the `map` template takes two types, the key type and the value type. They do not have to be the same:

```c++
    map<string, vector<int>> sequence;
    sequence["ints"] = { 1, 2, 3, 4, 5, 6 };
    sequence ["squares"] = { 1, 4, 9, 16, 25, 36 };
    sequence["fib"] = { 1, 1, 2, 3, 5, 8, 13 };
```

or another (inefficient) way to make an array of doubles

```c++
map<unsigned int, double> a;
a[0] = 123;
a[5] = 34;
int x = a[3]; // 0. Non-existent keys map to default values of the value type
```

Some implementations of Javascript actually look something like this for arrays.

# Map Iterators

You can iterate through the keys and values of a `map` with iterators. The order in which the iterators visit the elements of the map is not guaranteed, so do not depend on it. For example, using the definition of `m` above,

```c++
for(auto i = person.begin(); i != person.end(); i++) {
    cout << (*i).first << ": " << (*i).second << "\n";
}
```

which can also be written

```c++
for(auto i = person.begin(); i != person.end(); i++) {
    cout << i->first << ": " << i->second << "\n";
}
```

When you dereference an iterator in a `map` you get a `pair` object (also defined by the STL). It will have the same types as the map. In the above example, `*i` as the type `pair<string,string>`. The pair object has a `first` and `second` field that references to the actual key and value of the map to which the iterator refers.

You can erase key/value pairs from a map using either keys or iterators.

```c++
m.erase("First");
m.erase(m.begin());
```

More information about map containers can be found [here](http://www.cplusplus.com/reference/map/map/).

# DB Example

To illustrate how to use the various STL containers, we will build a simple database class for storing information about celestial bodies. The class will support:

- Construction of a new database
- Insertion of a new planet name, mass (in earths) and distance (in astronomical units).
- Assigning entries unique keys
- Deletion of a database entry
- Finding an entry by key
- Search the database with a lambda

The code for this example is in `examples/db.cc`, `examples/db.h`, and `examples/db_tests.cc`.

# DB Example: Public Methods

The methods described above are declared formally below.

```c++
class DB {

    // Rows in the database will be of the form
    // [key,name,mass,distance]
    typedef tuple<int,string,double,double> Row;

    DB();
    DB &insert(const string, double, double);
    DB &drop(int);
    Row find(int) const;
    vector<Row> where(function<bool(const Row)> f) const;

};
```

Note the `typedef` allows us to avoid writing out the entire `tuple` definition over and over again.

Also note the use of `const` on the string argument, and in `find` and `where`.

# DB Example: Underlying Implementation

Hidden from the user, we need to actually store that data somewhere. One way to do that is with an associative map that makes integer keys to tuple values.

We will also need a counter for the keys.

```c++
class DB {

  // ...

  private:

    typedef tuple<string,double,double> Value;
    map<int,Value> _data;
    int _next_key;

};
```

Note that it is customary to prefix private variables with an underscore.

# DB Example: The constructor

In `db.cc` we implement all the methods, starting with the constructor, which just sets the key counter to zero. Since we are using the STL library, we do not need to do any memory allocation.

```c++
DB::DB() : _next_key(0) {}
```

# DB Example: Insertion

To insert an entry, we compute a key, then map that key to a new tuple that contains the value.

Note we return `*this` so we can method chain.

```c++
DB &DB::insert(const string name, double mass, double distance) {

    int key = _next_key++;
    _data[key] = make_tuple(name, mass, distance);
    return *this;

}
```

# DB Example: Drop

To drop the entry corresponding to a given key, we use map's `find` method. It returns an iterator, which we have to make sure is not pointing to `end()`.

Note we return `*this` so we can method chain.

```c++
DB &DB::drop(int key) {

    auto e = _data.find(key);

    if ( e != _data.end() ) {
        _data.erase(e);
    }

    return *this;
}
```

# DB Example: Find

To find an entry with a given key, we use map's `find` again. If no such entry is found, an exception is thrown.

```c++
DB::Row DB::find(int key) const {

    auto e = _data.find(key);

    if ( e != _data.end() ) {
        return to_row(key,e->second);
    } else {
        throw runtime_error("Could not find an entry with the given key");
    }

}
```

This method has to convert the tuple found in the map to a row so it can return it to the user. To make this cleaner, we have defined a private `to_row` function as follows:

```c++
DB::Row DB::to_row(int key, const DB::Value value) const {
    return make_tuple(key, get<0>(value), get<1>(value), get<2>(value));
}
```

Note the use of tuple's templated `get` method.

# DB Example: Where

Where is pretty interesting. We have to collect all the rows for which the supplied function evaluates to true.

```c++
vector<DB::Row> DB::where(function<bool(const DB::Row)> f) const {

    vector<Row> rows;

    for( auto [key, value] : _data ) {
        auto row = to_row(key,value);
        if ( f(row) == true ) {
            rows.push_back(row);
        }
    }

    return rows;

}
```

# DB Example: Testing

Here are a few tests.

```c++
TEST(DB,Basics) {

    DB db;

    db.insert("earth", 1, 1)
        .insert("mars", 0.11, 1.524)
        .insert("moon", 0.011, 1.05)
        .insert("exoplanet one", 1, 1054.4)
        .insert("jupiter", 318, 5.2);

    ASSERT_EQ(NAME(db.find(0)), "earth");

    auto rows = db.where([](DB::Row row) { return  MASS(row) < 1; });

    ASSERT_EQ(rows.size(), 2);

    try {
        db.drop(2)
            .find(2);
        FAIL();
    } catch ( runtime_error e ) {
        ASSERT_STREQ(e.what(), "Could not find an entry with the given key");
    }

}
```

Note that the macros KEY, NAME, MASS, and DISTANCE have been defined for convenience:

```c++
#define KEY get<0>
#define NAME get<1>
#define MASS get<2>
#define DISTANCE get<3>
```
