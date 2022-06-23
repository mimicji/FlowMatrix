#ifndef __FLOWMATRIX_UTILS_CU_SPARSE_CORE_H_
#define __FLOWMATRIX_UTILS_CU_SPARSE_CORE_H_

#include <cuSparseMatrix.cuh>


namespace FlowMatrix
{


class CuSparseCore
{
#ifdef TIME_MEASUREMENT
private:
    unsigned long total_nnz;
    int spgemm_count, spgemm_safe_count;
    std::chrono::duration<double> spgemm_timer, spgemm_safe_timer;
public:
    void resetTimer();
    void printTimer();
#endif // TIME_MEASUREMENT

// Member variables
private:
    // Prepare two buffers for spgemm and add
    void *buffer1, *buffer2;
    size_t buffer1_size, buffer2_size;

    // Shared Matrix Desc. This is not Cusparse Matrix Desc.
    cusparseMatDescr_t matDescr;

public:
    // Provide two streams and attached handles
    cudaStream_t stream, associate_stream;
    cusparseHandle_t handle, associate_handle;



    // SPGEMM function pointer
    // Usage: 
    //      rvalue = (this->*(sp_core->spgemm))(__args__);
    cuSparseMatrix* (CuSparseCore::*spgemm)(cuSparseMatrix &, cuSparseMatrix &, value_type_t);

// Member functions:
public:
    CuSparseCore(bool always_use_safe = false);
    ~CuSparseCore();

    // Sync both streams
    void sync();

    // Sparse matrix add sparse matrix
    //      matC = alpha*matA + belta*matB
    cuSparseMatrix* add(cuSparseMatrix &matA, cuSparseMatrix &matB, float alpha = 1.0f, float belta = 1.0f, bool use_associate = false);

    // Two implementation of SPGEMM. _spgemm_safe is faster but deprecated :(
    cuSparseMatrix* _spgemm(cuSparseMatrix &matA, cuSparseMatrix &matB, value_type_t alpha = 1.0f);
    cuSparseMatrix* _spgemm_safe(cuSparseMatrix &matA, cuSparseMatrix &matB, value_type_t alpha = 1.0f);
};

}




#endif
