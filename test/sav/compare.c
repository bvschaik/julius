#include "sav_compare.h"

#include <stdio.h>

int main(int argc, char **argv)
{
    if (argc != 3) {
        printf("Usage: %s FILE1 FILE2\n", argv[0]);
        return 1;
    }
    return compare_files(argv[1], argv[2]);
}
