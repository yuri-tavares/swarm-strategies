#ifndef _COMMONCONFIG_H_
#define _COMMONCONFIG_H_

//Waypoints chosen randomly
const int waypoints[3][2] = { {100,100}, {-999999, 100}, {999999,100}/*, {0,-999999}, {0,999999}*/ };
#define DEAD_ITERATIONS 100000 //Maximum iterations for a experiment
//#define DEBUG_FORCES //If defined, it allow compilation of force visualization codes

//how much is next to the target region to consider that the robot touched it. Useful because of the precision of double floating point numbers.
const double epsilon = 0.000000;

const double Kr = 1; //constant for robot controller

//number of iterations to wait for the last robot changes its colour. 
const int ITERATION_FOR_CHANGING_COLOR = 1;

//Extra time to finish simulation;
const unsigned long long FINISH_TIME = 100000;
#endif
