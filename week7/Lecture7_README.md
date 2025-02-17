# Software Engineering For Embedded Applications

### Review: Overloading "=", a.k.a. the Assignment Operator

```c++
DoubleArray a, b;
a = b; a.assign(b);

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

Think of this code as defining a method called `=` that would be called with `set y to be =(x)` (psuedocode).
This method uses the keyword `this`, which refers to the object being operated on. In our C array class, we passed a pointer (usually called `da`) to the object as the first argument to every function. In C++ the pointer is implicit, but can be access if needed with `this`. In the above code, `this` refers to the assignee (the left hand side of `y = x`) and `other` refers to the object being copied. Thus, the first line checks that these two are not the same (if they are, there is nothing to do -- although you could leave this out and truely make a copy).

Also note that the method accesses the private members of `other`, which is okay because this is a class method.

### Review: Returning Referenes

Notice that the method returns a reference to the result of the copy. This is actually not needed to assign the left hand side (that's done in the body of the method), but is needed because an expression like `y = x` itself has a value, which should be a reference to `y`. This is so you can do things like

```c++
y = x = z;
```

which is equivalent to

```c++
y = (x = z);
```

Returning a reference to `*this` is called _method chaining_.

# Generic Algorithms

The STL uses function polymorphism to let you apply simple algorithms, such as finding and sorting, to pretty much any sequential container. One such method is called `find`. Here are a few examples:

```c++
using namespace std;

vector<string> v = {"The", "quick", "brown", "fox"};
string s = "this is a string";
int a[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

auto f1 = find(v.begin(), v.end(), "quick");
cout << *f1; // "quick"

auto f2 = find(s.begin(), s.end(), 'i');
cout << *f2; // `i`

auto f3 = find(begin(a), end(a), 6);
cout << *f3; // 6
```

In the first two cases, `find` is using the `begin` and `end` methods of the `vector` and `string` containers. In the last case, we use the global methods `begin` and `end` to construct new iterators for the C style array `a`, which doesn't have any methods of its own.

The `find` algorithm works on _any_ object that defines `==`. Other algorithms, such as `sort` works on any object that defines `<`. That's why they are called _generic_.

# More Read-Only Algorithms

Here are a few more _read only_ algorithms (ones that do not change their arguments) like find:

```c++
using namespace std;

vector<int> v = {1, 2, 3, 4, 5},
            w = {2, 3, 4, 5, 6, 6, 7, 8};
accumulate(v.begin(), v.end(), 0);    // add all elements in v to initial sum (== 0)
count(v.begin(), v.end(), 3);         // number of occurences of 3 in v
equal(v.begin(), v.end(), w.begin()); // true if first v.size() elements are equal to the
                                      // first v.size() elements of w, which must have equal
                                      // or greater length
```

These work equally well on any sequential container of values for which operations like `==` and `+` are defined.

Algorithms that change the underlying array, include things like `sort` and `fill`:

```c++
using namespace std;

sort(v.begin(), v.end());                 // sort the elements
fill(v.begin(), v.end(), 1);              // replace element with third argument (== 1)
fill_n(v.begin(), 3, 1);                  // replace three elements at the beginning with 1
copy(v.begin(), v.end(), w.begin() + 3);  // copy v into w starting at position 3
replace(v.begin(), v.end(), 3, -1);        // replace element at position 3 with -1
```

Further documentation on generic algorithms can be found [here](http://www.cplusplus.com/reference/algorithm/).

# Lambda Expressions

You can define functions in two ways in C++. The first is the standard way, as in this example which squares its argument

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
using namespace std;

vector<int> v = { 1, 2, 3, 4, 5 };
transform(
    v.begin(),    // start of elements to transform
    v.end(),      // end of elements to transform
    v.begin(),    // start of where to put elements to transform
    [](double x) { return x*x; }
);
```

Note that lambda expressions do not return function pointers. Rather, they return function _objects_, which are instantiations of a _function_ class which overrides the `()` application operator.

# Capturing Variables

The square brackets of a lambda expression can list variables to _capture_ from the surrounding scope. For example, if we define a function like this:

```c++
using namespace std;

void add_to_all(vector<int> &v, int x) {
    transform(
        v.begin(),
        v.end(),
        v.begin(),
        [](int y) { return x+y; } // WRONG!
    );
}
```

the C++ compiler will complain that the lambda expression does not have access to the variable `x` in its scope. We can fix this by _capturing_ `x` as follows:

```c++
using namespace std;

void add_to_all(vector<int> &v, int x) {
    transform(
        v.begin(),
        v.end(),
        v.begin(),
        [x](int y) { return x+y; } // RIGHT!
    );
}
```

# Callbacks

Sometimes you want to send a function to another function. For example, you might do

```c++
sleep_then(10, []() { cout << "Ten seconds have passed\n"; });
cout << "I am waiting\n";
```

If the sleep function executes in another thread waiting for ten seconds and then running its callback, then you would get the output:

```
I am waiting
Ten seconds have passed
```

We will get to code like this in a few weeks.

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
    for (auto x : u) {
        std::cout << x << " ";
    }
    std::cout << "\n";
}
```

# Associative Containers

Associative containers are different from sequential containers in that they index the container with keys, kind of like a `struct`, dictionary, or database. The associative containers defined in the STL are 'map', 'set', 'multimap', and 'multiset'. We will focus here on 'map', which can be used to illustrate the main features.

In a `map`, a set of keys are used to index a set of values. For example, you might define a `map` as follows:

```c++
using namespace std;

map<string, string> person;
person["First"] = "Alan";
person["Last"] = "Turing";
person["job"] = "Code breaker";
```

Note that the `map` template takes two types, the key type and the value type. They do not have to be the same:

```c++
using namespace std;

map<string, vector<int>> sequence;
sequence["ints"] = { 1, 2, 3, 4, 5, 6 };
sequence ["squares"] = { 1, 4, 9, 16, 25, 36 };
sequence["fib"] = { 1, 1, 2, 3, 5, 8, 13 };
```

or another (inefficient) way to make an array of doubles

```c++
using namespace std;

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

# Event Loop Management

Many embedded systems run _event loops_, which are essentially endless while loops of the following form:

    while alive
        read sensors
        compute control values
        send values
        check for and respond to events
    end

In fact, you have likely seen (or will see) event loops in which thousands of lines of sequential code were put inside `main()` in a while loop. While such an arrangement may be acceptable for small projects, big projects with many contributors need more structure.

# Elma

Starting this week, we will be building an _Event Loop MAnager_ called _elma_, which includes (or will include), support for:

- Defining modular processes
- Scheduling a process to run at a desired frequency
- Communicating among processes with _channels_
- Communicating among processes with _events_ (next week).

Note that _elma_ is a new project being developed by and for this course. Thus, it will have bugs, missing features, poor documentation, and low test coverage. However, we as a class will be building _elma_ week by week.

# Getting Elma

To run a docker container with Elma already installed, do

```bash
docker run -v $PWD:/source -it klavins/elma:latest bash  // MacOS, Linux
docker run -v /$PWD:/source -it klavins/elma:latest bash  // Windows
```

or similar, depending on your configuration. This will load an image that Professor Klavins has already prepared and put up on [Dockerhub](https://hub.docker.com/repository/docker/klavins/elma).

The Github repo for Elma is here: [https://github.com/klavinslab/elma](https://github.com/klavinslab/elma).

Documentation about Elma can be found here: [http://klavinslab.org/elma](http://klavinslab.org/elma).

An example project using elma can be found here: [https://github.com/klavinslab/elma_project](https://github.com/klavinslab/elma_project).

# Overview

Before we delve into the details of how _elma_ works, here is an example of how to use _elma_ to define a simple cruse control class. First, we define a new process.

```c++
  class CruiseControl : public elma::Process {
    public:
      CruiseControl(std::string name) : Process(name) {}
      void init() {}
      void start() {}
      void update() {
        if (channel("Velocity").nonempty()) {
          speed = channel("Velocity").latest();
        }
        channel("Throttle").send(-KP * (speed - desired_speed));
      }
      void stop() {}
    private:
      double speed = 0;
      const double desired_speed = 50.0,
                              KP = 0.5;
  };
```

This process _inherits_ code from a base `Process` class, which we will describe later. It defines several required methods: `init`, `start`, `update`, and `stop`. The only method that does anything is the `update` method, which reads a value from a `Velocity` channel (presumably connected to the car's sensor), computes a control law, and sends the result via a `Throttle` channel to the car.

# A Car

To define an event loop using the CruiseControl process, you would do something like:

```c++
// Declare a new process manager
elma::Manager m;

// Declare processes
Car car("Car"); // class definition not shown
CruiseControl cc("Control");
elma::Channel throttle("Throttle");
elma::Channel velocity("Velocity");

m.schedule(car, 10_ms)    // car updates every 10ms
  .schedule(cc, 10_ms)    // controller updates every 10ms
  .add_channel(throttle)  // register the throttle channel
  .add_channel(velocity)  // register the velocity channel
  .init()                 // initialize all processes
  .run(10_s);             // run for 10s
```

# New Concepts

The design of _elma_ requires a few concepts we have not yet discussed:

- Time
- Inheritance
- Process management
- Inter-process communication

We will cover these first, and then show how we use these concepts.

# Time and Ratios

One of the big problems in code that uses time is that it is hard to remember (and for others to figure out) what units of time you are using at any given point in the code. This is because if you represent a time point or a duration as an integer, there is no associated information about what the units are.

The [chrono](http://www.cplusplus.com/reference/chrono/) library solves this with a new templated class called `duration`, which in turn uses `ratio`.

A `std::ratio` allows you to represent constant ratios as types. For example, to representation the ratio two thirds, you would write

```c++
#include <ratio>
typedef std::ratio<2,3> two_thirds;
std::cout << two_thirds::num << "/" <<two_thirds::den << "\n";
```

Built into the library are a number of predefined ratios, such as `std::milli` or `std::kilo`, which represent the ratios 1:1000 and 1000:1 respectively.

# Durations

A `std::duration` is a time span template that takes a `count` type and a `period` ratio (in seconds). For example, a millisecond duration type is defined by

```c++
typedef std::chrono::duration<int,std::milli> milliseconds_type;
```

which specifies that a value of type `milliseconds_type` will give you a duration in milliseconds accurate to 1/1000 of a second as an integer. You can make a duration of 10ms as follows:

```c++
auto x = milliseconds_type(10);
```

You can convert to a different representation by sending a duration to the constructor of another duration. For example, to express 10ms in seconds (as 0.01s), you can do

```c++
typedef std::chrono::duration<double,std::ratio<1,1>> seconds_type;
auto y = seconds_type(x);
std::cout << "ten ms = " << x.count() << "ms\n"; // prints 10 ms
std::cout << "ten ms = " << y.count() << "s\n";  // prints 0.01s
```

Note that we can't print a `duration` directly, as it is a complex type. Instead, we call the `count()` method, which returns the number of periods in the duration. The number of periods is, of course, dependent on the size of the period.

# Duration Arithmetic

Durations have standard arithmetic defined on them. The result of adding two durations is to get, not surprisingly, the sum of the durations. For example, the expression

```c++
auto z = x + y; // 20 ms
```

adds the two durations together to get a new duration. Note that in this case `x` and `y` have different period sizes and counts. According to the specification, "when two duration objects of different types are involved, the one with the longest period (as determined by common_type) is converted before the operation." Thus, y is converted to `milliseconds_type` and then added to `x` to get `z`.

# Zero

There is a special duration, zero, which can be used as follows:

```c++
auto z = seconds_type::zero();
```

or

```c++
auto another_z = milliseconds_type::zero();
```

# Elma's timing needs

The goal is for _elma_ to manage when processes execute at approximately millisecond resolution, and allow them to specify when things should happen. Thus, we will need to

- Know what time it is
- Add and subtract times
- Convert between different units of time

# Clocks and Timepoints

So that we know what time it is, the `chrono` library provides several clocks. We will use the `high_resolution_clock`, which on most systems will have a resolution of one nanosecond. To get the current time with the clock, you write

```c++
std::chrono::high_resolution_clock::time_point t = high_resolution_clock::now();
```

In this case `t` represents the current time, but it isn't much of use, unless you use it relative to some other time. For example, you can ask for the amount of time since 1970 (known as the beginning of time for computers) via:

```c++
std::cout << t.time_since_epoch().count() << " ns since 1970\n";
typedef std::chrono::duration<double,std::ratio<3600*24*365,1>> years;
auto y = years(t.time_since_epoch());
std::cout << y.count() << " years since 1970\n";
```

which prints something like

```bash
1707787416070272743 ns since 1970
54.1536 years since 1970
```

# Tracking Program Execution

More likely, you would use time points to mark different times in the execution of a program. For example,

```c++
std::chrono::high_resolution_clock::time_point t1, t2;
t1 = std::chrono::high_resolution_clock::now();
f(); // a function that might take a while
t2 = std::chrono::high_resolution_clock::now();
std::cout << "f took " << (t2 - t1).count() << " ns\n";
```

Note that the "`-`" operator on `time_points` is defined to return a `duration`.

# Inheritance

In _elma_, our users will write their own process classes that the process manager will manage.

In particular, the manager will keep a list of processes and apply the same methods to all of them. There are two problems to solve with this arrangement:

- Given that lists in C++ have to have elements of all of the same type, how can the manager keep a list of different process types?
- How do we ensure that our users implement the interface methods that the process manager expects?

In C++ and many other languages these problems are solved with _inheritance_, in which a _derived_ or _child_ class gets all the methods and data of a _base_ or _parent_ class, and then adds its own unique differences.

Here, we will define an _abstract_ base class called `Process` that includes all of the methods that the manager expects.

Crucially, we will declare the methods that inheriting classes must provide, by leaving those methods _unimplemented_ in the base class.

# Processes in Elma

For example, here is part of the `Process` class definition in `elma`.

```c++
  class Process {

      public:

      typedef enum { UNINITIALIZED, STOPPED, RUNNING } status_type;

      Process(std::string name) : _name(name), _status(UNINITIALIZED) {}
      virtual ~Process() = default;

      // Interface for derived classes
      virtual void init() = 0;       // pure virtual methods that
      virtual void start() = 0;      // must be defined by child class
      virtual void update() = 0;
      virtual void stop() = 0;

      virtual std::string name() { return _name; }  // virtual method that may be re-defined by child class
      status_type status() { return _status; }

      private:

      std::string _name;
      status_type _status;

  };
```

# Virtual Methods

The methods declared as `virtual` and set to `0` are called _pure virtual_ functions and are not implemented by `Process`. Pure virtual methods must be implemented by child classes, unless they too are intended to be virtual.

If you try to construct an object of type `Process`, you will get a compiler error that looks something like:

```bash
error: invalid cast to abstract class type 'elma::Process'
```

This declaration of `Process` above also includes a _virtual_ method `name`, which has an implementation, but can be overridden.

Thus, virtual as opposed to pure virtual methods actually have implementations. Derived classes _may_ override them, but do not have to.

# Derived Classes

Interestingly, derived classes can also override non-virtual methods. However, doing so will have consequences. Suppose we derive a class as follows.

```c++
class BoringProcess : public elma::Process {
    void init() {}    // overriding pure virtual
    void start() {}
    void update() {}
    void stop() {}
    std::string name() { return "nothing"; }             // overiding a virtual
    status_type status() { return UNINITIALIZED; }  // overrinding a non-virtual
}
```

Now suppose we define two variables to refer to processs:

```c++
BoringProcess p("Boring!");
elma::Process * q = &p; // Tell C++ to think of q as a process, not a BoringProcess
q->update();            // Calls BoringProcess.update()
q->name();              // Calls BoringProcess.name()
q->status();            // Calls Process.status() (probably not what we wanted)
```

# Inheritance Summary

To summarize:

- If you want to enforce derived classes to implement a method, use _pure virtual_ (=0)
- If you want to make it optional for derived classes to override a method, use _virtual_.
- Avoid overriding non-virtual methods.

# Instance variables

Processes have a number of quantites to keep track of, such as their status, the time since they were last updated, and so on. We define the following private data members for `Process`:

```c++
private:

std::string _name;
status_type _status;
std::chrono::high_resolution_clock::duration _period,          // request time between updates
                                _previous_update, // duration from start to update before last
                                _last_update;     // duration from start to last update
std::chrono::high_resolution_clock::time_point _start_time;    // time of most recent start
int _num_updates;                                 // number of times update() has been called
elma::Manager * _manager_ptr;                           // a pointer to the manager
```

Variables that are specific to an instance of a class are called _instance variables_. Note that by convention we are prefixing all instance variables by an underscore "`_`", so it is easy to see which variables are which. Doing so also makes it less likely that derived classes will override them by accident (unless they use the same convention). We do not want any derived class to access these variables directly, so they are declared `private`.

# Getters

Derived classes might want to know the value of these variables are, so we provide a public interface to all of them:

```c++
inline std::string name() { return _name; }
inline status_type status() { return _status; }
inline std::chrono::high_resolution_clock::duration period() { return _period; }
inline int num_updates() { return _num_updates; }
inline std::chrono::high_resolution_clock::time_point start_time() { return _start_time; }
inline std::chrono::high_resolution_clock::duration last_update() { return _last_update; }
inline std::chrono::high_resolution_clock::duration previous_update() { return _previous_update; }
```

The keyword `inline` states that compiled code using these methods will simply replace method calls by the body of the function, so that no function calls will be made when they are used. Using `inline` is generally faster. If the function body gets big and is used frequently, `inline` takes up a lot of space in the compiled code, so `inline` is generally used only with very short function bodies.

# Process / Manager Interface

The process manager needs to update the timing information and status of a process as it manipulates it. To enable this in such a way that our user does not need to think about it, we define a version of each of the main methods that are just for the manager to use. In particular, we add to our `Process` class:

```c++
class Manager; // declare that Manager is a class,
               // defined elsewhere

class Process {

  ...

  friend Manager; // give Manager class access to
                  // private methods and data

  ...

  private:
  // Manager interface
  void _init();
  void _start(std::chrono::high_resolution_clock::duration elapsed);
  void _update(std::chrono::high_resolution_clock::duration elapsed);
  void _stop();

  ...

};
```

The argument to `_start` and `_update` is the duration of time since the manager was started. The manager does not call the the user versions of these methods directly. It calls the underscored private interface instead.

# Initialization

First, the `_init` method should change the status of the process and then call the user's `init` method:

```c++
void Process::_init() {
    _status = STOPPED;
    init();
}
```

# Starting

The `_start` method sets the status of the process to `RUNNING`, and initializes the start time, last update time, and the number of updates. Then it calls the user's `start` method.

```c++
void Process::_start(std::chrono::high_resolution_clock::duration elapsed) {
    _status = RUNNING;
    _start_time = std::chrono::high_resolution_clock::now();
    _last_update = elapsed;
    _num_updates = 0;
    start();
}
```

# Updating

The `_update` method updates the previous update and last update, calls the user's `update` method and then increments the number of udpates.

```c++
void Process::_update(std::chrono::high_resolution_clock::duration elapsed) {
    _previous_update = _last_update;
    _last_update = elapsed;
    update();
    _num_updates++;
}
```

# Stopping

Finally, the `_stop` method calls the

```c++
void Process::_stop() {
    _status = STOPPED;
    stop();
}
```

# Other Process Methods

We define a couple of convenience methods for `Process` that allow the user to get at the underlying timing information as follows:

```c++
double Process::milli_time() {
    std::chrono::duration<double, std::milli> time = last_update();
    return time.count();
}
double Process::delta() {
    std::chrono::duration<double, std::milli> diff = last_update() - previous_update();
    return diff.count();
}
```

The first method returns the number of milliseconds since the last update, and the second returns the difference between the last update and the previous update durations. Note that `_last_update` is updated just before the user's `update` method is called, so it should be approximately the current amount of time since the process started.

# The Manager

The manager class is defined as follows:

```c++
class Manager {

    public:

    Manager() {}

    Manager& schedule(Process& process, std::chrono::high_resolution_clock::duration period);

    Manager& all(std::function<void(Process&)> f);

    Manager& init();
    Manager& start();
    Manager& stop();
    Manager& run(std::chrono::high_resolution_clock::duration);
    Manager& update();

    inline std::chrono::high_resolution_clock::time_point start_time() { return _start_time; }
    inline std::chrono::high_resolution_clock::duration elapsed() { return _elapsed; }

    private:

    std::vector<Process *> _processes;
    std::chrono::high_resolution_clock::time_point _start_time;
    std::chrono::high_resolution_clock::duration _elapsed;

};
```

We go through these methods one by one.

# Adding a process

First, the Manager keeps a vector of process pointers, `_processes`. To add some processes, the user just does something like

```c++
m.schedule(p, 1_ms)
 .schedule(p, 2_ms);
```

Note that the `schedule` method returns a reference to the manager itself so that you can do `method chaining` on the manager. This makes code a bit easier to read. The implementation of `schedule` is

```c++
Manager& Manager::schedule(Process& process, std::chrono::high_resolution_clock::duration period) {

    process._period = period;
    process._manager_ptr = this;
    _processes.push_back(&process);
    return *this;

}
```

The manager accesses the processes' private data to set the period and manager pointer, adds the process to the list, and then returns a reference to itself for method chaining.

# All

The next method, `all` is a convenience method for running a function on all the processes in the list. It takes as an argument a `function` using the `std::functional` library. This allows you to send in either function pointers or lambdas. Its definition is

```c++
Manager& Manager::all(std::function< void(Process&) > f) {
    for(auto process_ptr : _processes) {
        f(*process_ptr);
    }
    return *this;
}
```

# Using all

Using `all`, it is easy to start, stop, and init all of the process:

```c++
Manager& Manager::init() {
    return all([](Process& p) { p._init(); });
}

Manager& Manager::start() {
    return all([this](Process& p) { p._start(_elapsed); });
}

Manager& Manager::stop() {
    return all([](Process& p) { p._stop(); });
}
```

Note that since the `start` method needs to send the `elapsed` time to the processes' `_start` method, the lambda in the expression has to capture `this`. Also, since `all` returns `*this`, so do all of these methods that use `all`.

# Update

The more interesting method is `update`, which should only update a process at the requested frequency.

```c++
Manager& Manager::update() {
    return all([this](Process& p) {
        if ( _elapsed > p.last_update() + p.period() ) {
            p._update(_elapsed);
        }
    });
}
```

# Run

Finally, we define a `run` method that runs all processes for the requested amount of time

```c++
Manager& Manager::run(high_resolution_clock::duration runtime) {

    _start_time = std::chrono::high_resolution_clock::now();
    _elapsed = std::chrono::high_resolution_clock::duration::zero();

    start();

    while ( _elapsed < runtime ) {
        update();
        _elapsed = high_resolution_clock::now() - _start_time;
        // TODO: sleep until next update,
        // to give processor back to the OS
    }

    stop();

    return *this;

}
```

This `run` method is not ideal, because it does nothing but check the condition in the `while` loop and the condition in `update` over and over, only occasionally actually calling a processes' `_update` method.

The actual `run` method in elma will sleep until the next update.

# Putting it Together

Here is an example that uses all of the methods we have defined so far.

```c++
class MyProcess : public elma::Process {
  public:
    MyProcess(std::string name) : Process(name) {}
    void init() {}
    void start() {}
    void update() {
      std::cout << name() << ": "
                << milli_time()
                << "ms\n";
    }
    void stop() {}
};
```

# Testing the Example

```c++
TEST(Manager,Schedule) {

  elma::Manager m;
  MyProcess p("A"), q("B");

  m.schedule(p, 1_ms)
   .schedule(q, 5_ms)
   .init()
   .run(11_ms);

  ASSERT_EQ(p.num_updates(), 10);
  ASSERT_EQ(q.num_updates(), 2);

}
```

Running this in a test prints out

```
A: 1.0025ms
A: 2.0032ms
A: 3.0065ms
A: 4.0101ms
B: 5.0036ms
A: 5.0558ms
A: 6.0564ms
A: 7.0572ms
A: 8.0612ms
A: 9.0652ms
B: 10.0139ms
A: 10.0689ms
```

Note that the test may not always pass. This is because the operating system may lag for whatever reason, not giving the manager exclusive access to the CPU. In this sense, `elma` running on Linux is _not_ a real time system, as it cannot guarantee the frequency with which a process will run.

# Interprocess Communication

There are many ways that concurrent systems deal with process communication. Here are a few:

- **Shared variables:** in some global space that all processes have access to. The downside is the lack of enforceable conventions on how variables are accessed.
- **Events:** Processes can trigger and listen to events. Each event has associated data. Interrupts and interrupt handlers are an example. We will describe how elma does this later.
- **Channels:** FIFO (First In First Out) queues that let some processes publish data that other processes can subscribe to. This is common with embedded real time systems where, for example, a sensor is sending data continuously that other processes consume.

# Channels

We will start with channels. Here is a class definition.

```c++
class Channel {

    public:

    Channel(std::string name) : _name(name), _capacity(100) {}
    Channel(std::string name, int capacity) : _name(name), _capacity(capacity) {}

    Channel& send(double);
    Channel& flush(double);
    double latest();
    double earliest();

    inline int size() { return _queue.size(); }
    inline bool empty() { return _queue.size() == 0; }
    inline bool nonempty() { return _queue.size() > 0; }
    inline std::string name() { return _name; }
    inline int capacity() { return _capacity; }

    private:

    std::string _name;
    int _capacity;
    std::deque<double> _queue;

};
```

The main data structure is the double ended queue. We will push new data onto the front of the queue, and get the data off of the back of the queue. Most processes will likely just use the latest value, but some processes might want to know many values previous, so our channel records them, up to a limit.

# Channel Implementation

The implementation of these methods is straightforward, and in some ways just a wrapper around the `deque`.

```c++
Channel& Channel::send(double value) {
    _queue.push_front(value);
    while ( _queue.size() >= capacity() ) {
        _queue.pop_back();
    }
    return *this;
}

Channel& Channel::flush(double) {
    _queue.clear();
    return *this;
}

double Channel::latest() {
    if ( _queue.size() == 0 ) {
        throw std::range_error("Tried to get the latest value in an empty channel.");
    }
    return _queue.front();
}

double Channel::earliest() {
    if ( _queue.size() == 0 ) {
        throw std::range_error("Tried to get the earliest value in an empty channel.");
    }
    return _queue.back();
}
```

Note that we throw exceptions if a user process tries to access an empty channel.

# Adding Channels to the Manager

To make channels accessible to the manager, we add a new private datum `_channels`:

```c++
std::map<string, Channel *> _channels;
```

and a method for adding and accessing them

```c++
Manager& Manager::add_channel(Channel& channel) {
    _channels[channel.name()] = &channel;
    return *this;
}

Channel& Manager::channel(std::string name) {
    if ( _channels.find(name) != _channels.end() ) {
      return *(_channels[name]);
    } else {
        throw std::domain_error("Tried to access an unregistered or non-existant channel.");
    }
}
```

# Using Channels in Processes

To make channels accessible to processes, we basically just go through the `_manager_ptr` in the process:

```c++
Channel& Process::channel(std::string name) {
    return _manager_ptr->channel(name);
}
```

# Example: Cruise Control

As an example, we will develop a simple cruise control system in which we model a car with one process and a control system with another. The underlying mathematics look like this. Let `v` be the velocity of a car, `m` its mass, `k` the coefficient of rolling friction, and `u` the force being applied by the engine on the car. Then a very simplified model of the car's velocity comes from Newton's second law (`f = ma`):

```
m dv/dt = -k v + u
```

This is a continuous model, but our processes are discrete. We can discretize the model using the definition of derivative:

```
dv/dt = lim(h->0) (v(t) - v(t-h)) / h
```

For `h` small, `dv/dt` is thus approximately

```
dv/dt = (v(t) - v(t-h)) / h
```

or

```
v(t) = v(t-h) + h dv/dt
     = v(t-h) - h (k v - u) / m
```

In our case, `h` is given by the difference between the last update and the previous update, which is available to user processes via the `delta()` function.

# A Control Interface for a Car

```c++
class Car : public elma::Process {
  public:
    ControllableCar(std::string name) : Process(name) {}
    void init() {}
    void start() {
      velocity = 0;
      force = 0;
    }
    void update() {
      if (channel("Throttle").nonempty()) {
        force = channel("Throttle").latest();
      }
      velocity += (delta() / 1000) * (-k * velocity + force) / m;
          velocity += (delta() / 1000) * (-k * velocity + force) / m;
          channel("Velocity").send(velocity);
          std::cout << "t: "  << milli_time() << " ms\t"
                    << " u: " << force        << " N\t"
                    << " v: " << velocity     << " m/s\n";
    }
    void stop() {}
  private:
    double velocity;
    double force;
    const double k = 0.02;
    const double m = 1000;
};
```

# The Cruise Controller

A very simple controller uses proportional feedback. Basically, we set

```
u = -Kp (v - v_des)
```

where `Kp` is the proportional gain and `v_des` is the desired velocity. To make a control process, we write

```c++
class CruiseControl : public elma::Process {
  public:
    CruiseControl(std::string name) : Process(name) {}
    void init() {}
    void start() {
      speed = 0;
    }
    void update() {
      if (channel("Velocity").nonempty()) {
        speed = channel("Velocity").latest();
      }
      channel("Throttle").send(-KP * (speed - desired_speed));
    }
    void stop() {}
  private:
    double speed;
    const double desired_speed = 50.0,
                            KP = 0.5;
};
```

# Composing Processes

To set everything up, we add the two processes and the two channels to the manager, initialize everything, and run the system.

```c++
elma::Manager m;

Car car("Car");
CruiseControl cc("Control");
elma::Channel throttle("Throttle");
elma::Channel velocity("Velocity");

m.schedule(car, 10_ms)
 .schedule(cc, 10_ms)
 .add_channel(throttle)
 .add_channel(velocity)
 .init()
 .run(10000_ms);
```

Note that the order of the scheduling is important. We want the controller to run right after the car. If we reversed the order, the controller would be using an older sensor value.

# Example data

The output of the system for the first few steps looks like:

```
t: 20.0035 ms    u: 15707.5 N    v: 0.157106 m/s
t: 30.0042 ms    u: 15658.1 N    v: 0.313699 m/s
t: 40.0083 ms    u: 15609 N      v: 0.469852 m/s
t: 50.0098 ms    u: 15559.9 N    v: 0.625474 m/s
t: 60.0162 ms    u: 15511 N      v: 0.780684 m/s
t: 70.0192 ms    u: 15462.2 N    v: 0.935352 m/s
t: 80.021 ms     u: 15413.7 N    v: 1.08952 m/s
t: 90.054 ms     u: 15365.2 N    v: 1.24368 m/s
t: 100.056 ms    u: 15316.8 N    v: 1.39688 m/s
t: 110.061 ms    u: 15268.7 N    v: 1.54963 m/s
t: 120.064 ms    u: 15220.7 N    v: 1.70189 m/s
t: 130.069 ms    u: 15172.9 N    v: 1.85369 m/s
t: 140.073 ms    u: 15125.2 N    v: 2.00501 m/s
t: 150.076 ms    u: 15077.6 N    v: 2.15583 m/s
t: 160.079 ms    u: 15030.2 N    v: 2.30617 m/s
...
```

# Testing Processes

Testing processes can be challenging, because they are not simply functions that return values, but dynamic entities. One way to proceed is to define test processes that you compose with the process you have defined to see if it does the right thing. For example, suppose you have a process that keeps track of the minimum and maximum values it has seen on a channel over time. To test such a process, you might build a test process that sends values to the channel and also knows what minimum and maximum values it has sent. Then you compose the two processes together and check that it works. Although this does not test all the behavior, it does test some of the expected behavior. And new tests can be defined as issues arise.

For an example of this approach, see the [how_to_test](https://github.com/sosper30/eep520-wi24/tree/main/week7/how_to_test) directory of this week's notes.
