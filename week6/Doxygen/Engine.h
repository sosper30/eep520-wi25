{\rtf1\ansi\ansicpg1252\cocoartf2759
\cocoatextscaling0\cocoaplatform0{\fonttbl\f0\fswiss\fcharset0 Helvetica;}
{\colortbl;\red255\green255\blue255;}
{\*\expandedcolortbl;;}
\margl1440\margr1440\vieww11520\viewh8400\viewkind0
\pard\tx720\tx1440\tx2160\tx2880\tx3600\tx4320\tx5040\tx5760\tx6480\tx7200\tx7920\tx8640\pardirnatural\partightenfactor0

\f0\fs24 \cf0 /**\
 * @file Engine.h\
 * @brief This file contains the Engine class declaration.\
 * @author Sep Makhsous\
 * @date 2024-02-05\
 */\
\
#ifndef ENGINE_H\
#define ENGINE_H\
\
/**\
 * @class Engine\
 * @brief The Engine class represents a car engine with basic functionality.\
 * \
 * This class provides the necessary functionalities of a car's engine, \
 * including starting and stopping the engine, along with some basic attributes \
 * like horsepower and cylinder count.\
 *\
 * @author Sep Makhsous\
 * @date 2024-02-05\
 */\
class Engine \{\
private:\
    int horsepower; ///< Horsepower of the engine.\
    int cylinderCount; ///< Number of cylinders in the engine.\
\
public:\
    /**\
     * @brief Constructor for the Engine class.\
     * @param horsepower The horsepower of the engine.\
     * @param cylinderCount The number of cylinders in the engine.\
     */\
    Engine(int horsepower, int cylinderCount);\
\
    /**\
     * @brief Starts the engine.\
     * @return void\
     */\
    void start();\
\
    /**\
     * @brief Stops the engine.\
     * @return void\
     */\
    void stop();\
\
    // Additional methods can be added here\
\};\
\
#endif // ENGINE_H\
}