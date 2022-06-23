#include <cuSparseCore.cuh>

using namespace FlowMatrix;

int main(){
    // Test A+B = C
    //      A-B = D
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
    int   hC_csrOffsets[] = { 0, 3, 5, 9, 11 };
    int   hC_columns[]    = { 0, 2, 3, 1, 3, 0, 1, 2,3, 1, 3 };
    float hC_values[]     = { 2.0f, 2.0f, 5.0f, 7.0f,  4.0f,
                              10.0f, 6.0f, 13.0f, 7.0f, 16.0f,
                              9.0f};
    const int C_nnz       = 11;
    int   hD_csrOffsets[] = { 0, 3, 5, 9, 11 };
    int   hD_columns[]    = { 0, 2, 3, 1, 3, 0, 1, 2, 3, 1, 3 };
    float hD_values[]     = { 0.0f, 2.0f, 1.0f, 1.0f, -4.0f,  
                              0.0f, -6.0f, -1.0f, 7.0f, 0.0f, 9.0f};
    // const int D_nnz       = 8;

    // Init cusparse handler
    auto sp_core_1 = new CuSparseCore();
    auto sp_core_2 = new CuSparseCore();

    // Create matrix
    cuSparseMatrix matA(sp_core_1->stream, A_num_rows, A_num_cols, A_nnz, hA_csrOffsets, hA_columns, hA_values);
    cuSparseMatrix matB(sp_core_2->stream, B_num_rows, B_num_cols, B_nnz, hB_csrOffsets, hB_columns, hB_values);
    sp_core_1->sync();
    sp_core_2->sync();

    // Compute
    cuSparseMatrix *matCPtr = sp_core_1->add(matA, matB);
    cuSparseMatrix *matDPtr = sp_core_2->add(matA, matB, 1.0, -1.0);

    // To host memory
    matCPtr->toHost(sp_core_1->stream);
    matDPtr->toHost(sp_core_2->stream);
    sp_core_1->sync();
    sp_core_2->sync();

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
    if (correct)
            printf("SpGEAM ADD test PASSED\n");
    else {
            printf("SpGEAM ADD test FAILED: Wrong Result\n");
            return EXIT_FAILURE;
    }

    // printf("RowPtr: ");
    // for (int i = 0; i < A_num_rows + 1; i++) {
    //     printf("%d ", matDPtr->h_csrOffsets[i]);
    // }  
    // printf("\nCol: ");
    // for (int i = 0; i < matDPtr->nnz; i++) {
    //     printf("%d ", matDPtr->h_columns[i]);
    // }
    // printf("\nValue: ");
    // for (int i = 0; i < matDPtr->nnz; i++) {
    //     printf("%0.2f ", matDPtr->h_values[i]);
    // }
    // printf("\n");

    for (int i = 0; i < A_num_rows + 1; i++) {
        if (matDPtr->h_csrOffsets[i] != hD_csrOffsets[i]) {
            correct = 0;
            break;
        }
    }       
    for (int i = 0; i < matDPtr->nnz; i++) {
            if (matDPtr->h_columns[i] != hD_columns[i] ||
            matDPtr->h_values[i]  != hD_values[i]) { // direct floating point
                correct = 0;                         // comparison is not reliable
                break;
            }
    }

    if (correct)
            printf("SpGEAM Subtraction test PASSED\n");
    else {
            printf("SpGEAM Subtraction test FAILED: Wrong Result\n");
            return EXIT_FAILURE;
    }

    // Free pointer
    delete matCPtr;
    delete matDPtr;
}