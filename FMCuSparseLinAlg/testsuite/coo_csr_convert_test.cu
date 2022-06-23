#include <cuSparseCore.cuh>

using namespace FlowMatrix;

int main(){
    const int A_num_rows = 4;
    const int A_num_cols = 4;
    const int A_nnz      = 9;
    // const int B_num_rows = 4;
    // const int B_num_cols = 4;
    // const int B_nnz      = 9;
    int   hA_csrOffsets[] = { 0, 3, 4, 7, 9 };
    int   hA_columns[]    = { 0, 2, 3, 1, 0, 2, 3, 1, 3 };
    float hA_values[]     = { 1.0f, 2.0f, 3.0f, 4.0f, 5.0f,
                              6.0f, 7.0f, 8.0f, 9.0f };
    int   hB_cooOffsets[] = { 0, 0, 0, 1, 2, 2, 2, 3, 3};
    int   hB_columns[]    = { 0, 2, 3, 1, 0, 2, 3, 1, 3};
    float hB_values[]     = { 1.0f, 2.0f, 3.0f, 4.0f, 5.0f,
                              6.0f, 7.0f, 8.0f, 9.0f };
    // const int D_nnz       = 8;

    // Init cusparse handler
    auto sp_core = new CuSparseCore();

    // Create matrix
    cuSparseMatrix matA(sp_core->stream, A_num_rows, A_num_cols, A_nnz, hA_csrOffsets, hA_columns, hA_values);
    
    // Convert to coo
    matA.toCoo(sp_core->handle, false);
    matA.toCsr(sp_core->handle, true);

    // To host memory
    matA.toHost(sp_core->stream);
    sp_core->sync();

    // Verification
    int correct = 1;
    if (matA.nnz != A_nnz || matA.num_rows != A_num_rows || matA.num_cols != A_num_cols)
    {
        correct = 0;
    }
    for (int i = 0; i < matA.num_rows + 1; i++) {
            if (matA.h_csrOffsets[i] != hA_csrOffsets[i]) 
            {
                correct = 0;
                break;
            }
    }
    for (int i = 0; i < matA.nnz; i++) {
            if (matA.h_columns[i] != hB_columns[i] ||
                matA.h_values[i]  != hB_values[i]  ||
                matA.h_cooOffsets[i] != hB_cooOffsets[i]) 
            { 
                correct = 0;                         
                break;
            }
    }
    if (correct)
            printf("COO&CSR Conversion test PASSED\n");
    else {
            printf("COO&CSR Conversion test FAILED: Wrong Result\n");
            return EXIT_FAILURE;
    }
}