import sys 
sys.path.insert(1, '../common/')
from cool_path_throughput import Kmax


s = float(sys.argv[1])
d = float(sys.argv[2])
print(Kmax(s,d))
