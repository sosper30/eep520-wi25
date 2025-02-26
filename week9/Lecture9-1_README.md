# Software Engineering For Embedded Applications


Last week we learned:

- Events and event handlers (`emit` and `watch`)
- Finite state machines
- JSON

This week, we apply our skills to building robot control systems using a new multi-robot simulation environment, called ENVIRO, build on top of ELMA.

# Review: Events and Event Methods

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

# Review: Watching Events

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

# Review: Emitting an Event

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

A process would typically emit an event in its `start`, `update`, or `stop` method.

For example, suppose you wanted to simulate a driver with a `Driver` process.

```c++
void Driver::start() {
    emit(Event("desired speed", 40));
}
```

When this process is started, it will emit the event, which the cruise control process will respond to.

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

In the manager, we can then prevent events from propagating if their `_propgate` instance variable is set to true as follows:

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

This capability will become particularly useful in the next section on finite state machines.

# Finite State Machines
A finite state machine, or FSM, is a fundamental object in embedded systems. They consist of a set of states and a set of labeled transitions between states. Here is a simple example of a toggle switch.

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


# Introducing ENVIRO

ENVIRO is a multi robot simulation environment with the following features:

- Robot behaviors are defined with C++ _plugins_ containing ELMA processes
- Robots can be controlled with simple methods, like `track_velocity` or `apply_force`
- Robot can use range sensors to sense the environment
- Inanimate objects and static objects (like walls) can be added to the environment
- JSON configuration files are used to define the environment

# New Concepts

- Robot Motion Control
- Shared Object Libraries
- Multiple Inheritance
- Client/Server Architectures

# Docker Container with Enviro

To start a Docker container with ENVIRO pre-loaded into it, do:

```bash
docker run -p80:80 -p8765:8765 -v $PWD:/source -it klavins/enviro:v1.61 bash
```
Note:
There are different version of the klavins/enviro image, so it is possible that only one image would run on your computer. To know which image is suitable for your computer you may need to run all the versions once.
Here are all the versions of the image:
```bash
docker run -p80:80 -p8765:8765 -v $PWD:/source -it klavins/enviro:v1.6 bash
```
```bash
docker run -p80:80 -p8765:8765 -v $PWD:/source -it klavins/enviro:v1.5 bash
```
```bash
docker run -p80:80 -p8765:8765 -v $PWD:/source -it klavins/enviro:v1.4 bash
```
```bash
docker run -p80:80 -p8765:8765 -v $PWD:/source -it klavins/enviro:v1.3 bash
```
```bash
docker run -p80:80 -p8765:8765 -v $PWD:/source -it klavins/enviro:v1.2 bash
```
```bash
docker run -p80:80 -p8765:8765 -v $PWD:/source -it klavins/enviro:1.1 bash
```
```bash
docker run -p80:80 -p8765:8765 -v $PWD:/source -it klavins/enviro:1.01 bash
```
```bash
docker run -p80:80 -p8765:8765 -v $PWD:/source -it klavins/enviro:alpha bash
```
The above commands do the following:

- The `-p80:80 option maps _port_ 80 from the Docker container to ports on your host computer. This should allow you to go to

  > http://localhost
  > with your web browser and see the ENVIRO client. It should say "Error: Failed to fetch. Is the server running?" at this point.

- The `-p8765:8765` option allows the ENVIRO client to communicate with the `enviro` server (not yet running) via a _WebSocket_.

- The command `esm start` uses the <u>e</u>nviro <u>s</u>etup <u>m</u>anager to start a web server from which your browser can access the ENVRIO client.

# Creating a Project

To create a new project, use `esm` as follows:

```bash
mkdir my_project
cd my_project
esm init
esm start
```

This will create the following directories and files

```
Makefile
config.json
defs/
    README.md
lib/
    README.md
src/
    Makefile
```

Make the project and start the enviro server as follows.

```bash
make
enviro
```

Then navigate to `http://localhost` you should see a rectangular walled area.

You can press `Ctrl-C` to stop the enviro server.

# Adding a Robot

To add a robot to your project, you have to create some new files and edit the `config.json` file. First, do

```bash
esm generate MyRobot
```

This will create the files

```
defs/
  my_robot.json         // Defines the shape, mass, and other parameters of the robot
src/
  my_robot.h            // Contains classes inheriting from elma::Process that define the behavior
  my_robot.cc           // Contains the implementation of the classes in my_robot.h
```

To compile the robot code, do

```
make
```

This will make the file `lib/my_robot.so`, which is a **shared object file** containing the compiled code for your robot.

# Using the Robot

To place the robot into the simulation, change the `agents` entry in `config.json` to

```json
"agents": [
    {
        "definition": "defs/my_robot.json",
        "style": { "fill": "lightgreen", "stroke": "black" },
        "position": {
            "x": 0,
            "y": 0,
            "theta": 0
        }
    }
]
```

Now you should be able to run

```bash
enviro
```

and see a green square in the environment. That's your robot!

Press `Ctrl-C` to stop enviro.

# Change the Look of Your Robot

Edit the file `defs/my_robot.json` to change what the robot looks like. For example, we can give it a gripper.

```json
{
  "name": "MyRobot",
  "type": "dynamic",
  "description": "A Robot with a (static) gripper",
  "shape": [
    { "x": -10, "y": 10 },
    { "x": 10, "y": 10 },
    { "x": 10, "y": 2 },
    { "x": 12, "y": 2 },
    { "x": 12, "y": 6 },
    { "x": 16, "y": 6 },
    { "x": 16, "y": 4 },
    { "x": 14, "y": 4 },
    { "x": 14, "y": -4 },
    { "x": 16, "y": -4 },
    { "x": 16, "y": -6 },
    { "x": 12, "y": -6 },
    { "x": 12, "y": -2 },
    { "x": 10, "y": -2 },
    { "x": 10, "y": -10 },
    { "x": -10, "y": -10 },
    { "x": -9, "y": 0 }
  ],
  "friction": {
    "collision": 5,
    "linear": 40,
    "rotational": 600
  },
  "sensors": [],
  "mass": 1,
  "controller": "lib/my_robot.so"
}
```

Start `enviro` to reload the simulation.

Press `Ctrl-C` to stop the simulation.

# Make the Robot Move

Edit the file `src/my_robot.h` and change the `update` method in the `MyRobotController` class to

```c++
void update() {
    track_velocity(10,0.15);
}
```

The `track_velocity(v,omega)` method attempts to make the robot move at `v` units per second in the direction it is pointing while rotating at `omega` radians per second. It is implemented as a proportional controller, where the force and torque applied to the robot are

> -K<sub>L</sub> ( v<sub>actual</sub> - v )

> -K<sub>R</sub> ( w<sub>actual</sub> - w )

Since the actual speed and angular velocity are changing over time, we have to run this command in the `update` function of the robot controller so that it can continuously adjust the force and toque applied to the robot.

Rebuild the project and run:

```bash
make
enviro
```

You should see your robot moving around in circles.

Press `Ctrl-C` to stop the simulation.

# Add More Robots

Edit `config.json` and add several more MyRobot entries to the `agents` list. Do this by copying the one that's there, but make sure to change the initial positions (and possibly angles) so that the robots are not initialized to be all on top of each other. For example, try putting four robots in the locations (100,0), (-100, 0), (0, 100), and (0,-100).

Rebuild the project and run:

```bash
make
enviro
```

You should see more robots.

Press `Ctrl-C` to stop the simulation.

# Add Some Blocks

First, create a new agent type

```bash
esm generate Block
```

Then add some blocks to the `agents` list in `config.json`. These should look something like

```json
{
  "definition": "defs/block.json",
  "style": { "fill": "orange", "stroke": "black" },
  "position": {
    "x": 50,
    "y": 0,
    "theta": 0
  }
}
```

By default, new agents will have no friction with the environment, so they will move around as though on ice. To damp this movement with the friction parameters in `defs/block.json`, change the `update` method in `src/block.h` to

```c++
void update() {
    damp_movement();
}
```

Rebuild the project and run:

```bash
make
enviro
```

You should see some orange blocks being pushed around by the robots.

Press `Ctrl-C` to stop the simulation.

# In Class Work

Now you try the above steps.

# Enviro Architecture

Enviro is built on top of Elma. You define processes and state machines just like you would with elma, but also add some interfacing to the enviro simulator.

<img src='https://raw.githubusercontent.com/klavins/EEP520-W20/master/week_8/images/enviro-architecture.png' width=80%>

# Shared Object Libraries

The issue: The ENVIRO code is a few thousand lines long. We don't want to have to recompile it every time we add a new agent type. Somehow we need to compile agent code into object code and then link it **dynamically** into ENVIRO.

To do this, we compile each `src/my_robot.cc` file into a `lib/my_robot.so` file -- a shared object file. In a shared object file, the addresses of all data and functions are stored relative to some starting point. When a running executable dynamically links a shared object file, it resolves those relative addresses into actual addresses.

The Makefile compiles shared object files using g++ as in

```bash
g++ -std=c++17 -Wno-psabi -ggdb  -shared -fPIC -I /usr/local/src/enviro/server/include -I /usr/local/src/Chipmunk2D/include/chipmunk  my_robot.cc -o ../lib/my_robot.so
```

Note the `-shared` flag.

# Linking a Shared Object Library

When Enviro runs, it loads the shared object files stated in `defs/my_robot.json` using the `dlopen` function.

```c++
Agent * Agent::create_from_specification(json spec, World& world) {
    auto file = spec["definition"]["controller"].get<std::string>();
    auto handle = dlopen(file.c_str() , RTLD_LAZY);
    if (!handle) {
        std::cerr << "Error: " << file << "\n";
        throw std::runtime_error(dlerror());
    }
    auto create_agent = AGENT_CREATE_TYPE dlsym(handle, "create_agent");
    auto agent_ptr = (Agent*) create_agent(spec, world);
    agent_ptr->set_destroyer(AGENT_DESTROY_TYPE dlsym(handle, "destroy_agent"));
    return agent_ptr;
}
```

It then looks for methods called `create_agent` and `destroy_agent` in the loaded library. These are defined in your `my_robot.h` file via the line

```c++
DECLARE_INTERFACE(MyRobot)
```

which is a macro that defines those functions for your agent:

```c++
#define DECLARE_INTERFACE(__CLASS_NAME__)                                         \
extern "C" __CLASS_NAME__* create_agent(json spec, enviro::World& world) {        \
    return new __CLASS_NAME__(spec, world);                                       \
}                                                                                 \
extern "C" void destroy_agent( __CLASS_NAME__* object ) {                         \
    delete object;                                                                \
}
```

Fortunately, you don't have to worry to much about all of this, as it is all handled for you.

# The Agent Class

In `src/my_robot.h`, the `MyRobot` class is defined as follows:

```c++
class MyRobot : public Agent {
    public:
    MyRobot(json spec, World& world) : Agent(spec, world) {
        add_process(c);
    }
    private:
    MyRobotController c;
};
```

Behind the scenes, classes that inherit from `Agent` will set up the physics of the agent, and coordiate various other activites with the manager.

This class is also a container for processes. You could add more if you would like, so that it looks like:

```c++
class MyRobot : public Agent {
    public:
    MyRobot(json spec, World& world) : Agent(spec, world) {
        add_process(c);
        add_process(p);
    }
    private:
    MyRobotController c;
    MyOtherProces p;
};
```

# Agent Processes

The `MyRobotController` class is defined as follows:

```c++
class MyRobotController : public Process, public AgentInterface {
    public:
    MyRobotController() : Process(), AgentInterface() {}
    void init() {}
    void start() {}
    void update() {
        track_velocity(10,0.15);
    }
    void stop() {}
};
```

Note that it inherits from `Process`, so everything you know about Elma processes applies.

It also inherits from `AgentInterface`, which provides methods like `track_velocity`. Inheriting from two different classes is called **_multiple inheritance_**.

# Multiple Inheritance

Here is a simple example of multiple inheritance:

```c++
#include <iostream>

class A {
    public:
    int f(int x) { return x+1; }
};

class B {
    public:
    int g(int x) { return 2*x; }
};

class C : public A, public B {
    public:
    int h(int x) { return x/2; }
};

int main(void) {

    C c;

    std::cout << c.f(1) << ","
              << c.g(1) << ","
              << c.h(1) << "\n";

}
```

Thus,

```c++
class MyRobotController : public Process, public AgentInterface {
    ...
}
```

allows `MyRobotController` objects to access methods in both the `Process` class and the `AgentInterface` class.

# The Agent Interface

The `AgentInterface` class allows classes that derive from it to use the following methods:

```c++
// State
cpVect position();
cpVect velocity();
cpFloat angle();
cpFloat angular_velocity();

// Id
int id();

// Actuators
void apply_force(double thrust, double torque);
void track_velocity(double linear_velocity, double angular_velocity,
                    double kL=10, double kR=200);
void damp_movement();
void move_toward(double x, double y, double vF=10, double vR=200);
void teleport(double x, double y, double theta);

// Sensors
double sensor_value(int index);
std::vector<double> sensor_values();
```

For these methods to be available to a `Process/AgentInterface` object `c`, you **must** call

```c++
add_process(c)
```

from within the `Agent` constructor.

For example, try replacing `track_velocity` with something like `move_toward(0,0)` in the `MyRobot` class.

# Client Events

When you click on something in the ENVIRO client, an event is sent back to ENVIRO server. These events are one of the following:

- **_screen_click_** events with values of the form

  ```json
  {
    "x": 123,
    "y": 234
  }
  ```

  where the x,y values are the position in the client where the mouse was clicked.

- **_agent_click_** events with values of the form

  ```json
  {
    "x": 12,
    "y": 23,
    "id": 3
  }
  ```

  In this case, the x,y position is the position on the agent where the mouse was clicked, and the `id` is an integer identifying the agent.

# Using Click Events

Here is an example of how to use the click events. First, make a new project

```bash
cd ..
mkdir chaser
cd chaser
esm init
esm generate Chaser
esm generate Target
```

Change `defs/chaser.json` so that it uses the `MyRobot` shape from the previous example.

Add a chaser and a target to `config.json`.

Test with `enviro` to see if you get a robot and a block.

# Watch for Click Events

In `src/target.h`, you can make the target watch and respond to click events as follows:

```c++
class TargetController : public Process, public AgentInterface {

    public:
    TargetController() : Process(), AgentInterface() {}

    void init() {
        watch("screen_click", [this](Event e) {
            teleport(e.value()["x"], e.value()["y"], 0);
            emit(Event("goal_change", {
                { "x", e.value()["x"] },
                { "y", e.value()["y"] }
            }));
        });
    }

    void start() {}
    void update(){
        damp_movement();
    }
    void stop() {}

};
```

When the user clicks on the screen, the `Target` agent moves to that location, and emits a `goal_change` event, which we will respond to in a minute.

Compile this code and test to see the target move.

# Chase the Target

To get the robot to move, change the `ChaserController` to

```c++
class ChaserController : public Process, public AgentInterface {

    public:
    ChaserController() : Process(), AgentInterface() {}

    void init() {
        goal_x = 0;
        goal_y = 0;
        watch("goal_change", [this](Event e) {
            goal_x = e.value()["x"];
            goal_y = e.value()["y"];
        });
    }
    void start() {}
    void update() {
        move_toward(goal_x, goal_y);
    }
    void stop() {}

    double goal_x, goal_y;

};
```

# In Class Work

Try the above example.

# State Machines

You can use finite state machines in ENVIRO code as well. As an example, let's build a robot controller that has two states

- MoveForward
- Rotate

and that switches between the two states and random times.

First, we'll need a new project

```c++
cd ..
mkdir wanderer
cd wanderer
esm init
esm generate Wanderer
```

# Changing the Controller to a State Machine

In the `WandererController` class, change the base class from `Process` to `StateMachine`:

```c++
class WandererController : public StateMachine, public AgentInterface {
  public:
  WandererController() : StateMachine() {
      ...
```

# Switching at Random Times

To emit events that will eventually cause the state to change, we can override the `WandererController::update()` method with something like

```c++
void update() {
    if ( rand() % 100 <= 5 ) {
        emit(Event("tick"));
    }
    StateMachine::update();
}
```

This condition in the `if` statement is true only about 5% of the time, so only occasiinally ticks. The last line in the update function ensures that the superclass, `StateMachine`, has a chance to run its `update` method as well.

# The States

We then create states

```c++
    class MovingForward : public State, public AgentInterface {
        void entry(const Event& e) {}
        void during() {}
        void exit(const Event& e) {}
    };

    class Rotating : public State, public AgentInterface {
        void entry(const Event& e) {}
        void during() {}
        void exit(const Event& e) {}
    };
```

and add them to the state machine via instance variables.

```c++
    class WandererController : public StateMachine, public AgentInterface {
        // rest of code not shown
        MovingForward moving_forward;
        Rotating rotating;
    };
```

# The Transitions

The initial state and transitions are added to the constructor:

```c++
WandererController() : StateMachine() {
    set_initial(moving_forward);
    tick_name = "tick_" + std::to_string(rand() % 1000);
    add_transition("tick", moving_forward, rotating);
    add_transition("tick", rotating, moving_forward);
    moving_forward.set_tick_name(tick_name);
    rotating.set_tick_name(tick_name);
}
```

# Adding Movement

In the `MovingForward` state, we move forward:

```c++
void during() {
    track_velocity(10, 0);
}
```

and in the `Rotating` state, we rotate

```c++
void during() {
    track_velocity(0,1);
}
```

Compiling and running this code should show a robot wandering around the arena.

# In Class Work

We have prepared a demo for wanderers which can be found in this week's "wanderers" folder.

1. Run the demo. You should see 6 wanderers in an environment with 3 blocks.

2. Make all the wanderers black and move twice faster.

# Sensors

You can add a range sensor to a wanderer by modifyin `defs/wanderer.json`:

```json
"sensors": [
    {
        "type": "range",
        "location": { "x": 16, "y": 0 },
        "direction": 0
    }
]
```

This will create a range sensor, like a laser range finder, sticking out of the front of the robot (angle 0). You can then access the value of the sensor using

```c++
double v = sensor_value(0);
```

# Updating the Wanderers

The wandering robot gets stuck a lot. One way to fix this is to change the state functions to something like:

```c++
class MovingForward : public State, public AgentInterface {
    public:
    void entry(const Event& e) {}
    void during() {
        track_velocity(4,0); // Go forward
        if ( sensor_value(0) < 40 ) {
            emit(Event("ping"));
        }
    }
    void exit(const Event& e) {}
};

class Rotating : public State, public AgentInterface {
    public:
    void entry() {}
    void during() {
        track_velocity(0,2); // Rotate
        if ( sensor_value(0) > 140 ) {
            emit(Event("ping"));
        }
    }
    void exit(const Event& e) {}
};
```

In the `WandererController` constructor you would then have

```c++
add_transition("ping", moving_forward, rotating);
add_transition("ping", rotating, moving_forward);
```
