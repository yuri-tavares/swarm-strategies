#!/usr/bin/python

import numpy as np
import matplotlib.pyplot as plt
import math
import sys
import random
import os
import subprocess
from operator import itemgetter
from scipy.stats import t

def calcConfInt(p):
  mean,var,size = np.mean(p),np.var(p),len(p)
  alpha = 0.01
  df = size - 1
  unbiased_sd = math.sqrt((size/df)*var)
  t_variate = t.ppf(1-alpha/2,df)
  uppervalue = mean + t_variate * unbiased_sd/math.sqrt(size)
  return uppervalue

def instantF(T,s,d,v,N):
  # ~ T = T - 0.1 
  if 0 < s and s <= math.sqrt(3)*d/4:
    de = 2*math.sqrt(d**2 - (2*s)**2)
    v1 = math.floor(v*T/de)
    v2 = math.floor(v*T/de + 0.5)
  else:
    v1 = math.floor(v*T/d)
    v2 = math.floor(v*T/d + 0.5)
  f = (1./T)*(v1 + v2)
  return f
  

def plotInstantEquation(Tf,Ns,folder,lbl,colour='brown'):
  v = 1
  if folder == 's0.3d1':
    s,d = 0.3, 1.0
  else:
    s,d = 0.45, 1.0
  Ys = [instantF(Tf[i],s,d,v,Ns[i]) for i in range(len(Tf))]
  plt.plot(Tf, Ys, label=lbl, color=colour,linestyle='--',alpha=0.8,marker='x')


def plotLimitEquation(Xf,folder,lbl,colour='magenta'):
  v = 1
  if folder == 's0.3d1':
    s,d = 0.3, 1.0
  else:
    s,d = 0.45, 1.0
  if 0 < s and s <= math.sqrt(3)*d/4:
    f = v/(d*math.sqrt(1-((2*s)/d)**2))
  else:
    f = 2*v/d
  Ys = [f]*len(Xf)
  plt.plot(Xf, Ys, label=lbl, color=colour)

def funF(T,v,d,s):
  return instantF(T,s,d,v,0)

def main2():
  s,d,v = 0.3,1,1
  dp = math.sqrt(d**2 - (2*s)**2)
  xAxis = np.linspace(dp/v, 10*dp/v, num=100)
  yAxis = [funF(t,v,d,s) for t in xAxis]
  plt.plot(xAxis, yAxis,color='black',label='Instant',alpha=0.7,marker='x')
  yAxis = [v/dp for t in xAxis]
  plt.plot(xAxis, yAxis,color='red',label='Asymptotic',alpha=0.7,marker='.')
  plt.legend(loc='lower right');
  plt.show()

def main():
    path = ["s0.3d1","s0.45d1"]
    algorithms = ["s0.3d1","s0.45d1"];
    algorithmVariables = ['X','X'];
    algorithmLimitLabels = ['Equation 11','Equation 13'];
    algorithmInstantLabels = ['Equation 10','Equation 12'];
    algorithmsTextLabels = ['Time (s)']*2;
    outputLabels = ["Throughput (1/s)"]*2
    lineNumber = [-1]*2 #negative number is throughput
    algorithmsSymbol = ["-"]*2;
    nSamples = [1]*2;
    
    
    for a in range(len(algorithms)):
      # sort by number, not by folder's name.
      dirValuesVars = sorted([(i,float(i.replace(algorithmVariables[a]+'_',''))) for i in os.listdir(path[a]) if algorithmVariables[a]+'_' in i], key=itemgetter(1))
      dirVars = [i[0] for i in dirValuesVars]
      valuesVar = [i[1] for i in dirValuesVars]
      data = np.zeros((len(valuesVar),nSamples[a]));
      Ts = np.zeros((len(valuesVar),nSamples[a]));
      Ns = np.zeros((len(valuesVar),nSamples[a]));
      PNys = np.zeros((len(valuesVar),nSamples[a]));
      PNxs = np.zeros((len(valuesVar),nSamples[a]));
      dataMean = np.zeros((len(valuesVar)));
      dataUpCi = np.zeros((len(valuesVar)));
    
      for n in range(len(valuesVar)):
        for s in range(nSamples[a]):
          dataFile = open(algorithms[a]+"/"+dirVars[n]+"/log_"+str(s));
          dataFileStr = dataFile.readlines();
          if lineNumber[a] > 0:
            data[n, s] = int(dataFileStr[lineNumber[a]-1]);
          else:
            lineTime, lineNumRobots, linePy, linePx = 8, 9, 10, 11
            N = float(dataFileStr[lineNumRobots-1])
            ti = float(dataFileStr[lineTime-1])/1e6 -0.1
            Ts[n,s] = ti
            PNys[n,s] = float(dataFileStr[linePy-1]) 
            PNxs[n,s] = float(dataFileStr[linePx-1]) - 0.1 # discounted 0.1 because I considered the initial position in the start of the simulation
            Ns[n,s] = N
            tp =  (N - 1)/ti
            data[n, s] = tp
        if nSamples[a] > 1:
          dataMean[n] = np.mean(data[n,:]);
          dataUpCi[n] = calcConfInt(data[n,:]);
        else:
          dataMean[n] = data[n,0];
          dataUpCi[n] = 0;
      if nSamples[a] > 1:
        plt.errorbar(valuesVar,dataMean[:], yerr=[m - n for m,n in zip(dataUpCi[:],dataMean[:])], label='Simulation',marker=algorithmsSymbol[a]);
      else:
        plt.plot(Ts,dataMean[:],label='Simulation',color='blue',marker='.')
      
      plotInstantEquation(Ts,Ns,algorithms[a],algorithmInstantLabels[a],colour='brown')
      plotLimitEquation(Ts,algorithms[a],algorithmLimitLabels[a],colour='green')
      plt.legend(loc='lower right');
      plt.xlabel(algorithmsTextLabels[a]);
      plt.ylabel(outputLabels[a]);
      
      outFileName = "".join(filter(lambda i: i not in ["/",";","*"], outputLabels[a]))
      prefix = algorithms[a] + ' ' if algorithms[a] != '.' else ''
      plt.savefig(prefix + algorithmVariables[a] + ' ' + outFileName + ".pdf",bbox_inches="tight",pad_inches=0.01);
      plt.clf()
    
main();
