#include <cuSparseCore.cuh>

using namespace FlowMatrix;

CuSparseCore::CuSparseCore(bool always_use_safe)
{
    // Create stream
    CHECK_CUDA(cudaStreamCreate(&this->stream));
    CHECK_CUDA(cudaStreamCreate(&this->associate_stream));

    // Create handle
    CHECK_CUSPARSE( cusparseCreate(&this->handle) );
    CHECK_CUSPARSE(cusparseSetStream(this->handle, this->stream));
    CHECK_CUSPARSE( cusparseCreate(&this->associate_handle));
    CHECK_CUSPARSE(cusparseSetStream(this->associate_handle, this->associate_stream));

    // Create buffer
    this->buffer1_size = BUFFER_INIT_SIZE;
    CHECK_CUDA(cudaMallocAsync((void**) &this->buffer1, this->buffer1_size, this->stream));
    this->buffer2_size = BUFFER_INIT_SIZE;
    CHECK_CUDA(cudaMallocAsync((void**) &this->buffer2, this->buffer2_size, this->stream));

    // Create MatDesc
    CHECK_CUSPARSE( cusparseCreateMatDescr(&this->matDescr) ); 
    cusparseSetMatType(this->matDescr, CUSPARSE_MATRIX_TYPE_GENERAL);
    cusparseSetMatIndexBase(this->matDescr, CUSPARSE_INDEX_BASE_ZERO);

    // Set Spgemm
    if (always_use_safe)
        this->spgemm = &CuSparseCore::_spgemm_safe;
    else
        this->spgemm = &CuSparseCore::_spgemm;

#ifdef TIME_MEASUREMENT
    this->resetTimer();
#endif // TIME_MEASUREMENT
}

CuSparseCore::~CuSparseCore()
{
    // Free buffer
    CHECK_CUDA(cudaFreeAsync(this->buffer1, this->stream));
    CHECK_CUDA(cudaFreeAsync(this->buffer2, this->stream));

    // Destroy handle
    CHECK_CUSPARSE(cusparseDestroy(this->handle));
    CHECK_CUSPARSE(cusparseDestroy(this->associate_handle));

    // Destroy stream
    CHECK_CUDA(cudaStreamDestroy(this->stream));
    CHECK_CUDA(cudaStreamDestroy(this->associate_stream));

    // Destory MatDesc
    CHECK_CUSPARSE( cusparseDestroyMatDescr(this->matDescr) );
}

#ifdef TIME_MEASUREMENT
void CuSparseCore::resetTimer()
{
    this->total_nnz = 0;
    this->spgemm_count = 0;
    this->spgemm_safe_count = 0;
    this->spgemm_timer = std::chrono::duration<double>::zero();
    this->spgemm_safe_timer = std::chrono::duration<double>::zero();
}

void CuSparseCore::printTimer()
{
    // Print counter
    if (this->total_nnz > 0)
        std::cout << "Total NNZ:\t" << this->total_nnz  << std::endl;
    if (this->spgemm_count > 0)
        std::cout << "SPGEMM_DEFAULT:\t" << this->spgemm_count << "\tTime: " << this->spgemm_timer.count() << std::endl;
    if (this->spgemm_safe_count > 0)
        std::cout << "SPGEMM_SAFE:\t" << this->spgemm_safe_count << "\tTime: " << this->spgemm_safe_timer.count() << std::endl;
}
#endif // TIME_MEASUREMENT


cuSparseMatrix *CuSparseCore::_spgemm(
    cuSparseMatrix &matA, 
    cuSparseMatrix &matB, 
    value_type_t alpha /*=1.0f*/)
{
#ifdef DEBUG
    assert(matA.d_csrOffsets != NULL);
    assert(matA.d_columns != NULL);
    assert(matA.d_values != NULL);
    assert(matB.d_csrOffsets != NULL);
    assert(matB.d_columns != NULL);
    assert(matB.d_values != NULL);
#endif
    // Check matDescr
    if ((matA.matDescr == 0))
    {
        CHECK_CUSPARSE(cusparseCreateCsr(&matA.matDescr, 
            matA.num_rows, matA.num_cols, matA.nnz,
            matA.d_csrOffsets, matA.d_columns, matA.d_values,
            CUSPARSE_INDEX_32I, CUSPARSE_INDEX_32I,
            CUSPARSE_INDEX_BASE_ZERO, CUDA_R_32F));
    }
    if ((matB.matDescr == 0))
    {
        CHECK_CUSPARSE(cusparseCreateCsr(&matB.matDescr, 
            matB.num_rows, matB.num_cols, matB.nnz,
            matB.d_csrOffsets, matB.d_columns, matB.d_values,
            CUSPARSE_INDEX_32I, CUSPARSE_INDEX_32I,
            CUSPARSE_INDEX_BASE_ZERO, CUDA_R_32F));
    }

    // Get handle
    cusparseHandle_t &handle = this->handle;
    cudaStream_t &stream = this->stream;

    // We are perfroming the simpliest computation: matC = matA*matB
    cusparseOperation_t opA = CUSPARSE_OPERATION_NON_TRANSPOSE;
    cusparseOperation_t opB = CUSPARSE_OPERATION_NON_TRANSPOSE;
    value_type_t beta = float2value(0.0f);

    // Prepare tmp buffers
    size_t bufferSize1 = 0, bufferSize2 = 0;

#ifdef TIME_MEASUREMENT
    // Compute the intermediate product of A * B
    auto begin_time = std::chrono::high_resolution_clock::now();
#endif // TIME_MEASUREMENT

    // Init cusparse SpGEMM descriptor
    cusparseSpGEMMDescr_t spgemmDesc;
    CHECK_CUSPARSE(cusparseSpGEMM_createDescr(&spgemmDesc));

    // Init the result matrix
    cuSparseMatrix *matResultPtr = new cuSparseMatrix(matA.num_rows, matB.num_cols);

    // Create Desc
    CHECK_CUSPARSE(cusparseCreateCsr(&matResultPtr->matDescr, 
        matResultPtr->num_rows, matResultPtr->num_cols, 0,
        NULL, NULL, NULL,
        CUSPARSE_INDEX_32I, CUSPARSE_INDEX_32I,
        CUSPARSE_INDEX_BASE_ZERO, CUDA_R_32F));

    // Ask bufferSize1 bytes for external memory
    CHECK_CUSPARSE(
        cusparseSpGEMM_workEstimation(handle, opA, opB,
                                      &alpha, matA.matDescr, matB.matDescr, &beta, matResultPtr->matDescr,
                                      COMPUTE_TYPE, CUSPARSE_SPGEMM_DEFAULT,
                                      spgemmDesc, &bufferSize1, NULL) );

    if (UNLIKELY(this->buffer1_size < bufferSize1))
    {
        #ifdef DEBUG
            printf("spgemm: Buffer 1 is not large enough: current size is %lu but %lu is needed.\n", this->buffer1_size, bufferSize1);
        #endif
        CHECK_CUDA(cudaFreeAsync(this->buffer1, stream));
        CHECK_CUDA(cudaMallocAsync((void**) &this->buffer1, bufferSize1, stream));
        this->buffer1_size = bufferSize1;
    }

    // Inspect the matrices A and B to understand the memory requirement for
    // the next step
    CHECK_CUSPARSE(
        cusparseSpGEMM_workEstimation(handle, opA, opB,
                                      &alpha, matA.matDescr, matB.matDescr, &beta, matResultPtr->matDescr,
                                      COMPUTE_TYPE, CUSPARSE_SPGEMM_DEFAULT,
                                      spgemmDesc, &bufferSize1, this->buffer1) );
                                
    // Ask bufferSize2 bytes for external memory
    CHECK_CUSPARSE(
        cusparseSpGEMM_compute(handle, opA, opB,
                               &alpha, matA.matDescr, matB.matDescr, &beta, matResultPtr->matDescr,
                               COMPUTE_TYPE, CUSPARSE_SPGEMM_DEFAULT,
                               spgemmDesc, &bufferSize2, NULL));

    if (UNLIKELY(this->buffer2_size < bufferSize2))
    {
        #ifdef DEBUG
            printf("spgemm: Buffer 2 is not large enough: current size is %lu but %lu is needed.\n", this->buffer2_size, bufferSize2);
        #endif
        CHECK_CUDA(cudaFreeAsync(this->buffer2, stream));
        CHECK_CUDA(cudaMallocAsync((void**) &this->buffer2, bufferSize2, stream));
        this->buffer2_size = bufferSize2;
    }

    auto errorId = cusparseSpGEMM_compute(handle, opA, opB,
            &alpha, matA.matDescr, matB.matDescr, &beta, matResultPtr->matDescr,
            COMPUTE_TYPE, CUSPARSE_SPGEMM_DEFAULT,
            spgemmDesc, &bufferSize2, this->buffer2);

    // cusparseSpGEMM() has a limitation which uses too much memory.
    // If it happens, we fallback to a deprecated api, cusparseScsrgemm2().
    // See https://github.com/NVIDIA/CUDALibrarySamples/issues/38 for details
    if (UNLIKELY(errorId != CUSPARSE_STATUS_SUCCESS))
    {
        if (LIKELY(errorId == CUSPARSE_STATUS_INSUFFICIENT_RESOURCES))
        {
            #ifdef DEBUG
                printf("spgemm: Insufficient resources! Fallback to spgemm_safe()!\n");
            #endif
            // Destory SpGEMM descriptor
            CHECK_CUSPARSE( cusparseSpGEMM_destroyDescr(spgemmDesc) );
            
            // Clean-up
            delete matResultPtr;
            
            // Switch routine
            matResultPtr = this->_spgemm_safe(matA, matB, alpha);
            
            // Set Sparse Desc for _spgemm
            CHECK_CUSPARSE(cusparseCreateCsr(&matResultPtr->matDescr, 
                matResultPtr->num_rows, matResultPtr->num_cols, matResultPtr->nnz,
                matResultPtr->d_csrOffsets, matResultPtr->d_columns, matResultPtr->d_values,
                CUSPARSE_INDEX_32I, CUSPARSE_INDEX_32I,
                CUSPARSE_INDEX_BASE_ZERO, CUDA_R_32F));

            // Return
            return matResultPtr;
        }
        else
        {
            // Not because of insufficient res. Raise this error.
            CHECK_CUSPARSE(errorId);
        }
    }

    // Get result matrix non-zero entries
    matResultPtr->nnz = 0;
    CHECK_CUSPARSE( cusparseSpMatGetSize(matResultPtr->matDescr, &matResultPtr->num_rows, 
        &matResultPtr->num_cols, &matResultPtr->nnz) );

    // Allocate result matrix
    CHECK_CUDA( cudaMallocAsync((void**) &matResultPtr->d_csrOffsets, (matA.num_rows + 1) * sizeof(int),stream) );
    CHECK_CUDA( cudaMallocAsync((void**) &matResultPtr->d_columns, matResultPtr->nnz * sizeof(int),     stream) );
    CHECK_CUDA( cudaMallocAsync((void**) &matResultPtr->d_values,  matResultPtr->nnz * sizeof(float),   stream) );

    // Update result matrix with the new pointers
    CHECK_CUSPARSE(
        cusparseCsrSetPointers(matResultPtr->matDescr, matResultPtr->d_csrOffsets, matResultPtr->d_columns, matResultPtr->d_values) );
    
    // Copy the final products to the result matrix
    CHECK_CUSPARSE(
        cusparseSpGEMM_copy(handle, opA, opB,
                            &alpha, matA.matDescr, matB.matDescr, &beta, matResultPtr->matDescr,
                            COMPUTE_TYPE, CUSPARSE_SPGEMM_DEFAULT, spgemmDesc) );

    // Destory descriptor
    CHECK_CUSPARSE( cusparseSpGEMM_destroyDescr(spgemmDesc) );

#ifdef TIME_MEASUREMENT
    auto end_time = std::chrono::high_resolution_clock::now();
#endif // TIME_MEASUREMENT

#ifdef TIME_MEASUREMENT
    // Update counter
    this->total_nnz += matResultPtr->nnz;
    this->spgemm_count++;
    this->spgemm_timer += end_time - begin_time;
#endif 

    // Return result
    return matResultPtr;
}

cuSparseMatrix *CuSparseCore::_spgemm_safe(
    cuSparseMatrix &matA, 
    cuSparseMatrix &matB, 
    value_type_t alpha /*=1.0f*/)
{
    // Get handle
    cusparseHandle_t &handle = this->handle;
    cudaStream_t &stream = this->stream;

    // Init result
    cuSparseMatrix *matResultPtr = new cuSparseMatrix(matA.num_rows, matB.num_cols);

#ifdef TIME_MEASUREMENT
    auto begin_time = std::chrono::high_resolution_clock::now();
#else
    assert(matA.num_cols == matB.num_rows);
#endif

    // step 1: create an opaque structure
    csrgemm2Info_t info = NULL;
    CHECK_CUSPARSE(cusparseCreateCsrgemm2Info(&info));

    // step 2: allocate buffer for csrgemm2Nnz and csrgemm2
    size_t bufferSize1;
    CHECK_CUSPARSE(cusparseScsrgemm2_bufferSizeExt(
        handle, matA.num_rows, matB.num_cols, matA.num_cols, &alpha,
        this->matDescr, matA.nnz, matA.d_csrOffsets, matA.d_columns,
        this->matDescr, matB.nnz, matB.d_csrOffsets, matB.d_columns,
        NULL,  /*beta*/
        this->matDescr, 0, NULL, NULL, //nnzD, csrRowPtrD, csrColIndD,
        info,
        &bufferSize1));
    
    if (UNLIKELY(this->buffer1_size < bufferSize1))
    {
        printf("spgemm_safe: Buffer 1 is not large enough: current size is %lu but %lu is needed.\n", this->buffer1_size, bufferSize1);
        CHECK_CUDA(cudaFreeAsync(this->buffer1, stream));
        CHECK_CUDA(cudaMallocAsync((void**) &this->buffer1, bufferSize1, stream));
        this->buffer1_size = bufferSize1;
    }
    void *&buffer = this->buffer1;

    // step 3: compute csrRowPtr
    int nnzTotalDevHostPtr = -1;
    CHECK_CUDA( cudaMallocAsync((void**) &matResultPtr->d_csrOffsets, (matA.num_rows + 1) * sizeof(int),stream) );  
    
    CHECK_CUSPARSE(cusparseXcsrgemm2Nnz(
            handle, matA.num_rows, matB.num_cols, matA.num_cols,
            this->matDescr, matA.nnz, matA.d_csrOffsets, matA.d_columns,
            this->matDescr, matB.nnz, matB.d_csrOffsets, matB.d_columns,
            this->matDescr, 0, NULL, NULL,//nnzD, csrRowPtrD, csrColIndD,
            this->matDescr, matResultPtr->d_csrOffsets, &nnzTotalDevHostPtr,
            info, buffer));
    matResultPtr->nnz = (int64_t) nnzTotalDevHostPtr;

    // step 4: finish sparsity pattern and value
    CHECK_CUDA( cudaMallocAsync((void**) &matResultPtr->d_columns, matResultPtr->nnz * sizeof(int),     stream));
    CHECK_CUDA( cudaMallocAsync((void**) &matResultPtr->d_values,  matResultPtr->nnz * sizeof(float),   stream));

    CHECK_CUSPARSE(cusparseScsrgemm2(
        handle, matA.num_rows, matB.num_cols, matA.num_cols, &alpha,
        this->matDescr, matA.nnz, matA.d_values, matA.d_csrOffsets, matA.d_columns,
        this->matDescr, matB.nnz, matB.d_values, matB.d_csrOffsets, matB.d_columns,
        NULL, // beta
        this->matDescr, 0, NULL, NULL, NULL, //nnzD, csrValD, csrRowPtrD, csrColIndD,
        this->matDescr, matResultPtr->d_values, matResultPtr->d_csrOffsets, matResultPtr->d_columns,
        info, buffer));

    // Destory Csrgemm Info
    CHECK_CUSPARSE(cusparseDestroyCsrgemm2Info(info));

#ifdef TIME_MEASUREMENT
    auto end_time = std::chrono::high_resolution_clock::now();
    this->total_nnz += matResultPtr->nnz;
    this->spgemm_safe_count++;
    this->spgemm_safe_timer += end_time - begin_time;
#endif

    return matResultPtr;
}

cuSparseMatrix* CuSparseCore::add(
    cuSparseMatrix &matA, 
    cuSparseMatrix &matB, 
    float alpha/* = 1.0f */, 
    float belta /* = 1.0f */,
    bool use_associate /*= false*/)
{
#ifdef DEBUG
    assert(matA.d_csrOffsets != NULL);
    assert(matA.d_columns != NULL);
    assert(matA.d_values != NULL);
    assert(matB.d_csrOffsets != NULL);
    assert(matB.d_columns != NULL);
    assert(matB.d_values != NULL);
#endif
    // Get handle
    cusparseHandle_t &handle = use_associate ? this->associate_handle : this->handle;
    cudaStream_t &stream = use_associate ? this->associate_stream : this->stream;
    void *&buffer = use_associate ? this->buffer2 : this->buffer1;
    size_t &buffer_size = use_associate ? this->buffer2_size : this->buffer1_size;

    // Init the result matrix
    cuSparseMatrix *matResultPtr = new cuSparseMatrix(matA.num_rows, matA.num_cols);

    // Malloc row ptr for the result matrix
    CHECK_CUDA( cudaMallocAsync((void**) &matResultPtr->d_csrOffsets, (matA.num_rows + 1) * sizeof(int), stream) );
    
    // Get buffer size
    size_t requiredBufferSize;
    CHECK_CUSPARSE(
        cusparseScsrgeam2_bufferSizeExt(handle, matResultPtr->num_rows, matResultPtr->num_cols, 
            &alpha, this->matDescr, matA.nnz, matA.d_values, matA.d_csrOffsets, matA.d_columns,
            &belta, this->matDescr, matB.nnz, matB.d_values, matB.d_csrOffsets, matB.d_columns,
            this->matDescr, matResultPtr->d_values, matResultPtr->d_csrOffsets, matResultPtr->d_columns, 
            &requiredBufferSize    
    ));

    if (UNLIKELY(buffer_size < requiredBufferSize))
    {
        #ifdef DEBUG
            printf("add: Buffer 1 is not large enough: current size is %lu but %lu is needed.\n", buffer_size, requiredBufferSize);
        #endif
        CHECK_CUDA(cudaFreeAsync(buffer, stream));
        CHECK_CUDA(cudaMallocAsync((void**) &buffer, requiredBufferSize, stream));
        buffer_size = requiredBufferSize;
    }
    int nnz_result = -1;

    // if (!
    CHECK_CUSPARSE(
        cusparseXcsrgeam2Nnz(handle, matResultPtr->num_rows, matResultPtr->num_cols, 
            this->matDescr, matA.nnz, matA.d_csrOffsets, matA.d_columns,
            this->matDescr, matB.nnz, matB.d_csrOffsets, matB.d_columns,
            this->matDescr, matResultPtr->d_csrOffsets, &nnz_result,
            buffer));
    // )
    // {
    //     matResultPtr->nnz = 0;
    //     CHECK_CUDA(cudaFreeAsync(matResultPtr->d_csrOffsets, stream));
    //     matResultPtr->d_csrOffsets = NULL;
    //     return matResultPtr;
    // };
    matResultPtr->nnz = (int64_t) nnz_result;
    
    // Malloc column and values
    CHECK_CUDA( cudaMallocAsync((void**) &matResultPtr->d_columns, matResultPtr->nnz * sizeof(int),     stream));
    CHECK_CUDA( cudaMallocAsync((void**) &matResultPtr->d_values,  matResultPtr->nnz * sizeof(float),   stream));
    
    // Finish computation
    CHECK_CUSPARSE(
        cusparseScsrgeam2(handle, matResultPtr->num_rows, matResultPtr->num_cols, 
            &alpha, this->matDescr, matA.nnz, matA.d_values, matA.d_csrOffsets, matA.d_columns,
            &belta, this->matDescr, matB.nnz, matB.d_values, matB.d_csrOffsets, matB.d_columns,
            this->matDescr, matResultPtr->d_values, matResultPtr->d_csrOffsets, matResultPtr->d_columns, 
            buffer 
    ));

    return matResultPtr;
}

void CuSparseCore::sync()
{
    cudaStreamSynchronize(this->stream);
    cudaStreamSynchronize(this->associate_stream);
}


// void CuSparseCore::compress(int streamId, cuSparseMatrix &matA)
// {
//     float tol = 0.5;

//     // Get handle
//     cusparseHandle_t handle = this->streamPool->getCusparseHandle(streamId);
//     cudaStream_t stream = this->streamPool->getStreamFromPool(streamId);
    
//     // Malloc a temp buffer to store nnz per row
//     int *nnz_per_row;
//     int *new_nnz;
//     CHECK_CUDA( cudaMallocManaged((void**) &nnz_per_row, (matA.num_rows+1) * sizeof(int)) );
//     CHECK_CUDA( cudaMallocManaged((void**) &new_nnz, sizeof(int)) );

//     memset( nnz_per_row, 0, (matA.num_rows+1) * sizeof(int));

//     // Get nnz per row
//     CHECK_CUSPARSE( cusparseSnnz_compress(
//         handle, matA.num_rows, this->matDescr, 
//         matA.d_values, matA.d_csrOffsets, nnz_per_row,
//         new_nnz, tol));
    
//     // Malloc new space for the compressed matrix
//     float *csrVal;
//     int *csrRowPtr;
//     int *csrColInd;
//     CHECK_CUDA( cudaMallocAsync( &csrVal, sizeof(float) * (*new_nnz), stream));
//     CHECK_CUDA( cudaMallocAsync( &csrRowPtr, sizeof(int) * (matA.num_rows+1), stream));
//     CHECK_CUDA( cudaMallocAsync( &csrColInd, sizeof(int) * (*new_nnz), stream));

//     CHECK_CUSPARSE( cusparseScsr2csr_compress( 
//         handle,matA.num_rows, matA.num_cols, this->matDescr, 
//         matA.d_values, matA.d_columns, matA.d_csrOffsets,
//         matA.nnz,  nnz_per_row,
//         csrVal, csrRowPtr, csrColInd, tol));
    
//     CHECK_CUDA( cudaMemcpyAsync(&matA.nnz, new_nnz, sizeof(int),
//         cudaMemcpyDeviceToHost, stream) );

//     // Free malloced memory
//     CHECK_CUDA( cudaFreeAsync(matA.d_values, stream) );
//     CHECK_CUDA( cudaFreeAsync(matA.d_columns, stream) );
//     CHECK_CUDA( cudaFreeAsync(matA.d_csrOffsets, stream) );

//     // Set pointers
//     matA.d_values = csrVal;
//     matA.d_columns = csrColInd;
//     matA.d_csrOffsets = csrRowPtr;
// }