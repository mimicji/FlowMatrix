#ifndef __FLOWMATRIX_UTIL_DYNAMIC_FMRULE_H_
#define __FLOWMATRIX_UTIL_DYNAMIC_FMRULE_H_

#include "ProtoRuleDB.hpp"
#include "FMGranularity.hpp"
#include "trace.hpp"
#include "VState.hpp"
#include <sys/stat.h>


namespace FlowMatrix
{

typedef struct _coo_pair_t{
    int col;
    int row;

    bool operator<(const struct _coo_pair_t &other) const
    {
        return ((row < other.row) || (row == other.row && col < other.col) );
    }
} coo_pair_t;

class FMRule
{
public:
    // Arr
    int matrix_size, nnz;
    int *csr_row, *csr_col;
    int *coo_row;
    float *value;
    bool isCoo;

    // Meta
    FMGranularity granu;
    instr_t instr_info;
    std::set<int> overrided_set;

public:
    /* In dynamic view, we initialize FMRule with:
     *  1. TaintInduce Rule in protobuf format
     *  2. The instruction info, including eflags and mem addr
     *  3. The superset that the rule needs to be mapped to
     */
    FMRule(const acorn_obj::TaintRule &rule, 
           const instr_t &instr_info,
           VState &vstate,
           bool isCoo, 
           int &num_valid_indices,
           int *&valid_indices);
    FMRule(const instr_t &instr_info, VState &vstate);
    ~FMRule();

    static void getSuperSet(const int num_indices_1, const int *indices_1,
        const int num_indices_2, const int *indices_2,
        int &superset_num_indices, int *&superset_indices,
        int &num_missing_indices_1, int *&missing_indices_1,
        int &num_missing_indices_2, int *&missing_indices_2);
    
    void print(VState &vstate);
    void store4py(const std::string &dir_path) const;
    void coo(int &matrix_size, int &nnz, int* &row, int* &col, float* &value);
};

} // End of namespace FlowMatrix

#endif // __FLOWMATRIX_UTIL_DYNAMIC_FMRULE_H_
