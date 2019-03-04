typedef struct BitArray BitArray;
typedef struct Proxy Proxy;


BitArray * bitArrayConstruct (size_t size);
int bitArrayDestruct (BitArray * obj);
size_t bitArraySize (BitArray * obj);
__int8_t bitArrayGet (BitArray * obj, size_t index);
__int8_t bitArraySet (BitArray * obj, size_t index, int value);
__int8_t bitArrayPrint (BitArray * obj);