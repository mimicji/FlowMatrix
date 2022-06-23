#include "FMRule.hpp"
#include "ProtoRuleDB.hpp"
#include "trace.hpp"
#include "VState.hpp"
#include "DBCore.hpp"

using namespace FlowMatrix;

#define USE_TRANSACTION false



const char *rawbytes_list[] = {
    "\x48\x31\xd2", // xor rdx, rdx
    "\x48\x89\xd3", // mov rbx, rdx
    "\x48\x89\xd9", // mov rcx, rbx
};

typedef struct
{
    uint64_t memAddr;
    int sizeInByte;
} sample_memref_t;

const sample_memref_t memrefs[] = {
    (sample_memref_t){.memAddr = 0, .sizeInByte = 0},
    (sample_memref_t){.memAddr = 0, .sizeInByte = 0},
    (sample_memref_t){.memAddr = 0, .sizeInByte = 0},
};

instr_t* forgeInstr(const char **list_of_rawbytes, const sample_memref_t *list_of_memrefs, int instr_num)
{
    instr_t* instr_list = new instr_t[instr_num];
    for (int i=0; i<instr_num; i++)
    {
        instr_t &this_instr = instr_list[i];
        this_instr.instr_id = i+1;
        this_instr.instr_len = strlen(list_of_rawbytes[i]);
        this_instr.rflags = 0; // Don't care
        this_instr.mem_ref_num = (list_of_memrefs[i].sizeInByte!=0) ? 1:0;
        this_instr.mem_ref_addr[0] = list_of_memrefs[i].memAddr;
        assert(this_instr.instr_len < 16);
        memcpy(this_instr.raw_bytes, rawbytes_list[i], this_instr.instr_len);
    }
    return instr_list;
}

int main(int argc, char** argv)
{   
    if (argc != 3)
    {
        printf("Usage: %s <TraceName> <PathToDB>\n", argv[0]);
        exit(0);
    }
    std::string trace_name(argv[1]);
    std::string dbPath(argv[2]);

    int num_instr = sizeof(memrefs)/sizeof(sample_memref_t);
    assert(num_instr == sizeof(rawbytes_list)/sizeof(uint8_t *));
    instr_t *instr_list = forgeInstr(rawbytes_list, memrefs, num_instr);
    

    // Init vstate with default settings
    VState vstate = VState(FMGranularity::byte);
    vstate.init_reg_state(FMRegListConfig_default);
    if (vstate.granu == FMGranularity::bit) 
        trace_name += "_bit";

    // Init database
    DBCore db(dbPath, trace_name);

    // Extend vstate with mem accesses in FM trace
    for (int i=0; i<num_instr; i++)
    {
        if (instr_list[i].mem_ref_num != 0)
            vstate.append(instr_list[i].mem_ref_addr[0], 8*instr_list[i].mem_ref_num);
    }
        
    // Store VState
    db.StoreVState(vstate);

    // Open Rule DB
    ProtoRuleDB ruleDB = ProtoRuleDB("/opt/flowing", "AMD64");

    // Make rule!
    for (int i=0; i<num_instr; i++)
    {
        instr_t &instr = instr_list[i];
        acorn_obj::TaintRule rule;

        std::string rawbyte_str = uint8_to_hex_string(instr.raw_bytes, instr.instr_len);
        std::string asm_str;
        disassemble_one_instr_x86(instr.raw_bytes, instr.instr_len, asm_str);
        int num_valid_indices = 0;
        int *valid_indices = NULL;
        if(ruleDB.loadRule(rawbyte_str, rule) == 0)
        {
            // printf("Loaded from proto %s\n", raw_byte_str.c_str());
            FMRule fmrule = FMRule(rule, instr, vstate, true, num_valid_indices, valid_indices);
            if (fmrule.nnz > 0)
            {
                int matrix_size, nnz;
                int* row, * col;
                float* value;
                fmrule.coo(matrix_size, nnz, row, col, value);
                db.Store(instr.instr_id, instr.instr_id, matrix_size, nnz, row, col, value, num_valid_indices, valid_indices, rawbyte_str, asm_str, USE_TRANSACTION);
            }
            else
            {
                if (num_valid_indices > 0)
                    db.Store(instr.instr_id, instr.instr_id, vstate.size, 0, NULL, NULL, NULL, num_valid_indices, valid_indices, rawbyte_str, asm_str, USE_TRANSACTION);
                else
                    db.Store(instr.instr_id, instr.instr_id, vstate.size, 0, NULL, NULL, NULL, 0, NULL, rawbyte_str, asm_str, USE_TRANSACTION);
            }
        }
        else
        {
            db.Store(instr.instr_id, instr.instr_id, vstate.size, 0, NULL, NULL, NULL, 0, NULL, rawbyte_str, asm_str, USE_TRANSACTION);
        }

        if (valid_indices != NULL) delete [] valid_indices;

    }

    delete [] instr_list;
}