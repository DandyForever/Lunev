typedef struct BitArray BitArray;
typedef struct Proxy Proxy;
typedef struct ArrayIterator Iterator;

BitArray * bitArrayConstruct (size_t size);
int bitArrayDestruct (BitArray * obj);
size_t bitArraySize (BitArray * obj);
__int8_t bitArrayGet (BitArray * obj, size_t index);
__int8_t bitArraySet (BitArray * obj, size_t index, int value);
__int8_t bitArrayPrint (BitArray * obj);

Iterator * iteratorConstruct (BitArray * obj);
__int8_t iteratorDestruct (Iterator * it);
__int8_t iteratorNext (Iterator * it);
__int8_t iteratorGetElem (Iterator * it);
int bitArrayFind (BitArray * obj, size_t start, size_t end, int value);
