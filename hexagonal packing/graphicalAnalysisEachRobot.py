#!/usr/bin/python

import sys
sys.path.insert(1, '../common/')
from hexagonal_packing_throughput import throughput
import numpy as np
import matplotlib.pyplot as plt
from math import pi, sqrt,cos
import random
import os
import subprocess
from operator import itemgetter
import glob
from functools import reduce

def plotInstantEquation(Tf,N,s,d,v,th,lbl,lastStr,colour='red'):
  Ys = [throughput(Tf[i],s,d,v,th) for i in range(len(Tf))]
  a3, = plt.plot(Tf, Ys, label=lbl, color=colour,linestyle='--',alpha=0.8,marker='x') 
  vv =  0.2
  plt.text(Tf[-1],Ys[-1]+vv,lastStr,horizontalalignment='right')
  return a3

def L(theta,s,d,v):
  if theta != pi/6:
    f = (4*v*s)/(sqrt(3)*d**2) - (2*v*cos(theta-pi/6))/(sqrt(3)*d)
  else:
    f = ((2*v)/(sqrt(3)*d))*(2*s/d - 1)
  return f

def plotLowerLimitEquation(Xf,s,d,v,theta,lbl,colour='#1f77b4'):
  Ys = [L(theta,s,d,v)]*len(Xf)
  a2, = plt.plot(Xf, Ys, label=lbl, color=colour)
  return a2

def Up(theta,s,d,v):
  if theta != pi/6:
    f = (4*v*s)/(sqrt(3)*d**2) + (2*v*cos(theta-pi/6))/(sqrt(3)*d)
  else:
    f = ((2*v)/(sqrt(3)*d))*(2*s/d + 1)
  return f

def plotUpperLimitEquation(Xf,s,d,v,theta,lbl,colour='#ff7f0e'):
  Ys = [Up(theta,s,d,v)]*len(Xf)
  a4, =plt.plot(Xf, Ys, label=lbl, color=colour)
  return a4

def searchLastRobotOnRectangle(X, s, d, robotDataSorted):
  Li = []
  cx = waypoint[0] + s + X - s
  cy = waypoint[1]
  for i in range(len(robotDataSorted)):
    # epsilon was subtracted from 0 due to numerical errors
    if -epsilon <= cx - robotDataSorted[i][5] and cx - robotDataSorted[i][5] <= 2*d:
      Li.append((robotDataSorted[i],i)) #a tuple formed by the data and its position.
  # It sorts in relation to the difference by X (the more next to zero is better), then decreasingly sorts by the distance from the centre of the target by x-axis. If there are two or more position with the same x-coordinate, choose the more next to the center.  
  dfun = lambda rd: hypot(rd[0][5] - cx, rd[0][4] - cy)
  L2 = sorted(Li,key= lambda rd: (-rd[0][5], dfun(rd)))
  return L2[0][1] #returns the position of the "least" element


def plotTheta(T,s,d,v,nPoint,Low=0,High=pi/3):
  '''
  Plot the throughput function varying the angle from [0,pi/3].
  '''
  xAxis = np.linspace(Low, High, num=nPoint, endpoint=True)
  yAxis = [throughput(T,s,d,v,th) for th in xAxis]
  plt.plot(xAxis, yAxis,color='#1f77b4')

def main():
  plt.rcParams.update({'font.size': 15})
  nExp = 4
  T = [4   ,30  , 43, 11]
  s = [2   ,2.5 ,3.5,  5]
  d = [0.13,0.66,1  ,0.4]
  v = [1]*nExp
  for i in range(len(T)):
    for samp in [99,100]:
      plotTheta(T[i],s[i],d[i],v[i],samp)
      plt.xlabel('θ (rad)');
      plt.ylabel("Throughput (1/s)");
      fname = f"Th{s[i]}s{d[i]}d{T[i]}T1v{samp}samp".replace('.','_')
      plt.savefig(fname+".pdf",bbox_inches="tight",pad_inches=0.01);
      plt.clf()

def correctColourLabels(fig = plt.figure()):
  '''
    Remove repeated labels in the legend.
    Argument:
      fig: (optional) figure to plot.
  ''' 
  # get the current axis
  ax = fig.gca()
  # get names in labels
  _, names = ax.get_legend_handles_labels()
  # this is the loop to change Labels and colours
  for i,p in enumerate(ax.get_lines()):
      # check if label name already exists
      if p.get_label() in names[:i]:
          # find ist index
          idx = names.index(p.get_label())
          # set colour
          p.set_c(ax.get_lines()[idx].get_c())
          # hide label in auto-legend
          p.set_label('_' + p.get_label())

def main2():
  plt.rcParams.update({'font.size': 15})
  samples = 51
  s,v,d = [3,6],[1,1],[1,1]
  for a in range(len(s)):
    xAxis = np.linspace(0,pi/3,endpoint=False,num=samples)
    yAxis = [L(t,s[a],d[a],v[a]) for t in xAxis]
    plt.plot(xAxis,yAxis,label='Lower bound', color='#ff7f0e')
    vv = 0.9
    plt.text(xAxis[-1],yAxis[-1]+vv,f"s={s[a]}",horizontalalignment='right')
    yAxis = [Up(t,s[a],d[a],v[a]) for t in xAxis]
    plt.plot(xAxis,yAxis,label='Upper bound', color='#1f77b4')
    yAxis = [(2/sqrt(3))*(2*s[a]/d[a]+1)*(v[a]/d[a])]*len(xAxis)
    plt.plot(xAxis,yAxis,label='Limit by cir. pack.', color='green',linestyle='--')
  correctColourLabels()
  plt.legend(loc='center right');
  plt.xlabel('θ (rad)');
  plt.ylabel("Throughput (1/s)");
  plt.savefig("limitsHexPack.pdf",bbox_inches="tight",pad_inches=0.01);


def readVectors(a, paths, s ,d ,v, V):
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
  robotDataSorted = sorted(robotData, key= lambda x: x[3]) 
  #count robots with same spent time
  uniqueTime = sorted(set([i[3] for i in robotDataSorted]))
  dicttime = {t:0 for t in uniqueTime}
  for i in robotDataSorted: dicttime[i[3]]+=1
  increasingNumRobotsPerTime = [dicttime[uniqueTime[0]]]
  for t in range(1,len(uniqueTime)):
    increasingNumRobotsPerTime.append(increasingNumRobotsPerTime[t-1] + dicttime[uniqueTime[t]])
  # find the first time value more than s/v
  firstPos = 0
  for i in range(1,len(uniqueTime)):
    if uniqueTime[i] > (s[a]/v[a])*1e6 + uniqueTime[0]:
      firstPos = i
      break
  V[a] = range(firstPos+10,len(uniqueTime)-1,15)
  Ts = [(uniqueTime[i-1] - uniqueTime[0])/1e6 for i in V[a]]
  Ns = [increasingNumRobotsPerTime[i-1]   for i in V[a]]
  Tp = [(Ns[i]-1.)/Ts[i] for i in range(len(Ts))]
  return Ts, Ns, Tp
    

def main4():
    plt.rcParams.update({'font.size': 15})
    nExp = 8
    s = [3,6,3,6,3,6,3,6]
    d = [1]*nExp
    v = [1]*nExp
    theta = [0,0,15*pi/180,15*pi/180,30*pi/180,30*pi/180,50*pi/180,50*pi/180]
    paths = ["variousLogs/0deg3s/X_70/","variousLogs/0deg6s/X_70/","variousLogs/15deg3s/X_70/","variousLogs/15deg6s/X_70/","variousLogs/30deg3s/X_70/","variousLogs/30deg6s/X_70/","variousLogs/50deg3s/X_70/","variousLogs/50deg6s/X_70/"]
    algorithmInstantLabels = ['Instantaneous']*nExp
    algorithmsTextLabels = ['Time (s)']*nExp
    algorithmsTextLabels2 = ['Num. of robots']*nExp
    outputLabels = ["Throughput (1/s)"]*nExp
    outputLabels2 = ["Time (s)"]*nExp
    location = ['center right']*nExp
    graphPerImg = 2
    algorithmLowerLimitLabels = ['Lower bound']*nExp
    algorithmUpperLimitLabels = ['Upper bound']*nExp
    Colour1 = ['#1f77b4']*nExp
    Colour4 = ['brown']*nExp
    Colour3 = ['#ff7f0e']*nExp
    lastPointTextAnnotation = ['s=3','s=6' ]*int(nExp/graphPerImg)
        
    nGrp = 1
    V = [[] for j in range(nExp)]
    for a in range(len(paths)):
      Ts, Ns, Tp = readVectors(a, paths, s, d, v, V)
      
      a1, = plt.plot(Ns,Ts,label='Simulation (' + lastPointTextAnnotation[a]+')',marker='.')
      
      if nGrp == 0:
        plt.legend();
        plt.xlabel(algorithmsTextLabels2[a]);
        plt.ylabel(outputLabels2[a]);
        
        outFileName = "".join(filter(lambda i: i not in ["/",";","*"], outputLabels2[a]))
        prefix = paths[a] + ' ' if paths[a] != '.' else ''
        prefix = "".join(filter(lambda i: i not in ["/",";","*"], prefix))
        plt.savefig(prefix  + ' ' + outFileName + ".pdf",bbox_inches="tight",pad_inches=0.01);
        plt.clf()      
      nGrp = (nGrp + 1) % graphPerImg
    
    nGrp = 1
    V = [[] for j in range(nExp)]
    for a in range(len(paths)):
      Ts, Ns, Tp = readVectors(a, paths, s, d, v, V)
      
      a1, = plt.plot(Ts,Tp,label='Simulation',color='green',marker='.')
      a2 = plotLowerLimitEquation(Ts,s[a],d[a],v[a],theta[a],algorithmLowerLimitLabels[a],Colour1[a])
      a3 = plotInstantEquation(Ts,Ns,s[a],d[a],v[a],theta[a],algorithmInstantLabels[a],lastPointTextAnnotation[a],Colour4[a])
      a4 = plotUpperLimitEquation(Ts,s[a],d[a],v[a],theta[a],algorithmUpperLimitLabels[a],Colour3[a])
      
      if nGrp == 0:
        plt.legend([a1,a2,a3,a4],['Simulation',algorithmLowerLimitLabels[a],algorithmInstantLabels[a],algorithmUpperLimitLabels[a]],loc=location[a]);
        plt.xlabel(algorithmsTextLabels[a]);
        plt.ylabel(outputLabels[a]);
        
        outFileName = "".join(filter(lambda i: i not in ["/",";","*"], outputLabels[a]))
        prefix = paths[a] + ' ' if paths[a] != '.' else ''
        prefix = "".join(filter(lambda i: i not in ["/",";","*"], prefix))
        plt.savefig(prefix  + ' ' + outFileName + ".pdf",bbox_inches="tight",pad_inches=0.01);
        plt.clf()      
      nGrp = (nGrp + 1) % graphPerImg
      
main4();
