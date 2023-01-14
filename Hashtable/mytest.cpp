//test driver for cache.cpp

#include "cache.h"
#include <random>
#include <vector>
const int MINSEARCH = 0;
const int MAXSEARCH = 7;
// the following array defines sample search strings for testing
string searchStr[MAXSEARCH+1]={"c++","python","java","scheme","prolog","c#","c","js"};
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
    bool testNormalInsertion(Cache&, vector<Person> newDataList);

    bool testFindError(Cache&);
    bool testFindNonColliding(Cache&);
    bool testFindColliding(Cache&);

    bool testRemoveNonColliding(Cache&);
    bool testRemoveCollidingNoRehash(Cache&);

    bool testInsertionRehashTrigger(Cache&);
    bool testInsertionRehashCompletion(Cache&);

    bool testDeletionRehashTrigger(Cache&);
    bool testDeletionRehashCompletion(Cache&);
};

unsigned int hashCode(const string str);

int main(){
    Tester Test;

    {
        cout << "Test 1: Insertion | Normal Case: ";
        vector<Person> dataList;
        Random RndID(MINID,MAXID);
        Random RndStr(MINSEARCH,MAXSEARCH);
        Cache cache(MINPRIME, hashCode);

        string tArray[3] = {"c", "c++", "c#"};

        for (int i=0;i<3;i++){
            // generating random data
            Person dataObj = Person(tArray[i], RndID.getRandNum());
            // saving data for later use
            dataList.push_back(dataObj);
            // inserting data in to the Cache object
            cache.insert(dataObj);
        }

        if (Test.testNormalInsertion(cache, dataList) == true) {
            cout << "Test Passed!" << endl;
        } else {
            cout << "Test Failed!" << endl;
        }
    }

    {
        cout << "Test 2: GetPerson | Error Case: ";
        vector<Person> dataList;
        Random RndID(MINID,MAXID);
        Random RndStr(MINSEARCH,MAXSEARCH);
        Cache cache(MINPRIME, hashCode);

        if (Test.testFindError(cache) == true) {
            cout << "Test Passed!" << endl;
        } else {
            cout << "Test Failed!" << endl;
        }
    }

    {
        cout << "Test 3: GetPerson | NonColliding Case: ";
        Random RndID(MINID,MAXID);
        Random RndStr(MINSEARCH,MAXSEARCH);
        Cache cache(MINPRIME, hashCode);


        if (Test.testFindNonColliding(cache) == true) {
            cout << "Test Passed!" << endl;
        } else {
            cout << "Test Failed!" << endl;
        }
    }

    {
        cout << "Test 4: GetPerson | Colliding Case: ";
        vector<Person> dataList;
        Random RndID(MINID,MAXID);
        Random RndStr(MINSEARCH,MAXSEARCH);
        Cache cache(MINPRIME, hashCode);


        for (int i=0;i<20;i++){
            // generating random data
            Person dataObj = Person(searchStr[RndStr.getRandNum()], RndID.getRandNum());
            // saving data for later use
            dataList.push_back(dataObj);
            // inserting data in to the Cache object
            cache.insert(dataObj);
        }

        if (Test.testFindColliding(cache) == true) {
            cout << "Test Passed!" << endl;
        } else {
            cout << "Test Failed!" << endl;
        }
    }

    {
        cout << "Test 5: Remove | NonColliding Case: ";
        vector<Person> dataList;
        Random RndID(MINID,MAXID);
        Random RndStr(MINSEARCH,MAXSEARCH);
        Cache cache(MINPRIME, hashCode);

        if (Test.testRemoveNonColliding(cache) == true) {
            cout << "Test Passed!" << endl;
        } else {
            cout << "Test Failed!" << endl;
        }
    }

    {
        cout << "Test 6: Remove | Colliding Case w/ out Rehash: ";
        vector<Person> dataList;
        Random RndID(MINID,MAXID);
        Random RndStr(MINSEARCH,MAXSEARCH);
        Cache cache(MINPRIME, hashCode);

        if (Test.testRemoveCollidingNoRehash(cache) == true) {
            cout << "Test Passed!" << endl;
        } else {
            cout << "Test Failed!" << endl;
        }
    }

    {
        cout << "Test 7: Insertion | Rehash Triggered Case: ";
        vector<Person> dataList;
        Random RndID(MINID,MAXID);
        Random RndStr(MINSEARCH,MAXSEARCH);
        Cache cache(MINPRIME, hashCode);

        if (Test.testInsertionRehashTrigger(cache) == true) {
            cout << "Test Passed!" << endl;
        } else {
            cout << "Test Failed!" << endl;
        }
    }

    {
        cout << "Test 8: Insertion | Rehash Completion Case: ";
        vector<Person> dataList;
        Random RndID(MINID,MAXID);
        Random RndStr(MINSEARCH,MAXSEARCH);
        Cache cache(MINPRIME, hashCode);


        for (int i=0;i<20;i++){
            // generating random data
            Person dataObj = Person(searchStr[RndStr.getRandNum()], RndID.getRandNum());
            // saving data for later use
            dataList.push_back(dataObj);
            // inserting data in to the Cache object
            cache.insert(dataObj);
        }

        if (Test.testInsertionRehashCompletion(cache) == true) {
            cout << "Test Passed!" << endl;
        } else {
            cout << "Test Failed!" << endl;
        }
    }

    {
        cout << "Test 9: Deletion | Rehash Triggered Case: ";
        vector<Person> dataList;
        Random RndID(MINID,MAXID);
        Random RndStr(MINSEARCH,MAXSEARCH);
        Cache cache(MINPRIME, hashCode);

        if (Test.testDeletionRehashTrigger(cache) == true) {
            cout << "Test Passed!" << endl;
        } else {
            cout << "Test Failed!" << endl;
        }
    }

    {
        cout << "Test 10: Deletion | Rehash Completion Case: ";
        vector<Person> dataList;
        Random RndID(MINID,MAXID);
        Random RndStr(MINSEARCH,MAXSEARCH);
        Cache cache(MINPRIME, hashCode);

        if (Test.testDeletionRehashCompletion(cache) == true) {
            cout << "Test Passed!" << endl;
        } else {
            cout << "Test Failed!" << endl;
        }
    }
    return 0;
}

unsigned int hashCode(const string str) {
   unsigned int val = 0 ;
   const unsigned int thirtyThree = 33 ;  // magic number from textbook
   for ( unsigned int i = 0 ; i < str.length(); i++)
      val = val * thirtyThree + str[i] ;
   return val ;
}

bool Tester::testNormalInsertion(Cache& cache, vector<Person> oldDataList) {
    vector<Person> newDataList;
    Random RndID(MINID,MAXID);
    Random RndStr(MINSEARCH,MAXSEARCH);
    bool result = true;
    int originalSize = cache.m_currentSize;
    int addSize = 4;
    string addArray[addSize] = {"java", "scheme", "python", "js"};


    for (int i=0;i<addSize;i++){
        // generating random data
        Person dataObj = Person(addArray[i], RndID.getRandNum());
        // saving data for later use
        newDataList.push_back(dataObj);
        // inserting data in to the Cache object
        cache.insert(dataObj);
    }

    // detects if everything is there and if the objects are in the correct placement...
    for (vector<Person>::iterator it = newDataList.begin(); it != newDataList.end(); it++){
        result = result && (*it == cache.getPerson((*it).getKey(), (*it).getID()));
        int hashKey = hashCode((*it).getKey()) % cache.m_currentCap;
        result = result && (*it == cache.m_currentTable[hashKey]);

    }

    // looks for previous items in list and if they exist also...
    for (vector<Person>::iterator it = oldDataList.begin(); it != oldDataList.end(); it++){
        result = result && (*it == cache.getPerson((*it).getKey(), (*it).getID()));
        int hashKey = hashCode((*it).getKey()) % cache.m_currentCap;
        result = result && (*it == cache.m_currentTable[hashKey]);
    }
    

    if(originalSize + addSize != cache.m_currentSize) {
        result = false;
    }

    return result;
}

bool Tester::testFindError(Cache& cache) {
    vector<Person> newDataList;
    Random RndID(MINID,MAXID);
    int originalSize = cache.m_currentSize;
    int addSize = 4;
    string addArray[4] = {"java", "scheme", "python", "js"};


    for (int i=0;i<addSize;i++){
        // generating random data
        Person dataObj = Person(addArray[i], RndID.getRandNum());
        // saving data for later use
        newDataList.push_back(dataObj);
        // inserting data in to the Cache object
        cache.insert(dataObj);
    }

    // testing by finding a key that doesn't exist within the array...
    Person foundPerson1 = cache.getPerson("c#", 3000);
    if (foundPerson1 == EMPTY) {
        return true;
    } else {
        return false;
    }
}

bool Tester::testFindNonColliding(Cache& cache) {
    vector<Person> newDataList;
    Random RndID(MINID,MAXID);
    Random RndStr(MINSEARCH,MAXSEARCH);
    int addSize = 5;


    string tArray[addSize] = {"c", "c++", "c#", "java", "python"};

    for (int i=0;i<addSize;i++){
        // generating random data
        Person dataObj = Person(tArray[i], RndID.getRandNum());
        // saving data for later use
        newDataList.push_back(dataObj);
        // inserting data in to the Cache object
        cache.insert(dataObj);
    }

    // detects if everything is there and if the objects are in the correct placement...
    for (vector<Person>::iterator it = newDataList.begin(); it != newDataList.end(); it++){
        if(cache.getPerson((*it).getKey(), (*it).getID()) == EMPTY) {
            return false;
        }
    }

    return true;
}

bool Tester::testFindColliding(Cache& cache) {
    vector<Person> newDataList;
    Random RndID(MINID,MAXID);
    Random RndStr(MINSEARCH,MAXSEARCH);
    int addSize = 30;

    // just enough to not trigger a rehash but have collisions atleast.
    for (int i=0;i<addSize;i++){
        // generating random data
        Person dataObj = Person(searchStr[RndStr.getRandNum()], RndID.getRandNum());
        // saving data for later use
        newDataList.push_back(dataObj);
        // inserting data in to the Cache object
        cache.insert(dataObj);
    }

    // detects if everything is there and if the objects are in the correct placement...
    for (vector<Person>::iterator it = newDataList.begin(); it != newDataList.end(); it++){

        if(cache.getPerson((*it).getKey(), (*it).getID()) == EMPTY) { // there shouldnt be an empty person...
            return false;
        }
    }

    return true;
}

bool Tester::testRemoveNonColliding(Cache& cache) {
    vector<Person> newDataList;
    Random RndID(MINID,MAXID);
    Random RndStr(MINSEARCH,MAXSEARCH);
    bool result = true;
    int addSize = 5;


    string tArray[addSize] = {"c", "c++", "c#", "java", "python"};

    for (int i=0;i<addSize;i++){
        // generating random data
        Person dataObj = Person(tArray[i], RndID.getRandNum());
        // saving data for later use
        newDataList.push_back(dataObj);
        // inserting data in to the Cache object
        cache.insert(dataObj);
    }

    // detects if everything is there and if the objects are in the correct placement...
    Person targetPerson = newDataList.back();
    cache.remove(targetPerson);

    // If it's not empty, then... remove operation failed.
    if(((not (cache.getPerson(targetPerson.getKey(), targetPerson.getID()) == EMPTY))) == true) { 
        return false;
    }

    return true;
}

bool Tester::testRemoveCollidingNoRehash(Cache& cache) {
    vector<Person> newDataList;
    Random RndID(MINID,MAXID);
    Random RndStr(MINSEARCH,MAXSEARCH);
    bool result = true;
    int addSize = 30;



    for (int i=0;i<addSize;i++){
        // generating random data
        Person dataObj = Person(searchStr[RndStr.getRandNum()], RndID.getRandNum());
        // saving data for later use
        newDataList.push_back(dataObj);
        // inserting data in to the Cache object
        cache.insert(dataObj);
    }

    // detects if everything is there and if the objects are in the correct placement...
    for (int i = 0; i < addSize / 2; i++){
        Person targetPerson = newDataList.back();
        cache.remove(targetPerson);

        // If it's not empty, then... remove operation failed.
        if(((not (cache.getPerson(targetPerson.getKey(), targetPerson.getID()) == EMPTY))) == true) { 
            return false;
        }
        newDataList.pop_back();
    }

    return true;
}

bool Tester::testInsertionRehashTrigger(Cache& cache) {
    vector<Person> newDataList;
    Random RndID(MINID,MAXID);
    Random RndStr(MINSEARCH,MAXSEARCH);
    bool result = true;
    int addSize = 51;

    Person* oldTable = cache.m_currentTable;

    for (int i=0;i<addSize;i++){
        // generating random data
        Person dataObj = Person(searchStr[RndStr.getRandNum()], RndID.getRandNum());
        // saving data for later use
        newDataList.push_back(dataObj);
        // inserting data in to the Cache object
        cache.insert(dataObj);
    }

    // currTable should be old table...
    if((cache.m_oldTable == oldTable)) {
        return true;
    } else {
        return false;
    }


}

bool Tester::testInsertionRehashCompletion(Cache& cache) {
    vector<Person> newDataList;
    Random RndID(MINID,MAXID);
    Random RndStr(MINSEARCH,MAXSEARCH);
    bool result = true;
    int addSize = 53;

    Person* oldTable = cache.m_currentTable;

    for (int i=0;i<addSize;i++){
        // generating random data
        Person dataObj = Person(searchStr[RndStr.getRandNum()], RndID.getRandNum());
        // saving data for later use
        newDataList.push_back(dataObj);
        // inserting data in to the Cache object
        cache.insert(dataObj);
    
    }

    // make sure that all the live values from m_oldTable has been transferred over to new table...
    // all the removed and deleted data has been deleted in newDataList.pop_back()
    for (vector<Person>::iterator it = newDataList.begin(); it != newDataList.end(); it++){
        Person targetPerson = cache.getPerson((*it).getKey(), (*it).getID());
        if(targetPerson == EMPTY) { // there shouldnt be an empty person...
            return false;
        }
    }

    // m_oldTable should be nil.. meaning hash is fully done.
    if((cache.m_oldTable == nullptr)) {
        return true;
    } else {
        return false;
    }
}

bool Tester::testDeletionRehashTrigger(Cache& cache) {
    vector<Person> newDataList;
    Random RndID(MINID,MAXID);
    Random RndStr(MINSEARCH,MAXSEARCH);
    bool result = true;
    int addSize = 100;

    for (int i=0;i<addSize;i++){
        // generating random data
        Person dataObj = Person(searchStr[RndStr.getRandNum()], RndID.getRandNum());
        // saving data for later use
        newDataList.push_back(dataObj);
        // inserting data in to the Cache object
        cache.insert(dataObj);
    }


    Person* oldTable = cache.m_currentTable;

    // detects if everything is there and if the objects are in the correct placement...
    for (int i = 0; i < 80; i++){
        Person targetPerson = newDataList.back();
        cache.remove(targetPerson);

        newDataList.pop_back();
    }

    // currTable should be old table...
    if((cache.m_oldTable == oldTable)) {
        return true;
    } else {
        return false;
    }
}

bool Tester::testDeletionRehashCompletion(Cache& cache) {
    vector<Person> newDataList; // i think problem is there 2 of the same values...
    Random RndID(MINID,MAXID);
    Random RndStr(MINSEARCH,MAXSEARCH);
    bool result = true;
    int addSize = 100; 

    Person* oldTable = cache.m_currentTable;

    int i=0;
    while (i < addSize){
        // generating random data
        Person dataObj = Person(searchStr[RndStr.getRandNum()], RndID.getRandNum());

        // good if we cannot find the person's existance...
        if(cache.getPerson(dataObj.getKey(), dataObj.getID()) == EMPTY) {
            // saving data for later use
            newDataList.push_back(dataObj);
            // inserting data in to the Cache object
            cache.insert(dataObj);
            i++;
        }

    }



    // remove items enough to complete hash
    for (int i = 0; i < 82; i++){ // trigger should be completed at this point...
        Person targetPerson = newDataList.back();    

        cache.remove(targetPerson);

        newDataList.pop_back();


    }


    // make sure that all the live values from m_oldTable has been transferred over to new table...
    // all the removed and deleted data has been deleted in newDataList.pop_back()
    for (vector<Person>::iterator it = newDataList.begin(); it != newDataList.end(); it++){
        Person targetPerson = cache.getPerson((*it).getKey(), (*it).getID());
        if(targetPerson == EMPTY) { // there shouldnt be an empty person...
            return false;
        }
    }

    // m_oldTable should be nullptr.. meaning hash is fully done.
    if((cache.m_oldTable == nullptr)) {
        return true;
    } else {
        return false;
    }
}
