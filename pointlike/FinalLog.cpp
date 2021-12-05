#include "FinalLog.h"


//alocating  statics members
unsigned int  FinalLog::num_robots, FinalLog::numFinished, FinalLog::numFinishedAtTarget;
bool  FinalLog::initiated;
ofstream  FinalLog::logFile;
unsigned int FinalLog::numTotalIterationsReachGoal; // Total number of iterations
unsigned int FinalLog::numTotalIterationsExitGoal; // Total number of iterations
unsigned int FinalLog::numMaxIterationsReachGoal; //Maximum number of iterations to reach the goal
unsigned int FinalLog::numMaxIterationsExitGoal; //Maximum number of iterations to exit from goal
unsigned int FinalLog::numTotalStalls; //number of times that the robots stalled
Stg::usec_t FinalLog::simTime; //simulation time in microseconds
string FinalLog::path;
double FinalLog::pNy;
double FinalLog::m_x_0;

void FinalLog::init(string p){
  if (!initiated){
    simTime = 0;
    numTotalIterationsReachGoal = 0;
    numTotalIterationsExitGoal = 0;
    numMaxIterationsReachGoal = 0;
    numMaxIterationsExitGoal = 0;
    numTotalStalls = 0;
    initiated = true;
    numFinished = numFinishedAtTarget = 0;
    num_robots = 0;
    path = p;
  }
  num_robots++;
}

void FinalLog::refresh_not_at_target(unsigned int numIterationsReachGoal,
                                     unsigned int numIterationsExitGoal,
                                     unsigned int numStalls){
  numTotalStalls += numStalls;
  numTotalIterationsReachGoal += numIterationsReachGoal;
  numTotalIterationsExitGoal += numIterationsExitGoal;
  if (numIterationsReachGoal > numMaxIterationsReachGoal)
     numMaxIterationsReachGoal = numIterationsReachGoal;
  if (numIterationsExitGoal > numMaxIterationsExitGoal)
     numMaxIterationsExitGoal = numIterationsExitGoal;
}

void FinalLog::refresh(unsigned int numIterationsReachGoal,
                    unsigned int numIterationsExitGoal,
                    unsigned int numStalls,
                    Stg::usec_t sim_time,
                    double m_y,
                    double mx0){
  numTotalStalls += numStalls;
  numTotalIterationsReachGoal += numIterationsReachGoal;
  numTotalIterationsExitGoal += numIterationsExitGoal;
  if (numIterationsReachGoal > numMaxIterationsReachGoal)
     numMaxIterationsReachGoal = numIterationsReachGoal;
  if (numIterationsExitGoal > numMaxIterationsExitGoal)
     numMaxIterationsExitGoal = numIterationsExitGoal;
  if (sim_time > simTime)
    simTime = sim_time;
  pNy = m_y;
  m_x_0 = mx0;
}

void FinalLog::saveLog(){
  logFile.open((path.c_str()));
  logFile << numTotalIterationsReachGoal + numTotalIterationsExitGoal << endl;
  logFile << numMaxIterationsReachGoal + numMaxIterationsExitGoal<< endl;
  logFile << numTotalIterationsReachGoal << endl
          << numTotalIterationsExitGoal << endl;
  logFile << numMaxIterationsReachGoal << endl
          << numMaxIterationsExitGoal<< endl;
  logFile << numTotalStalls << endl;
  logFile << simTime << endl; //in microseconds
  logFile << numFinishedAtTarget << endl;
  logFile << pNy << endl; 
  logFile << m_x_0 << endl; 
  logFile.close();
}

void FinalLog::finish_not_at_target(){
  numFinished++;
  if (numFinished == num_robots){
     saveLog(); 
     exit(0);
  }
}

void FinalLog::finish(){
  numFinished++;
  numFinishedAtTarget++;
  if (numFinished == num_robots){
     saveLog(); 
     exit(0);
  }
}

