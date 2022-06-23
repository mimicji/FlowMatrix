#include <VState.hpp>

namespace
{

const std::string gpr_strs[] =
{
    "RAX",
    "RBX",
    "RCX",
    "RDX",
    "RDI",
    "RSI",
    "RSP",
    "RBP",
    "R8",
    "R9",
    "R10",
    "R11",
    "R12",
    "R13",
    "R14",
    "R15",
    "RFLAGS",
    "RIP",
};

const std::string fp_strs[] =
{
    "FP0",
    "FP1",
    "FP2",
    "FP3",
    "FP4",
    "FP5",
    "FP6",
    "FP7",
};

const std::string st_strs[] =
{
    // KH: Not implemented
};

const std::string dr_strs[] =
{
    // KH: Not implemented
};

const std::string cr_strs[] =
{
    // KH: Not implemented
};

const std::string kr_strs[] =
{
    // KH: Not implemented
};

const std::string xmm_strs[] =
{
    "XMM0",
    "XMM1",
    "XMM2",
    "XMM3",
    "XMM4",
    "XMM5",
    "XMM6",
    "XMM7",
    "XMM8",
    "XMM9",
    "XMM10",
    "XMM11",
    "XMM12",
    "XMM13",
    "XMM14",
    "XMM15",
};

const std::string ymm_strs[] =
{
    "YMM0",
    "YMM1",
    "YMM2",
    "YMM3",
    "YMM4",
    "YMM5",
    "YMM6",
    "YMM7",
    "YMM8",
    "YMM9",
    "YMM10",
    "YMM11",
    "YMM12",
    "YMM13",
    "YMM14",
    "YMM15",
    "YMM16",
    "YMM17",
    "YMM18",
    "YMM19",
    "YMM20",
    "YMM21",
    "YMM22",
    "YMM23",
    "YMM24",
    "YMM25",
    "YMM26",
    "YMM27",
    "YMM28",
    "YMM29",
    "YMM30",
    "YMM31",
};

const std::string zmm_strs[] =
{
    "ZMM0",
    "ZMM1",
    "ZMM2",
    "ZMM3",
    "ZMM4",
    "ZMM5",
    "ZMM6",
    "ZMM7",
    "ZMM8",
    "ZMM9",
    "ZMM10",
    "ZMM11",
    "ZMM12",
    "ZMM13",
    "ZMM14",
    "ZMM15",
    "ZMM16",
    "ZMM17",
    "ZMM18",
    "ZMM19",
    "ZMM20",
    "ZMM21",
    "ZMM22",
    "ZMM23",
    "ZMM24",
    "ZMM25",
    "ZMM26",
    "ZMM27",
    "ZMM28",
    "ZMM29",
    "ZMM30",
    "ZMM31",
};

} // anomynos namespace

namespace FlowMatrix
{

VState::VState(const enum FMGranularity &granu)
{
    this->granu = granu;
    this->size = 0;   
}

void VState::init_reg_state(const FMRegListConfig &reg_config)
{
    if (reg_config.gpr == 1)
    {
        for (std::string reg_name:gpr_strs)
            INSERT_TO_VSTATE_BY_REG_NAME(this, reg_name);
    }

    if (reg_config.fp == 1)
    {
        for (std::string reg_name:fp_strs)
            INSERT_TO_VSTATE_BY_REG_NAME(this, reg_name);
    }

    // Not implemented yet
    if (reg_config.st == 1)
    {
        for (std::string reg_name:st_strs)
            INSERT_TO_VSTATE_BY_REG_NAME(this, reg_name);
    }    

    // Not implemented yet
    if (reg_config.dr == 1)
    {
        for (std::string reg_name:dr_strs)
            INSERT_TO_VSTATE_BY_REG_NAME(this, reg_name);
    }  

    // Not implemented yet
    if (reg_config.cr == 1)
    {
        for (std::string reg_name:cr_strs)
            INSERT_TO_VSTATE_BY_REG_NAME(this, reg_name);
    }    

    // Not implemented yet
    if (reg_config.kr == 1)
    {
        for (std::string reg_name:kr_strs)
            INSERT_TO_VSTATE_BY_REG_NAME(this, reg_name);  
    }  

    // Not fully implemented yet
    if (reg_config.zmm == 1)
    {
        for (std::string reg_name:zmm_strs)
        {
            unsigned int reg_id;  
            assert(getRegId("AMD64", reg_name, reg_id) == 0);   
            this->append(reg_id, 512);                                                 
        }    
    }
    else
    {
        // ~zmm
        if (reg_config.ymm == 1)
        {
            for (std::string reg_name:ymm_strs)
            {
                unsigned int reg_id;  
                assert(getRegId("AMD64", reg_name, reg_id) == 0);   
                this->append(reg_id, 256);                                                 
            }
                
        }
        else
        {
            // ~zmm && ~ymm
            if (reg_config.xmm == 1)
            {
                for (std::string reg_name:zmm_strs)
                {
                    unsigned int reg_id;  
                    assert(getRegId("AMD64", reg_name, reg_id) == 0);   
                    this->append(reg_id, 128);  
                }
            }
        }
    }

}

void VState::append(uint64_t id, uint64_t size_in_bit)
{
    this->id2offset_map[id] = this->size;
    this->offset2id_map[size] = id;
    this->size += std::max((uint64_t)1, GetSizeWithinGranu(size_in_bit,this->granu));
}

std::string VState::GetVStateNameByID(uint64_t id)
{
    std::string name = "";
    if (getRegName("AMD64", id, name) != -1)
    {
        // This is a regesiter.
        return name;
    }
    else
    {
        // This is a memory or syscall
        std::stringstream ss;
        if (id >= 0x1000 && id <= 0x4000)
        {
            ss << "Syscall_" << id-0x1000;
        }
        else
        {
            ss << "0x" << std::hex << id;
        }
        ss >> name;
        return name;
    }

    // Should not reach here.
    LOG("[E] Should never reach here.");
    DIE();

    // Make Compiler happy
    return name;
}

void VState::pp()
{
    // Print Granularity
    std::cout << "Granularity: ";
    switch(this->granu) 
    {
        case FMGranularity::bit:
            std::cout << "Bit-level\n";
            break;
        case FMGranularity::byte:
            std::cout << "Byte-level\n";
            break;
        case FMGranularity::qword:
            std::cout << "Qword-level\n";
            break;
        default:
            // Unknown granularity
            std::cout << "Unknown\n";
    }

    // Print Size
    std::cout << "Size: " << this->size << std::endl;

    // Prepare for printing continuous memory range
    uint64_t last_memory = -1;
    uint64_t starting_offset = 0;
    uint64_t starting_addr = -1;
    uint64_t ending_offset = 0;

    // Print Map
    std::cout << "Mapping:" << std::endl; 
    for (auto it = this->offset2id_map.begin(); it != this->offset2id_map.end(); it++)
    {
        const uint64_t offset = it->first;
        const uint64_t id = it->second; 
        auto next_it = std::next(it);
        uint64_t var_end;
        if (next_it == this->offset2id_map.end())
            var_end = this->size;
        else
            var_end = next_it->first - 1;

        // Also performs an internal cross-check
        assert(this->id2offset_map[id] == offset);

        std::string reg_name;
        if (getRegName("AMD64", id, reg_name) != -1)
        {
            // This is a register
            std::cout << "  [" << offset << "-" << var_end << "]\t";
            std::cout << reg_name << std::endl;
        }
        else
        {
            // This is a mem.
            if (last_memory + 1 == id)
            {
                // Continuous memory range
                ending_offset = var_end;
                last_memory = id;
                continue;
            }
            else
            {
                if (last_memory != (uint64_t)-1)
                {
                    std::cout << "  [" << starting_offset << "-" << ending_offset << "]\t";
                    std::cout << "0x" << std::hex << starting_addr << " - 0x" <<last_memory<< std::dec;
                    std::cout << " (" << (ending_offset - starting_offset + 1) * (1 << ((unsigned long) this->granu)*3) / 8
                    << " bytes)";
                    std::cout << std::endl;
                }
                starting_addr = id;
                last_memory = id;
                starting_offset = offset;
                ending_offset = var_end;
            }
            
        }
    }
}

// bool VState::store(const std::string &dir, const std::string &prefix)
// {
//     if 
//     std::string filename = dir + "/"
//     fopen(filename.c_str(), "w");
// }



} // namespace FlowMatrix