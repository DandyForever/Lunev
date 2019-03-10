#include <stdlib.h>

void * badCalloc (size_t nmemb, size_t size){
    if (!(rand () % 100))
        return NULL;

    else return calloc(nmemb, size);
}