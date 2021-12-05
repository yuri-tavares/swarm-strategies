import numpy as np
import matplotlib.pyplot as plt
import math
import sys
import random
import os
import subprocess
from operator import itemgetter
import glob

def instantF(T,s,d,v,N):
  ROUND=13
  if math.fabs(s - math.floor(round(s/d,ROUND))*d) <= math.fabs(s - math.ceil(round(s/d,ROUND))*d):
    I = int(math.floor(round(s/d,ROUND)))  
  else:
    I = int(math.ceil(round(s/d,ROUND)))
  dI = s - math.sqrt(s**2 - (s-I*d)**2)
  n = 0
  for i in range(0, int(math.floor(round(2*s/d,ROUND)))+1):
    di = s - math.sqrt(s**2 - (s-i*d)**2)
    ni = math.floor(round((v*T -di + dI)/d,ROUND)) + 1 if T >= (di - dI)/v else 0
    n = n + ni
  # ~ print(f"N-n = {N-n} N={N} n={n} T={T}")
  return (1./T)*(n-1)

def plotInstantEquation(Tf,Ns,s,d,v,lbl,colour='brown'):
  Ys = [instantF(Tf[i],s,d,v,Ns[i]) for i in range(len(Tf))]
  aa, = plt.plot(Tf, Ys, label=lbl, color=colour,linestyle='--',alpha=0.8,marker='x') 
  return aa

def plotLimitEquation(Xf,s,d,v,lbl,lastStr,colour='magenta'):
  f = math.floor(2*s/d + 1)*v/d
  Ys = [f]*len(Xf)
  aa, = plt.plot(Xf, Ys, label=lbl, color=colour)
  vv =  -0.7
  plt.text(Xf[-1],Ys[-1]+vv,lastStr,horizontalalignment='right')
  return aa

def readVectors(a, paths, startingRobot):
  #Save all data from individual robots logs
  robotFiles = glob.glob(paths[a]+"/"+"/robot*")
  robotData = []
  for f in robotFiles:
    robotFile = open(f)
    dataFileStr = robotFile.readlines();
    data = []
    for i in range(4):
      data.append(int(dataFileStr[i])) 
    for i in range(2):
      data.append(float(dataFileStr[4+i])) 
    robotData.append(data)
  # Sort robot data by target arrival time
  robotDataSorted = sorted(robotData, key= itemgetter(3))

  st = startingRobot[a]
  V = range(20,len(robotDataSorted)-5,20)
  Ts = [(robotDataSorted[st+i-1][3] - robotDataSorted[st][3])/1e6 for i in V]
  Ns = [i   for i in V]
  Tp = [(Ns[i]-1.)/Ts[i] for i in range(len(Ts))]
  
  return Ts, Tp, Ns


def main():
    startingRobot = [0,0]
    i = 2
    s = [3,6]
    d = [1]*i
    v = [1]*i
    paths = ["v1.0/X_70/s_3","v1.0/X_70/s_6"]
    algorithmLimitLabel = 'Asymptotic' 
    algorithmInstantLabel = 'Instantaneous' 
    lastPointTextAnnotation = ['s=3','s=6'  ]
    xAxisText = 'Time (s)'
    xAxisText2 = 'Num. of robots'
    outputLabel = "Throughput"
    outputLabel2 = "Time (s)"
    Colour1 = '#1f77b4'
    Colour4 = 'brown'
    
    plt.rcParams.update({'font.size': 15})
    
    
    for a in range(len(paths)): 
      Ts, Tp, Ns = readVectors(a, paths, startingRobot)
      plt.plot(Ns,Ts,label='Simulation (' + lastPointTextAnnotation[a]+')',marker='.')
    plt.legend();
    plt.xlabel(xAxisText2);
    plt.ylabel(outputLabel2);
      
    # ~ plt.show();
    outFileName = "".join(filter(lambda i: i not in ["/",";","*"], outputLabel2))
    plt.savefig(outFileName + ".pdf",bbox_inches="tight",pad_inches=0.01);
    plt.clf()
    
    for a in range(len(paths)): 
      Ts, Tp, Ns = readVectors(a, paths, startingRobot)
      
      a1, = plt.plot(Ts,Tp,label='Simulation',color='green',marker='.')
      a2 = plotInstantEquation(Ts,Ns,s[a],d[a],v[a],algorithmInstantLabel,colour=Colour4)
      a3 = plotLimitEquation(Ts,s[a],d[a],v[a],algorithmLimitLabel,lastPointTextAnnotation[a],colour=Colour1)
    plt.legend([a1,a2,a3],['Simulation',algorithmInstantLabel,algorithmLimitLabel]);
    plt.xlabel(xAxisText);
    plt.ylabel(outputLabel);
    
    outFileName = "".join(filter(lambda i: i not in ["/",";","*"], outputLabel))
    plt.savefig(outFileName + ".pdf",bbox_inches="tight",pad_inches=0.01);
    plt.clf()
    
     
    
main();
