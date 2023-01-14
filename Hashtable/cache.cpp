#include "cache.h"

Cache::Cache(int size, hash_fn hash){
    m_hash = hash;

    m_currentCap = returnNewCurrCap(size);
    m_currentTable = new Person [m_currentCap];
    // Initizing current hashtable
    for (int i = 0; i< m_currentCap; i++) {
        m_currentTable[i] = EMPTY;
    }
    m_currentSize = 0;
    m_currNumDeleted = 0;

    m_oldTable = nullptr;     // hash table
    m_oldCap = 0;       // hash table size (capacity)
    m_oldSize = 0;      // current number of entries, m_oldSize includes deleted entries
    m_oldNumDeleted = 0; // number of deleted entries
}

Cache::~Cache(){
    delete[] m_currentTable;
    m_currentTable = nullptr;
    m_currentSize = 0;
    m_currNumDeleted = 0;
    m_currentCap = 0;

    delete[] m_oldTable;
    m_oldTable = nullptr;
    m_oldSize = 0;
    m_oldNumDeleted = 0;
    m_oldCap = 0;
}

bool Cache::insert(Person person){

    int hashKey = m_hash(person.getKey()) % m_currentCap;

    // check if ID is valid and within range.
    if(person.getID() < MINID)
        return false;
    else if(person.getID() > MAXID)
        return false;
    else if(m_currentTable[hashKey] == person) { // if it's a duplicate. we can't have that here...
        return false;
    }
    
    // collision check
    if ((not (m_currentTable[hashKey] == EMPTY)) && (not (m_currentTable[hashKey] == DELETED)))  { // if the index is not empty... we need to start looking for the next one...
        int index = hashKey;
        int i = 0;

        // while the index we find are not empty...
        // goal is to continue looping until we find an empty or deleted space...
        // DELETED CASE: not Empty && not DELETED
        // true | false
        // EMPTY CASE: not Empty && not DELETED
        // false | true
        // OCC CASE: not Empty && not DELETED
        // true | true
        while ((not (m_currentTable[index] == EMPTY)) && (not (m_currentTable[index] == DELETED))) {
            index = ((hashKey) + (i * i)) % m_currentCap;

            if(m_currentTable[hashKey] == person) { // if it's a duplicate. we can't have that here...
                cout << "DUPLICATE WARNING!!!" << endl;
                return false;
            }
            i++;

        }

        m_currentTable[index] = person;
        m_currentSize++;
    } else {
        m_currentTable[hashKey] = person;
        m_currentSize++;
    }




    if (lambda() > 0.5) {
        rehash();
    } else if(m_oldTable != nullptr){

        continueRehash();
    }



    return true;
}

bool Cache::remove(Person person){
    bool toggle = false;

    if(m_oldTable != nullptr) {
        int hashKey = m_hash(person.getKey()) % m_oldCap;

        // check if hashkey is the right one, otherwise we repeat until it is...
        int index = hashKey;
        int i = 0;

        // continue while the bucket in the old table is not equal to the person or until we hit the end...
        while((not (m_oldTable[index] == person)) && i <= m_oldCap) { 
            index = ((hashKey) + (i * i)) % m_oldCap;
            i++;
        }


        // deletes from oldTable
        if(m_oldTable[index] == person) {
            m_oldTable[index] = DELETED;
            m_oldNumDeleted++;
            toggle = true;
        }
    }

    // if we couldn't find the answer in oldTable, we now look in newTable
    int index = m_hash(person.getKey()) % m_currentCap; // we use a new index...
    int i = 0;

    while((not (m_currentTable[index] == person)) && i <= m_currentCap) {
        index = ((m_hash(person.getKey()) % m_currentCap) + (i * i)) % m_currentCap;
        i++;
    }


    // deletes from currentTable
    if(m_currentTable[index] == person) {
        m_currentTable[index] = DELETED;
        m_currNumDeleted++;
        toggle = true;
    }
    
    
    if (toggle == false) { // couldn't find the person at all...
        return false;
    }


    if (deletedRatio() > .80) {
        rehash();
    } else if(m_oldTable != nullptr) { // whilst table is oldTable, we continue incrementally insertion...
        continueRehash();
    }

    return true;
}

Person Cache::getPerson(string key, int id) const{
    if(m_oldTable != nullptr) {
        int hashKey = m_hash(key) % m_oldCap;

        // check if hashkey is the right one, otherwise we repeat until it is...
        int index = hashKey;
        int i = 0;

        if(m_oldTable[index].getID() == id) {
            // returns from oldTable
            return m_oldTable[index];
        }

        // continue while the bucket in the old table is not equal to the person or until we hit the end...
        while((not (m_oldTable[index].getID() == id)) && i <= m_oldCap) { 
            index = ((hashKey) + (i * i)) % m_oldCap;
            if(m_oldTable[index].getID() == id) {
                // returns from oldTable
                return m_oldTable[index];
            }
            i++;
        }



    }


    // if we couldn't find the answer in oldTable, we now look in newTable
    int index = m_hash(key) % m_currentCap; // we use a new index...
    int i = 0;

    if(m_currentTable[index].getID() == id) {
        // returns from oldTable
        return m_currentTable[index];
    }

    // we continue looping as long as the index is either empty or not the right ID... and not execisively over the cap.
    while((not (m_currentTable[index].getID() == id)) && i <= m_currentCap) {

        index = ((m_hash(key) % m_currentCap) + (i * i)) % m_currentCap;

   
        if(m_currentTable[index].getID() == id) {
            // returns from oldTable
            return m_currentTable[index];
        }

        i++;
    }


    return EMPTY;
}


float Cache::lambda() const {
    return ((float)m_currentSize / (float)m_currentCap);
}

float Cache::deletedRatio() const {
    return ((float)m_currNumDeleted / (float)m_currentSize);
}

void Cache::dump() const {
    cout << "Dump for the current table: " << endl;
    if (m_currentTable != nullptr)
        for (int i = 0; i < m_currentCap; i++) {
            cout << "[" << i << "] : " << m_currentTable[i] << endl;
        }
    cout << "Dump for the old table: " << endl;
    if (m_oldTable != nullptr)
        for (int i = 0; i < m_oldCap; i++) {
            cout << "[" << i << "] : " << m_oldTable[i] << endl;
        }
}

bool Cache::isPrime(int number){
    bool result = true;
    for (int i = 2; i <= number / 2; ++i) {
        if (number % i == 0) {
            result = false;
            break;
        }
    }
    return result;
}

int Cache::findNextPrime(int current){
    //we always stay within the range [MINPRIME-MAXPRIME]
    //the smallest prime starts at MINPRIME
    if (current < MINPRIME) current = MINPRIME-1;
    for (int i=current; i<MAXPRIME; i++) { 
        for (int j=2; j*j<=i; j++) {
            if (i % j == 0) 
                break;
            else if (j+1 > sqrt(i) && i != current) {
                return i;
            }
        }
    }
    //if a user tries to go over MAXPRIME
    return MAXPRIME;
}


ostream& operator<<(ostream& sout, const Person &person ) {
    if (!person.m_key.empty())
        sout << person.m_key << " (ID " << person.m_id << ")";
    else
        sout << "";
  return sout;
}

bool operator==(const Person& lhs, const Person& rhs){
    return ((lhs.m_key == rhs.m_key) && (lhs.m_id == rhs.m_id));
}
