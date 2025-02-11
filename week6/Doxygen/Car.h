 /**
 * @file Car.h
 * @brief This file contains the Car class declaration.
 * @author Sep Makhsous
 * @date 2024-02-05
 */

#ifndef CAR_H
#define CAR_H

#include "Engine.h"

/**
 * @class Car
 * @brief The Car class represents a car with an engine and provides functionalities to interact with the engine.
 *
 * This class demonstrates the usage of the Engine class, allowing the car to start or stop its engine, 
 * showcasing the relationship and interaction between different objects/classes.
 * 
 * @author Sep Makhsous
 * @date 2024-02-05
 */
class Car {
private:
    Engine engine;

public:
    /**
     * @brief Constructor for the Car class.
     * @param engine An Engine object representing the car's engine..
     */
    Car(const Engine& engine);

    /**
     * @brief Starts the car's engine.
     * @return void
     */
    void startEngine();

    /**
     * @brief Stops the car's engine.
     * @return void
     */
    void stopEngine();

    // Additional methods can be added here
};

#endif // CAR_H