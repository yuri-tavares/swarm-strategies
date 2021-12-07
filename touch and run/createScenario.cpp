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

vector<coord> *  makeInitialPositions(double d, int n, int K, double s){
  if (K >= PI / asin(d / (2*s))) {
    cout << endl << "K is too big compared to the value of s and d" << endl;
    cout << "K should be  lower than " << floor(PI / asin(d / (2*s))) << endl;
    exit(1);
  }
  double angle = 2 * PI / K;
  double r = (s * sin(angle / 2) - d/2.) / (1 - sin(angle / 2));
  double dcurve = sqrt(pow(s + r,2) - pow(d/2. + r,2));
  
  // compute distance between robots
  double delay;
  if (2 * r * cos(angle / 2) < d) {
    delay = (r * (PI - angle))  + (d - 2 * r * cos(angle / 2)) / (sin(angle / 2));
  } else {
    delay = 2 * r * asin(d / (2 * r));
  }
  delay = max(delay, d);
  vector<coord>* L = new vector<coord>();
  int i;
  for (i = 0; i < n; i++){
    int l = i % K;
    int c = i / K;
    double ca = cos(l*angle);
    double sa = sin(l*angle);
    double d2sa = (d/2)*sa;
    double d2ca = (d/2)*ca;
    double X = waypoints[0][0] + (dcurve + c*delay)*ca + d2sa;
    double Y = waypoints[0][1] + (dcurve + c*delay)*sa - d2ca;
    coord p = {.x = X, .y = Y, .theta = PI + l*angle};
    L->push_back(p);
  }

  return L;
}

int main(int argc, char **argv)
{
  if (argc < 2)
  {
    cerr << endl
         <<"Invalid parameters" << endl;
    cerr << "Use: " << endl;
    cerr << "createScenario  <configuration file> [video]" << endl;
    exit(1);
  }
  string automatic;
  string confFile(argv[1]);
  ConfigFile cf(confFile);
  double d,s;
  int n,K;
  try{
    s = atof(cf.valueOf("s").c_str());
    d = atof(cf.valueOf("d").c_str());
    K = atoi(cf.valueOf("K").c_str());
    automatic = cf.valueOf("scenario");
  }
  catch(string st){
    cout << endl << "Configuration file is incorret: " << st << endl;
    exit(1);
  }
  try{
    n = atoi(cf.valueOf("n").c_str());
  }
  catch(string st){
    double testTime,v;
    try {
      testTime = atoll(cf.valueOf("time").c_str())/1.0e6;
      v = atof(cf.valueOf("v").c_str());
    }
    catch(string st2){
      cout << endl << "Configuration file is incorrect." << endl;
      cout << st << endl << st2 << endl; 
      exit(1);
    }
    double angle = 2 * PI / K;
    double r = (s * sin(angle / 2) - d/2.) / (1 - sin(angle / 2));
    //the testTime is extended by the time of the first robot to reach in the target region
    testTime += (r*(PI-angle)/2)/v;
    double laneLenght = testTime * v;  
    n = (int) ceil(1.1*K*laneLenght/d); //estimated length plus 10%
  }
  
  vector<coord> * positions = makeInitialPositions(d, n, K, s);
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
    << "sicklaser ()" << endl
          << "size [0.44 0.44 0.44]" << endl
    << "localization \"gps\"" << endl
    << "localization_origin [ 0 0 0 0 ]" << endl
    << ")" << endl 
    << endl
    << "model" << endl
    << "(" << endl
    << "  name \"toDraw\""<< endl
    << "  size [0.001 0.001 0.001]"<< endl
    << "  pose [0 0 0 0]"<< endl
    << "  ctrl \"draw.so " << confFile << " \""<< endl
    << "  obstacle_return 0" << endl
    << ")"<< endl << endl;
  
  for(vector<coord>::iterator it = positions->begin(); it != positions->end(); ++it)
  {
    output << "robot" << endl
      << "(" << endl
      << "  name \"robot" << numRobot << "\"" << endl
      << "  color \"red\"" << endl
      << setprecision(17)
      << "  pose [" << it->x << " " << it->y << " 0 " << (180/PI)*it->theta << "]" << endl
      << "  ctrl \"coordination.so " << confFile << " " << numRobot <<" " << "\"" << endl
      << ")" << endl << endl;
    
    numRobot++;
  }
   
  output.close();
  return 0;
}
