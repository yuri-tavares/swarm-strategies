/*
  This file implements a robot without coordination.
*/
#ifndef _ROBOT_H_
#define _ROBOT_H_


#include <stdlib.h>
#include <stage.hh>
#include <iostream>
#include <fstream>
#include <string>
#include <iomanip>
#include "FinalLog.h"
#include "ConfigFile.h"
#include "commonConfig.h"
#include "commonDefs.h"
#include "util.h"
#include "option.hh"
#ifdef DEBUG_FORCES
#include "forcevisualizer.h"
#endif

using namespace std;
using namespace Stg;

/*Implements a robot without coordination*/
class Robot
{
  public:
   /*Constructor arguments:
       conf: file name with experiments variables.
   */
   Robot(string const& conf);
  
   //Initialize all robot data (pose, log file, velocity, etc.)
   void init(int id);
   
   //Finish robot. Here only display a message for looking with the simulation is still running.
   void finish();

   //Implements the main loop of robot. 
   //Also contain robot controller and 
   //probabilistic finite state machine codes
   void mainLoop();
   
   //Pointers to classes used in Stage
   ModelPosition* pos;
   ModelRanger* laser;
   World* theWorld; //ゴ ゴ ゴ ゴ
   
   //This member allows visualize forces for debug
   #ifdef DEBUG_FORCES
     ForceVisualizer fv;
   #endif
  private:
   //save data of this robot on own log
   void saveMyLog();
  
   //returns the distance between a robot and your goal
   double pho();
   
   //Initializes values for sensing with laser, depending on the world file used.
   void init_laser();

   //Reads a config file and return the parameters
   static void readConfigFile(string const& confFileName);

   //Check if the config file was already read.
   static bool alreadyReadConfig;

   //linear velocity of the robot
   static double linSpeed;

   //Radius of target area
   static double waypointDist;
   
   //Folder where the logs will be saved
   static string folder;
   
   //Log file name
   static string log_name;
   
   //Maximum time in miliseconds to test
   static unsigned long long testTime;
   
   //The name of a robot, used in some functions of Player/Stage
   string m_name;

   //Actual x coordinate of robot position
   double m_x;

   //Actual y coordinate of robot position
   double m_y;

   //Actual theta orientation of robot
   double m_th;
   
   //Extra time to finish simulation;
   const unsigned long long FINISH_TIME = 100000;
   
   //Identifier of the robot. Used in communication and for generate the name of robot
   int m_id;
  
   //State of robot in the experiment
   int m_state;
 
   //Actual number of iterations.
   int numIterations;

   //number of iterations until reach the goal. The number of iterations from 
   //reach the goal until exit a specified area from goal will be the difference
   //between this value and numIterations
   int numIterationsReachGoal;

   //x coordinate to the goal position
   double destinationX;
  
   //y coordinate to the goal position
   double destinationY;

   //Indicates if the robot finished your execution
   bool finished;
   
   //Indicates if the robot finishes your execution by time limit
   bool finishedBySimTime;
   
   //Indicates if the robot arrived at target area
   bool arrivedAtTarget;
   
   //Used for changing the colour at the ending of the experiment
   int alreadyChanged;

   //FOV and SAMPLES for laser. These values are token from Stage world file and computed in init_laser()
   double LASER_FOV,LASER_SAMPLES;

   //check if robot is already stalled
   bool alreadyStalled;

   //Number of times that robot became stalled
   int stalls;
   
   ofstream log;
};

//Function used on Stage simulation. It specifies 
//what the robot will do while walking
//See commonMethods.cpp for implementation in the folder common
int PositionUpdate(Model *pos, Robot *robot);


#endif
