Authors:  
Saurabh Kulkarni: skulkarni27@wisc.edu  
Amos Kendall: akendall3@wisc.edu  
Rami Dahman: rami@cs.wisc.edu  

# Run the server:
`java -jar KVServer.jar none`

There are 3 cache modes: 
"none" uses raw sqlite. 
"lru <N>" uses strict LRU
with a cache size of N. If capacity is reached and there is a miss, the least
recently used item is kicked to make room. 
"fifo <N>" does not keep track of item usage times, and thus reduces the overhead 
from gets and updates. When capacity is reached, 30% of the first cached 
items are deleted. 


# Compile and run testBench:
cp your testBench to ./testBench.cc
```
g++ testBench.cc -L. -l 739kv -o testBench
export LD_LIBRARY_PATH='$LD_LIBRARY_PATH:.'
./testBench
```

# Persistent Storage
Storage will be created in ./DB/store.db. 
All necessary files will be created automatically.
You can delete the entire folder in between test runs to get the correct response on new puts
