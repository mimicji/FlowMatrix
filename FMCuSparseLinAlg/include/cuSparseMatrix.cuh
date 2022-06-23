#ifndef __FLOWMATRIX_UTILS_CU_SPARSEMATRIX_H_
#define __FLOWMATRIX_UTILS_CU_SPARSEMATRIX_H_

#include <cuCommon.cuh>
#include <algorithm>

namespace FlowMatrix{

// Sparse Matrix in CSR format. By default it's on device.
class cuSparseMatrix
{
public:
    cusparseSpMatDescr_t matDescr;
    int64_t num_rows, num_cols, nnz;
    index_type_t *d_csrOffsets, *d_cooOffsets, *d_columns;
    value_type_t *d_values;
    index_type_t *h_csrOffsets, *h_cooOffsets, *h_columns;
    value_type_t *h_values;

    cuSparseMatrix();
    cuSparseMatrix(int num_rows, int num_cols);
    cuSparseMatrix(cudaStream_t &stream, int num_rows, int num_cols, int nnz, int *csrOffsets, int *columns, float *values);
    ~cuSparseMatrix();

    void fromDB(cusparseHandle_t &handle, int num_rows, int num_cols, int nnz, const int *cooOffsets, const int *columns, const float *values);

    void toCoo(cusparseHandle_t &handle, bool keepCsr = false);
    void toCsr(cusparseHandle_t &handle, bool keepCoo = false);

    void setMainDiagonal(cusparseHandle_t &handle, int num_rows, int num_cols, int nnz, int *indices, float value = 1.0f);
    void setMatrixByDevicePtrs(int num_rows, int num_cols, int nnz, int *d_csrOffsets, int *d_columns, float *d_values);
    void toDevice(cudaStream_t &stream);
    void toHost(cudaStream_t &stream, bool keepOnDevice = false);
    void print() const;

    void freeDeviceMem(cudaStream_t &stream);
};

} // End of namespace
#endif