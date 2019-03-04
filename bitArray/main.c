#include <stdio.h>
#include "BitArray.h"

int main() {
    BitArray * myBitArray = bitArrayConstruct(12);
    printf ("%ld\n", bitArraySize(myBitArray));
    bitArrayPrint(myBitArray);
    bitArraySet(myBitArray, 2, 1);
    bitArrayPrint(myBitArray);
    printf ("%d\n", bitArrayGet(myBitArray, 2));
    bitArrayDestruct(myBitArray);
    return 0;
}