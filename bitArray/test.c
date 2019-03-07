#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "BitArray.h"

int main (){
    BitArray * myBitArray = NULL;

    printf ("TEST #1\n");
    printf ("ZERO SIZE CONSTRUCT TEST\n");
    bitArrayConstruct(0);
    if (errno == EINVAL){
        printf ("ZERO SIZE ERROR\n");
        printf ("TEST #1 is OK\n");
    }
    else{
        printf ("TEST #1 is FAILED\n");
    }

    ///---------------------------------------------------------

    printf ("\nTEST #2\n");
    printf ("MEMORY ALLOCATION TEST\n");
    for (int i = 0; i < 10000; i++){
        myBitArray = bitArrayConstruct(i);
        if (myBitArray == NULL && errno == ENOMEM){
            printf ("NOT ENOUGH MEMORY ERROR\n");
        }
        else
            bitArrayDestruct(myBitArray);
    }
    printf ("TEST #2 is OK\n");

    ///----------------------------------------------------------

    printf ("\nTEST #3\n");
    printf ("ZERO POINTER DESTRUCTOR TEST\n");
    bitArrayDestruct(NULL);
    if (errno == EINVAL){
        printf ("ZERO POINTER ERROR\n");
        printf ("TEST #3 is OK\n");
    }
    else
        printf ("TEST #3 is FAILED\n");

    ///----------------------------------------------------------

     printf ("\nTEST #4\n");
     printf ("SIZE TEST\n");
     for (int i = 0; i < 1000; i++) {
         myBitArray = bitArrayConstruct(10);
         if (bitArraySize(myBitArray) == 64) {
             printf("TEST #4 is OK\n");
         } else
             printf("TEST #4 is FAILED\n");
     }

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
    myBitArray = bitArrayConstruct(130);
    if (bitArrayPrint(NULL) != -1){
        printf ("TEST #6 is FAILED\n");
    }

    else if (bitArraySet(NULL, 10, 1) != -1){
        printf ("TEST #6 is FAILED\n");
    }

    else if (bitArraySet(myBitArray, 300, 0) != -1){
        printf ("TEST #6 is FAILED\n");
    }

    else if (bitArraySet(myBitArray, 10, 3) != -1){
        printf ("TEST #6 is FAILED\n");
    }

    else if (bitArrayGet(NULL, 10) != -1){
        printf ("TEST #6 is FAILED\n");
    }

    else if (bitArrayGet(myBitArray, 399) != -1){
        printf ("TEST #6 is FAILED\n");
    }

    else if (bitArrayFind(NULL, 10, 190, 1) != -1){
        printf ("TEST #6 is FAILED\n");
    }

    else if (bitArrayFind(myBitArray, 300, 190, 1) != -1){
        printf ("TEST #6 is FAILED\n");
    }

    else if (bitArrayFind(myBitArray, 10, 3000, 1) != -1){
        printf ("TEST #6 is FAILED\n");
    }

    else
        printf ("TEST #6 is OK\n");

    ///-------------------------------------------------------------

    printf ("\nTEST #7\n");
    printf ("NORMAL WORK TEST\n");

    for (int i = 0; i < 100; i++){
        if (bitArraySet(myBitArray, i, rand() % 2) != 0)
            printf ("SET FAILURE\nTEST #7 is FAILED\n");
        if (bitArrayPrint(myBitArray) != 0)
            printf ("PRINT FAILURE\n TEST #7 is FAILED\n");
    }
    bitArraySet(myBitArray, 10, 1);
    bitArraySet(myBitArray, 20, 0);
    bitArraySet(myBitArray, 30, 1);
    if (bitArrayGet(myBitArray, 10) != 1){
        printf ("GET FAILURE\n TEST #7 is FAILED\n");
    }
    else{
        bitArraySet(myBitArray, 10, 0);
        if (bitArrayGet(myBitArray, 10) != 0){
            printf ("GET FAILURE\n TEST #7 is FAILED\n");
        }
        else
            printf ("TEST #7 is OK\n");
    }

    ///--------------------------------------------------------------

    printf ("\nTEST #8\n");
    printf ("FIND TEST\n");

    for (int i = 0; i < 100; i++){
        bitArraySet(myBitArray, i, 0);
    }

    bitArraySet(myBitArray, 10, 1);
    if (bitArrayFind(myBitArray, 0, 190, 0) != 10)
        printf ("FIND FAILURE\n");


    bitArraySet(myBitArray, 128, 1);
    if (bitArrayFind(myBitArray, 20, 190, 0) != 128)
        printf ("FIND FAILURE\n");

    if (bitArrayFind(myBitArray, 129, 190, 0) != -1)
        printf ("FIND FAILURE\n");

    bitArraySet(myBitArray, 129, 1);
    if (bitArrayFind(myBitArray, 129, 190, 0) != 129)
        printf ("FIND FAILURE\n");

    bitArraySet(myBitArray, 128, 0);
    bitArraySet(myBitArray, 129, 0);
    if (bitArrayFind(myBitArray, 15, 190, 0) != -1)
        printf ("FIND FAILURE\n");

    for (int i = 0; i < 100; i++){
        bitArraySet(myBitArray, i, 1);
    }

    bitArraySet(myBitArray, 10, 0);
    if (bitArrayFind(myBitArray, 0, 190, 0) != 10)
        printf ("FIND FAILURE\n");


    bitArraySet(myBitArray, 128, 0);
    if (bitArrayFind(myBitArray, 20, 190, 0) != 128)
        printf ("FIND FAILURE\n");

    if (bitArrayFind(myBitArray, 129, 190, 0) != -1)
        printf ("FIND FAILURE\n");

    bitArraySet(myBitArray, 129, 0);
    if (bitArrayFind(myBitArray, 129, 190, 0) != 129)
        printf ("FIND FAILURE\n");

    bitArraySet(myBitArray, 128, 1);
    bitArraySet(myBitArray, 129, 1);
    if (bitArrayFind(myBitArray, 15, 190, 0) != -1)
        printf ("FIND FAILURE\n");

    printf ("TEST #8 is OK\n");

    ///------------------------------------------------------------------

    printf ("\nTEST #9\n");
    printf ("ITERATOR INVALID ARGUMENT TEST\n");

    Iterator * it = NULL;

    for (int i = 0; i < 10000; i++){
        it = iteratorConstruct(myBitArray);
        if (it == NULL)
            printf ("NOT ENOUGH MEMORY ERROR\n");
        else{
            iteratorDestruct(it);
        }
    }

    if (iteratorConstruct(NULL) != NULL)
        printf ("CONSTRUCT FAILURE\n");

    if (iteratorDestruct(NULL) != -1)
        printf ("DESTRUCT FAILURE\n");

    if (iteratorNext(NULL) != -1)
        printf ("NEXT FAILURE\n");

    if (iteratorGetElem(NULL) != -1)
        printf ("GET FAILURE\n");

    printf ("TEST #9 is OK\n");

    ///---------------------------------------------------------------------

    it = iteratorConstruct(myBitArray);

    printf ("\nTEST #10\n");
    printf ("ITERATOR NORMAL WORK TEST\n");

    for (int i = 0; i < 192; i++){
        iteratorNext(it);
        iteratorGetElem(it);
    }

    printf ("TEST #10 is OK\n");

    free (it);
    free (myBitArray);

    return 0;
}