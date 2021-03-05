Authors:  
Saurabh Kulkarni: skulkarni27@wisc.edu  
Amos Kendall: akendall3@wisc.edu  
Rami Dahman: rami@cs.wisc.edu  

# Run the server:
You can use the startKV2.sh bash script to start multiple servers on the same machine each listening on a different consecutive port.
Example starts 4 servers on port 50000, 50001, 50002 and 50003
`source startKV2.sh 4`
You can also start them individually by invoking the jar directly
`java -jar KVServer2_2.jar 50000 0 1`
arg0 = port number
arg1 = server id (should be consecutive, unique integers, e.g. 0, 1, 2, 3)
arg2 = cluster size

# Compile and run testBench:
cp your testBench to ./testBench.cc
```
g++ testBench.cc -L. -l 739kv -o testBench
export LD_LIBRARY_PATH='$LD_LIBRARY_PATH:.'
./testBench
```

# Persistent Storage
Storage will be created in ./DB<PortNumber>/store.db. 
All necessary files will be created automatically.
You can delete the entire folder in between test runs to get the correct response on new puts
