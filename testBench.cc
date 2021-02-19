#include "kv739.h"
#include "stdio.h"
#include <stdlib.h>
#include <time.h>
#include <cstring>
#include <sys/time.h>

#define SUCCESS 1
#define FAIL    0

#define INVALID_KEY_SIZE 150
#define INVALID_VALUE_SIZE 2500

#define CONNECTED     0
#define DISCONNECTED -1

#define VALID_SERVER   true
#define INVALID_SERVER false

#define GET_NOT_PRESENT 1
#define GET_FAILURE    -1
#define GET_SUCCESS     0

#define PUT_NOT_PRESENT 1
#define PUT_FAILURE    -1
#define PUT_SUCCESS     0

#define NO_STATS        0
#define STATS           1
#define OP_COUNT        5000

//#define TARGET "127.0.0.1:5000"
#define TARGET "localhost:50051"

void rand_str(char*, size_t);
void testMessage(int status);
int testInitShutdown();
int testPut();
int testGet();
int testRandomThroughput();
int testHotColdThroughput();
int testPutLatency();
int testGetLatency();
int testPutThroughput();
struct workload* pickWorkload(struct workload *hot, struct workload *cold, int ratio);
struct kv* randomWorkloadKv(struct workload *w);
void permaGet();
void permaPut();
void putKvWorkload(struct workload *w, int results, char* name = (char *) "dummy.txt");
void getKvWorkload(struct workload *w, int results, char* name = (char *) "dummy.txt");
double get_duration(timespec start, timespec end);

struct kv {
    char* key;
    char* value;
};

struct workload {
    struct kv** data;
    int size;
};

kv*  initKv(int keySize = 12, int valueSize = 20);
workload* initKvWorkload(int count);
void cleanWorkload(workload*);


//Run our test bench!
int main(int argc, char** argv){

    struct timespec start;
    struct timespec end;  
    int tval = timespec_get(&start, TIME_UTC);      //start clock
    //run tests
    setvbuf(stdout, NULL, _IONBF, 0);
    //testMessage(testInitShutdown());
    if( kv739_init((char*) TARGET) != CONNECTED ) {
        printf("Failed to init, exiting test\n");
        return FAIL;
    }
    //testMessage(testPut());
    //testMessage(testGet());

    testPutLatency();
    testGetLatency();
    testPutThroughput();
    testRandomThroughput();
    testHotColdThroughput();
    //permaPut();
    //permaGet();
    kv739_shutdown();
    tval = timespec_get(&end, TIME_UTC);            //stop the clock   
    printf("OVERALL TEST duration %f\n", get_duration(start, end));
    return SUCCESS;
}

//Parse success/fail
void testMessage(int status){
    if (status)
        printf("Test Status: Success\n");
    else
        printf("Test Status: Fail\n");
}

//test init and shutdown methods
int testInitShutdown(){
    printf("Starting init/shutdown test\n");
    //init invalid = -1
    int res = kv739_init((char*) "crazyHost:50051");
    if(res!=DISCONNECTED){
        printf("Invalid connection returned incorrect value, %d\n", res);
        return FAIL;
    }

    //init valid = 0
    res = kv739_init((char*) TARGET);
    if(res!=CONNECTED){
        printf("Valid connection returned incorrect value, %d\n", res);
        return FAIL;
    }

    //init valid second time = -1
    res = kv739_init((char*) TARGET);
    if(res!=DISCONNECTED){
        printf("Second connection returned incorrect value, %d\n", res);
        return FAIL;
    }

    //shutdown init valid second time = 0
    kv739_shutdown();
    res = kv739_init((char*) TARGET);
    if(res!=CONNECTED){
        printf("Second valid connection returned incorrect value, %d\n", res);
        return FAIL;
    }
    kv739_shutdown();
    return SUCCESS;
}

//Test put behavior
//Requires a fresh DB since put keys will already exist on subsequent runs
int testPut(){
    printf("put test: Starting\n");
    kv* validKv   = initKv();
    kv* validKv2  = initKv();
    
    char* oldValue = (char*) malloc(sizeof(char) * 2049);
    int res;

    //put existing key = 0 (assert old value = old value)
    oldValue = (char*) malloc(sizeof(char) * 2049);
    res = kv739_put(validKv->key, validKv2->value, oldValue);
    if(res != PUT_NOT_PRESENT) {
        printf("put test: existing key failed with a return value of %d\n", res);
        return FAIL;
    }
    free(oldValue);

    //put existing key = 0 (assert old value = old value)
    oldValue = (char*) malloc(sizeof(char) * 2049);
    if(kv739_put(validKv->key, validKv2->value, oldValue) != PUT_SUCCESS || strcmp(oldValue, validKv2->value) != 0) {
        printf("put test: existing key failed, %s\n", validKv->key);    // TODO can't diff this from prev test
        printf("\t expected old value %s, got old value %s\n", validKv2->value, oldValue);
        return FAIL;
    }
    free(oldValue);

    //put new key 2 = 1
    oldValue = (char*) malloc(sizeof(char) * 2049);
    if(kv739_put(validKv2->key, validKv2->value, oldValue) != PUT_NOT_PRESENT) {
        printf("put test: valid key failed\n");
        return FAIL;
    }
    free(oldValue);

    //put existing key 2 = 0 (assert old value = old value)
    oldValue = (char*) malloc(sizeof(char) * 2049);
    if(kv739_put(validKv2->key, validKv->value, oldValue) != PUT_SUCCESS || strcmp(oldValue, validKv2->value) != 0) {
        printf("put test: existing key failed, %s\n", validKv->key);
        printf("\t expected old value %s, got old value %s\n", validKv2->value, oldValue);
        return FAIL;
    }
    free(oldValue);

    return SUCCESS;
}

//Test get behaviour
int testGet(){
    printf("get test: starting\n");

    // TODO same as before, don't need to do invalids but leaving in
    kv *invalidKv = initKv(INVALID_KEY_SIZE, INVALID_VALUE_SIZE);
    kv *validKv   = initKv();
    kv *validKv2  = initKv();
    kv *validKv3  = initKv();
    char* value = (char*) malloc(sizeof(char) * 2049);
    kv739_put(validKv->key, validKv->value, value);
    kv739_put(validKv2->key, validKv2->value, value);

    //get missing key = 1
    int res = kv739_get(validKv3->key, value);
    if(res != 1){
        printf("get test: invalid key failed, %d\n", res);
        return FAIL;
    }

    //get valid key = 0 (assert key is correct)
    if( (kv739_get(validKv->key, value) != 0) || strcmp(value, validKv->value) != 0 ){
        printf("get test: valid key failed, %s\n", value);
        return FAIL;
    }

    free(value);
    return SUCCESS;
}

//test the throughput of put
//Tests both insert and update and prints results to the screen
int testPutThroughput(){
    printf("Starting put throughput tests\n");
    //set up data
    workload *w = initKvWorkload(OP_COUNT);
    struct timespec start;
    struct timespec end;  

    //insert
    printf("Insert throughput:\t");
    int tval = timespec_get(&start, TIME_UTC);      //start clock
    putKvWorkload(w, NO_STATS);                     //do workload     
    tval = timespec_get(&end, TIME_UTC);            //stop the clock   
    printf("Duration %f, total ops %d\n", get_duration(start, end), OP_COUNT);

    //get new values
    for(int i = 0; i < w->size; i++){
        rand_str(w->data[i]->value, 21);
    }

    //update
    printf("Update throughput:\t");
    tval = timespec_get(&start, TIME_UTC);          //start clock
    putKvWorkload(w, NO_STATS);                     //do workload
    tval = timespec_get(&end, TIME_UTC);            //stop the clock
    printf("Duration %f, total ops %d\n", get_duration(start, end), OP_COUNT);
    
    return SUCCESS;
}

//test a set of keys being put in random order
int testRandomThroughput(){
    int size = 110;
    char* value = (char*) malloc(sizeof(char) * 2049);
    printf("Starting random get workload\n");

    //generate a workload
    workload *workload= initKvWorkload(size);
    putKvWorkload(workload, NO_STATS);

    //start the clock
    struct timespec start;
    int tval = timespec_get(&start, TIME_UTC);

    //random samples form the workload and do stuff
    printf("op count = %d\n", OP_COUNT);
    for(int i = 0; i < OP_COUNT; i++){
        if (i % 100 == 0) {
          printf("i = %d\n", i);
        }
        int pick = (double) rand() / RAND_MAX * size;
        kv *curKv = workload->data[pick];
        if (kv739_get(curKv->key, value) == GET_FAILURE) {
          printf("Failure!\n");
          return FAIL;
        }
    }

    //stop the clock
    struct timespec end;
    tval = timespec_get(&end, TIME_UTC);

    double duration = get_duration(start, end);
    printf("Duration %f, total ops %d\n", duration, OP_COUNT);

    //clean-up
    cleanWorkload(workload);
    free(value);
    return SUCCESS;
}

//test puts on two sets of keys
//Hot: small set used 9/10 of the time
//Cold: larger set used 1/10 of the time
int testHotColdThroughput(){

    char* value = (char*) malloc(sizeof(char) * 2049); 
    printf("Starting hot/cold get throughput workload\n");
    
    //create hot and cold workloads
    int hotSize = 10;
    workload *hotWorkload = initKvWorkload(hotSize);
    putKvWorkload(hotWorkload, NO_STATS);
    int coldSize = 100;
    workload *coldWorkload = initKvWorkload(coldSize);
    putKvWorkload(coldWorkload, NO_STATS);

    //start the clock
    struct timeval start;
    int tval = gettimeofday(&start, NULL);
    if(tval != 0){ //error checking gettimeofday
	    fprintf(stderr,"'gettimeofday' did not return successfully.\n");
	    exit(1);
    }

    //perform opps
    printf("ops to perform: %d\n", OP_COUNT);
    for(int i = 0; i < OP_COUNT; i++){
        if (i % 100 == 0) {
        
          printf("i = %d\n", i);
        }
        //flip a weighted coin to decide if we use hot or cold 1/10 uses cold
        workload *workload = pickWorkload(hotWorkload, coldWorkload, 10);
        kv *curKv = randomWorkloadKv(workload);
        if (kv739_get(curKv->key, value) == GET_FAILURE) {
          printf("Failure!\n");
          return FAIL;
        }
    }
    //stop the clock
    struct timeval end;
    tval = gettimeofday(&end, NULL);
    if(tval != 0){ //error checking gettimeofday
	    fprintf(stderr,"'gettimeofday' did not return successfully.\n");
	    exit(1);
    }
    double duration = (double) ((end.tv_sec - start.tv_sec) + (double)(end.tv_usec - start.tv_usec) / 1000000);
    printf("Duration %f, total ops %d\n", duration, OP_COUNT);

    //cleanup
    cleanWorkload(hotWorkload);
    cleanWorkload(coldWorkload);
    free(value);
    return SUCCESS;
}

//test for put latency of insersts and then updates
int testPutLatency(){
    printf("Starting put latency tests\n");
    workload *w = initKvWorkload(2000);
    putKvWorkload(w, STATS, (char *) "putInsert.txt");

    //new values
    for(int i = 0; i < w->size; i++){
        rand_str(w->data[i]->value, 21);
    }
    putKvWorkload(w, STATS, (char *) "putUpdate.txt");
    return SUCCESS;
}

//test get latency for hot/cold and random
int testGetLatency(){
    printf("Starting get latency tests\n");
    //create and put keys
    workload *w = initKvWorkload(110);
    putKvWorkload(w, NO_STATS);

    //do random workload and write results to file
    getKvWorkload(w, STATS, (char *) "getRandom.txt");

    //create hot and cold workloads and put keys
    int hotSize = 10;
    workload *hotWorkload = initKvWorkload(hotSize);
    putKvWorkload(hotWorkload, NO_STATS);
    int coldSize = 100;
    workload *coldWorkload = initKvWorkload(coldSize);
    putKvWorkload(coldWorkload, NO_STATS);

    //do hot cold workload on write output to file
    FILE *fp= fopen("getHotCold.txt", "w");
    char *buf = (char*) malloc(sizeof(char) * 100);
    char *value = (char*) malloc(sizeof(char) * 2500);
    struct timespec start;
    struct timespec end;
    printf("op count: %d\n", OP_COUNT);
    for(int i = 0; i < OP_COUNT; i++){
        if (i % 100 == 0) {
          printf("i = %d\n", i);
        }
        //flip a weighted coin to decide if we use hot or cold 1/10 uses cold
        workload *workload = pickWorkload(hotWorkload, coldWorkload, 10);
        kv *curKv = randomWorkloadKv(workload);
        
        timespec_get(&start, TIME_UTC);
        if (kv739_get(curKv->key, value) == GET_FAILURE) {
          printf("Failure!\n");
          return FAIL;
        }
        timespec_get(&end, TIME_UTC);
        //tuple with duration, hot/cold 1=hot
        sprintf(buf, "%.3f,%d\n", get_duration(start, end) * 1000000, (workload == hotWorkload));
        fputs(buf,fp);
    }
    free(value);
    free(buf);
    fclose(fp);
    return SUCCESS;
}

//create a random string
void rand_str(char*dest, size_t length) {
    char charset[] = "0123456789"
                     "abcdefghijklmnopqrstuvwxyz"
                     "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    while (length-- > 0) {
        size_t index = (double) rand() / RAND_MAX * (sizeof charset - 1);
        *dest++ = charset[index];
    }
    *dest = '\0';
}

//create a key/value pair
kv* initKv(int keySize, int valueSize){
    kv *curKv = (kv*) malloc(sizeof(kv));
    char* key = (char*) malloc(sizeof(char) * keySize);
    char* value = (char*) malloc(sizeof(char) * valueSize);
    rand_str(key,keySize);
    rand_str(value, valueSize);
    curKv->key = key;
    curKv->value = value;
    return curKv;
}

workload* pickWorkload(workload *hot, workload *cold, int ratio){
    unsigned int flip = (double) rand() / RAND_MAX * ratio;
    workload *w;
    //lose the flip and switch to cold
    if (!flip)
        w = cold;
    else
        w = hot;
    return w;
}

kv* randomWorkloadKv(struct workload *w){
    int pick = (double) rand() / RAND_MAX * w->size;
    kv *curKv = w->data[pick];
    return curKv;
}

void permaGet(){
    char *value = (char *) malloc(sizeof(char *)*2500);
    struct timespec start;
    struct timespec end;
    char *buf = (char*) malloc(sizeof(char) * 100);
    workload *w = initKvWorkload(110);
    putKvWorkload(w, NO_STATS);
    while(1){
        timespec_get(&start, TIME_UTC);
        for(int i = 0; i < OP_COUNT; i++){
            kv *curKv = randomWorkloadKv(w);
            if (kv739_get(curKv->key, value) == GET_FAILURE) {
              printf("Failure!\n");
              return;
            };
        }
        timespec_get(&end, TIME_UTC);
        double duration = get_duration(start, end);
        printf("Duration %f, total ops %d, ops/s %f.3\n", duration, OP_COUNT, OP_COUNT/  duration);
    }
    free(value);
}


void permaPut(){
    char *value = (char *) malloc(sizeof(char *)*2500);
    char *buf = (char*) malloc(sizeof(char) * 100);
    int keySize = 10;
    int valueSize = 20;
    kv *curKv = initKv(keySize, valueSize);
    while(1){
        rand_str(curKv->value, valueSize);
        FILE *fp = fopen("lastKey.txt", "a");
        sprintf(buf, "%s,%s, %s\n", curKv->key, curKv->value, value);
        fputs(buf,fp);
        fclose(fp);
        if (kv739_put(curKv->key, curKv->value, value) == PUT_FAILURE) {
          printf("Failure!\n");
          return;
        }
        fp = fopen("lastKey.txt", "w");
        sprintf(buf, "%s,%s, %s\n", curKv->key, curKv->value, value);
        fputs(buf,fp);
        fclose(fp);
    }
    free(value);
}

//perform puts for a workload
//if result, print latency results to file <name>
void putKvWorkload(workload* w, int results, char* name){
    FILE *fp;
    if (results) {
        fp = fopen(name, "w");
    }
    char *buf = (char*) malloc(sizeof(char) * 100);
    char *value = (char*) malloc(sizeof(char) * 2500);
    struct timespec start;
    struct timespec end;
    int tval;
    printf("Size is %d\n", w->size);
    for(int i = 0; i < w->size; i++){
        if (i % 100 == 0) {
          printf("i is %d\n", i);
        }
        if(results){
            int tval = timespec_get(&start, TIME_UTC);
        }
        kv *curKv = w->data[i];
        kv739_put(curKv->key, curKv->value, value);
        if(results){
            tval = timespec_get(&end, TIME_UTC);
            sprintf(buf, "%.3f\n", get_duration(start, end) * 1000000);
            fputs(buf,fp);
        }
    }
    free(value);
    free(buf);
    if (results){
        fclose(fp);
    }
}

//perform random gets for a workload
//if result, print latency results to file <name>
void getKvWorkload(workload* w, int results, char* name){
    FILE *fp;
    if (results) {
        fp = fopen(name, "w");
    }
    char *buf = (char*) malloc(sizeof(char) * 100);
    char *value = (char*) malloc(sizeof(char) * 2500);
    struct timespec end;
    struct timespec start;
    printf("Starting get workload size %d\n", OP_COUNT);
    for(int i = 0; i < OP_COUNT; i++){
        if (i % 100 == 0) {
          printf("i = %d\n", i);
        }
        kv *curKv = randomWorkloadKv(w);
        int tval = timespec_get(&start, TIME_UTC);
        kv739_get(curKv->key, value);
        tval = timespec_get(&end, TIME_UTC);
        if(results){
            double duration = get_duration(start, end);
            sprintf(buf, "%.3f\n", duration * 1000000);
            fputs(buf,fp);
        }
    }
    free(value);
    free(buf);
    if (results){
        fclose(fp);
    }
}

//calculate the duration between two timespecs
double get_duration(timespec start, timespec end){
    return (double) ((end.tv_sec - start.tv_sec) + (double)(end.tv_nsec - start.tv_nsec) / 1000000000);
}

//create a set of key value pairs
workload* initKvWorkload(int count){
    kv** data = (kv**) calloc(count+1, sizeof(kv));
    for(int i = 0; i < count; i++){
        data[i] = initKv();
    }
    workload *w = (workload*) malloc(sizeof(workload));
    w->data = data;
    w->size = count;
    return w;
}

//clean up all key values pairs in the set
void cleanWorkload(workload* w){
    int i = 0;
    kv * curKv = w->data[i];
    while(curKv){
        free(curKv);
        curKv = w->data[++i];
    }
    free(w->data);
    free(w);
}
