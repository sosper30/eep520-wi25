{\rtf1\ansi\ansicpg1252\cocoartf2759
\cocoatextscaling0\cocoaplatform0{\fonttbl\f0\fswiss\fcharset0 Helvetica;}
{\colortbl;\red255\green255\blue255;}
{\*\expandedcolortbl;;}
\margl1440\margr1440\vieww11520\viewh8400\viewkind0
\pard\tx720\tx1440\tx2160\tx2880\tx3600\tx4320\tx5040\tx5760\tx6480\tx7200\tx7920\tx8640\pardirnatural\partightenfactor0

\f0\fs24 \cf0 /**\
 * @file main.cpp\
 * @brief Main entry point demonstrating the usage of Car and Engine classes.\
 * @author Sep Makhsous\
 * @date 2024-02-05\
 */\
\
#include "Car.h"\
#include "Engine.h"\
\
/**\
 * Main function demonstrating the creation and usage of Car and Engine objects.\
 * @return int Returns 0 on successful execution.\
 */\
int main() \{\
    // Create an Engine object with 150 horsepower and 4 cylinders\
    Engine myEngine(150, 4);\
\
    // Create a Car object with the specified engine\
    Car myCar(myEngine);\
\
    // Start the car's engine\
    myCar.startEngine();\
\
    // Stop the car's engine\
    myCar.stopEngine();\
\
    return 0;\
\}\
}