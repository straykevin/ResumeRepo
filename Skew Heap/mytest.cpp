#include "iqueue.h"
#include <random>
// the followings are sample priority functions to be used by IQueue class
// users can define their own priority functions
// Priority functions compute an integer priority for a crop.  Internal
// computations may be floating point, but must return an integer.
int priorityFn1(const Crop &crop);// works with a MAXHEAP
int priorityFn2(const Crop &crop);// works with a MINHEAP

enum RANDOM {UNIFORMINT, UNIFORMREAL, NORMAL};
class Random {
public:
    Random(int min, int max, RANDOM type=UNIFORMINT, int mean=50, int stdev=20) : m_min(min), m_max(max), m_type(type)
    {
        if (type == NORMAL){
            //the case of NORMAL to generate integer numbers with normal distribution
            m_generator = std::mt19937(m_device());
            //the data set will have the mean of 50 (default) and standard deviation of 20 (default)
            //the mean and standard deviation can change by passing new values to constructor 
            m_normdist = std::normal_distribution<>(mean,stdev);
        }
        else if (type == UNIFORMINT) {
            //the case of UNIFORMINT to generate integer numbers
            // Using a fixed seed value generates always the same sequence
            // of pseudorandom numbers, e.g. reproducing scientific experiments
            // here it helps us with testing since the same sequence repeats
            m_generator = std::mt19937(10);// 10 is the fixed seed value
            m_unidist = std::uniform_int_distribution<>(min,max);
        }
        else{ //the case of UNIFORMREAL to generate real numbers
            m_generator = std::mt19937(10);// 10 is the fixed seed value
            m_uniReal = std::uniform_real_distribution<double>((double)min,(double)max);
        }
    }
    void setSeed(int seedNum){
        // we have set a default value for seed in constructor
        // we can change the seed by calling this function after constructor call
        // this gives us more randomness
        m_generator = std::mt19937(seedNum);
    }

    int getRandNum(){
        // this function returns integer numbers
        // the object must have been initialized to generate integers
        int result = 0;
        if(m_type == NORMAL){
            //returns a random number in a set with normal distribution
            //we limit random numbers by the min and max values
            result = m_min - 1;
            while(result < m_min || result > m_max)
                result = m_normdist(m_generator);
        }
        else if (m_type == UNIFORMINT){
            //this will generate a random number between min and max values
            result = m_unidist(m_generator);
        }
        return result;
    }

    double getRealRandNum(){
        // this function returns real numbers
        // the object must have been initialized to generate real numbers
        double result = m_uniReal(m_generator);
        // a trick to return numbers only with two deciaml points
        // for example if result is 15.0378, function returns 15.03
        // to round up we can use ceil function instead of floor
        result = std::floor(result*100.0)/100.0;
        return result;
    }
    
    private:
    int m_min;
    int m_max;
    RANDOM m_type;
    std::random_device m_device;
    std::mt19937 m_generator;
    std::normal_distribution<> m_normdist;//normal distribution
    std::uniform_int_distribution<> m_unidist;//integer uniform distribution
    std::uniform_real_distribution<double> m_uniReal;//real uniform distribution

};

class Tester{
    public:

    bool testInsertMinNormal(IQueue& queue, int numCrops);
    bool testInsertMaxNormal(IQueue& queue, int numCrops);

    bool testRemoveMinNormal(IQueue& queue, int numCrops);
    bool testRemoveMaxNormal(IQueue& queue, int numCrops);

    bool testSetPriorFuncNormal(IQueue& queue);
    
    bool testQueueMergeEmpty(IQueue& queue);

    bool testCopyConstructorNormal(IQueue& queue);
    bool testCopyConstructorEdge(IQueue& queue);

    bool testAssignmentNormal(IQueue& queue);
    bool testAssignmentEdge(IQueue& queue);

    bool testDeQueueEmpty();

    bool testMergeQueueDiffPrior(IQueue& queue1, IQueue& queue2);


    // helper functions
    private:
    int countQueueSize(Node* currNode) {    
        if(currNode == nullptr)
            return 0;
        else
            return countQueueSize(currNode->m_left) + 1 + countQueueSize(currNode->m_right);
    };

    //testHeaps should always be called last... they clear out the queues.
    bool testHeapAscending(IQueue& queue) { // for min heap
        int maxPrior = priorityFn2(queue.getNextCrop());

        // let's make sure that the insertion properties are being met. if priority numbers decreases as the values pop, there's a problem.
        while(queue.m_heap != nullptr) {
            int currPrior = priorityFn2(queue.getNextCrop());

            if(maxPrior > currPrior) { // value for min heap is ascending as it travels through heap...
                return false;
            }
            
            maxPrior = currPrior;
        }

        return true;
    }

    bool testHeapDescending(IQueue& queue) { //
        int maxPrior = priorityFn1(queue.getNextCrop());

        // let's make sure that the insertion properties are being met. if priority numbers decreases as the values pop, there's a problem.
        while(queue.m_heap != nullptr) {
            int currPrior = priorityFn1(queue.getNextCrop());

            if(maxPrior < currPrior) { // value for min heap is descending as it travels through heap...
                return false;
            }
            
            maxPrior = currPrior;
        }

        return true;
    }
};

int main(){

    Tester Test;

    Random idGen(MINCROPID,MAXCROPID);
    Random temperatureGen(MINTEMP,MAXTEMP);
    int temperature = temperatureGen.getRandNum();
    Random moistureGen(MINMOISTURE,MAXMOISTURE);
    Random timeGen(MINTIME,MAXTIME);
    int time = timeGen.getRandNum();
    Random typeGen(MINTYPE,MAXTYPE);

    {
        cout << "Test 1: Testing Insert Min | Normal Case: ";
        IQueue queue(priorityFn2, MINHEAP);
        int numCrops = 300;

        for (int i = 0; i < numCrops; i++){
            Crop aCrop(idGen.getRandNum(),
                        temperature,
                        moistureGen.getRandNum(),
                        time,
                        typeGen.getRandNum());
            queue.insertCrop(aCrop);
        }

        if(Test.testInsertMinNormal(queue, numCrops) == true) {
            cout << "Test Passed" << endl;
        } else {
            cout << "Test Failed" << endl;
        }

    }

    {
        cout << "Test 2: Testing Insert Max | Normal Case: ";
        IQueue queue(priorityFn1, MAXHEAP);
        int numCrops = 300;

        for (int i = 0; i < numCrops; i++){
            Crop aCrop(idGen.getRandNum(),
                        temperature,
                        moistureGen.getRandNum(),
                        time,
                        typeGen.getRandNum());
            queue.insertCrop(aCrop);
        }



        if(Test.testInsertMaxNormal(queue, numCrops) == true) {
            cout << "Test Passed" << endl;
        } else {
            cout << "Test Failed" << endl;
        }
    }

    {
        cout << "Test 3: Testing Remove Min | Normal Case: ";
        IQueue queue(priorityFn2, MINHEAP);
        int numCrops = 300;

        for (int i = 0; i < numCrops; i++){
            Crop aCrop(idGen.getRandNum(),
                        temperature,
                        moistureGen.getRandNum(),
                        time,
                        typeGen.getRandNum());
            queue.insertCrop(aCrop);
        }




        if(Test.testRemoveMinNormal(queue, numCrops) == true) {
            cout << "Test Passed" << endl;
        } else {
            cout << "Test Failed" << endl;
        }
    }

    {
        cout << "Test 4: Testing Remove Max | Normal Case: ";
        IQueue queue(priorityFn1, MAXHEAP);
        int numCrops = 300;

        for (int i = 0; i < numCrops; i++){
            Crop aCrop(idGen.getRandNum(),
                        temperature,
                        moistureGen.getRandNum(),
                        time,
                        typeGen.getRandNum());
            queue.insertCrop(aCrop);
        }




        if(Test.testRemoveMaxNormal(queue, numCrops) == true) {
            cout << "Test Passed" << endl;
        } else {
            cout << "Test Failed" << endl;
        }
    }

    {
        cout << "Test 5: Testing SetPriorityFunc | Normal Case: ";
        IQueue queue(priorityFn1, MAXHEAP);
        int numCrops = 300;

        for (int i = 0; i < numCrops; i++){
            Crop aCrop(idGen.getRandNum(),
                        temperature,
                        moistureGen.getRandNum(),
                        time,
                        typeGen.getRandNum());
            queue.insertCrop(aCrop);
        }




        if(Test.testSetPriorFuncNormal(queue) == true) {
            cout << "Test Passed" << endl;
        } else {
            cout << "Test Failed" << endl;
        }
    }

    {
        cout << "Test 6: Testing QueueMerge | Merge Empty with Normal Case: ";
        IQueue queue(priorityFn2, MINHEAP);
        int numCrops = 300;

        for (int i = 0; i < numCrops; i++){
            Crop aCrop(idGen.getRandNum(),
                        temperature,
                        moistureGen.getRandNum(),
                        time,
                        typeGen.getRandNum());
            queue.insertCrop(aCrop);
        }




        if(Test.testQueueMergeEmpty(queue) == true) {
            cout << "Test Passed" << endl;
        } else {
            cout << "Test Failed" << endl;
        }
    }

    {
        cout << "Test 7: Testing Copy Constructor | Normal Case: ";
        IQueue queue(priorityFn1, MAXHEAP);
        int numCrops = 300;

        for (int i = 0; i < numCrops; i++){
            Crop aCrop(idGen.getRandNum(),
                        temperature,
                        moistureGen.getRandNum(),
                        time,
                        typeGen.getRandNum());
            queue.insertCrop(aCrop);
        }




        if(Test.testCopyConstructorNormal(queue) == true) {
            cout << "Test Passed" << endl;
        } else {
            cout << "Test Failed" << endl;
        }
    }

    {
        cout << "Test 8: Testing Copy Constructor | Edge Case: ";
        IQueue queue(priorityFn1, MAXHEAP);
        int numCrops = 2;

        for (int i = 0; i < numCrops; i++){
            Crop aCrop(idGen.getRandNum(),
                        temperature,
                        moistureGen.getRandNum(),
                        time,
                        typeGen.getRandNum());
            queue.insertCrop(aCrop);
        }




        if(Test.testCopyConstructorEdge(queue) == true) {
            cout << "Test Passed" << endl;
        } else {
            cout << "Test Failed" << endl;
        }
    }

    {
        cout << "Test 8: Testing Assignment | Normal Case: ";
        IQueue queue(priorityFn1, MAXHEAP);
        int numCrops = 300;

        for (int i = 0; i < numCrops; i++){
            Crop aCrop(idGen.getRandNum(),
                        temperature,
                        moistureGen.getRandNum(),
                        time,
                        typeGen.getRandNum());
            queue.insertCrop(aCrop);
        }




        if(Test.testAssignmentNormal(queue) == true) {
            cout << "Test Passed" << endl;
        } else {
            cout << "Test Failed" << endl;
        }
    }

    {
        cout << "Test 9: Testing Assignment | Edge Case: ";
        IQueue queue(priorityFn1, MAXHEAP);
        int numCrops = 2;

        for (int i = 0; i < numCrops; i++){
            Crop aCrop(idGen.getRandNum(),
                        temperature,
                        moistureGen.getRandNum(),
                        time,
                        typeGen.getRandNum());
            queue.insertCrop(aCrop);
        }




        if(Test.testAssignmentEdge(queue) == true) {
            cout << "Test Passed" << endl;
        } else {
            cout << "Test Failed" << endl;
        }
    }

    {
        cout << "Test 10: Testing DeQueue | Empty Case: ";
        IQueue queue(priorityFn1, MAXHEAP);
        int numCrops = 300;

        for (int i = 0; i < numCrops; i++){
            Crop aCrop(idGen.getRandNum(),
                        temperature,
                        moistureGen.getRandNum(),
                        time,
                        typeGen.getRandNum());
            queue.insertCrop(aCrop);
        }




        if(Test.testDeQueueEmpty() == true) {
            cout << "Test Passed" << endl;
        } else {
            cout << "Test Failed" << endl;
        }
    }

    {
        cout << "Test 11: Testing MergeQueue | Diff Priority Case: ";
        IQueue queue1(priorityFn1, MAXHEAP);
        IQueue queue2(priorityFn2, MINHEAP);
        int numCrops = 300;

        for (int i = 0; i < numCrops; i++){
            Crop aCrop(idGen.getRandNum(),
                        temperature,
                        moistureGen.getRandNum(),
                        time,
                        typeGen.getRandNum());
            queue1.insertCrop(aCrop);
            queue2.insertCrop(aCrop);
        }




        if(Test.testMergeQueueDiffPrior(queue1, queue2) == true) {
            cout << "Test Passed" << endl;
        } else {
            cout << "Test Failed" << endl;
        }
    }

    return 0;
}

bool Tester::testInsertMinNormal(IQueue& queue, int numCrops) {
    // if the heap size does not much the numCrops, there's a problem...
    if (countQueueSize(queue.m_heap) != numCrops) {
        return false;
    }

    return testHeapAscending(queue);
}

bool Tester::testInsertMaxNormal(IQueue& queue, int numCrops) {

    // if the heap size does not much the numCrops, there's a problem...
    if (countQueueSize(queue.m_heap) != numCrops) {
        return false;
    }

    return testHeapDescending(queue); 
}


bool Tester::testRemoveMinNormal(IQueue& queue, int numCrops) {
    return testHeapAscending(queue);
}


bool Tester::testRemoveMaxNormal(IQueue& queue, int numCrops) {
    return testHeapDescending(queue); 
}

bool Tester::testSetPriorFuncNormal(IQueue& queue) {
    IQueue maxQueue(priorityFn1, MAXHEAP);
    IQueue testQueue(queue); // queue is originally a priorityFn2 + min heap

    while(queue.m_heap != nullptr) { // changes data from min heap to max heap
        maxQueue.insertCrop(queue.getNextCrop());
    }

    testQueue.setPriorityFn(priorityFn1, MAXHEAP); // function we're testing...
    
    while(maxQueue.m_heap != nullptr) {
        Crop maxQueueCrop = maxQueue.getNextCrop();
        Crop testQueueCrop = testQueue.getNextCrop();
        if ((maxQueueCrop.getCropID() != testQueueCrop.getCropID()) || (maxQueueCrop.getTemperature() != testQueueCrop.getTemperature()) 
            || (maxQueueCrop.getMoisture() != testQueueCrop.getMoisture()) || (maxQueueCrop.getTimeString() != testQueueCrop.getTimeString())
            || (maxQueueCrop.getTypeString() != testQueueCrop.getTypeString())) {
            return false;
        }
    }

    return testQueue.m_size == 0; // if testQueue is empty, that means everything has been deleted, the count was the same, and that no differences were found...
}

bool Tester::testQueueMergeEmpty(IQueue& queue) {
    IQueue emptyQueue(priorityFn2, MINHEAP);
    
    int numCrops = countQueueSize(queue.m_heap);
    emptyQueue.mergeWithQueue(queue);


    if(countQueueSize(emptyQueue.m_heap) != numCrops) { // if size is not same, theres a problem...
        return false;
    }

    return testHeapAscending(emptyQueue); // let's see if the merge has kept the heap property in tact!

}

bool Tester::testCopyConstructorNormal(IQueue& queue) {
    IQueue testQueue(queue);

    if (countQueueSize(queue.m_heap) != countQueueSize(testQueue.m_heap)) { // if size isn't same, there is a problem!
        return false;
    }

    if (queue.getPriorityFn() != testQueue.getPriorityFn()) {
        return false;
    } else if (queue.getHeapType() != testQueue.getHeapType()) {
        return false;
    }

    while(queue.m_heap != nullptr) { // we compare queues...
        Crop queueCrop = queue.getNextCrop();
        Crop testQueueCrop = testQueue.getNextCrop();
        if ((queueCrop.getCropID() != testQueueCrop.getCropID()) || (queueCrop.getTemperature() != testQueueCrop.getTemperature()) 
            || (queueCrop.getMoisture() != testQueueCrop.getMoisture()) || (queueCrop.getTimeString() != testQueueCrop.getTimeString())
            || (queueCrop.getTypeString() != testQueueCrop.getTypeString())) {
            return false;
        }
    }

    return testQueue.m_size == 0; // if testQueue is empty, that means everything has been deleted, the count was the same, and that no differences were found...
}

bool Tester::testCopyConstructorEdge(IQueue& queue) { // queue should be of size 0, 1, 2
    IQueue testQueue(queue);

    if (countQueueSize(queue.m_heap) != countQueueSize(testQueue.m_heap)) { // if size isn't same, there is a problem!
        return false;
    }

    if (queue.getPriorityFn() != testQueue.getPriorityFn()) {
        return false;
    } else if (queue.getHeapType() != testQueue.getHeapType()) {
        return false;
    }

    while(queue.m_heap != nullptr) { // we compare queues...
        Crop queueCrop = queue.getNextCrop();
        Crop testQueueCrop = testQueue.getNextCrop();
        if ((queueCrop.getCropID() != testQueueCrop.getCropID()) || (queueCrop.getTemperature() != testQueueCrop.getTemperature()) 
            || (queueCrop.getMoisture() != testQueueCrop.getMoisture()) || (queueCrop.getTimeString() != testQueueCrop.getTimeString())
            || (queueCrop.getTypeString() != testQueueCrop.getTypeString())) {
            return false;
        }
    }

    return testQueue.m_size == 0; // if testQueue is empty, that means everything has been deleted, the count was the same, and that no differences were found...
}

bool Tester::testAssignmentNormal(IQueue& queue) {
    IQueue testQueue = queue;

    if (countQueueSize(queue.m_heap) != countQueueSize(testQueue.m_heap)) { // if size isn't same, there is a problem!
        return false;
    }

    if (queue.getPriorityFn() != testQueue.getPriorityFn()) {
        return false;
    } else if (queue.getHeapType() != testQueue.getHeapType()) {
        return false;
    }

    while(queue.m_heap != nullptr) { // we compare queues...
        Crop queueCrop = queue.getNextCrop();
        Crop testQueueCrop = testQueue.getNextCrop();
        if ((queueCrop.getCropID() != testQueueCrop.getCropID()) || (queueCrop.getTemperature() != testQueueCrop.getTemperature()) 
            || (queueCrop.getMoisture() != testQueueCrop.getMoisture()) || (queueCrop.getTimeString() != testQueueCrop.getTimeString())
            || (queueCrop.getTypeString() != testQueueCrop.getTypeString())) {
            return false;
        }
    }

    return testQueue.m_size == 0; // if testQueue is empty, that means everything has been deleted, the count was the same, and that no differences were found...
}

bool Tester::testAssignmentEdge(IQueue& queue) { // queue should be of size 0, 1, 2
    IQueue testQueue = queue;

    if (countQueueSize(queue.m_heap) != countQueueSize(testQueue.m_heap)) { // if size isn't same, there is a problem!
        return false;
    }

    if (queue.getPriorityFn() != testQueue.getPriorityFn()) {
        return false;
    } else if (queue.getHeapType() != testQueue.getHeapType()) {
        return false;
    }

    while(queue.m_heap != nullptr) { // we compare queues...
        Crop queueCrop = queue.getNextCrop();
        Crop testQueueCrop = testQueue.getNextCrop();
        if ((queueCrop.getCropID() != testQueueCrop.getCropID()) || (queueCrop.getTemperature() != testQueueCrop.getTemperature()) 
            || (queueCrop.getMoisture() != testQueueCrop.getMoisture()) || (queueCrop.getTimeString() != testQueueCrop.getTimeString())
            || (queueCrop.getTypeString() != testQueueCrop.getTypeString())) {
            return false;
        }
    }

    return testQueue.m_size == 0; // if testQueue is empty, that means everything has been deleted, the count was the same, and that no differences were found...
}

bool Tester::testDeQueueEmpty() {
    IQueue testQueue(priorityFn2, MINHEAP);

    try {
        testQueue.getNextCrop(); // exception should be thrown
        return false; // did not throw out of range
    } catch (domain_error& err) {
        return true;
    }
}

bool Tester::testMergeQueueDiffPrior(IQueue& queue1, IQueue& queue2) {
    try {
        queue1.mergeWithQueue(queue2); // exception should be thrown
        return false; // did not throw out of range
    } catch (domain_error& err) {
        return true;
    }
}

int priorityFn1(const Crop &crop) {
    //needs MAXHEAP
    //priority value is determined based on some criteria
    //priority value falls in the range [30-116]
    //the highest priority would be 110+6 = 116
    //the lowest priority would be 30+0 = 30
    //the larger value means the higher priority
    int priority = crop.getTemperature() + crop.getType();
    return priority;
}

int priorityFn2(const Crop &crop) {
    //needs MINHEAP
    //priority value is determined based on some criteria
    //priority value falls in the range [0-103]
    //the highest priority would be 0+0 = 0
    //the lowest priority would be 100+3 = 103
    //the smaller value means the higher priority
    int priority = crop.getMoisture() + crop.getTime();
    return priority;
}
