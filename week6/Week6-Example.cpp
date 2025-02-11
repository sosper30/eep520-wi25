//Tuples
//auto
// polymorphism



//Function that returns a tuple containing a user's name, age, and account balance. 
#include <tuple>
#include <string>
#include <iostream>

//tuple example
std::tuple<std::string, int, float> getUserDetails(){
    std::string name = "Sep Makhsous";
    int age = 33;
    float balance = 1000000.25;

    return std::make_tuple(name, age, balance);
}

int main(){
    //we get to use auto to define a new array to save the return values of our tuple
    auto [name, age, balance] = getUserDetails();

    std::cout<<"Name: "<<name<<std::endl;
    //repeat the same thing to display age and balance

    return 0;
}

//--------------------------------------------------------


//polymorphism

#include<iostream>


// showing how to do compile-time polymorphism through function overloading
// you can see that the function name 'add' is the same but with different parameters.
class Calculator{
    public:

    int add(int a, int b){
        return a+b;
    }

    int add(int a, int b, int c){
        return a+b+c;
    }

    double add(double a, double b){
        return a+b;
    }
};

int main (){

    Calculator calc;

    auto sum = calc.add(3.3,4.2);

    std::cout<<"Sum of 3.3 and 4.2:"<<calc.add(3.3, 4.2)<<std::endl;


}

//--------------------------------------------------------

//using virtual 

class Base {
    public:

    //virtual function declared in the base class allows for runtime polymorphism
    virtual void print() const { std::cout << "Base Class print Function\n";}
};

class Derived : public Base {
    public:
    void print() const override {std::cout << "Derived Class print Function\n";}
};

int main(){
    // write a test that uses print() and figure out if we successfully replaced Base with Derived 
    Base* basePtr;
    Derived derivedObj;

    basePtr = &derivedObj;

    basePtr->print();

    return 0;
}