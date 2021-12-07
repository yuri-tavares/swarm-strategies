import numpy as np
import matplotlib.pyplot as plt
from math import tan, sin, asin, cos, pi, sqrt, floor, ceil, hypot, log, fabs, fmod
import math
import sys
sys.path.insert(1, '../common/')
from cool_path_throughput import limitF
import random
import os
import subprocess
from operator import itemgetter
import glob

def plotLimitEquation(Xf,s,d,v,K,lastStr,lbl,colour='green'):
  Ys = [limitF(K,s,v,d)]*len(Xf)
  a2, = plt.plot(Xf, Ys, label=lbl, color=colour)
  vx = 0
  if s == 6:
    if K == 19:
      vy = 0.5
      vx = 120
    else:
      vy = 0.3
  else:
    if K == 10:
      vy = 0.2
    else:
      vy = 0.1
  plt.text(Xf[-1]+vx,Ys[-1]+vy,lastStr,horizontalalignment='right')
  return a2

def instantF(T,N,K,s,v,d):
  angle = 2*pi/K
  r = (s*sin(angle/2) - d/2.)/(1-sin(angle/2))
  if (2 * r * cos(angle / 2) < d): 
    delay = (r * (pi - angle))  + (d - 2 * r * cos(angle / 2)) / (sin(angle / 2));
  else:
    delay = 2 * r * asin(d / (2 * r));
  delay = max(delay, d);
  ROUND=13
  v1 = round((v*T)/delay+1,ROUND)
  Nc = K*floor(v1)
  # ~ print(f"Nc={Nc} N={N} N-Nc={N-Nc} T={T} delay={delay} (v*T)/delay+1={(v*T)/delay+1}") # Code for debugging
  return (Nc-1.)/T

def plotInstantEquation(Tf,Ns,s,d,v,K,lastStr,lbl='Instantaneous',colour='brown'):
  Ys = [instantF(Tf[i],Ns[i],K,s,v,d) for i in range(len(Tf))]
  # ~ print(f"s={s},K={K}") # Code for debugging
  # ~ [print(f"T={Tf[i]},N={Ns[i]},f={Ys[i]}") for i in range(len(Tf))] # Code for debugging
  a3, = plt.plot(Tf, Ys, label=lbl, color=colour,linestyle='--',alpha=0.8,marker='x')
  vv = 0.1
  plt.text(Tf[-1],Ys[-1]+vv,lastStr,horizontalalignment='right')
  return a3

def plotSimulation(Ts,Tp,lastStr,lbl='Simulation',colr='#2ca02c',mrkr='.'):
  a1, = plt.plot(Ts,Tp,label=lbl,color=colr,marker=mrkr)
  vv = 0.1
  plt.text(Ts[-1],Tp[-1]-vv,lastStr,horizontalalignment='right')
  return a1

def plotSimulation2(Ts,Tp,lastStr,lbl='Simulation',mrkr='.'):
  a1, = plt.plot(Ts,Tp,label=lbl + ' (' + lastStr +')',marker=mrkr)
  return a1

def plotKsLimit(Ks,s,d,v,lastStr,lbl,colour='#1f77b4',mark='.',line='-'):
  Ys = [limitF(K,s,v,d) for K in Ks]
  plt.plot(Ks, Ys, label=lbl, color=colour,marker=mark,linestyle=line)
  vv = 0.1
  plt.text(Ks[-1],Ys[-1]+vv,lastStr,horizontalalignment='right')


def plotKsSimulation(Ks,Tps,lastStr,lbl='Simulation',clor='#ff7f0e',mkr='x'):
  plt.plot(Ks, Tps,label=lbl,color=clor,marker=mkr)
  vv = 0.1
  plt.text(Ks[-1],Tps[-1]+vv,lastStr,horizontalalignment='right')

def readLogs(path):
  #Save all data from individual robots logs
  robotFiles = glob.glob(path+"/"+"/robot*")
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
  return (increasingNumRobotsPerTime, uniqueTime)


def main2():
  '''
  Plots K versus asymptotitc throughput for different values of d and s.
  '''
  plt.rcParams.update({'font.size': 15})
  s = [3]*2 + [6]*2 
  d = [1,2]*2
  v = [1]*len(s)
  colour = ['#1f77b4', '#ff7f0e', '#2ca02c', '#d62728', "lime","limegreen", 'forestgreen','darkgreen','springgreen','seagreen','mediumseagreen']
  marklines = [('.','-'),('x','-'),('.','--'),('x','--')]
  algorithmLimitLabels = [f"s={s[i]},d={d[i]}" for i in range(len(s))]
  for i in range(len(s)): 
    Kmax = floor(pi/asin(d[i]/(2*s[i])))
    Ks = [k for k in range(3,Kmax+1)] 
    plotKsLimit(Ks,s[i],d[i],v[i],"",algorithmLimitLabels[i],colour[i],marklines[i][0],marklines[i][1])
    plt.legend(loc=0);
  plt.xticks(range(5,40,5))
  plt.xlabel("K");
  plt.ylabel("Throughput (1/s)");
  plt.savefig("KsThroughput.pdf",bbox_inches="tight",pad_inches=0.01);


def main3():
  '''
  Plot comparison between asymptotic throughput and simulation obtained throughput versus K. 
  '''
  plt.rcParams.update({'font.size': 15})
  s = [3,6]
  nExp=2
  v = [0.1]*nExp
  d = [1]*nExp
  algorithmLimitLabels =  ['Asymptotic']*nExp
  lastPointTextAnnotation = ['s='+str(s[a]) for a in range(nExp)]
  opt_str=""
  prefixPath = "v"+str(v[0])+opt_str 
  location = 'upper right'
  outputLabels = "Throughput (1/s)"
  for a in range(len(s)):
    if s[a] == 6:
      Ks = range(3,34,1)
    else:
      Ks = range(3,17,1)
    Tps = []
    for k in Ks:
      path = prefixPath + "/"+str(s[a])+"s/K_"+str(k)
      (increasingNumRobotsPerTime, uniqueTime) = readLogs(path)
      V = range(3,len(uniqueTime),1)
      Ts = [(uniqueTime[i-1] - uniqueTime[0])/1e6  for i in V]
      Ns = [increasingNumRobotsPerTime[i-1]   for i in V]
      Tp = [(Ns[i]-1.)/Ts[i] for i in range(len(Ts))]
      Tps.append(Tp[-1]) 
    plotKsLimit(Ks,s[a],d[a],v[a],'',algorithmLimitLabels[a])
    plotKsSimulation(Ks,Tps,lastPointTextAnnotation[a])
  plt.legend(loc=location);
  plt.xlabel("K");
  plt.ylabel(outputLabels);
  plt.savefig("KsCoolPath.pdf",bbox_inches="tight",pad_inches=0.01);
  plt.show()
  plt.clf()


def main():
    '''
    Plots the results for experimental data. 
    '''
    plt.rcParams.update({'font.size': 15})
    nExp = 4
    s = [3]*2 + [6]*2
    d = [1]*nExp
    v = [0.1]*nExp
    K = [10,16,19,33]
    opt_str=""
    prefixPath = "v"+str(v[0])+opt_str 
    paths = [prefixPath+"/"+str(s[a])+"s/K_"+str(K[a]) for a in range(nExp)]
    algorithmInstantLabels = ['Instantaneous']*nExp
    algorithmsTextLabels = ['Time (s)']*nExp
    algorithmsTextLabels2 = ['Num. of robots']*nExp
    outputLabels = ["Throughput (1/s)"]*nExp
    outputLabels2 = ["Time (s)"]*nExp
    location = ['upper right']*nExp
    graphPerImg = 2
    algorithmLimitLabels =  ['Asymptotic']*nExp
    SimulationLabels =  ['Simulation']*nExp
    Colour1 = ['#1f77b4']*nExp
    Colour4 = ['brown']*nExp
    Colour2 = ['green']*nExp
    lastPointTextAnnotation = ['s='+str(s[a])+', K='+str(K[a]) for a in range(nExp)]
    
    nGrp = 1 % graphPerImg
    V = [[] for j in range(nExp)]
    for a in range(len(paths)):
      (increasingNumRobotsPerTime, uniqueTime) = readLogs(paths[a])
      print(paths[a])
      print(increasingNumRobotsPerTime)
      print(uniqueTime)
      # I ignore the first because it has time equals to zero and the second value because it gives a too higher throughput (the graph will looks like a line with it).
      V[a] = range(3,len(uniqueTime))
      Ts = [(uniqueTime[i-1] - uniqueTime[0])/1e6  for i in V[a]]
      Ns = [increasingNumRobotsPerTime[i-1]   for i in V[a]]
      Tp = [(Ns[i]-1.)/Ts[i] for i in range(len(Ts))]
      plotSimulation2(Ns,Ts,lastPointTextAnnotation[a],SimulationLabels[a])

      
      if nGrp == 0:
        plt.legend(loc=location[a]);
        plt.xlabel(algorithmsTextLabels2[a]);
        plt.ylabel(outputLabels2[a]);
        
        outFileName = "".join(filter(lambda i: i not in ["/",";","*"], outputLabels2[a]))
        prefix = paths[a] + ' ' if paths[a] != '.' else ''
        prefix = "".join(filter(lambda i: i not in ["/",";","*"], prefix))
        plt.savefig(prefix  + ' ' + outFileName + "K" +str(K[a]) + ".pdf",bbox_inches="tight",pad_inches=0.01);
        # ~ plt.show()
        plt.clf()  
      nGrp = (nGrp + 1) % graphPerImg
    
    
    nGrp = 1 % graphPerImg
    V = [[] for j in range(nExp)]
    for a in range(len(paths)):
      (increasingNumRobotsPerTime, uniqueTime) = readLogs(paths[a])
      print(paths[a])
      print(increasingNumRobotsPerTime)
      print(uniqueTime)
      # Same comment here as above.
      V[a] = range(3,len(uniqueTime))
      Ts = [(uniqueTime[i-1] - uniqueTime[0])/1e6  for i in V[a]]
      Ns = [increasingNumRobotsPerTime[i-1]   for i in V[a]]
      Tp = [(Ns[i]-1.)/Ts[i] for i in range(len(Ts))]
      a1 = plotSimulation(Ts,Tp,'',SimulationLabels[a],Colour2[a])
      a2 = plotLimitEquation(Ts,s[a],d[a],v[a],K[a],lastPointTextAnnotation[a],algorithmLimitLabels[a],Colour1[a])
      a3 = plotInstantEquation(Ts,Ns,s[a],d[a],v[a],K[a],'',algorithmInstantLabels[a],Colour4[a])

      
      if nGrp == 0:
        plt.legend([a1,a2,a3],[SimulationLabels[a],algorithmLimitLabels[a],algorithmInstantLabels[a]],loc=location[a]);
        plt.xlabel(algorithmsTextLabels[a]);
        plt.ylabel(outputLabels[a]);
        
        outFileName = "".join(filter(lambda i: i not in ["/",";","*"], outputLabels[a]))
        prefix = paths[a] + ' ' if paths[a] != '.' else ''
        prefix = "".join(filter(lambda i: i not in ["/",";","*"], prefix))
        plt.savefig(prefix  + ' ' + outFileName + "K" +str(K[a]) + ".pdf",bbox_inches="tight",pad_inches=0.01);
        plt.clf()  
      nGrp = (nGrp + 1) % graphPerImg


main();
