#include "NuSMV-2.6.0/NuSMV/code/nusmv/core/dd/dd.h"
#include "NuSMV-2.6.0/NuSMV/code/nusmv/core/dd/DDMgr.h"
#include "NuSMV-2.6.0/NuSMV/code/nusmv/core/utils/array.h"

#include <stdio.h>



int computeAndWritePrimes(DDMgr_ptr dd, bdd_ptr b, const char* pathToTheFile){
    array_t * primes = bdd_compute_primes(dd, b);

    FILE* file = fopen(pathToTheFile, "w");

    if(file != NULL){
        fprintf(file, primes->space);

        fclose(file);
    }else{
        printf("We failed to open the specified file");
        return 1;
    }

    return 0;
}

void hello(void) {
    printf("Hello, World!\n");
}