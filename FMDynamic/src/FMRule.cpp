#include "FMRule.hpp"

namespace {
    /* Requirements: 
     *   COO matrix is sorted in (i,j) order.
     * In this way, coo_col is the same with csr_col,
     * as well as the value array. 
     * This function will malloc a new array for csr_row.
     * It is caller's duty to free it.
     */ 
    void coo2csr(const int *coo_row, const int nnz, const int row_size, int *&csr_row)
    {
        const int row_ptr_size = row_size + 1;
        csr_row = new int[row_ptr_size];
        memset(csr_row, 0, sizeof(int)*row_ptr_size);
        for (int i = 0; i < nnz; i++)
            csr_row[coo_row[i] + 1]++;
        for (int i = 0; i < row_size; i++)
            csr_row[i + 1] += csr_row[i];
    }

    void csr2coo(const int *csr_row, const int nnz, const int row_size, int *&coo_row)
    {
        coo_row = new int[nnz];
        memset(coo_row, 0, sizeof(int)*nnz);
        int row_index = 0;
        for (int i = 0; i < nnz; i++)
        {
            while(csr_row[row_index+1]<=i) row_index++;
            coo_row[i] = row_index;
        }
        assert(row_index+1 == row_size);
        assert(csr_row[row_index+1] == nnz);
    }
    // bool doesFileExist (const std::string& name) {
    //     struct stat buffer;   
    //     return (stat (name.c_str(), &buffer) == 0); 
    // }
}

namespace FlowMatrix{

FMRule::FMRule(const instr_t &instr_info, VState &vstate)
{
    this->matrix_size = vstate.size;
    this->nnz = 0;
}

FMRule::FMRule(const acorn_obj::TaintRule &rule, 
                const instr_t &instr_info,
                VState &vstate,
                bool isCoo,
                int &num_valid_indices,
                int *&valid_indices /* = NULL */)
{
    // Set Metadata
    this->isCoo = isCoo;
    this->granu = vstate.granu;
    this->matrix_size = vstate.size;
    memcpy(&this->instr_info, &instr_info, sizeof(instr_t));

    // Prepare state mapping
    std::map<uint64_t, uint64_t> local2global_map;
    uint64_t current_offset = 0;

    // Register state
    for (auto reg_itr : rule.state_format().reg_list())
    {
        uint32_t ori_reg_size, reg_size, enclosing_reg_id;
        
        // Find register Id first
        if ((getEncloseReg("AMD64", reg_itr, enclosing_reg_id)!=0) ||
            (getRegSize("AMD64", reg_itr, ori_reg_size)!=0) ||
            (getRegSize("AMD64", enclosing_reg_id, reg_size)!=0))
        {
            // Cannot file enclosing register. Rule does not consist with ISA?
            printf("[E]: Cannot find enclosing register for %d.\n", reg_itr);
            DIE();
        }

        // Find the register in vstate
        auto it = vstate.id2offset_map.find(enclosing_reg_id);
        if (it == vstate.id2offset_map.end())
        {
            // The register is not in the vstate
            printf("[E]: Cannot find register %d in the vstate!\n", enclosing_reg_id);
            DIE();                
        }
        
        // Update the mapping info
        for (uint64_t i=0; i<reg_size; i++)
        {
            uint64_t i_in_granu = GetSizeWithinGranu(i, this->granu);
            uint64_t global_offset = it->second + i_in_granu;
            local2global_map[current_offset + i] = global_offset;
            overrided_set.insert(global_offset);
        }

        current_offset += ori_reg_size;
    }

    // Memory state
    uint64_t mem_ref_idx = 0;
    uint64_t mem_ref_addr = 0;
    bool isSucc = true;
    for (auto mem_itr : rule.state_format().mem_list())
    {
        uint64_t mem_size = getMemSize(mem_itr) * 8;
        if (!isMemValue(mem_itr))
        {
            // We don't propagate taint for mem addr
            current_offset += mem_size;
            continue;
        }

        if (mem_ref_idx < this->instr_info.mem_ref_num)
        {
            mem_ref_addr = this->instr_info.mem_ref_addr[mem_ref_idx];
        } 
        else 
        {
            // Check if any mem ref given
            if (this->instr_info.mem_ref_num == 0)
            {
                isSucc = false;
                this->nnz = 0;
                num_valid_indices = 0;
                LOG("[W]: No mem ref found! Give up!");
                this->instr_info.Print();
                break;
            }
            else
            {   
                // We expect a memory here, but not found.
                isSucc = false;
                this->nnz = 0;
                num_valid_indices = 0;
                // LOG("[W] A memory size of %lu is expected! Give up!", mem_size);
                // this->instr_info.Print();
                break;
            } 
        }
        if (!isSucc) return;

        // Find the address in vstate
        auto it = vstate.id2offset_map.find(mem_ref_addr);
        if (it == vstate.id2offset_map.end())
        {
            // The address is not in the vstate
            LOG("[E]: Cannot find address 0x%lx in the vstate!", mem_ref_addr);
            DIE();                
        }

        // Update the mapping info
        for (uint64_t i=0; i<mem_size; i++)
        {
            uint64_t i_in_granu = GetSizeWithinGranu(i, this->granu);
            uint64_t global_offset = it->second + i_in_granu;
            local2global_map[current_offset + i] = global_offset;
            overrided_set.insert(global_offset);
        }

        mem_ref_idx++;
        current_offset += mem_size;
    }

    // Make matrix!
    std::map<coo_pair_t, float> coo_pairs;
    this->nnz = 0;
    for (auto use_def : rule.use_def())
    {
        auto use_it = local2global_map.find(use_def.use());
        if (use_it == local2global_map.end())
        {
            // This is a memory address slot.
            continue;
        }
        for (auto dst_offset : use_def.defs())
        {
            auto def_it = local2global_map.find(dst_offset);
            if (def_it == local2global_map.end())
            {
                // This is a memory address slot.
                continue;
            }
            coo_pair_t this_pair;
            this_pair.row = use_it->second;
            this_pair.col = def_it->second;
            // pair.value = 1.0;
            coo_pairs.insert(std::pair<coo_pair_t, float>(this_pair, 1.0));
        }
    }

    // Get valid_indices
    {
        num_valid_indices = 0;
        valid_indices = new int[overrided_set.size()];
        for (auto i: overrided_set)
        {
            valid_indices[num_valid_indices] = i;
            num_valid_indices++;
        }
        assert((size_t)num_valid_indices == overrided_set.size());
    }
    // // main diagonal -= 1
    // for (uint64_t i=0; i<vstate.size; i++)
    // {
    //     if (overrided_set.find(i)!=overrided_set.end())
    //     {
    //         coo_pair_t diagonal_pair;
    //         diagonal_pair.row = (int) i;
    //         diagonal_pair.col = (int) i;
    //         auto pair_in_map = coo_pairs.find(diagonal_pair);
    //         if (pair_in_map!=coo_pairs.end())
    //         {
    //             coo_pairs.erase(pair_in_map);
    //         }
    //         else{
    //             coo_pairs.insert(std::pair<coo_pair_t, float>(diagonal_pair, -1.0));
    //         }
    //     }
    // }

    // Make coo matrix
    this->nnz = coo_pairs.size();
    int *coo_col = new int[this->nnz];
    int *coo_row = new int[this->nnz];
    float *coo_value = new float[this->nnz];

    int iter = 0;
    for (auto pair_it=coo_pairs.begin(); pair_it!=coo_pairs.end(); ++pair_it)
    {
        coo_col[iter] = pair_it->first.col;
        coo_row[iter] = pair_it->first.row;
        coo_value[iter] = pair_it->second;
        iter++;
    }        

    // Convert to csr matrix
    this->csr_col = coo_col;
    this->value = coo_value;
    if (!this->isCoo)
    {
        coo2csr(coo_row, this->nnz, this->matrix_size, this->csr_row);
        delete [] coo_row;
    }
    else
    {
        this->coo_row = coo_row;
    }
}

void FMRule::getSuperSet(const int num_indices_1, const int *indices_1,
        const int num_indices_2, const int *indices_2,
        int &superset_num_indices, int *&superset_indices,
        int &num_missing_indices_1, int *&missing_indices_1,
        int &num_missing_indices_2, int *&missing_indices_2)
{
    // Prerequisites: both input indices must be sorted in ascending order.

    // First, check how many entities are shared
    int idx_1 = 0;
    int idx_2 = 0;
    int num_overlapped = 0;
    while (idx_1 < num_indices_1 && idx_2 < num_indices_2)
    {
        if (indices_1[idx_1] == indices_2[idx_2])
        {
            num_overlapped++;
            idx_1++;
            idx_2++;
            continue;
        }
        else if (indices_1[idx_1] > indices_2[idx_2])
        {
            idx_2++;
            continue;
        }
        else
        {
            idx_1++;
            continue;
        }
    }

    // Based on the shared entities, calculate size of each array and malloc
    superset_num_indices = num_indices_1 + num_indices_2 - num_overlapped;
    num_missing_indices_1 = superset_num_indices - num_indices_1;
    num_missing_indices_2 = superset_num_indices - num_indices_2;
    superset_indices = new int[superset_num_indices];
    missing_indices_1 = new int[num_missing_indices_1];
    missing_indices_2 = new int[num_missing_indices_2];

    // Fill-in each array
    idx_1 = 0;
    idx_2 = 0;
    int superset_idx = 0;
    int miss_idx_1 = 0;
    int miss_idx_2 = 0;
    while (idx_1 < num_indices_1 && idx_2 < num_indices_2)
    {
        if (indices_1[idx_1] == indices_2[idx_2])
        {
            superset_indices[superset_idx] = indices_1[idx_1];
            superset_idx++;

            idx_1++;
            idx_2++;
            continue;
        }
        else if (indices_1[idx_1] > indices_2[idx_2])
        {
            missing_indices_1[miss_idx_1] = indices_2[idx_2];
            miss_idx_1++;

            superset_indices[superset_idx] = indices_2[idx_2];
            superset_idx++;

            idx_2++;
            continue;
        }
        else
        {
            missing_indices_2[miss_idx_2] = indices_1[idx_1];
            miss_idx_2++;

            superset_indices[superset_idx] = indices_1[idx_1];
            superset_idx++;

            idx_1++;
            continue;
        }
    }
    while(idx_1 < num_indices_1)
    {
        missing_indices_2[miss_idx_2] = indices_1[idx_1];
        miss_idx_2++;
        superset_indices[superset_idx] = indices_1[idx_1];
        superset_idx++;
        idx_1++;  
    }
    while(idx_2 < num_indices_2)
    {
        missing_indices_1[miss_idx_1] = indices_2[idx_2];
        miss_idx_1++;
        superset_indices[superset_idx] = indices_2[idx_2];
        superset_idx++;
        idx_2++;
    }

    // Correctness verification
    assert(miss_idx_1 == num_missing_indices_1);
    assert(miss_idx_2 == num_missing_indices_2);
    assert(superset_idx == superset_num_indices);
}

FMRule::~FMRule()
{
    delete [] this->csr_col;
    if (this->isCoo)
    {
        delete [] this->coo_row;
    }
    else
    {
        delete [] this->csr_row;
    }
    delete [] this->value;
}

void FMRule::print(VState &vstate)
{
    this->instr_info.Print();
    std::cout << "Matrix Size: " << this->matrix_size << std::endl;
    std::cout << "NNZ: " << this->nnz << std::endl;
    PrintGranularity(this->granu);


    for (auto i: overrided_set)
    {
        auto it = vstate.offset2id_map.find(i);
        if (it !=vstate.offset2id_map.end())
        {
            uint64_t this_id = it->second;

            // Get this var range
            uint64_t next_offset = vstate.size;
            auto next_it = std::next(it);
            if (next_it != vstate.offset2id_map.end()) 
                next_offset = next_it->first;
            std::cout << "  [" << it->first << "-" << next_offset-1 << "] ";

            // Is register?
            std::string reg_name;
            if (getRegName("AMD64", this_id, reg_name) != -1)
            {
                // This is a register
                std::cout << reg_name << std::endl;
            }
            else
            {
                // Nope, this is a mem
                std::cout << "0x" << std::hex << this_id << std::dec << std::endl; 
            }
        }
    }

    int *coo_col = this->csr_col;
    if (!this->isCoo) 
    {
        csr2coo(this->csr_row, this->nnz, this->matrix_size, this->coo_row);
    }

    std::cout << "Dataflows:" << std::endl;

    for (int i=0; i<this->nnz; i++)
    {
        if (!isFloatEqual(this->value[i], -1.0))            
        {
            std::cout << "\t" << this->coo_row[i] << "->" << coo_col[i] << "\n";
        }
    }
    std::cout << "Dataflows that are not inherited:" << std::endl;
    for (int i=0; i<this->nnz; i++)
    {
        if (isFloatEqual(this->value[i], -1.0))
        {
            assert(this->coo_row[i] == coo_col[i]);
            std::cout << "\t~" << this->coo_row[i] << "\n";
        }

    }
    if (!this->isCoo) 
    {
        delete [] this->coo_row;
    }    
}

void FMRule::store4py(const std::string &dir_path) const
{
    std::string index_str = std::to_string(this->instr_info.instr_id);
    std::string row_file_name = dir_path + "/" + index_str + "_" + index_str + ".fm_row";
    std::string col_file_name = dir_path + "/" + index_str + "_" + index_str + ".fm_col";
    std::string val_file_name = dir_path + "/" + index_str + "_" + index_str + ".fm_val";
    std::string meta_file_name = dir_path + "/" + index_str + "_" + index_str + ".fm_meta";

    auto mptr = fopen(meta_file_name.c_str(),"wb");
    auto rptr = fopen(row_file_name.c_str(),"wb");
    auto cptr = fopen(col_file_name.c_str(),"wb");
    auto vptr = fopen(val_file_name.c_str(),"wb");

    fwrite(&this->matrix_size, sizeof(int), 1, mptr);

    // We always export COO for python
    if (!this->isCoo)
    {
        int *coo_row;
        csr2coo(this->csr_row, this->nnz, this->matrix_size, coo_row);
        fwrite(coo_row, sizeof(int), nnz, rptr);
        delete [] coo_row;
    }
    else
    {
        fwrite(this->coo_row, sizeof(int), nnz, rptr);
    }
    fwrite(this->csr_col, sizeof(int), nnz, cptr);
    fwrite(this->value, sizeof(float), nnz, vptr);

    fclose(mptr);
    fclose(rptr);
    fclose(cptr);
    fclose(vptr);
}

void FMRule::coo(int &matrix_size, int &nnz, int* &row, int* &col, float* &value)
{
    matrix_size = this->matrix_size;
    nnz = this->nnz;
    if (nnz <= 0)
    {
        row = NULL;
        col = NULL;
        value = NULL;
    }
    else
    {
        row = this->coo_row;
        col = this->csr_col;
        value = this->value;
    }
}

} // End of namespace FlowMatrix