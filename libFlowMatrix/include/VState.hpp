#ifndef __LIBFLOWMATRIX_VSTATE_H__
#define __LIBFLOWMATRIX_VSTATE_H__

#include <FMGranularity.hpp>
#include <isa.h>
#include <FMCommon.hpp>
#include <map>

#define INSERT_TO_VSTATE_BY_REG_NAME(vstate, regname) {         \
            unsigned int reg_id, reg_size;                      \
            assert(getRegId("AMD64", reg_name, reg_id) == 0);   \
            assert(getRegSize("AMD64", reg_id, reg_size) == 0); \
            vstate->append(reg_id, reg_size);                     \
        }                                                       

namespace FlowMatrix
{

typedef struct _FMRegListConfig
{
    unsigned char gpr;  // General Purpose Registers
    unsigned char fp;   // Floating Point Registers 
    unsigned char st;   // ST registers
    unsigned char dr;   // Debug Registers  /* KH: We don't deal with synonyms */
    unsigned char cr;   // Control Registers
    unsigned char kr;   // k0-k7, Opmask registers
    unsigned char xmm;  // XMM registers
    unsigned char ymm;  // YMM registers, setting as true overrides xmm
    unsigned char zmm;  // ZMM registers, setting as true overrides xmm
} FMRegListConfig;

const FMRegListConfig FMRegListConfig_default =
{
    .gpr = 1,                 
    .fp  = 1,                  
    .st  = 0,                 
    .dr  = 0,                  
    .cr  = 0,                 
    .kr  = 0,                  
    .xmm = 0,                  
    .ymm = 1,                  
    .zmm = 0,              
};

class VState
{
    public:
        enum FMGranularity granu;
        uint64_t size;
        std::map<uint64_t, uint64_t> id2offset_map, offset2id_map;

        VState(const enum FMGranularity &granu);
        void init_reg_state(const FMRegListConfig &reg_config);
        void append(uint64_t id, uint64_t size_in_bit);
        void pp();
        static std::string GetVStateNameByID(uint64_t id);

        VState &merge(VState &another_state, uint64_t *row, uint64_t *col);
        // bool store(const std::string &dir, const std::string &prefix="");
};

} // namespace FlowMatrix

#endif // __LIBFLOWMATRIX_VSTATE_H__
