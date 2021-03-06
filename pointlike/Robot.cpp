/*
  This file implements a robot without coordination.
*/

#include "Robot.h"


//Initializes values for sensing with laser, depending on the world file used.
void Robot::init_laser(){
  //init laser configuration for use in getBearing()
  ModelRanger::Sensor sensor = laser->GetSensors()[0];
  LASER_FOV  = rtod(sensor.fov);
  LASER_SAMPLES = sensor.sample_count;
  laser->vis.showArea.set(0);
}

//Function used on Stage simulation. It specifies 
//what the robot will do while walking
//See Robot.h for signature
int PositionUpdate(Model *pos, Robot *robot)
{
  robot->mainLoop();
  return 0;
}

//returns the distance between a robot and your goal
double Robot::pho()
{
   return hypot(m_x - destinationX, m_y - destinationY);
}

/*Constructor arguments:
    conf: file name with experiments variables.
*/
Robot::Robot(string const& confFile){
  readConfigFile(confFile);
}

//Reads a config file and return the parameters
void Robot::readConfigFile(string const& confFileName){
  ConfigFile cf(confFileName);
  try{
    linSpeed     = atof(cf.valueOf("v").c_str());
    folder       = cf.valueOf("folder");
    log_name     = cf.valueOf("log");
  }
  catch (string s) {
    cout << "Configuration file is incorret: " << s << endl;
    exit(1);
  }
  try{
    testTime = atoll(cf.valueOf("time").c_str());
  }
  catch (string s) {
    testTime = 0xFFFFFFFFFFFFFFFF;
  }
}

//Initialize all robot data (pose, log file, velocity, etc.)
void Robot::init(int id)
{
  m_id = id;
  m_name = "robot" + intToStr(id);
  Pose pose = pos->GetPose();
  m_x = m_x_0 = pose.x;
  m_y = pose.y;
  m_th = pose.a;
  numIterations = numIterationsReachGoal = 0;
  #ifdef GENERAL_LOG
    log.open(("logs/"+m_name).c_str());
  #endif
  FinalLog::init(folder+"/"+log_name);
  init_laser();
  finished = finishedBySimTime = false;
  stalls = 0; 
  alreadyStalled = false;
  destinationX = waypoints[0][0];
  destinationY = waypoints[0][1];
  m_state = GOING;
  pos->SetColor(GOING_COLOR);
}

//Finish robot. Here only display a message for looking with the simulation is still running.
void Robot::finish()
{
  //This message is to see how many robots ended while experimentation scripts are ongoing, then I can see if it is stopped.
  cout << "Robot " << m_id << " finished!" << endl;
   #ifdef GENERAL_LOG
   log.close();
   #endif
}


//Implements the main loop of robot. 
//Also contain robot controller and 
//probabilistic finite state machine codes
void Robot::mainLoop(){
  numIterations++;

  //how many times robot stall?
  if (pos->Stalled()){
    if (!alreadyStalled){
      stalls++;
      alreadyStalled = true;
    }
  }
  else{
    alreadyStalled = false;
  }

  Pose pose = pos->GetPose();
  m_x = pose.x;
  m_y = pose.y;
  m_th = pose.a;


  if ((pho() <= 0.000000000004) && !finished)
  {
    finished = true;
    m_state = GOING_OUT;
    numIterationsReachGoal = numIterations;
    numIterations = 0;
    FinalLog::refresh(numIterationsReachGoal, numIterations, stalls, theWorld->SimTimeNow(), m_y, m_x_0);
    pos->SetColor(END_COLOR);
    FinalLog::finish();
    finish();
  }

  if ((testTime - FINISH_TIME < theWorld->SimTimeNow()) && !finishedBySimTime ){
    finishedBySimTime = true;
    numIterationsReachGoal = numIterations;
    numIterations = 0;
    FinalLog::refresh_not_at_target(numIterationsReachGoal, numIterations, stalls);
    FinalLog::finish_not_at_target();
    pos->SetColor(END_COLOR);
    finish();
  }
  this->pos->SetXSpeed(linSpeed);


  #ifdef GENERAL_LOG
  log << m_x << " " << m_y << " " 
      << destinationX << " " << destinationY << " "
      << waitX << " "<< waitY << " "
      << fx << " "<<  fy << " "
      << linSpeed << " "<<  rotSpeed << " "
      << inFrontMisses << " " << m_state << " "
      << linAccel << " " << rotAccel
      << endl;
  #endif
}



//Pointer to a new robot.
//Every call of this library will create a new robot
//using this pointer.
Robot* robot;


extern "C" int Init(Model *mod, CtrlArgs *args){
  vector<string> tokens;
  Tokenize(args->worldfile, tokens); 
  if (tokens.size() < 3){
    cout << "Wrong number of arguments." << endl;
    cout << "Usage:" << endl
         << "  coordination.so <config file> <robot id>" << endl;
    exit(1);
  }
  robot = new Robot(tokens[1]); 
  ModelPosition *pmod = (ModelPosition*) mod;
  robot->pos = pmod;
  robot->theWorld = pmod->GetWorld(); //??? ??? ??? ???
  robot->pos->AddCallback(Model::CB_UPDATE, (model_callback_t)PositionUpdate, robot );
  robot->laser = (ModelRanger*)mod->GetChild( "ranger:1" );
  
  robot->laser->Subscribe(); // starts the laser updates
  robot->pos->Subscribe(); // starts the position updates

  robot->init(atoi(tokens[2].c_str()));
  #ifdef DEBUG_FORCES
  robot->pos->AddVisualizer( &robot->fv, true );
  #endif
  return 0;
}

