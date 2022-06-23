#ifndef __FLOWMATRIX_UTIL_DYNAMIC_DBCORE_H_
#define __FLOWMATRIX_UTIL_DYNAMIC_DBCORE_H_

#include "cuSparseMatrix.cuh"
#include "FMRule.hpp"
#include "sqlite3.h"
#include <sstream>
#include <chrono>

#define TRANSACTION_BUFFER_SIZE 32

namespace FlowMatrix
{

typedef struct _matrix_storage_row_t
{
    int left;
    int right;
    cuSparseMatrix *matrix_ptr;
    int num_valid_indices;
    const int *valid_indices;
} matrix_storage_row_t;

class DBCore
{
private: 

#ifdef TIME_MEASUREMENT
    // Counter and timer
    int load_count, load_fail_count, store_count, offload_count, onload_count;
    std::chrono::duration<double> load_timer, load_fail_timer, store_timer, offload_timer, onload_timer;
#endif // TIME_MEASUREMENT


public:
    // Variables
    sqlite3* db;
    char* sql_cmd_buffer;
    std::string table_name;
    cudaStream_t stream;
    bool has_stream;

    // Transaction buffer
    matrix_storage_row_t transaction_buffer[TRANSACTION_BUFFER_SIZE];
    unsigned int buffer_idx;

public:
    DBCore(const std::string &dbPath, const std::string &traceName);
    DBCore(const std::string &dbPath);

    ~DBCore();

#ifdef TIME_MEASUREMENT
    void resetTimer();
    void printTimer();
#endif

    void Store(int left, int right,                                 
        int matrix_size, int nnz, int* row, int* col, float* value, 
        int num_valid_indices, const int *valid_indices,
        std::string rawbytes="", std::string asm_str="", bool isTranscation = false);
    void StoreFromDevice(cusparseHandle_t &handle,
        int left, int right, 
        cuSparseMatrix *matrix_ptr, 
        int num_valid_indices, const int *valid_indices);
    void StoreSyscall(syscall_t &syscall);
    void StoreVState(VState &vstate);
    cuSparseMatrix *LoadToDevice(cusparseHandle_t &handle, int left, int right, int &num_valid_indices, int *&valid_indices);

    void StartTransaction();
    void EndTransaction();
    void RemoveAllCachedRules();
    void Commit();

    int GetTraceSize();
    std::string ListAllTraces();
    std::string PrintTrace(const int instr_id, const int len = -1);

    int GetSyscallId(const int instr_id);
    std::string PrintSyscallByIdx(const int syscall_idx);
    int GetSyscallsByname(std::string syscallName, syscall_t*&syscalls, int start, int end, int num =0);
    bool GetInfluenceRangeByInstrIdx(const int instr_id, int &length, int *&indices);
    void SetMemInfluence(uint64_t base, uint64_t size, std::set<int> *influence_set_ptr);
    std::map<int, std::string> *GetOffsetName(std::set<int>&offset);
    std::vector<int> GetInstrIdxByASM(std::string asm_str, int start, int end);
};


}


#endif