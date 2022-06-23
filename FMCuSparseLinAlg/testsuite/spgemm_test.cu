#include <cuSparseCore.cuh>

using namespace FlowMatrix;

int main(){

    const int A_num_rows = 4;
    const int A_num_cols = 4;
    const int A_nnz      = 9;
    const int B_num_rows = 4;
    const int B_num_cols = 4;
    const int B_nnz      = 8;
    int   hA_csrOffsets[] = { 0, 3, 4, 7, 9 };
    int   hA_columns[]    = { 0, 2, 3, 1, 0, 2, 3, 1, 3 };
    float hA_values[]     = { 1.0f, 2.0f, 3.0f, 4.0f, 5.0f,
                              6.0f, 7.0f, 8.0f, 9.0f };
    int   hB_csrOffsets[] = { 0, 2, 4, 7, 8 };
    int   hB_columns[]    = { 0, 3, 1, 3, 0, 1, 2, 1 };
    float hB_values[]     = { 1.0f, 2.0f, 3.0f, 4.0f, 5.0f,
                              6.0f, 7.0f, 8.0f };
    int   hC_csrOffsets[] = { 0, 4, 6, 10, 12 };
    int   hC_columns[]    = { 0, 1, 2, 3, 1, 3, 0, 1, 2, 3, 1, 3 };
    float hC_values[]     = { 11.0f, 36.0f, 14.0f, 2.0f,  12.0f,
                              16.0f, 35.0f, 92.0f, 42.0f, 10.0f,
                              96.0f, 32.0f };
    const int C_nnz       = 12;

    // Init cusparse handler
    auto sp_core = new CuSparseCore();

    // Create matrix
    cuSparseMatrix matA(sp_core->stream, A_num_rows, A_num_cols, A_nnz, hA_csrOffsets, hA_columns, hA_values);
    cuSparseMatrix matB(sp_core->stream, B_num_rows, B_num_cols, B_nnz, hB_csrOffsets, hB_columns, hB_values);
    
    // Compute
    cuSparseMatrix *matCPtr = sp_core->_spgemm(matA, matB);
    cuSparseMatrix *matDPtr = sp_core->_spgemm_safe(matA, matB);

    // To host memory
    matCPtr->toHost(sp_core->stream);
    matDPtr->toHost(sp_core->stream);
    sp_core->sync();

    // Verification
    int correct = 1;
    for (int i = 0; i < A_num_rows + 1; i++) {
            if (matCPtr->h_csrOffsets[i] != hC_csrOffsets[i]) {
            correct = 0;
            break;
            }
    }
    for (int i = 0; i < C_nnz; i++) {
            if (matCPtr->h_columns[i] != hC_columns[i] ||
            matCPtr->h_values[i]  != hC_values[i]) { // direct floating point
            correct = 0;                         // comparison is not reliable
            break;
            }
    }
    for (int i = 0; i < A_num_rows + 1; i++) {
        if (matDPtr->h_csrOffsets[i] != hC_csrOffsets[i]) {
        correct = 0;
        break;
        }
    }
    for (int i = 0; i < C_nnz; i++) {
            if (matDPtr->h_columns[i] != hC_columns[i] ||
            matDPtr->h_values[i]  != hC_values[i]) { // direct floating point
            correct = 0;                         // comparison is not reliable
            break;
            }
    }

    if (correct)
            printf("SpGEMM test PASSED\n");
    else {
            printf("SpGEMM test FAILED: Wrong Result\n");
            return EXIT_FAILURE;
    }

    // Free pointer
    delete matCPtr;

}