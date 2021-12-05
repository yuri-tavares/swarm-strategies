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

bool Robot::alreadyReadConfig = false;
double Robot::linSpeed;
double Robot::waypointDist;
string Robot::folder;
string Robot::log_name;
unsigned long long Robot::testTime;
int Robot::K_path;
double Robot::d;

//Reads a config file and return the parameters
void Robot::readConfigFile(string const& confFileName){
  if (alreadyReadConfig) return;
  ConfigFile cf(confFileName);
  try{
    linSpeed     = atof(cf.valueOf("v").c_str());
    d            = atof(cf.valueOf("d").c_str());
    waypointDist = atof(cf.valueOf("s").c_str());
    folder       = cf.valueOf("folder");
    log_name     = cf.valueOf("log");
    K_path       = atoi(cf.valueOf("K").c_str());
  }
  catch (string s) {
    cout << endl << "Robot.o: Configuration file is incorret: " << s << endl;
    exit(1);
  }
  try{
    testTime = atoll(cf.valueOf("time").c_str());
  }
  catch (string s) {
    testTime = 0xFFFFFFFFFFFFFFFF;
  }
  double K = K_path, s = waypointDist, v = linSpeed;
  double angle = 2 * PI / K;
  double r = (s * sin(angle / 2) - d/2.) / (1 - sin(angle / 2));
  //the testTime is extended by the time of the first robot to reach in the target region
  testTime += 1e6*(r*(PI-angle)/2)/v;
  alreadyReadConfig = true;
}

//Initialize all robot data (pose, log file, velocity, etc.)
void Robot::init(int id)
{
  m_id = id;
  m_name = "robot" + intToStr(id);
  Pose pose = pos->GetPose();
  m_x  = pose.x;
  m_y = pose.y;
  m_th = pose.a;
  numIterations = numIterationsReachGoal = 0;
  FinalLog::init(folder+"/"+log_name);
  init_laser();
  finished = finishedBySimTime = false;
  stalls = 0; 
  alreadyStalled = false;
  arrivedAtTarget = false;
  alreadyChanged = 1;
  destinationX = waypoints[0][0];
  destinationY = waypoints[0][1];
  m_state = GOING;
  pos->SetColor(GOING_COLOR);
  
  //the steering radius is adjusted based on the number of paths K and delta.
  mysteeringRad = ( d/2. - waypointDist * sin(PI/K_path) )/( sin(PI/K_path) - 1 );
  maxDistForCurve = sqrt(waypointDist * (waypointDist + 2*mysteeringRad) - mysteeringRad*d);
  
  rotSpeed = linSpeed/mysteeringRad;
  if (fabs(rotSpeed) > PI/2){
    cout << endl << "Current turning speed is above the maximum. Use a smaller K." << endl;
    exit(1);
  }
  finalTh = m_th + PI - 2*PI/K_path;
  init_laser();
}

//Finish robot. Here only display a message for looking with the simulation is still running.
void Robot::finish()
{
   //This message is to see how many robots ended while experimentation scripts are ongoing, then I can see if it is stopped.
   cout << "Robot " << m_id << " finished!" << endl;
}

//Subtract two angle values (in radians). The result value lies between -2 PI and 2 PI. 
double Robot::angDiff(double end, double begin)
{
   double returnMe = end - begin;
  
   if (returnMe > PI)
      returnMe = -(2*PI - returnMe);
   else if (returnMe < -PI)
      returnMe = 2*PI + returnMe;
    
   return returnMe;
}

//Check distance between robots, however it is not exact because of the oblique rays.
void Robot::obstacleChecking()
{  
   double distance;

   const std::vector<meters_t>& scan = laser->GetSensors()[0].ranges;
   uint32_t sample_count = scan.size();
   
   for(uint32_t i = 0; i < sample_count; i++)
   {
      distance = scan[i];
      if (distance <= d)
      {
        cout << distance << " " << m_id << endl;
      }
   }
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

  //~ obstacleChecking(); // not exact because of the oblique rays

  if (finished && arrivedAtTarget){
    pos->SetColor(END_COLOR);
    // This conditions serves to give some time before finishing simulation. 
    if (alreadyChanged % ITERATION_FOR_CHANGING_COLOR == 0){
      FinalLog::finish();
    }
    alreadyChanged++;
  }

  if ((pho() < waypointDist + epsilon) && !finished)
  {
    finished = true;
    arrivedAtTarget = true;
    m_state = GOING_OUT;
    numIterationsReachGoal = numIterations;
    numIterations = 0;
    FinalLog::refresh(numIterationsReachGoal, numIterations, stalls, theWorld->SimTimeNow(), m_y, m_x);
    FinalLog::notify_finish();
    saveMyLog();
    finish();
  }

  if ((testTime - FINISH_TIME < theWorld->SimTimeNow()) && !finishedBySimTime ){
    finishedBySimTime = true;
    numIterationsReachGoal = numIterations;
    numIterations = 0;
    FinalLog::refresh_not_at_target(numIterationsReachGoal, numIterations, stalls);
    FinalLog::notify_finish_not_at_target();
    // this robot does not save own log.
    finish();
    FinalLog::finish();
  }
  this->pos->SetXSpeed(linSpeed);
  if (pho() <= maxDistForCurve){
    this->pos->SetTurnSpeed(rotSpeed);
  }
  else{
    if (finished){
      this->pos->SetTurnSpeed(Kr*angDiff(finalTh, m_th));
    }
    else
      this->pos->SetTurnSpeed(0);
  }
}

void Robot::saveMyLog(){
  log.open((folder+"/"+m_name).c_str());
  log << m_id << endl
      << numIterationsReachGoal << endl
      << numIterations << endl
      << theWorld->SimTimeNow() << endl
      << setprecision(17)
      << m_y << endl
      << m_x << endl
      << endl;
  log.close();
}

//Pointer to a new robot.
//Every call of this library will create a new robot
//using this pointer.
Robot* robot;


extern "C" int Init(Model *mod, CtrlArgs *args){
  vector<string> tokens;
  Tokenize(args->worldfile, tokens); 
  if (tokens.size() < 3){
    cout << endl;
    cout << "Wrong number of arguments." << endl;
    cout << "Usage:" << endl
         << "  coordination.so <config file> <robot id>" << endl;
    exit(1);
  }
  robot = new Robot(tokens[1]); 
  ModelPosition *pmod = (ModelPosition*) mod;
  robot->pos = pmod;
  robot->theWorld = pmod->GetWorld(); //ゴ ゴ ゴ ゴ
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

