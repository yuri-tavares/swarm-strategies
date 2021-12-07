#ifndef _FINALLOG_H_
#define _FINALLOG_H_

#include "commonConfig.h"
#include <fstream>
#include <string>
#include <stdlib.h>
#include <sstream>
#include <stage.hh>

using namespace std;

class FinalLog{
  private:
    static unsigned int num_robots,numFinished,numFinishedAtTarget;
    static bool initiated;
    static ofstream logFile;
    static unsigned int numTotalIterationsReachGoal; // Total number of iterations
    static unsigned int numTotalIterationsExitGoal; // Total number of iterations
    static unsigned int numMaxIterationsReachGoal; //Maximum number of iterations to reach the goal
    static unsigned int numMaxIterationsExitGoal; //Maximum number of iterations to exit from goal
    static unsigned int numTotalStalls; //number of times that the robots stalled
    static Stg::usec_t simTime; //simulation time in microseconds
    static void saveLog();
    static string path;

  public:
    static void init(string path);
    static void refresh_not_at_target(unsigned int numIterationsReachGoal,
                               unsigned int numIterationsExitGoal,
                               unsigned int numStalls);
                               
    static void refresh(unsigned int numIterationsReachGoal,
                        unsigned int numIterationsExitGoal,
                        unsigned int numStalls,
                        Stg::usec_t sim_time);
                        
    static void notify_finish();
    static void finish();
    static void notify_finish_not_at_target();
};
#endif
