#ifndef __LIBFLOWMATRIX_FMGRANULARITY_H__
#define __LIBFLOWMATRIX_FMGRANULARITY_H__

#include <stdint.h>

// #define GetSizeWithinGranu(size, granularity) (size / (1 << ((unsigned long) granularity)*3))
#define PrintGranularity(granularity) {std::cout << "Granularity: "; if (granularity==0) std::cout << "Bit\n"; if (granularity==1) std::cout << "Byte\n"; if (granularity==2) std::cout << "Qword\n";}
namespace FlowMatrix
{

enum FMGranularity : unsigned char
{
    bit=0, 
    byte=1, 
    qword=2,    /*Currently not supported*/
};

inline uint64_t GetSizeWithinGranu(uint64_t sizeInBit, enum FMGranularity granularity)
{
    uint64_t RealSize = sizeInBit / (1 << ((unsigned long) granularity)*3);
    if (RealSize==0 && sizeInBit >0)
        return 1;
    else
        return RealSize;
}

} // namespace FlowMatrix

#endif // __LIBFLOWMATRIX_FMGRANULARITY_H__