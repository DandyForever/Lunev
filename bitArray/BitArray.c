#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <stdbool.h>
#include "BadCalloc.h"
#include "BitArray.h"

const __uint8_t BITINELEM = sizeof (__uint64_t) * 8;

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

    size_t numOfElements = (size + BITINELEM - 1) / BITINELEM;
    obj -> data_ = (__uint64_t *) badCalloc (numOfElements, sizeof (__uint64_t));
    if (obj -> data_ == NULL){
        errno = ENOMEM;
        free (obj);
        return NULL;
    }

    obj -> size_ = size;

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
    return obj -> size_;
}

__int8_t bitArrayPrint (BitArray * obj){
    if (obj == NULL){
        errno = EINVAL;
        return -1;
    }

    size_t blocks = (obj -> size_ + BITINELEM - 1) / BITINELEM;
    size_t printedBits = 0;

    for (int i = 0; i < blocks; i++){
        for (int shift = BITINELEM - 1; shift >= 0; shift--){
            if (obj -> data_[i] & (((__uint64_t) 1) << shift))
                printf ("%d ", 1);
            else
                printf ("%d ", 0);

            printedBits++;

            if (printedBits > obj -> size_)
                break;
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

    if (index > obj -> size_ - 1){
        errno = EINVAL;
        return -1;
    }

    Proxy tmp = {
            .ptr_ = obj -> data_ + index / BITINELEM,
            .shift_ = BITINELEM - index % BITINELEM - 1
    };

    return !!((*tmp.ptr_) & (((__uint64_t) 1) << tmp.shift_));
}

__int8_t bitArraySet (BitArray * obj, size_t index, int value){
    if (obj == NULL || (value != 0 && value != 1) ||
            index  > obj -> size_ - 1){
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
        *tmp.ptr_ &= ~(((__uint64_t) 1) << tmp.shift_);

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

    if (it -> index_ != it -> array_ -> size_ - 1) {
        errno = 0;
        it -> index_++;

        if (it -> shift_ != 0){
            it ->shift_--;
        }
        else{
            it -> elem_++;
            it -> shift_ = BITINELEM - 1;
        }

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
    return !!(*(it -> elem_) & ((__uint64_t) 1 << it -> shift_));
}

int bitArrayFind (BitArray * obj, size_t start, size_t end, int value){
    if (obj == NULL){
        errno = EINVAL;
        return -1;
    }

    if (start > obj -> size_ - 1 || end > obj -> size_ - 1 || end < start || (value != 0 && value != 1)){
        errno = EINVAL;
        return -1;
    }

    Proxy tmp = {
            .shift_ = start % BITINELEM,
            .ptr_ = obj -> data_ + start / BITINELEM
    };

    if (tmp.shift_ != 0){
        __uint64_t current = *tmp.ptr_;
        if (!value) current = ~current;

        current &= (__uint64_t) -1 >> tmp.shift_;
        if (end - start < BITINELEM - 1)
            current &= (__uint64_t) -1 << (BITINELEM - end % BITINELEM - 1);

        if (!current){
            start += BITINELEM - tmp.shift_;
            tmp.shift_ = BITINELEM - 1;
            tmp.ptr_++;
            if (start > end)
                return -1;
        }
        else {
            return start - tmp.shift_ + __builtin_clzll(current);
        }
    }

    if (value)
        while (!(*tmp.ptr_ & ~((__uint64_t) 0)) && end - start >= BITINELEM){
            tmp.ptr_++;
            start += BITINELEM;
        }
    else
        while (!(~(*tmp.ptr_) & ~((__uint64_t) 0)) && end - start >= BITINELEM){
            tmp.ptr_++;
            start += BITINELEM;
        }

    __uint64_t current = *tmp.ptr_;
    if (!value) current = ~current;

    if (end - start < BITINELEM - 1)
        current &= (__uint64_t) -1 << (BITINELEM - 1 - end % BITINELEM);

    if (!current) return -1;
    else return start + __builtin_clzll(current);
}