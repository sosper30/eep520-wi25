
// Lets create a class called DoubleArray

class DoubleArray{
    public:

    //default constructor: Creates a new instance of the DoubleArray

    DoubleArray();

    //Parameterized constructor: this is another constructoryjay allows the creation of a DoubleArray object
    //with specificinitial values

    DoubleArray(double a, double b, double step);

    //have another contructor that duplcates an existing array

    DoubleArray(const DoubleArray& other);

    //Assignment operator: allows you to set on DoubleArray object equal to another. 
    //It returns a reference to the obect to allow for changing assignments

    DoubleArray& operation=(const DoubleArray& other);

    //Destructor: a special fundtion that is called when an instance of the class is distroyed. releases memory
    ~DoubleArray();

    //Getter: a function that retrives a property's value. it returns the value of the element at the specified index
    double get(int index) const;

    int size() const; //also a Getter. lets you know the number of elemenets stored.

    //Setter
    //a function that sets a property's value. or you can update the value needs to know the index 

    void set(int index, double value);


    private:

    //private members are intental variables used by the class to maintain its state and are not accessible from outside of class

    int capacity;

    int origin;

    int end;

    double *buffer;

    const int INITIAL_CAPACITY = 10;

    int index_to_offset(int index) const;

    int offset_to_index(int offset) const;


    //returns true or false. it checks if the given offset is outside of the bounds of the buffer
    bool out_of_buffer(int offset) const;

    //resizes the buffer
    void extend_buffer(void);


}