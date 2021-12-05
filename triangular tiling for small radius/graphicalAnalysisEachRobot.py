#!/usr/bin/python

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
  if 0 < s and s <= math.sqrt(3)*d/4:
    de = 2*math.sqrt(d**2 - (2*s)**2)
    v1 = math.floor(round(v*T/de,ROUND))
    v2 = math.floor(round(v*T/de + 0.5,ROUND))
  else:
    v1 = math.floor(round(v*T/d,ROUND))
    v2 = math.floor(round(v*T/d+0.5,ROUND))
  f = (1./T)*(v1 + v2)
  return f
  

def plotInstantEquation(Tf,Ns,s,d,v,lbl,colour='brown'):
  Ys = [instantF(Tf[i],s,d,v,Ns[i]) for i in range(len(Tf))]
  plt.plot(Tf, Ys, label=lbl, color=colour,linestyle='--',alpha=0.8,marker='x')


def plotLimitEquation(Xf,s,d,v,lbl,lastStr,colour='magenta'):
  if 0 < s and s <= math.sqrt(3)*d/4:
    f = v/(d*math.sqrt(1-((2*s)/d)**2))
  else:
    f = 2*v/d
  Ys = [f]*len(Xf)
  plt.plot(Xf, Ys, label=lbl, color=colour)
  vv =  -0.05 if lastStr == 's=0.45' else 0.02
  plt.text(Xf[-1],Ys[-1]+vv,lastStr,horizontalalignment='right')

def funF(T,v,d,s):
  return instantF(T,s,d,v,0)


def main():
    startingRobot = [0,0]
    i = 2
    s = [0.3,0.45]
    d = [1]*i
    v = [1]*i
    paths = ["variousLogs0.3d1","variousLogs0.45d1"]
    algorithmLimitLabels = ['Asymptotic']*i 
    algorithmInstantLabels = ['Instantaneous']*i 
    lastPointTextAnnotation = ['s=0.3','s=0.45'  ]
    xAxisText = 'Time (s)'
    outputLabel = "Throughput (1/s)"
    Colour1 = ['#1f77b4']*i
    Colour4 = ['brown']*i
    
    plt.rcParams.update({'font.size': 15})
    
    for a in range(len(paths)):
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
      V = range(2,len(robotDataSorted)+1,5)
      Ts = [(robotDataSorted[st+i-1][3] - robotDataSorted[st][3])/1e6 for i in V]
      Ns = [i   for i in V]
      Tp = [(Ns[i]-1.)/Ts[i] for i in range(len(Ts))]
      
      
      plt.plot(Ts,Tp,label='Simulation',color='green',marker='.')
      plotInstantEquation(Ts,Ns,s[a],d[a],v[a],algorithmInstantLabels[a],colour=Colour4[a])
      plotLimitEquation(Ts,s[a],d[a],v[a],algorithmLimitLabels[a],lastPointTextAnnotation[a],colour=Colour1[a])
    plt.legend(loc='center right');
    plt.xlabel(xAxisText);
    plt.ylabel(outputLabel);
      
    outFileName = "".join(filter(lambda i: i not in ["/",";","*"], outputLabel))
    plt.savefig(outFileName + ".pdf",bbox_inches="tight",pad_inches=0.01);
    plt.clf()
    
     
    
main();
