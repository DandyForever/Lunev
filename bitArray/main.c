#include <stdio.h>
#include "BitArray.c"

int main() {
    BitArray * myBitArray = bitArrayConstruct(100);
    printf ("%ld\n", bitArraySize(myBitArray));
    bitArrayPrint(myBitArray);
    bitArraySet(myBitArray, 2, 1);
    bitArraySet(myBitArray, 63, 1);
    bitArraySet(myBitArray, 64, 1);
    bitArraySet(myBitArray, 90, 1);
    bitArraySet(myBitArray, 127, 1);
    bitArrayPrint(myBitArray);
    printf ("%d\n", bitArrayGet(myBitArray, 2));

    Iterator * it = iteratorConstruct(myBitArray);
    do {
        printf ("%d", iteratorGetElem(it));
    } while (!iteratorNext(it));
    iteratorDestruct(it);
    bitArrayDestruct(myBitArray);
    return 0;
}