/*
  This file implement a shared library using functions to draw lines on Stage.
*/

#include <stage.hh>
#include "commonConfig.h"
#include "ConfigFile.h"
#include "util.h"

using namespace Stg;
using namespace std;

#define PI M_PI

//Define a point
struct _Ponto{
  double px;
  double py;
};

typedef struct _Ponto Ponto;

//class that implement Rects Visualizer
class RegionsVisualizer: public Visualizer{
  private:
    double corridorLength,x0;
    double waypointDist; // radius of target area.

    //Reads a config file and intializes the private members.
    void readConfigFile(string const& confFileName){
      double v = 0.;
      ConfigFile cf(confFileName);
      try{
        waypointDist = atof(cf.valueOf("s").c_str());
        v = atof(cf.valueOf("v").c_str());
      }
      catch(string s) {
        cout << "draw.so: Configuration file is incorret: " << s << endl;
        exit(1);
      }
      try{
        double testTime = atoll(cf.valueOf("time").c_str())/1.0e6;
        corridorLength = 1.1 * testTime * v; //estimated lenght plus 10%
      }
      catch(string s){
        try{
          corridorLength  = atof(cf.valueOf("X").c_str());
        }
        catch(string s){
          cout << "draw.so: Configuration file is incorret: " << s << endl;
          exit(1);
        }
      }
      x0 = waypoints[0][0] + waypointDist;
    }
    
    //Draw target region and corridor
    void drawRegions(){
      // upper corridor lines
      glBegin(GL_LINES);
        glVertex2f(x0, waypoints[0][1] + waypointDist);
        glVertex2f(x0 + (corridorLength - waypointDist), waypoints[0][1] + waypointDist);
      glEnd();
      
      // left side vertical line
      glBegin(GL_LINES);
        glVertex2f(x0, waypoints[0][1] + waypointDist);
        glVertex2f(x0, waypoints[0][1] - waypointDist);
      glEnd();
      
      // bottom corridor lines
      glBegin(GL_LINES);
        glVertex2f(x0, waypoints[0][1] - waypointDist);
        glVertex2f(x0 + (corridorLength - waypointDist), waypoints[0][1] - waypointDist);
      glEnd();
      
      double angle;
      
      // circle of target size
      glBegin(GL_LINE_LOOP);
        for (angle=0.0; angle < 2*PI; angle+=10*(PI/180.0))
        {
           glVertex2f(cos(angle)*waypointDist + waypoints[0][0],
                      sin(angle)*waypointDist + waypoints[0][1]);
        }
      glEnd();
      
      // semicircle on right based on the maximum width of the corridor and the target area radius.
      double cx, cy;
      cx = waypoints[0][0] + corridorLength;
      cy = waypoints[0][1];
      glBegin(GL_LINE_STRIP);
        for (angle=-PI/2; angle <= PI/2; angle+=9*(PI/180.0))
        {
           glVertex2f(cos(angle)*waypointDist + cx,
                      sin(angle)*waypointDist + cy);
        }
      glEnd();
    }
  
  public:
    RegionsVisualizer(string const& cfgfilename):Visualizer("reg","vis_reg"){
      readConfigFile(cfgfilename);
    }
  
    void Visualize( Model* mod, Camera* cam ){
      drawRegions();
    }
};

RegionsVisualizer* rect;

extern "C" int Init(Model *mod, CtrlArgs *args){
  vector<string> tokens;
  Tokenize(args->worldfile, tokens); 
  rect = new RegionsVisualizer(tokens[1]);
  mod->Subscribe();
  mod->AddVisualizer( rect, true );
  return 0;
}
