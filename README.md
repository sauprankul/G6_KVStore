Authors:  
Saurabh Kulkarni: skulkarni27@wisc.edu  
Amos Kendall: akendall3@wisc.edu  
Rami Dahman: rami@cs.wisc.edu  

# Run the server:
You can use the startKV2.sh bash script to start multiple servers on the same machine each listening on a different consecutive port.

`source startKV2.sh <configFile>`

See default.config for an example that starts 4 servers on port 50000, 50001, 50002 and 50003

You can also start them individually by invoking the jar directly

`java -jar KVServer2_*.jar <configfile> <id>`

- arg0 = config file with hostname:port list
- arg1 = server id (should be consecutive, unique integers from 0 to n-1, e.g. 3, 1, 0, 2)

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
