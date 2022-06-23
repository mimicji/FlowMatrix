#include "FMRule.hpp"
#include "ProtoRuleDB.hpp"
#include "trace.hpp"
#include "VState.hpp"
#include "DBCore.hpp"
#include "SyscallHook.hpp"

using namespace FlowMatrix;

enum FMGranularity granu = FMGranularity::byte;

#define DEFAULT_REG_OFFSET_RAX 0
#define DEFAULT_REG_OFFSET_RBX 8
#define DEFAULT_REG_OFFSET_RCX 16
#define DEFAULT_REG_OFFSET_RDX 24
#define DEFAULT_REG_OFFSET_RDI 32
#define DEFAULT_REG_OFFSET_RSI 40
#define DEFAULT_REG_OFFSET_R8 64
#define DEFAULT_REG_OFFSET_R9 72


#define USE_TRANSACTION true

const int SYSCALL_AGRS_OFFSET[] = 
    {DEFAULT_REG_OFFSET_RDI, DEFAULT_REG_OFFSET_RSI, DEFAULT_REG_OFFSET_RDX,
     DEFAULT_REG_OFFSET_RCX, DEFAULT_REG_OFFSET_R8, DEFAULT_REG_OFFSET_R9
    };

void setRegSyscallDataFlow(const int regBase, const enum FMGranularity granu,
    const int syscall_idx, int *row, int *col, int &current_nnz)
{
    for (uint64_t i=0; i<GetSizeWithinGranu(64, granu); i++)
    {
        // Self to self
        row[current_nnz] = regBase+i;
        col[current_nnz] = regBase+i;
        current_nnz++;

        // Self to syscall
        row[current_nnz] = regBase+i;
        col[current_nnz] = syscall_idx;
        current_nnz++;
    }
}

void setReverseRegSyscallDataFlow(const int regBase, const enum FMGranularity granu,
    const int syscall_idx, int *row, int *col, int &current_nnz)
{
    for (uint64_t i=0; i<GetSizeWithinGranu(64, granu); i++)
    {
        row[current_nnz] = syscall_idx;
        col[current_nnz] = regBase+i;
        current_nnz++;
    }
}

void printUsage(char** argv)
{
    std::cout << "Usage:" << argv[0] << " <TraceName> <TracePath> <DBPath> [BitLevel]" << std::endl;
}

int main(int argc, char** argv)
{
    std::string trace_path, trace_name, db_path;

    if (argc == 4)
    {
        trace_name = std::string(argv[1]);
        trace_path = std::string(argv[2]);
        db_path = std::string(argv[3]);
    }
    else if (argc == 5)
    {
        trace_name = std::string(argv[1]);
        trace_path = std::string(argv[2]);
        db_path = std::string(argv[3]);
        granu = FMGranularity::bit;
    }
    else
    {
        printUsage(argv);
        exit(0);
    }

    // Load FM trace
    trace_handle_t *trace_handle = Trace::LoadTrace(trace_path);

    // Init vstate with default settings
    VState vstate = VState(granu);
    vstate.init_reg_state(FMRegListConfig_default);

    if (vstate.granu == FMGranularity::bit) trace_name += "_bit";

    // Open database
    DBCore db(db_path, trace_name);

    // Store syscall
    std::vector<syscall_t> syscall_list;
    syscall_t this_syscall;
    LOG("[I] Store syscalls...");
    while (trace_handle->syscall_trace.ReadNext(this_syscall))
    {
        syscall_list.push_back(this_syscall);
        vstate.append(syscall_list.size() + 0x1000, 1);
        db.StoreSyscall(this_syscall);
    };

    // Extend vstate with mem accesses in FM trace
    mem_ref_t mem_ref;
    LOG("[I] Prepare vstates...");
    while (trace_handle->mem_ref_set.ReadNext(mem_ref))
    {
        vstate.append(mem_ref.mem_ref_addr, 8);
    }

    // Store VState
    LOG("[I] Store vstates...");
    db.StoreVState(vstate);
    
    // Close FM trace
    Trace::CloseTrace(trace_handle);

    // Open Rule DB
    ProtoRuleDB ruleDB = ProtoRuleDB("/opt/flowing", "AMD64");

    // Reopen the trace
    trace_handle = Trace::LoadTrace(trace_path);
    instr_t instr;
    acorn_obj::TaintRule rule;
    if (USE_TRANSACTION) db.StartTransaction();
    LOG("[I] Parsing rules...");
    int syscall_idx = 1;
    while (trace_handle->instr_trace.ReadNext(instr))
    {
        if (instr.instr_id % 1000 == 0)
        {
            LOG("[I] Parsing rules %u ...", instr.instr_id);
            if (USE_TRANSACTION) db.EndTransaction();
            if (USE_TRANSACTION) db.StartTransaction();
        }
        std::string rawbyte_str = uint8_to_hex_string(instr.raw_bytes, instr.instr_len);
        std::string asm_str;
        disassemble_one_instr_x86(instr.raw_bytes, instr.instr_len, asm_str);
        int num_valid_indices;
        int *valid_indices = NULL;
        
        // Syscall
        if (syscall_list.size()>0 && syscall_list[syscall_idx].instr_id == instr.instr_id)
        {
            syscall_t &this_syscall = syscall_list[syscall_idx];
            auto it = vstate.id2offset_map.find(syscall_idx+0x1000);
            if(it==vstate.id2offset_map.end())
            {
                LOG("Syscall not in map %d: id=%d @ %d", syscall_idx, this_syscall.syscall_id, this_syscall.instr_id);
                DIE();
            }
            int syscall_offset = it->second;

            // LOG("Syscalls have been parsing to %d: id=%d @ %d with %d", syscall_idx, this_syscall.syscall_id, this_syscall.instr_id, syscall_offset);
            syscall_idx++;
            uint64_t buffer_base = -1;
            uint64_t buffer_size = -1;
            bool isSource = false;
            if (get_syscall_buffer(this_syscall, buffer_base, buffer_size, isSource))
            {
                if (buffer_size != 0 && buffer_base != 0 && 
                    buffer_size != (uint64_t) -1 && buffer_base != (uint64_t) -1)
                {
                    auto it_base = vstate.id2offset_map.lower_bound(buffer_base);
                    const uint64_t buffer_end = buffer_base+buffer_size-1;
                    auto it_end = vstate.id2offset_map.upper_bound(buffer_end);
                    LOG("Buffer (%p, %p)", (void *)buffer_base, (void *)buffer_end);
                    assert(it_base->first <= buffer_base+buffer_size-1);
                    assert(it_end->first >= buffer_base);
                    const uint64_t buffer_base_offset = it_base->second;
                    const uint64_t buffer_end_offset = it_end->second -1;
                    const uint64_t realBufferSize = buffer_end_offset - buffer_base_offset + 1;
                    assert(buffer_base_offset <= buffer_end_offset);
                    assert(realBufferSize <= buffer_size);

                    int *row, *col;
                    float *value;
                    int nnz = 1 +
                            GetSizeWithinGranu(realBufferSize*8, vstate.granu) +
                            GetSizeWithinGranu(64, vstate.granu)*(this_syscall.nargs*2+1);
                            //GetSizeWithinGranu(64, vstate.granu)*(this_syscall.nargs*2);

                    if (!isSource) nnz+= GetSizeWithinGranu(realBufferSize*8, vstate.granu);
                    num_valid_indices = 
                            1 +
                            GetSizeWithinGranu(realBufferSize*8, vstate.granu) +
                            GetSizeWithinGranu(64, vstate.granu)*(this_syscall.nargs+1);;
                            //GetSizeWithinGranu(64, vstate.granu)*(this_syscall.nargs);;
                    valid_indices = new int[num_valid_indices];
                    row = new int[nnz];
                    col = new int[nnz];
                    value = new float[nnz];
                    std::fill_n(value, nnz, 1.0f);

                    int current_nnz = 0;

                    if (this_syscall.nargs >= 4) setRegSyscallDataFlow(DEFAULT_REG_OFFSET_RCX, granu, syscall_offset, row, col, current_nnz);
                    if (this_syscall.nargs >= 3) setRegSyscallDataFlow(DEFAULT_REG_OFFSET_RDX, granu, syscall_offset, row, col, current_nnz);
                    if (this_syscall.nargs >= 1) setRegSyscallDataFlow(DEFAULT_REG_OFFSET_RDI, granu, syscall_offset, row, col, current_nnz);
                    if (this_syscall.nargs >= 2) setRegSyscallDataFlow(DEFAULT_REG_OFFSET_RSI, granu, syscall_offset, row, col, current_nnz);
                    if (this_syscall.nargs >= 5) setRegSyscallDataFlow(DEFAULT_REG_OFFSET_R8, granu, syscall_offset, row, col, current_nnz);
                    if (this_syscall.nargs >= 6) setRegSyscallDataFlow(DEFAULT_REG_OFFSET_R9, granu, syscall_offset, row, col, current_nnz);
                    setReverseRegSyscallDataFlow(DEFAULT_REG_OFFSET_RAX, granu, syscall_offset, row, col, current_nnz);
                    //assert((uint64_t)current_nnz == GetSizeWithinGranu(64, vstate.granu)*(this_syscall.nargs*2));
                    assert((uint64_t)current_nnz == GetSizeWithinGranu(64, vstate.granu)*(this_syscall.nargs*2+1));

                    row[current_nnz] = syscall_offset;
                    col[current_nnz] = syscall_offset;
                    current_nnz++;

                    if (isSource)
                    {
                        // Source syscall
                        // LOG("Source %p,%p with size %lu", (void*)buffer_base, (void*)(buffer_base+buffer_size-1), buffer_size);
                        for (uint64_t i=0; i<GetSizeWithinGranu(realBufferSize*8, granu); i++)
                        {
                            row[current_nnz] = syscall_offset;
                            col[current_nnz] = buffer_base_offset+i;
                            current_nnz++;
                            // printf("[%d] %d, %d\n", current_nnz, syscall_offset, buffer_base_offset+i);
                        }
                    }
                    else
                    {
                        // Sink syscall
                        for (uint64_t i=0; i<GetSizeWithinGranu(realBufferSize*8, granu); i++)
                        {
                            row[current_nnz] = buffer_base_offset+i;
                            col[current_nnz] = syscall_offset;
                            current_nnz++;
                            row[current_nnz] = buffer_base_offset+i;
                            col[current_nnz] = buffer_base_offset+i;
                            current_nnz++;
                        }
                    }
                    if(current_nnz != nnz)
                    {
                        LOG("%d: nnz %d expected but %d",syscall_idx-1, nnz, current_nnz);
                        DIE();
                    }

                    std::set<int> valid_indices_set;
                    valid_indices_set.insert(row, row+nnz);
                    valid_indices_set.insert(col, col+nnz);
                    if(valid_indices_set.size() != (size_t)num_valid_indices)
                    {
                        // LOG("%d: num_valid_indices %d expected but %d",syscall_idx-1, num_valid_indices, valid_indices_set.size());
                        // auto it_b = valid_indices_set.lower_bound(buffer_base_offset);
                        // auto it_u = valid_indices_set.upper_bound(buffer_end_offset);
                        // LOG("Erase from %lu to %lu", *it_b, *it_u);
                        // valid_indices_set.erase(it_b, it_u);
                        // LOG("After erased: %lu", valid_indices_set.size());
                        // for (auto & this_value : valid_indices_set)
                        // {
                        //     printf("%d\n", this_value);
                        // }
                        DIE();
                    }

                    uint64_t idx_cnt=0;
                    for (auto &idx : valid_indices_set)
                    {
                        valid_indices[idx_cnt] = idx;
                        idx_cnt++;
                    }
                    assert(idx_cnt == valid_indices_set.size());

                    // printf("Valid Indices[%d]\n", num_valid_indices);
                    // for (int i=0; i<num_valid_indices; i++)
                    // {
                    //     printf("%d ",valid_indices[i]);
                    //     if (i%8==7) printf("\n");
                    // }
                    // printf("\n");

                    db.Store(instr.instr_id, instr.instr_id, vstate.size, nnz, row, col, value, num_valid_indices, valid_indices, rawbyte_str, asm_str, USE_TRANSACTION);
                    delete [] valid_indices;  
                    continue;
                }
            }
        }

        // Not a syscall
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

        // // For debug, we don't need that many rules
        // if (instr.instr_id > 100) break;
    };

    if (USE_TRANSACTION) db.EndTransaction();

    Trace::CloseTrace(trace_handle);
}