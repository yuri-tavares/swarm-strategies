/*
  Creates automatically scenarios for stage.
*/

#include <iostream>
#include <fstream>
#include <vector>
#include <cstdlib>
#include <math.h>
#include <string.h>
#include <algorithm> 
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


/*
Returns the distance from vertical line in x0 to starting robot at queue i, inclined by angle th, in a queue starting at px in x-axis.
*/
double discount(int i,double th, double d, double x0, double px){
  double R;
  if (i >= 0){
    R = fabs(fmod(((x0 + i*d*cos(th+2.*PI/3.) - px)/cos(th)) - epsilon/2,d));
  }
  else{
    R = fabs(fmod(((x0 - i*d*cos(th-2.*PI/3.) - px)/cos(th)) - epsilon/2,d));
  }
  return R;
}

/*
  Updates the variables lastX and lastY if the robot is nearer to the corridor's semicircular tail centre than the previous value lastX and lastY. These variables need to be initialised with (x0,y0) before the first use.
  Arguments:
    lastX, lastY: previous values of the possible last robot coordinates (passed by reference);
    px, py: new values to evaluate;
    x0,y0: coordinates of the first robot to reach the target;
    s: target area's radius
    vT: corridor plus semicircle radius
*/ 
inline void updateIfItIsTheLastRobot(double &lastX, double &lastY, double px, double py, double x0, double y0, double s, double vT){
  if (fabs(vT - s + x0 - px) + fabs(y0 - py) < fabs(vT - s + x0 - lastX) + fabs(y0 - lastY)){
    lastX = px;
    lastY = py;
  }
}

/*
Translation to -(lx,ly) followed by rotation transform by th radians.
Arguments:
  x,y: point to rotate
  lx,ly: coordinates of the center of the axis system
  th: angle to rotate in radians.
*/
coord Rotation(double x,double y,double lx,double ly,double th){
  coord p;
  p.x = cos(th)*(x-lx) - sin(th)*(y-ly);
  p.y = sin(th)*(x-lx) + cos(th)*(y-ly);
  return p;
}

/*
Rotation transform by -th radians followed by  translation to (lx,ly) .
Arguments:
  x,y: point to rotate
  lx,ly: coordinates of the center of the axis system
  th: angle to rotate in radians.
*/
coord invRotation(double x,double y,double lx,double ly,double th){
  coord p;
  p.x = cos(-th)*x - sin(-th)*y + lx;
  p.y = sin(-th)*x + cos(-th)*y + ly;
  return p;
}

/*
Transform from hexagonal point coordinate system in relation to the last robot position to euclian space.
*/
coord UT(double xh,double yh,double d){
  coord p;
  p.x = d*xh -(d/2)*yh;
  p.y = (sqrt(3)*d/2)*yh;
  return p;
}

/*
Returns a list of two-dimensional points, spaced by d units, inside a semicircle rotated to right by 90 degrees, whose center position is at (x0 + X - s, y0 + (Y2 - Y1)/2.).

  |------- X ------|
  |- X - s --|- s -|
  _________________
                   ⎞
  (cx,cy) -> o     ⎟
  _________________⎠

Arguments:
  lastX, lastY: position of the last robot.
  d: distance between points;
  th: orientation angle in radians;
  (x0,y0): base point;
  X: width of the corridor;
  Y1, Y2: top and bottom widths measured from y0. Y1 + Y2 is the height of the corridor.
*/
vector<coord>* rangePointInsideCircle(double lastX, double lastY, double d, double th, double x0, double y0, double X, double Y1, double Y2){
  vector<coord> *L = new vector<coord>();
  double s = (Y1 + Y2)/2. ;
  double cx, cy;
  int B,U;
  cx = x0 + X - s; 
  if (X > s)
    B = (int)ceil((2*(sin(PI/3-th)*(cx-lastX) + cos(PI/3-th)*(y0-lastY-s)))/(sqrt(3)*d));
  else
    B = (int)ceil(-((2*sqrt(2*s*X-X*X))/(sqrt(3)*d))*sin(th+PI/6));
  if (X > s || atan((s/2 - sin(th)*(X-s))/(sqrt(3)*s/2 + cos(th)*(X - s))) < PI/2 - th)
    U = (int)floor((2*(sin(PI/3-th)*(cx-lastX)+cos(PI/3-th)*(y0-lastY)+s))/(sqrt(3)*d));
  else
    U = (int)floor(((2*sqrt(2*s*X - X*X))/(sqrt(3)*d))*cos(th-PI/3));
  coord C = Rotation(cx,y0,lastX,lastY,-th);
  cx = C.x;
  cy = C.y;
  double dcth = d*cos(2*PI/3+th), dsth = d*sin(2*PI/3+th);
  int xh;
  for (xh=B; xh < U+1;xh++){
    double Delta,part1,C2,Li,Y2S,Y1S;
    double tm = (sqrt(3)*(d*xh-cx) - cy);
    Delta = 4*s*s - tm*tm ;
    if (Delta < 0) continue;
    part1 = d*xh - cx + sqrt(3)*cy;
    Y1S = (part1 - sqrt(Delta))/(2*d);
    C2  = (part1 + sqrt(Delta))/(2*d);
    if (X > s)
      Li = (sin(PI/2 - th)*(d*xh - cx) + cos(PI/2-th)*cy)/(d*sin(5*PI/6 - th));
    else
      Li = sin(PI/2 - th)*xh/sin(5*PI/6 - th);
    Y2S = (Li < C2)? Li : C2;
    if (fabs(Y2S - floor(Li)) <= epsilon)
      Y2S -= 1;
    coord Ig = UT(xh,ceil(Y1S),d), Fg = UT(xh,floor(Y2S),d);
    Ig = invRotation(Ig.x,Ig.y,lastX,lastY,-th);
    Fg = invRotation(Fg.x,Fg.y,lastX,lastY,-th);
    coord p = {.x = Fg.x,.y = Fg.y, .theta = PI};
    while (p.x <= Ig.x  && p.y >= Ig.y ){
      L->push_back(p);
      p = {.x = p.x - dcth, .y = p.y -dsth, .theta = PI};
    }
  }
  return L;
}


/*
Returns a list of two-dimensional points starting at P, spaced by d units, in the direction of the angle th while the x coordinate is in [x0, x0 + X] and the y coordinate is in [y0 - Y1, y0 + Y2], i.e., the point is inside the corridor for X = vT - s. Also, it outputs the coordinate of the last robot (lastX, lastY).
Arguments:
  P: starting point;
  d: distance between points;
  th: orientation angle in radians;
  (x0,y0): base point;
  vT: corridor lenght plus semicircle radius
  Y1, Y2: top and bottom widths measured from y0. Y1 + Y2 is the height of the corridor;
  lastX, lastY: previous values of the possible last robot coordinates;
Return:
  L: the list
  lastX, lastY: last robot coordinates until the moment (passed by reference).
*/
vector<coord>* rangePointsInRectangle(coord const& P, double d, double th, double x0, double y0, double vT, double Y1, double Y2, double &lastX, double &lastY){
  double s = (Y1+Y2)/2., X = vT - s;
  vector<coord> *L = new vector<coord>();
  coord p = P;
  double dcth = d*cos(th), dsth = d*sin(th);
  while (x0 - epsilon/2 <= p.x && p.x <= x0 + X + epsilon/2 && y0 - Y1 -epsilon/2 <= p.y && p.y <= y0 + Y2 + epsilon/2){
    updateIfItIsTheLastRobot(lastX, lastY, p.x, p.y, x0, y0, s, vT);
    L->push_back(p);
    p = {.x = p.x + dcth, .y = p.y + dsth, .theta = p.theta};
  }
  return L;
}


/*
Returns a list of robots' position and orientation inside a corridor using hexagonal tiling. The corridor measures X x (Y1 + Y2) units of area. Also, it returns the pair (lastX,lastY).
  
  |------- X ------|
  |- X - s --|- s -|
  _________________
                   ⎞
  (cx,cy) -> o     ⎟
  _________________⎠
  
  Arguments:
    th: angle of the hexagonal tiling in radians;
    (x0,y0): Position of the first robot to reach the target;
    v: linear velocity;
    X: the width of the corridor;
    Y1, Y2: top and bottom widths measured from y0. Y1 + Y2 is the height of the corridor;
    d: distance from each robot;
  Return:
    L: the list of robots' positions;
    lastX, lastY: values of the last robot coordinates (passed by reference).
*/
vector<coord> * makeInitialHexagonalTiling(double th, double x0, double y0, double v, double X, double Y1, double Y2, double d, double& lastX, double& lastY){
  double s, X2;
  s = (Y1 + Y2)/2.;
  X2 = X - s;
  vector< vector<coord> > L;
  int h1 = (int) floor((Y2*cos(th))/(sqrt(3)*d/2)); //U from paper
  int h2 = (int) floor((X2*sin(th) + Y1*cos(th))/(sqrt(3)*d/2)); //-T' from paper
  coord P = {.x = x0, .y = y0, .theta = PI};
  lastX = x0;
  lastY = y0;
  vector<coord> *L1, *L0;
  L0 = rangePointsInRectangle(P,d,th,x0,y0,X,Y1,Y2,lastX,lastY);
  if (L0->size()) L.push_back(*L0);
  int i;
  for (i=1;i <= h1;i++){
    double di = discount(i,th,d,x0,x0);
    /* I need to find out why, when I use 30 degrees, the discount does not need to be the complement in Z_d (i.e., d - discount). Maybe it is because, although mod(d - mod(q,d),d) = mod(-q,d), numerically it is not!*/ 
    if (th != PI/6 && th != 0) di = d - di;
    double dx,dy;
    dx = di*cos(th);
    dy = di*sin(th);
    P = {.x = x0+dx, .y = y0 + i * (sqrt(3)*d/(2*cos(th)))+dy, .theta = PI};
    L1 = rangePointsInRectangle(P,d,th,x0,y0,X,Y1,Y2,lastX,lastY);
    if (L1->size()) L.push_back(*L1);
  }
  for (i=-1;i > -h2-1;i--){
    if (i == -1){
      if (L0->size())
        P = {.x = (*L0)[0].x + d*cos(th-2.*PI/3.), .y = (*L0)[0].y + d*sin(th-2.*PI/3.), .theta = PI};
      else
        break;
    }
    else{
      P = {.x = L[L.size()-1][0].x + d*cos(th-2.*PI/3.), .y = L[L.size()-1][0].y + d*sin(th-2.*PI/3.), .theta = PI};
    }
    int I; //I is how many ds I need to add or subtract to P
    if (y0 - Y1 <= P.y){
      I = (int) floor(((-x0 + P.x   + epsilon/2)/cos(th))/d);
      P = {.x = P.x - I*d*cos(th), .y = P.y - I*d*sin(th), .theta = PI};
    }
    if (y0 - Y1 > P.y && fmod(th,PI) != 0. ){ // this is not a 'else', because after the previous 'if' the y coord. of P could be greater than y0 - Y1
      I = (int) ceil((((y0 - Y1) - P.y - epsilon/2)/sin(th))/d);
      P = {.x = P.x + (I)*d*cos(th), .y =  P.y + (I)*d*sin(th), .theta = PI};
    }
    L1 = rangePointsInRectangle(P,d,th,x0,y0,X,Y1,Y2,lastX,lastY);
    if (L1->size()) L.push_back(*L1);
  }
  //Compute the robot positions of the semicircled corridor tail based on the last robot so far.
  L1 = rangePointInsideCircle(lastX, lastY, d, th, x0, y0, X, Y1, Y2);
  //~ cout << "From vector = " << L1->size() << " From counter = " << countSemicircularTail(th,x0,y0,X,Y1,Y2,d,lastX,lastY) << endl; 
  if (L1->size()) L.push_back(*L1);
  //reduce vectors to just one.
  vector<coord> * Ret = new vector<coord>();
  for(vector< vector<coord> >::iterator it = L.begin(); it != L.end(); ++it){
    Ret->insert(Ret->end(), it->begin(), it->end());
  }
  return Ret;
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
  double X, th /*in degrees*/,
         x0,y0,Y1,Y2,d,s,v;
  double testTime;
  try{
    th = atof(cf.valueOf("theta").c_str()); // in degrees
    y0 = atof(cf.valueOf("y0").c_str());
    d  = atof(cf.valueOf("d").c_str());
    s  = atof(cf.valueOf("s").c_str());
    v  = atof(cf.valueOf("v").c_str());
    automatic = cf.valueOf("scenario");
  }
  catch(string s){
    cout << "createscenario: Configuration file is incorret: " << s << endl;
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
      cout << "createscenario: Configuration file is incorret: " << s << endl;
      exit(1);
    }
  }
  x0 = waypoints[0][0] + s;
  Y1 = y0 + s - waypoints[0][1];
  Y2 = s - y0 + waypoints[0][1];
  double lX, lY;
  vector<coord> * positions = makeInitialHexagonalTiling((PI*th/180.), x0, y0, v, X, Y1, Y2, d, lX, lY);
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
      << setprecision(17)
      << "  pose [" << it->x << " " << it->y << " 0 " << (180/PI)*it->theta << "]" << endl
      << "  ctrl \"coordination.so " << confFile << " " << numRobot <<" " << "\"" << endl
      << ")" << endl << endl;
    
    numRobot++;
  }
   
  output.close();
  return 0;
}
