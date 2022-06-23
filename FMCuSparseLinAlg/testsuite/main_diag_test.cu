#include <cuSparseCore.cuh>

using namespace FlowMatrix;

int main(){
    const int A_num_rows = 6;
    const int A_num_cols = 6;
    const int A_nnz      = 4;
    int   hA_csrOffsets[] = { 0, 0, 1, 2, 3, 4, 4 };
    int   hA_columns[]    = { 1, 2, 3, 4};
    float hA_values[]     = { 1.0f, 1.0f, 1.0f, 1.0f,};
    int   indices[]       = { 1, 2, 3, 4};

    // Init cusparse handler
    auto sp_core = new CuSparseCore();

    cuSparseMatrix matA;
    matA.setMainDiagonal(sp_core->handle, A_num_rows, A_num_cols, A_nnz, indices, 1.0f);

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
            if (matA.h_columns[i] != hA_columns[i] ||
                matA.h_values[i]  != hA_values[i])
            { 
                correct = 0;                         
                break;
            }
    }
    if (correct)
            printf("Set Main Diag test PASSED\n");
    else {
            printf("Set Main Diag test FAILED: Wrong Result\n");
            return EXIT_FAILURE;
    }
}