#ifndef __FLOWMATRIX_UTIL_DYNAMIC_QUERY_H_
#define __FLOWMATRIX_UTIL_DYNAMIC_QUERY_H_

#include <thread>
#include <cuSparseCore.cuh>
#include <DBCore.hpp>
#include <FMRule.hpp>

#define THREAD_NUM_LIMIT 8

int AlignAndMultiply(
    FlowMatrix::CuSparseCore *sp_core,
    FlowMatrix::cuSparseMatrix &MatrixA,
    int num_valid_indices_A,
    int *valid_indices_A,
    FlowMatrix::cuSparseMatrix &MatrixB,
    int num_valid_indices_B,
    int *valid_indices_B,
    FlowMatrix::cuSparseMatrix *&productMatrix_ptr,
    int &superset_num_indices,
    int *&superset_indices,
    int left = 0,           // Only for debugging
    int right = 0);

namespace FlowMatrix
{


class FMQuery
{
public:
    int thread_num;
    int traceLen, startPos, endPos;
    std::string traceName, dbPath;
    DBCore *db;
    CuSparseCore *sp_core;
private:
    int GetTaskScale();
public:
    FMQuery(std::string dbPath, std::string traceName, int traceLen, int startPos = 1, int endPos = 0, bool always_use_safe_spgemm = true);
    ~FMQuery();

    cuSparseMatrix *Query(int left, int right);
    void BuildTree(int num_not_stored_layer = 0, int thread_num = THREAD_NUM_LIMIT, bool always_use_safe_spgemm = false);

};

} // End of namespace

#endif