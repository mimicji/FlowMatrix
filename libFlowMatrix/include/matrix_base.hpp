#ifndef __LIBFLOWMATRIX_MATRIXBASE_H__
#define __LIBFLOWMATRIX_MATRIXBASE_H__

#include <FMCommon.hpp>

typedef float value_t;

class MatrixBase
{
public:
    bool isOnDevice;
    bool hasVal;    
    bool isCSR;     // True for CSR, False for COO

    int m;          // The number of rows in the matrix.
    int n;          // The number of columns in the matrix.
    int nnz;        // The number of nonzero elements in the matrix.

    int *row;       // CSR: size = m + 1; COO: size = nnz
    int *col;       // size = nnz
    value_t *val;   // size = nnz

    MatrixBase();

    int to_coo();
    int to_csr();
    void print();

    int store4py(const std::string &filename_prefix) const;
};

#endif // __LIBFLOWMATRIX_MATRIXBASE_H__
