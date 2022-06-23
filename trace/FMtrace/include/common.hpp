#ifndef __FLOWMATRIX_COMMON_H_
#define __FLOWMATRIX_COMMON_H_

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

#ifndef MAX
#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#endif

#ifndef MIN
#define MIN(x, y) (((x) < (y)) ? (x) : (y))
#endif

#ifndef PBSTR
#define PBSTR "||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||"
#define PBWIDTH 60
#endif 

#ifndef LOG
#define LOG(...) {printf("%s", currentDateTime().c_str()); printf(__VA_ARGS__); printf("\n");}
#endif
#ifndef DIE
#define DIE() {printf("Abort at %s:%d\n", __FILE__, __LINE__); exit(1);}
#endif

#ifndef isFloatEqual
#define isFloatEqual(a, b) (((a) < (b+0.005)) && ((a) > (b-0.005)))
#endif

const std::string currentDateTime();

void printProgress(double percentage);

std::string uint8_to_hex_string(const uint8_t *uint8_arrary, const uint64_t size);
int disassemble_one_instr_x86(uint8_t *raw_bytes, uint32_t len, std::string &result_asm, size_t addr = 0x0);


namespace FlowMatrix{

typedef struct sysent {
	unsigned int nargs;
	const char *sys_name;
} struct_syscall_info;

};

#endif