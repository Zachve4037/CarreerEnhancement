#include "file_stats.h"

#include <iostream>
#define STRSIZE 256

int global_array[100] = {-1};
int main(int argc, char* argv[])
{
    return global_array[100 + 100];
}