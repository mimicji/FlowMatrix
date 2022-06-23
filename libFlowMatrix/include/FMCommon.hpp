#ifndef __LIBFLOWMATRIX_FMCOMMON_H_
#define __LIBFLOWMATRIX_FMCOMMON_H_

#include <unistd.h>
#include <stdint.h>
#include <assert.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <cstring>
#include <iomanip>
#include <sstream>
#include <string>
#include <fstream>
#include <iostream>
#include <capstone/capstone.h>
#include <algorithm>

#define PBSTR "||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||"
#define PBWIDTH 60

#define LOG(...) {printf("%s", currentDateTime().c_str()); printf(__VA_ARGS__); printf("\n");}
#define DIE() {printf("Abort at %s:%d\n", __FILE__, __LINE__); exit(1);}

const std::string currentDateTime();



#endif // __LIBFLOWMATRIX_FMCOMMON_H_