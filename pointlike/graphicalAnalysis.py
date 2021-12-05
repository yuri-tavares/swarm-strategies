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


def plotEquation(Xf,colour='magenta'):
  s,d,v = 0.3, 1.0,  1
  f = v/(d*math.sqrt(1-((2*s)/d)**2))
  Ys = [f]*len(Xf)
  plt.plot(Xf, Ys, color=colour)
  
def main():
    path = ["."]
    algorithms = ["."];
    algorithmsLabels = ['X'];
    algorithmsTextLabels = ['Number of robots'];
    outputLabels = ["Throughput (1/ms)"]
    lineNumber = [-1] #negative number is throughput
    algorithmsSymbol = ["-"];
    nSamples = [1];
    
    
    for a in range(len(algorithms)):
      # sort by number, not by folder's name.
      dirValuesVars = sorted([(i,float(i.replace(algorithmsLabels[a]+'_',''))) for i in os.listdir(path[a]) if algorithmsLabels[a]+'_' in i], key=itemgetter(1))
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
            ti = float(dataFileStr[lineTime-1])/1e6  
            Ts[n,s] = ti
            PNys[n,s] = float(dataFileStr[linePy-1]) 
            PNxs[n,s] = float(dataFileStr[linePx-1]) 
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
        plt.errorbar(valuesVar,dataMean[:], yerr=[m - n for m,n in zip(dataUpCi[:],dataMean[:])], label=algorithmsLabels[a],marker=algorithmsSymbol[a]);
      else:
        plt.plot(Ns,dataMean[:],color='blue')
      Xr = [10 + 2*i for i in range(10)]
      
      plotEquation(Ns,colour='green')
      plt.xlabel(algorithmsTextLabels[a]);
      plt.ylabel(outputLabels[a]);
      
      outFileName = "".join(filter(lambda i: i not in ["/",";","*"], outputLabels[a]))
      prefix = algorithms[a] + ' ' if algorithms[a] != '.' else ''
      plt.savefig(prefix + algorithmsLabels[a] + ' ' + outFileName + ".pdf",bbox_inches="tight",pad_inches=0.01);
      plt.clf()
    
main();
