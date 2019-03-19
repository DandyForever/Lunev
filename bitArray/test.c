#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "BitArray.h"

int main (){

    int counter = 0;

    BitArray * myBitArray = NULL;

    printf ("TEST #1\n");
    printf ("ZERO SIZE CONSTRUCT TEST\n");
    bitArrayConstruct(0);
    if (errno == EINVAL){
        printf ("TEST #1 is OK\n");
    }
    else{
        printf ("TEST #1 is FAILED\n");
    }

    ///---------------------------------------------------------

    printf ("\nTEST #2\n");
    printf ("MEMORY ALLOCATION TEST\n");

    counter = 0;
    for (int i = 0; i < 10000; i++){
        myBitArray = bitArrayConstruct(i);
        if (myBitArray == NULL && errno == ENOMEM){
            counter++;
        }
        else
            bitArrayDestruct(myBitArray);
    }
    printf ("Memory allocation failure for %d times\n", counter);
    printf ("TEST #2 is OK\n");

    ///----------------------------------------------------------

    printf ("\nTEST #3\n");
    printf ("ZERO POINTER DESTRUCTOR TEST\n");
    bitArrayDestruct(NULL);
    if (errno == EINVAL){
        printf ("TEST #3 is OK\n");
    }
    else
        printf ("TEST #3 is FAILED\n");

    ///----------------------------------------------------------

     printf ("\nTEST #4\n");
     printf ("SIZE TEST\n");

     counter = 0;
     for (int i = 0; i < 1000; i++) {
         myBitArray = bitArrayConstruct(10);
         if (bitArraySize(myBitArray) == 10 || errno == EINVAL) {
             counter++;
         }
     }

     if (counter == 1000)
         printf ("TEST #4 is OK\n");
     else
         printf ("TEST #4 is FAILED\n");

     bitArrayDestruct(myBitArray);

     printf ("\nTEST #5\n");
     printf ("SIZE TEST ZERO POINTER\n");
     if (bitArraySize(NULL) == 0)
         printf ("TEST #5 is OK\n");
     else
         printf ("TEST #5 is FAILED\n");

     ///----------------------------------------------------------

    printf ("\nTEST #6\n");
    printf ("INVALID ARGUMENT TEST\n");

    counter = 0;
    myBitArray = bitArrayConstruct(130);
    if (bitArrayPrint(NULL) != -1){
        counter++;
    }

    if (bitArraySet(NULL, 10, 1) != -1){
        counter++;
    }

    if (bitArraySet(myBitArray, 300, 0) != -1){
        counter++;
    }

    if (bitArraySet(myBitArray, 10, 3) != -1){
        counter++;
    }

    if (bitArrayGet(NULL, 10) != -1){
        counter++;
    }

    if (bitArrayGet(myBitArray, 399) != -1){
        counter++;
    }

    if (bitArrayFind(NULL, 10, 190, 1) != -1){
        counter++;
    }

    if (bitArrayFind(myBitArray, 300, 190, 1) != -1){
        counter++;
    }

    if (bitArrayFind(myBitArray, 10, 3000, 1) != -1){
        counter++;
    }

    if (counter) printf ("TEST #6 is FAILED\n");
    else printf ("TEST #6 is OK\n");

    ///-------------------------------------------------------------

    printf ("\nTEST #7\n");
    printf ("NORMAL WORK TEST\n");

    counter = 0;
    for (int i = 0; i < 130; i++){
        if (bitArraySet(myBitArray, i, rand() % 2) != 0) {
            counter++;
        }
    }

    bitArrayPrint(myBitArray);

    bitArraySet(myBitArray, 10, 1);
    bitArraySet(myBitArray, 20, 0);
    bitArraySet(myBitArray, 30, 1);
    if (bitArrayGet(myBitArray, 10) != 1){
        counter++;
    }
    else{
        bitArraySet(myBitArray, 10, 0);
        if (bitArrayGet(myBitArray, 10) != 0){
            counter++;
        }
    }

    if (counter) printf ("TEST #7 is FAILED\n");
    else printf ("TEST #7 is OK\n");

    ///--------------------------------------------------------------

    printf ("\nTEST #8\n");
    printf ("FIND TEST\n");

    for (int i = 0; i < 130; i++){
        bitArraySet(myBitArray, i, 0);
    }

    if (bitArrayFind(myBitArray, 0, 63, 1) != -1)
        printf ("FIND FAILURE\n");

    bitArraySet(myBitArray, 10, 1);
    //printf ("find: %d\texpected: %d\n", bitArrayFind(myBitArray, 0, 129, 1), 10);
    if (bitArrayFind(myBitArray, 0, 129, 1) != 10)
        printf ("FIND FAILURE\n");


    bitArraySet(myBitArray, 128, 1);
    //printf ("find: %d\texpected: %d\n", bitArrayFind(myBitArray, 20, 129, 1), 128);
    if (bitArrayFind(myBitArray, 20, 129, 1) != 128)
        printf ("FIND FAILURE\n");

    //printf ("find: %d\texpected: %d\n", bitArrayFind(myBitArray, 129, 129, 1), -1);
    if (bitArrayFind(myBitArray, 129, 129, 1) != -1)
        printf ("FIND FAILURE\n");

    bitArraySet(myBitArray, 129, 1);
    //printf ("find: %d\texpected: %d\n", bitArrayFind(myBitArray, 129, 129, 1), 129);
    if (bitArrayFind(myBitArray, 129, 129, 1) != 129)
        printf ("FIND FAILURE\n");

    bitArraySet(myBitArray, 128, 0);
    bitArraySet(myBitArray, 129, 0);
    //printf ("find: %d\texpected: %d\n", bitArrayFind(myBitArray, 15, 129, 1), -1);
    if (bitArrayFind(myBitArray, 15, 129, 1) != -1)
        printf ("FIND FAILURE\n");

    for (int i = 0; i < 130; i++){
        bitArraySet(myBitArray, i, 1);
    }

    bitArraySet(myBitArray, 10, 0);
    //printf ("find: %d\texpected: %d\n", bitArrayFind(myBitArray, 0, 129, 0), 10);
    if (bitArrayFind(myBitArray, 0, 129, 0) != 10)
        printf ("FIND FAILURE\n");


    bitArraySet(myBitArray, 128, 0);
    //printf ("find: %d\texpected: %d\n", bitArrayFind(myBitArray, 20, 129, 0), 128);
    if (bitArrayFind(myBitArray, 20, 129, 0) != 128)
        printf ("FIND FAILURE\n");


    //printf ("find: %d\texpected: %d\n", bitArrayFind(myBitArray, 129, 129, 0), -1);
    if (bitArrayFind(myBitArray, 129, 129, 0) != -1)
        printf ("FIND FAILURE\n");

    bitArraySet(myBitArray, 129, 0);
    //printf ("find: %d\texpected: %d\n", bitArrayFind(myBitArray, 129, 129, 0), 129);
    if (bitArrayFind(myBitArray, 129, 129, 0) != 129)
        printf ("FIND FAILURE\n");

    bitArraySet(myBitArray, 128, 1);
    bitArraySet(myBitArray, 129, 1);
    //printf ("find: %d\texpected: %d\n", bitArrayFind(myBitArray, 15, 129, 0), -1);
    if (bitArrayFind(myBitArray, 15, 129, 0) != -1)
        printf ("FIND FAILURE\n");

    printf ("TEST #8 is OK\n");

    ///------------------------------------------------------------------

    printf ("\nTEST #9\n");
    printf ("ITERATOR INVALID ARGUMENT TEST\n");

    counter = 0;
    Iterator * it = NULL;

    for (int i = 0; i < 10000; i++){
        it = iteratorConstruct(myBitArray);
        if (it == NULL)
            counter++;
        else{
            iteratorDestruct(it);
        }
    }

    printf ("Memory allocation failure for %d times\n", counter);

    counter = 0;
    if (iteratorConstruct(NULL) != NULL)
        counter++;

    if (iteratorDestruct(NULL) != -1)
        counter++;

    if (iteratorNext(NULL) != -1)
        counter++;

    if (iteratorGetElem(NULL) != -1)
        counter++;

    if (counter) printf ("TEST #9 is FAILED\n");
    else printf ("TEST #9 is OK\n");

    ///---------------------------------------------------------------------

    it = iteratorConstruct(myBitArray);

    printf ("\nTEST #10\n");
    printf ("ITERATOR NORMAL WORK TEST\n");

    counter = 0;
    for (int i = 0; i <= 130; i++){
        if (iteratorGetElem(it) != bitArrayGet(myBitArray, i))
            counter++;
        iteratorNext(it);

    }

    if (counter != 1) printf ("TEST #10 is FAILED\n");
    else printf ("TEST #10 is OK\n");

    free (it);
    free (myBitArray);

    return 0;
}