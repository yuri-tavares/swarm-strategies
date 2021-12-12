/*
  Creates automatically scenarios for stage.
*/

#include <iostream>
#include <fstream>
#include <vector>
#include <cstdlib>
#include <math.h>
#include <string.h>
#include "commonConfig.h"
#include "ConfigFile.h"

#include <iomanip>

using namespace std;

#define PI M_PI

typedef struct coord
{
      double x;
      double y;
      double theta; //in radians
} coord;

vector<coord>* makeQueue(double X, double d){
  double Y = waypoints[0][1];
  vector<coord> * L = new vector<coord>();
  double x0 = waypoints[0][0]  + 1;
  coord p = {.x = x0, .y = Y, .theta = PI};
  while (p.x <= X + waypoints[0][0]){
    L->push_back(p);
    p = {.x = p.x + d, .y = Y, .theta = PI};
  }
  return L;
}

int main(int argc, char **argv)
{
  if (argc < 2)
  {
    cerr << "Invalid parameters" << endl;
    cerr << "Use: " << endl;
    cerr << "createScenario  <configuration file> [video]" << endl;
    exit(1);
  }
  string automatic;
  string confFile(argv[1]);
  ConfigFile cf(confFile);
  double X,d,v;
  double testTime;
  try{
    d  = atof(cf.valueOf("d").c_str());
    v  = atof(cf.valueOf("v").c_str());
    automatic = cf.valueOf("scenario");
  }
  catch(string s){
    cout << "Configuration file is incorret: " << s << endl;
    exit(1);
  }
  try{
    testTime = atoll(cf.valueOf("time").c_str())/1.0e6;
    X = 1.1 * testTime * v; //estimated lenght plus 10%
  }
  catch(string s){
    try{
      X  = atof(cf.valueOf("X").c_str());
    }
    catch(string s){
      cout << "Configuration file is incorret: " << s << endl;
      exit(1);
    }
  }
  vector<coord> * positions = makeQueue(X, d);
  int numRobot = 0;

  ofstream output((automatic + ".world").c_str());
  output << "# defines 'map' object used for floorplans" << endl
    << "include \"map.inc\"" << endl << endl

    << "# defines sick laser" << endl
    << "include \"sick.inc\"" << endl << endl
    << "# defines Pioneer-like robots" << endl
    << "include \"pioneer.inc\"" << endl << endl
    << "# set the resolution of the underlying raytrace model in meters" << endl
    << "resolution 0.1" << endl
    << "" << endl
    << "speedup -1" << endl
    << "" << endl
    << "# configure the GUI window" << endl
    << "window" << endl
    << "( " << endl
    << "  size [ 591.000 638.000 ] " << endl
    << "  center [ " << waypoints[0][0] <<  " " << waypoints[0][1] << " ] " << endl
    << "  show_data 1" << endl;

  if (argc >= 3){
    if (strcmp(argv[2],"video") == 0){
      output << "  screenshots 1" << endl;
    }
  }

  output << ")" << endl
    << "define robot pioneer2dx" << endl
    << "(" << endl
    << "  sicklaser ()" << endl
    << "  size [0.44 0.44 0.44]" << endl
    << "  localization \"gps\"" << endl
    << "  localization_origin [ 0 0 0 0 ]" << endl
    << ")" << endl << endl
    
    << "model" << endl
    << "(" << endl
    << "  name \"toDraw\""<< endl
    << "  size [0.001 0.001 0.001]"<< endl
    << "  pose [0 0 0 0]"<< endl
    << "  ctrl \"draw.so " << confFile  << " \""<< endl
    << "  obstacle_return 0" << endl
    << ")"<< endl << endl;
  
  for(vector<coord>::iterator it = positions->begin(); it != positions->end(); ++it)
  {
    output << "robot" << endl
      << "(" << endl
      << "  name \"robot" << numRobot << "\"" << endl
      << "  color \"red\"" << endl
      << "  pose [" << it->x << " " << it->y << " 0 " << (180/PI)*it->theta << "]" << endl
      << "  ctrl \"coordination.so " << confFile << " " << numRobot <<" " << "\"" << endl
      << ")" << endl << endl;
    
    numRobot++;
  }
   
  output.close();
  return 0;
}
