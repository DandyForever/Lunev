#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <stdbool.h>
#include "BadCalloc.h"
#include "BitArray.h"

struct BitArray {
    __uint64_t * data_;
    size_t size_;
};

struct Proxy {
    __uint8_t shift_;
    __uint64_t * ptr_;
};

struct ArrayIterator {
    BitArray * array_;
    __uint64_t * elem_;
    __uint8_t shift_;
    size_t index_;
};

BitArray * bitArrayConstruct (size_t size){
    if (size == 0){
        errno = EINVAL;
        return NULL;
    }

    BitArray * obj = (BitArray *) badCalloc(1, sizeof (BitArray));
    if (obj == NULL){
        errno = ENOMEM;
        return NULL;
    }

    size_t numOfElements = size / BITINELEM + (size % BITINELEM != 0);
    obj -> data_ = (__uint64_t *) badCalloc (numOfElements, sizeof (__uint64_t));
    if (obj -> data_ == NULL){
        errno = ENOMEM;
        free (obj);
        return NULL;
    }

    obj -> size_ = numOfElements;

    errno = 0;
    return obj;
}

int bitArrayDestruct (BitArray * obj){
    if (obj == NULL){
        errno = EINVAL;
        return -1;
    }

    if (obj -> data_ == NULL){
        errno = EINVAL;
        free (obj);
        return -1;
    }

    free (obj -> data_);
    free (obj);

    errno = 0;
    return 0;
}

size_t bitArraySize (BitArray * obj){
    if (obj == NULL){
        errno = EINVAL;
        return 0;
    }

    errno = 0;
    return obj -> size_ * BITINELEM;
}

__int8_t bitArrayPrint (BitArray * obj){
    if (obj == NULL){
        errno = EINVAL;
        return -1;
    }

    for (int i = 0; i < obj -> size_; i++){
        for (int shift = BITINELEM - 1; shift >= 0; shift--){
            if (obj -> data_[i] & (((__uint64_t) 1) << shift))
                printf ("%d ", 1);
            else
                printf ("%d ", 0);
        }
        printf (" ");
    }
    printf ("\n");

    errno = 0;
    return 0;
}

__int8_t bitArrayGet (BitArray * obj, size_t index){
    if (obj == NULL){
        errno = EINVAL;
        return -1;
    }

    if (index / BITINELEM + (index % BITINELEM != 0) > obj -> size_){
        errno = EINVAL;
        return -1;
    }

    Proxy tmp = {
            .ptr_ = obj -> data_ + index / BITINELEM,
            .shift_ = BITINELEM - index % BITINELEM - 1
    };

    if ((*tmp.ptr_) & (((__uint64_t) 1) << tmp.shift_))
        return 1;
    else
        return 0;
}

__int8_t bitArraySet (BitArray * obj, size_t index, int value){
    if (obj == NULL || value != 0 && value != 1 ||
            index / BITINELEM + (index % BITINELEM != 0) > obj -> size_){
        errno = EINVAL;
        return -1;
    }

    Proxy tmp = {
            .ptr_ = obj -> data_ + index / BITINELEM,
            .shift_ = BITINELEM - index % BITINELEM - 1
    };


    if (value)
        *(tmp.ptr_) |= (((__uint64_t) 1) << tmp.shift_);
    else
        *tmp.ptr_ &= (~((__uint64_t) 1) << tmp.shift_);

    errno = 0;
    return 0;
}

Iterator * iteratorConstruct (BitArray * obj){
    if (obj == NULL){
        errno = EINVAL;
        return NULL;
    }

    Iterator * it = (Iterator *) badCalloc (1, sizeof (Iterator));
    if (it == NULL){
        errno = ENOMEM;
        return NULL;
    }

    it -> array_ = obj;
    it -> elem_ = obj -> data_;
    it -> shift_ = BITINELEM - 1;
    it -> index_ = 0;

    printf ("%ld\n", it -> array_ -> size_);

    errno = 0;
    return it;
}

__int8_t iteratorDestruct (Iterator * it){
    if (it == NULL){
        errno = EINVAL;
        return -1;
    }

    free (it);

    errno = 0;
    return 0;
}

__int8_t iteratorNext (Iterator * it){
    if (it == NULL){
        errno = EINVAL;
        return -1;
    }

    if (it -> shift_ != 0) {
        errno = 0;
        it -> shift_--;
        return 0;
    }

    else if (it -> index_ != it -> array_ -> size_ - 1) {
        errno = 0;
        it -> index_++;
        it -> elem_++;
        it -> shift_ = BITINELEM - 1;
        return 0;
    }

    return 1;
}

__int8_t iteratorGetElem (Iterator * it){
    if (it == NULL){
        errno = EINVAL;
        return -1;
    }

    errno = 0;
    if (*(it -> elem_) & ((__uint64_t) 1 << it -> shift_))
        return 1;
    else
        return 0;
}