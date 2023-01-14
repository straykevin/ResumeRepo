// Date Created: November, 2022
#ifndef IQUEUE_H
#define IQUEUE_H
#include <stdexcept>
#include <iostream>
#include <string>
using namespace std;
class Grader;   // forward declaration (for grading purposes)
class Tester;   // forward declaration
class IQueue;   // forward declaration
// Constant parameters, min and max values
#define DEFAULTCROPID 100000
const int MINCROPID = 100001;// minimum crop ID
const int MAXCROPID = 999999;// maximum crop ID
// The temperature
const int MINTEMP = 30;     // lowest priority
const int MAXTEMP = 110;    // highest priority
// The soil moisture percentage
const int MINMOISTURE = 0;  // highest priority
const int MAXMOISTURE = 100;// lowest priority
// time of day, MORNING is the highest priority
enum TIME {MORNING,NOON,AFTERNOON,NIGHT};
const int MINTIME = MORNING;// highest priority
const int MAXTIME = NIGHT;  // lowest priority
// The type of plant based on the water requirements
enum PLANT {BEAN,MELON,MAIZE,SUNFLOWER,COTTON,CITRUS,SUGARCANE};
const int MINTYPE = BEAN;       // lowest priority
const int MAXTYPE = SUGARCANE;  // highest priority

enum HEAPTYPE {MINHEAP, MAXHEAP};

class Crop{
    public:
    friend class Grader; // for grading purposes
    friend class Tester; // for testing purposes
    friend class IQueue;
    Crop(){
        m_cropID = DEFAULTCROPID;m_temperature = MINTEMP;
        m_moisture = MAXMOISTURE;m_time = MAXTIME;
        m_type = MINTYPE;
    }
    Crop(int ID, int temperature, int moisture, int time, int type){
        if (ID < MINCROPID || ID > MAXCROPID) m_cropID = DEFAULTCROPID;
        else m_cropID = ID;
        if (temperature < MINTEMP || temperature > MAXTEMP) m_temperature = MINTEMP;
        else m_temperature = temperature;
        if (moisture < MINMOISTURE || moisture > MAXMOISTURE) m_moisture = MAXMOISTURE;
        else m_moisture = moisture;
        if (time < MINTIME || time > MAXTIME) m_time = MAXTIME;
        else m_time = time;
        if (type < MINTYPE || type > MAXTYPE) m_type = MINTYPE;
        else m_type = type;
    }
    int getCropID() const {return m_cropID;}
    int getTemperature() const {return m_temperature;}
    int getMoisture() const {return m_moisture;}
    int getTime() const {return m_time;}
    string getTimeString() const {
        string result = "UNKNOWN";
        switch (m_time)
        {
        case MORNING: result = "MORNING"; break;
        case NOON: result = "NOON"; break;
        case AFTERNOON: result = "AFTERNOON"; break;
        case NIGHT: result = "NIGHT"; break;
        default: break;
        }
        return result;
    }
    int getType() const {return m_type;}
    string getTypeString() const {
        string result = "UNKNOWN";
        switch (m_type)
        {
        case BEAN: result = "BEAN"; break;
        case MELON: result = "MELON"; break;
        case MAIZE: result = "MAIZE"; break;
        case SUNFLOWER: result = "SUNFLOWER"; break;
        case COTTON: result = "COTTON"; break;
        case CITRUS: result = "CITRUS"; break;
        case SUGARCANE: result = "SUGARCANE"; break;
        default: break;
        }
        return result;
    }
    private:
    int m_cropID;       // every crop is identified by a unique ID
    // m_temperature shows the temperature at the calculation time
    // the lower the temperature is the lower the priority is
    int m_temperature;  // 30-110 degree Fahrenheit
    // m_moisture shows how moist is the soil for the crop object at the calculation time
    // a value of 0 indicates the highest priority
    // a value of 100 indicates the lowest proprity
    int m_moisture;     // 0-100 %
    // m_time shows the time of the say at the calculation time
    // the time of day is divided into 4 windows
    // a value of 0 means a higher priority
    // a value of 3 means a lower priority
    int m_time;         // 0-3, an enum type is defined for this
    // m_type shows the type of a crop based on the plant watering requirement
    // a value of 0 means a lower priority, 
    // a value of 6 means a higher priority 
    int m_type;         // 0-6, an enum type is defined for this
};

class Node {
  public:
  friend class Grader; // for grading purposes
  friend class Tester; // for testing purposes
  friend class IQueue;
  Node(Crop crop) {  
    m_crop = crop;
    m_right = nullptr;
    m_left = nullptr;
  }
  Crop getCrop() const {return m_crop;}
  private:
  Crop m_crop;      // crop information
  Node * m_right;   // right child
  Node * m_left;    // left child
};

// Overloaded insertion operators for Crop and Node
ostream& operator<<(ostream& sout, const Crop& crop);
ostream& operator<<(ostream& sout, const Node& node);

// Priority function pointer type
typedef int (*prifn_t)(const Crop&);

class IQueue{
    public:
    friend class Grader; // for grading purposes
    friend class Tester; // for testing purposes
    
    IQueue(prifn_t priFn, HEAPTYPE heapType);
    ~IQueue();
    IQueue(const IQueue& rhs);
    IQueue& operator=(const IQueue& rhs);
    void insertCrop(const Crop& crop);
    Crop getNextCrop(); // Return the highest priority crop
    void mergeWithQueue(IQueue& rhs);
    void clear();
    int numCrops() const; // Return number of crops in queue
    void printCropsQueue() const; // Print the queue using preorder traversal
    prifn_t getPriorityFn() const;
    // Set a new priority function. Must rebuild the heap!!!
    void setPriorityFn(prifn_t priFn, HEAPTYPE heapType);
    HEAPTYPE getHeapType() const;
    void dump() const; // For debugging purposes

    private:
    Node * m_heap;          // Pointer to root of skew heap
    int m_size;             // Current size of the heap
    prifn_t m_priorFunc;    // Function to compute priority
    HEAPTYPE m_heapType;    // either a MINHEAP or a MAXHEAP

    void dump(Node *pos) const; // helper function for dump

    /******************************************
     * Private function declarations go here! *
     ******************************************/
    Node* merge(Node* left, Node* right);

    Node* mergeMax(Node* left, Node* right) {
        Node* newSubRoot = nullptr;
        if(left != nullptr && right != nullptr) {
            if(m_priorFunc(left->getCrop()) >= m_priorFunc(right->getCrop())) {
            left->m_right = merge(left->m_right, right);
            newSubRoot = left;
            } else {
            right->m_right = merge(right->m_right, left);
            newSubRoot = right;
            }
        } else if(left) {
            newSubRoot = left;
        } else if (right) {
            newSubRoot = right;
        }
        
        return newSubRoot;
    };

    Node* mergeMin(Node* left, Node* right) {
        Node* newSubRoot = nullptr;
        if(left != nullptr && right != nullptr) {
            if(m_priorFunc(left->getCrop()) <= m_priorFunc(right->getCrop())) {
            left->m_right = merge(left->m_right, right);
            newSubRoot = left;
            } else {
            right->m_right = merge(right->m_right, left);
            newSubRoot = right;
            }
        } else if(left) {
            newSubRoot = left;
        } else if (right) {
            newSubRoot = right;
        }
        
        return newSubRoot;
    };

    void clearRecursive(Node* currentNode) { // i know this works
        if(currentNode) {
            clearRecursive(currentNode->m_left);
            clearRecursive(currentNode->m_right);

            delete currentNode;
            currentNode = nullptr;
        }
    };

    Node* copyRecursive(Node* currentNode) { // i know this works...
        if(currentNode != nullptr) {
            Node* leftNode = copyRecursive(currentNode->m_left);
            Node* rightNode = copyRecursive(currentNode->m_right);

            Node* newNode = new Node(currentNode->getCrop());
            newNode->m_left = leftNode;
            newNode->m_right = rightNode;
            m_size++;


            return newNode;
        } else {
            return nullptr;
        }

    }

    void printPreOrder(Node* currNode) const {
        if (currNode == NULL) {
            return;
        }

    
        Crop crop = currNode->getCrop();
        cout << "[" << m_priorFunc(crop) << "] Crop ID:" <<  crop.getCropID() << ", current temperature: " << crop.getTemperature() << ", current soil moisture: " << crop.getMoisture() << "%, current time: " << crop.getTimeString() << ", plant type: " << crop.getTypeString() << endl;
    

        printPreOrder(currNode->m_left);

        printPreOrder(currNode->m_right);
    }
};
#endif
 
