import sys
sys.path.insert(1, '../common/')
from hexagonal_packing_throughput import bestTheta

def mainCall():
  s = float(sys.argv[1])
  d = float(sys.argv[2])
  T = float(sys.argv[3])
  v = float(sys.argv[4])
  (M,Th) = bestTheta(T,s,d,v,1000)
  print(Th)

mainCall()
