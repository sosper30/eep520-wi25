# Software Engineering For Embedded Applications

---

Last week we built a simple process manager that included an abstract base class, `Process`, from which users can derive their own processes. We also defined a `Channel` class that enabled processes to send streams of values to each other.

<img src='https://raw.githubusercontent.com/klavins/EEP520-W20/master/week_7/images/elma-structure.png' width=70%>

This week we will

- Upgrade the `Channel` class to enable sending _JSON_ data, instead of just `double` values.
- Add a new inter-process communication `Event` class.
- Add a set of classes for defining _Finite State Machines_.

The resulting structure looks like the above. Orange indicates classes that we will define today.

# Review

Last week, we started building an _Event Loop MAnager_ called _elma_, which includes (or will include), support for:

- Defining modular processes
- Scheduling a process to run at a desired frequency
- Communicating among processes with _channels_
- Communicating among processes with _events_.

Please start your docker container with something like

```bash
docker pull klavins/elma:latest
```

which should download the latest version from DockerHub, before doing

```bash
docker run -v $PWD:/source -it klavins/elma:latest bash
```

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
auto z = x+y; // 20 ms
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
using namespace std::chrono;
high_resolution_clock::time_point t = high_resolution_clock::now();
```

In this case, `t` represents the current time, but it isn't much of use, unless you use it relative to some other time. For example, you can ask for the amount of time since 1970 (known as the beginning of time for computers) via:

```c++
std::cout << t.time_since_epoch().count() << " ns since 1970\n";
typedef duration<double,std::ratio<3600*24*365,1>> years;
auto y = years(t.time_since_epoch());
std::cout << y.count() << " years since 1970\n";
```

which prints something like

```bash
1581621645325126700 ns since 1970
50.1529 years since 1970
```

# Tracking Program Execution

More likely, you would use time points to mark different times in the execution of a program. For example,

```c++
high_resolution_clock::time_point t1, t2;
t1 = high_resolution_clock::now();
f(); // a function that might take a while
t2 = high_resolution_clock::now();
std::cout << "f took " << (t2 - t1).count() << " ns\n";
```

Note that the "`-`" operator on `time_points` is defined to return a `duration`.

# Interprocess Communication

There are many ways that concurrent systems deal with process communication. Here are a few:

- **Shared variables:** in some global space that all processes have access to. The downside is the lack of enforceable conventions on how variables are accessed.
- **Events:** Processes can trigger and listen to events. Each event has associated data. Interrupts and interrupt handlers are an example. We will describe how elma does this later.
- **Channels:** First in first out queues that let processes send data that other processes can subscribe to. This is common with embedded real time systems where, for example, a sensor is sending data continuously that other processes consume.

# Channels

We will start with channels. Here is a class definition.

```c++
class Channel {

    public:

    Channel(string name) : _name(name), _capacity(100) {}
    Channel(string name, int capacity) : _name(name), _capacity(capacity) {}

    Channel& send(double);
    Channel& flush(double);
    double latest();
    double earliest();

    inline int size() { return _queue.size(); }
    inline bool empty() { return _queue.size() == 0; }
    inline bool nonempty() { return _queue.size() > 0; }
    inline string name() { return _name; }
    inline int capacity() { return _capacity; }

    private:

    string _name;
    int _capacity;
    deque<double> _queue;

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
map<string, Channel *> _channels;
```

and a method for adding and accessing them

```c++
Manager& Manager::add_channel(Channel& channel) {
    _channels[channel.name()] = &channel;
    return *this;
}

Channel& Manager::channel(string name) {
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
Channel& Process::channel(string name) {
    return _manager_ptr->channel(name);
}
```

# JSON

The _Javascript Object Notation_ or [JSON](http://www.json.org/) standard is a file and data format used almost ubiquitously in networked systems. It has become the standard way to represent arbitrary data types in a structured way, and to allow applications to communicate without much overhead in defining data structures and types. It is even used in some of the most popular databases, such as MongoDB, as the fundamental data structure for defining documents.

Some examples of where JSON is used include

- [Vehicle APIs](https://www.scip.ch/en/?labs.20180405)
- [Industrial Automation](https://openautomationsoftware.com/)
- [Databases](https://www.mongodb.com/)
- [Google Maps](https://www.sitepoint.com/google-maps-json-file/)
- [NOAA Data Services](https://www.ncdc.noaa.gov/access-data-service/api/v1/data?dataset=global-summary-of-the-year&dataTypes=DP01,DP05,DP10,DSND,DSNW,DT00,DT32,DX32,DX70,DX90,SNOW,PRCP&stations=ASN00084027&startDate=1952-01-01&endDate=1970-12-31&includeAttributes=true&format=json)

# Data Representation in JSON

Example:

```json
{
  "ta": [
    {
      "first": "Armagan",
      "last": "Er"
    }
  ],
  "weeks": [1, 2, 3, 4, 5, 6, 7, 8, 9, 10],
  "version": 3.0
}
```

- **_Atomic_** values
  - strings delimited by double quotes
  - numbers, with or without decimals
  - true and false
  - null
- **_Arrays_** of values atomic values, objects and/or otjer arrays, delimited by [ and ].
- **_Objects_** with key value pairs, where values are atomic, objects, or arrays

# Supporting JSON

The two key methods that one needs to support JSON in an application are

- _Serialization_, also knownn as _stringification_: Converting an application data structure into a JSON string.
- _Deserialization_, also known as _parsing_: Converting a JSON string into an application data structure.

For example, open the console on your web browser and type

```javascript
str = '{ "a": 1, "b": [2,3], "k": { "c": { "d": true } } }';
x = JSON.parse(str);
x.k.c.d; // true
JSON.stringify(x);
```

Note that if your string has a format error in it, you will get an error when you try to deserialize it:

```javascript
JSON.parse('{"x": [1,2}');
```

which results in the useful message:

```
VM282:1 Uncaught SyntaxError: Unexpected token } in JSON at position 10
    at JSON.parse (<anonymous>)
    at <anonymous>:1:6
```

# Adding JSON to your code (and to Elma)

Using the elma container, you can use the JSON library simply by including it.

```c++
#include <json/json.h>

using nlohmann::json;
```

# A JSON C++ Library

The JSON C++ library we will use is described in detail [here](https://github.com/nlohmann/json). For now, we will use a subset of the library's capabilities, mainly defining and accessing JSON values.

To defining a JSON value, you can simply declare it with the json type and assign it to a C++ value. For example,

```c++
json j = 3.14,
     k = "hello world",
     l = { 1,2,3 };
```

defines three JSON values with raw types double, string, and integer array. Note that since C++ does not use [ and ] to define arrays, we have to use { and }.

Since C++ does not have built in hash-like objects, to define an object, you have to use a special notation that does not look much like JSON. For example,

```c++
json j = { { "x", 0 }, { "y", 1 } }; // { "x": 0, "y": 1 }
```

# Nested JSON

If you want to define a nested object, you can write

```c++
json j = {
    {
        "ta", {
            {
                { "first", "Sanjar" },
                { "last", "Normuradov" }
            }
        },
    },
    {
        "weeks", { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 }
    },
    {
        "version", 2.0
    }
};
```

# Serializing a String

You can serialize a string directly in a definition, as in

```c++
json x = R"('{ "a": 1, "b": [2,3], "k": { "c": { "d": true } } }')"_json;
```

This magical bit of code uses the C++ [raw string operator](http://www.cplusplus.com/forum/general/190123/). The `_json` bit at the end is using a [user defined literal](https://en.cppreference.com/w/cpp/language/user_literal) (defined in the JSON library).

# Accessing Parts of Objects

To access parts of a json object, you use notation such as

```c++
int a = x["a"]; // 1
bool d = x["k"]["c"]["d"]; // true
```

Note that the assignment operator `=` has been overloaded to convert any kind of json value into a C++ value (such as `int` and `bool` above). You will get a runtime error if the value cannot be converted. For example, if you do

```c++
double k = x["k"];
```

you will get

```
C++ exception with description "[json.exception.type_error.302] type must be number, but is object" thrown in the test body.
```

# Downsides of JSON

This brings up one of the downsides of using JSON. With C++ you normally cannot even compile code that tries to assign a value of one type to an incompatible value of another type. But with JSON you can. If you would like to be very explicit, you can instead do

```c++
double k = x["k"].get<double>();
```

which uses the templated `get` method of the library. However, this will still throw a runtime error, since the compiler can't keep track of what values `x` may take on without running the program.

# Upgrading Channels to use JSON

The JSON library is incredibly easy to use. To modify the `Channel` object, we literally replace the keyword `double` with the type `json`. In the header file:

```c++
class Channel {
    //
    public:
    Channel& send(json);
    json latest();
    json earliest();
    //
    private:
    deque<json> _queue;
    //
};
```

# Implementation

In the implementation file

```c++
Channel& Channel::send(json value) {
    _queue.push_front(value);
    while ( _queue.size() > capacity() ) {
        _queue.pop_back();
    }
    return *this;
}

json Channel::latest() {
    if ( _queue.size() == 0 ) {
        throw Exception("Tried to get the latest value in an empty channel.");
    }
    return _queue.front();
}

// etc
```

# Events

An _event_ is a discrete occurrence that happens at a specific point in time. For example, a touch sensor might go from off to on, or a user might press a button. Events often have data associated with them. For example, how hard was the button pressed? To capture an event formally, we define a new class:

```c++
class Event {

    public:

        Event(std::string name, json value) : _name(name), _value(value), _empty(false) {}
        Event(std::string name) : _name(name), _value(0), _empty(true) {}

    private:
    std::string _name;
    json _value;
    bool _empty; // used when no value is needed or provided

};
```

We will expand upon this definition later, but for now, an event is little more than a container for a JSON value.

# Event Methods

To use an event, processes need to be able to

- **emit** an event, giving it a name in the process. The result should be that the manager broadcasts the occurence of the event to any other processes that are watching for it.
- **watch** for events with a specific name, responding to them with user defined functions.

# Keeping Track of Events in the Manager

To the `Manager` we add a private data member

```c++
// manager.h
private:
map<string, vector<std::function<void(Event&)>>> event_handlers;
```

# Watching Events

Next we add a `watch` method to Manager and a wrapper for it to `Process`.

```c++
// manager.cc
Manager& Manager::watch(string event_name, std::function<void(Event&)> handler) {
    event_handlers[event_name].push_back(handler);
    return *this;
}

// process.cc
void Process::watch(string event_name, std::function<void(Event&)> handler) {
    _manager_ptr->watch(event_name, handler);
}
```

# Where You Should use Watch

Typically, processes should set up event handlers in their `init` methods. For example, you could do

```c++
// Cruise control process watching for desired speed events from the driver
void CruiseControl::init() {
    watch("desired speed", [this](Event& e) {
        desired_speed = e.value();
    });
}
```

This method would be used by a `CruiseControl` process to respond to an event changing its desired speed. Note that the lambda sent to the event captures `this`, so that the variable `desired_speed` that is private to the CruiseControl object can be accessed.

# Emitting an Event

To emit an event, we define a `Manager` method and a `Process` wrapper that searches the event handler list for handlers that correspond to the emitted event.

```c++
// Manager.cc
Manager& Manager::emit(const Event& event) {
    Event e = event; // make a copy so we can change propagation
    if ( event_handlers.find(event.name()) != event_handlers.end() ) {
        for ( auto handler : event_handlers[event.name()] ) {
            handler(e);
        }
    }
    return *this;
}

// Process.cc
void Process::emit(string event_name, const Event& event) {
    _manager_ptr->emit(event_name, event);
}
```

# Example Event Emission

A process would typically emit an event in its `start`, `update`, or `stop` method.

For example, suppose you wanted to simulate a driver with a `Driver` process.

```c++
void Driver::start() {
    emit(Event("desired speed", 40));
}
```

When this process is started, it will emit the event, which the cruise control process will respond to.

# Example

See `examples/driving.cc` for a worked out example. A plot of the velocity over time from that example is here:

<img src='https://raw.githubusercontent.com/klavins/EEP520-W20/master/week_7/images/cruise-control-output.png' width=70%>

# Event Propagation

The way we currently have event manager defined, every handler that watches for events with a particular name will get run every time an event with that name is emitted. This may not be desired in some cases. For example, you may want that certain events take priority and prevent other handlers from being run. To get this behavior, we introduce _event propagation_.

# Changing Event Propagation

To the event class, we add a Boolean value to keep track of whether the event should be propagated.

```c++
// event.h
private:
bool _propagate;
```

as well as the methods:

```c++
// event.h
inline bool propagate() const { return _propagate; }
inline void stop_propagation() { _propagate = false; }
inline void reset() { _propagate = true; }
```

# Managing Propagation

In the manager, we can then prevent events from propagating if the-r `_propgate` instance variable is set to true as follows:

```c++
Manager& Manager::emit(const Event& event) {
    Event e = event; // new: make a copy so we can change propagation
    if ( event_handlers.find(event.name()) != event_handlers.end() ) {
        for ( auto handler : event_handlers[event.name()] ) {
            if ( e.propagate() ) { // new
                handler(e);
            }
        }
    }
    return *this;
}
```

# Using Event Propagation

This capability will become particularly useful in the next section on finite state machines.

# Finite State Machines

A finite state machine, or FSM, is a fundamental object in embedded systems. They consist of a set of states and a set of labelede transitions between states. Here is a simple example of a toggle switch.

<img src='https://raw.githubusercontent.com/klavins/EEP520-W20/master/week_7/images/toggle-switch.png' width=70%>

There are two states, `Off` and `On`. The FSM moves from one state to the other, every time a 'switch' input is recieved.

# Microwave Example

Another example is a microwave oven controller, which is designed to accept user input and keep the user from doing something bad (like tuning on the microwave when the door is open).

<img src="https://raw.githubusercontent.com/klavins/EEP520-W20/master/week_7/images/microwave.png" width="500"></image>

To implement FSMs in Elma, we will add three new classes: `State`, `Transition`, and `StateMachine`. The first is an abstract base class that users will override with their own state definitions. Transition is a container class that holds a source and desintation state and an event name. StateMachine will inherit from `Process` and will manager transitions.

# States

To represent states, we add the new class:

```c++
class State {

    friend class StateMachine;

    public:
    State();
    State(std::string name);

    inline std::string name() { return _name; }
    virtual void entry(Event& e) = 0;
    virtual void during() = 0;
    virtual void exit(Event& e) = 0;

    void emit(const Event& e);

    private:
    std::string _name;
    StateMachine * _state_machine_ptr;

};
```

# Transitions

To represent transitions, we add the new class:

```c++
class Transition {

    public:
    Transition(std::string event_name, State& from, State& to) :
        _from(from),
        _to(to),
        _event_name(event_name) {}

    inline State& from() const { return _from; }
    inline State& to() const { return _to; }
    inline string event_name() const { return _event_name; }

    private:
    State& _from;
    State& _to;
    string _event_name;

};
```

# State Machines

To represent state machines, we add the new class

```c++
class StateMachine : public Process {

    public:
    StateMachine(std::string name) : Process(name) {}
    StateMachine() : Process("unnamed state machine") {}

    StateMachine& set_initial(State& s);
    StateMachine& add_transition(std::string event_name, State& from, State& to);
    inline StateMachine& set_propagate(bool val) { _propagate = val; }

    State& current() { return *_current; }

    void init();
    void start();
    void update();
    void stop();

    private:
    vector<Transition> _transitions;
    State * _initial;
    State * _current;
    bool _propagate;

};
```

# State Machine Initialization

The class keeps track of the initial and current states as well as keeping a list of transitions. Since it inherits from `Process` and needs its `init`, `start`, `update`, and `stop` methods defined. The `init` method loops through all transitions and watches for events that trigger them.

```c++
void StateMachine::init() {
    for (auto transition : _transitions ) {
        watch(transition.event_name(), [this, transition](Event& e) {
            if ( _current->id() == transition.from().id() ) {
                _current->exit(e);
                _current = &transition.to();
                _current->entry(e);
                if ( !_propagate ) { // StateMachine has a wrapper flag for
                                     // whether it should propagate
                    e.stop_propagation();
                }
            }
        });
    }
}
```

# State Machine Starting and Updating

The start method sets the current state to the initial state

```c++
void StateMachine::start() {
    _current = _initial;
    _current->entry(Event("start"));
}
```

and the update method calls the current state's during method.

```c++
void StateMachine::update() {
    _current->during();
}
```

# Example: Binary Counter

See the `toggle-switch/` directory for how to implement the toggle switch.

<img src='https://raw.githubusercontent.com/klavins/EEP520-W20/master/week_7/images/toggle-switch.png' width=40%>

# Example: Microwave Oven

See the `microwave/` directory for how to implement the microwave oven.

<img src="https://raw.githubusercontent.com/klavins/EEP520-W20/master/week_7/images/microwave.png" width="500"></image>

# Documentation with Doxygen (using your terminal)

The documentation for Elma is current at [https://klavins.github.io/ECEP520/index.html](https://klavins.github.io/ECEP520/index.html). It is automatically generated by first running Doxygen with

```bash
make docs
```

and then copying the contents of the resulting `html` directory to `ECEP520/docs`. Github has a feature called _Github Pages_ that allows you to serve web pages from a repository. To set it up, I went to the ECEP520 repository, clicked settings, scrolled down to _Github Pages_, and chose `/docs` as the source directory.

# Documenting a Class

To document a class, you add comments to the code with the special syntax

```c++
//! My new class

//! Some details about my class
class MyClass {

};
```

# Documenting a Method

To document a method, you do something like

```c++
//! Compute something important
//! \param x A description of the first argument
//! \param s A description of the second argument
//! \return The value of the important thing computed
int f(int x, std::string s);
```

# Adding a Code Example

To add a code example to a comment, do

```c++
//! @code
//! int x = 1;
//! @endcode
```

# For more examples

See, for example, the comments the [elma code base](https://github.com/klavinslab/elma) which compiles to look like [this](http://klavinslab.org/elma/).
