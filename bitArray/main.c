#include <stdio.h>
#include "BitArray.c"

int main() {
    BitArray * myBitArray = bitArrayConstruct(130);
    printf ("%ld\n", bitArraySize(myBitArray));
    bitArrayPrint(myBitArray);
    bitArraySet(myBitArray, 2, 1);
    bitArraySet(myBitArray, 128, 1);
    bitArraySet(myBitArray, 129, 1);
    bitArrayPrint(myBitArray);
    printf ("%d\n", bitArrayGet(myBitArray, 2));
    printf ("%d\n", bitArrayFind(myBitArray, 130));

    Iterator * it = iteratorConstruct(myBitArray);
    do {
        printf ("%d", iteratorGetElem(it));
    } while (!iteratorNext(it));
    iteratorDestruct(it);
    bitArrayDestruct(myBitArray);
    return 0;
}