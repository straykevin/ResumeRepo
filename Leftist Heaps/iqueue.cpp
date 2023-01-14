
#include "iqueue.h"
IQueue::IQueue(prifn_t priFn, HEAPTYPE heapType)
{
    m_heap = nullptr; 
    m_size = 0; 
    m_priorFunc = priFn; 
    m_heapType = heapType;  
}

IQueue::~IQueue() {
  clear();
}

IQueue::IQueue(const IQueue& rhs) {
  m_heap = nullptr; 
  m_size = 0;
  m_priorFunc = rhs.m_priorFunc;
  m_heapType = rhs.m_heapType;

  m_heap = copyRecursive(rhs.m_heap);
}

IQueue& IQueue::operator=(const IQueue& rhs) {

  clear();

  if(this == &rhs) {
    return *this;
  }

  m_heap = rhs.m_heap;
  m_size = rhs.m_size;
  m_priorFunc = rhs.m_priorFunc;
  m_heapType = rhs.m_heapType;

  return *this;
}

void IQueue::insertCrop(const Crop& crop) {
    Node* newNode = new Node(crop);

    m_heap = merge(m_heap, newNode);
    m_size++;
}

Crop IQueue::getNextCrop() {
  
  if(m_heap == nullptr) {
    throw std::domain_error("You are attempting to get next crop from an empty skew-heap!");
  }

  Node* oldNode = m_heap;
  Crop nextCrop = oldNode->getCrop();

  m_heap = merge(m_heap->m_left, m_heap->m_right);


  delete oldNode;
  oldNode = nullptr;
  m_size--;

  return nextCrop;
}

void IQueue::mergeWithQueue(IQueue& rhs) {
  if(getPriorityFn() != rhs.getPriorityFn()) {
    throw std::domain_error("You attempted to merge queues with different priority functions!!");
  }

  if(this == &rhs) {
    return;
  }

  

  m_heap = merge(m_heap, rhs.m_heap);
  rhs.m_heap = nullptr;
}

void IQueue::clear() {
  clearRecursive(m_heap);

  m_heap = nullptr;
  m_size = 0;
  m_priorFunc = nullptr;

  
}

int IQueue::numCrops() const
{
  return m_size;
}

void IQueue::printCropsQueue() const {
  printPreOrder(m_heap);  
}

prifn_t IQueue::getPriorityFn() const {
  return m_priorFunc;
}

HEAPTYPE IQueue::getHeapType() const {
  return m_heapType;
}

void IQueue::setPriorityFn(prifn_t priFn, HEAPTYPE heapType) {
  IQueue oldIQueue(*this);

  clear();
  m_heap = nullptr;
  m_priorFunc = priFn;
  m_heapType = heapType;


  while(oldIQueue.m_heap != nullptr) {
    Crop currCrop = oldIQueue.getNextCrop();

    insertCrop(currCrop);
  }

}

Node* IQueue::merge(Node* left, Node* right) {

  Node* newSubRoot = nullptr;

  if(m_heapType == MAXHEAP) {
    newSubRoot = mergeMax(left, right);
  } else if (m_heapType == MINHEAP) {
    newSubRoot = mergeMin(left, right);
  }

  if(newSubRoot != nullptr) { // skew heap property
    swap(newSubRoot->m_left, newSubRoot->m_right);
  }


  return newSubRoot;
}


void IQueue::dump() const
{
  if (m_size == 0) {
    cout << "Empty skew heap.\n" ;
  } else {
    dump(m_heap);
    cout << endl;
  }
}

void IQueue::dump(Node *pos) const {
  if ( pos != nullptr ) {
    cout << "(";
    dump(pos->m_left);
    cout << m_priorFunc(pos->m_crop) << ":" << pos->m_crop.getCropID();
    dump(pos->m_right);
    cout << ")";
  }
}

ostream& operator<<(ostream& sout, const Crop& crop) {
  sout << "Crop ID: " << crop.getCropID() 
        << ", current temperature: " << crop.getTemperature()
        << ", current soil moisture: " << crop.getMoisture() << "%"
        << ", current time: " << crop.getTimeString()
        << ", plant type: " << crop.getTypeString();
  return sout;
}

ostream& operator<<(ostream& sout, const Node& node) {
  sout << node.getCrop();
  return sout;
}
