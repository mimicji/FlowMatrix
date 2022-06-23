#include <cuSparseMatrix.cuh>

using namespace FlowMatrix;

cuSparseMatrix::cuSparseMatrix()
{
    this->matDescr = 0;

    this->num_rows = -1;
    this->num_cols = -1;
    this->nnz = -1;

    this->d_csrOffsets = NULL;
    this->d_cooOffsets = NULL;
    this->d_columns = NULL;
    this->d_values = NULL;

    this->h_csrOffsets = NULL;
    this->h_cooOffsets = NULL;
    this->h_columns = NULL;
    this->h_values = NULL;
}

cuSparseMatrix::cuSparseMatrix(int num_rows, int num_cols)
{
    this->matDescr = 0;

    this->num_rows = num_rows;
    this->num_cols = num_cols;
    this->nnz = -1;

    this->d_csrOffsets = NULL;
    this->d_cooOffsets = NULL;
    this->d_columns = NULL;
    this->d_values = NULL;

    this->h_csrOffsets = NULL;
    this->h_cooOffsets = NULL;
    this->h_columns = NULL;
    this->h_values = NULL;
}

cuSparseMatrix::cuSparseMatrix(cudaStream_t &stream, int num_rows, int num_cols, int nnz, int *csrOffsets, int *columns, float *values)
{
    int *d_csrOffsets, *d_columns;
    float *d_values;

    CHECK_CUDA( cudaMallocAsync((void**) &d_csrOffsets,
                           (num_rows + 1) * sizeof(int), stream) );
    CHECK_CUDA( cudaMallocAsync((void**) &d_columns, nnz * sizeof(int), stream)   );
    CHECK_CUDA( cudaMallocAsync((void**) &d_values,  nnz * sizeof(float), stream) );

    CHECK_CUDA( cudaMemcpyAsync(d_csrOffsets, csrOffsets, (num_rows + 1)  * sizeof(int),
            cudaMemcpyHostToDevice, stream) );
    CHECK_CUDA( cudaMemcpyAsync(d_columns, columns, nnz * sizeof(int),
            cudaMemcpyHostToDevice, stream) );
    CHECK_CUDA( cudaMemcpyAsync(d_values, values,
            nnz * sizeof(float), cudaMemcpyHostToDevice, stream) );
    this->setMatrixByDevicePtrs(num_rows, num_cols, nnz, d_csrOffsets, d_columns, d_values);
}

void cuSparseMatrix::fromDB(cusparseHandle_t &handle, int num_rows, int num_cols, int nnz, const int *cooOffsets, const int *columns, const float *values)
{
    this->num_rows = num_rows;
    this->num_cols = num_cols;
    this->nnz = nnz;

    if (nnz <= 0) return;

    cudaStream_t stream; 
    CHECK_CUSPARSE(cusparseGetStream(handle, &stream));

    CHECK_CUDA( cudaMallocAsync((void**) &this->d_cooOffsets, 
        (this->nnz) * sizeof(int), stream));
    CHECK_CUDA( cudaMemcpyAsync(this->d_cooOffsets, cooOffsets,
        (this->nnz) * sizeof(int),
        cudaMemcpyHostToDevice, stream) );

    CHECK_CUDA( cudaMallocAsync((void**) &this->d_columns, 
        (this->nnz) * sizeof(int), stream));
    CHECK_CUDA( cudaMemcpyAsync(this->d_columns, columns, this->nnz * sizeof(int),
        cudaMemcpyHostToDevice, stream) );
    
    CHECK_CUDA( cudaMallocAsync((void**) &this->d_values, 
        (this->nnz) * sizeof(float), stream));
    CHECK_CUDA( cudaMemcpyAsync(this->d_values, values, this->nnz * sizeof(float),
        cudaMemcpyHostToDevice, stream) );    

    this->toCsr(handle);
}

void cuSparseMatrix::setMatrixByDevicePtrs(int num_rows, int num_cols, int nnz, int *d_csrOffsets, int *d_columns, float *d_values)
{
    if (d_csrOffsets == NULL || d_columns == NULL || d_values == NULL)
    {
        printf("Warning: Set matrix desc with NULL array!\n");
    }

    this->num_rows = num_rows;
    this->num_cols = num_cols;
    this->nnz = nnz;

    this->d_csrOffsets = d_csrOffsets;
    this->d_columns = d_columns;
    this->d_values = d_values;
    this->d_cooOffsets = NULL;

    this->h_csrOffsets = NULL;
    this->h_cooOffsets = NULL;
    this->h_columns = NULL;
    this->h_values = NULL;

    CHECK_CUSPARSE(cusparseCreateCsr(&this->matDescr, num_rows, num_cols, nnz,
        d_csrOffsets, d_columns, d_values,
        CUSPARSE_INDEX_32I, CUSPARSE_INDEX_32I,
        CUSPARSE_INDEX_BASE_ZERO, CUDA_R_32F));
}

void cuSparseMatrix::setMainDiagonal(cusparseHandle_t &handle, int num_rows, int num_cols, int nnz, int *indices, float value /*=1.0f*/)
{
    this->nnz = nnz;
    this->num_rows = num_rows;
    this->num_cols = num_cols;

    this->h_cooOffsets = new int[nnz];
    this->h_columns = new int[nnz];
    this->h_values = new float[nnz];

    memcpy(this->h_cooOffsets, indices, sizeof(int)*nnz);
    memcpy(this->h_columns, indices, sizeof(int)*nnz);
    std::fill_n(this->h_values, nnz, value); 

    // Transfer to GPU and convert to CSR
    cudaStream_t stream; 
    CHECK_CUSPARSE(cusparseGetStream(handle, &stream));
    this->toDevice(stream);
    this->toCsr(handle);
}

void cuSparseMatrix::toCsr(cusparseHandle_t &handle, bool keepCoo /*= false*/)
{
    if (UNLIKELY(this->d_csrOffsets != NULL))
    {
        printf("[W] Matrix is already in CSR format.\n");
        return;
    }
    assert(this->d_cooOffsets != NULL);
    cudaStream_t stream; 
    CHECK_CUSPARSE(cusparseGetStream(handle, &stream));
    CHECK_CUDA( cudaMallocAsync((void**) &this->d_csrOffsets,
        (this->num_rows + 1) * sizeof(int), stream) );
    CHECK_CUSPARSE(cusparseXcoo2csr(handle, this->d_cooOffsets, this->nnz,
        this->num_rows, this->d_csrOffsets, CUSPARSE_INDEX_BASE_ZERO));
    
    // Set Matrix descriptor
    CHECK_CUSPARSE(cusparseCreateCsr(&this->matDescr, 
        this->num_rows, this->num_cols, this->nnz,
        this->d_csrOffsets, this->d_columns, this->d_values,
        CUSPARSE_INDEX_32I, CUSPARSE_INDEX_32I,
        CUSPARSE_INDEX_BASE_ZERO, CUDA_R_32F));  
    
    if (!keepCoo)
    {
        CHECK_CUDA( cudaFreeAsync(this->d_cooOffsets, stream) );
        this->d_cooOffsets = NULL;
    }
}

void cuSparseMatrix::toCoo(cusparseHandle_t &handle, bool keepCsr /* = false */)
{
    if (UNLIKELY(this->d_cooOffsets != NULL))
    {
        printf("[W] Matrix is already in COO format.\n");
        return;
    }
    assert(this->d_csrOffsets != NULL);
    cudaStream_t stream; 
    CHECK_CUSPARSE(cusparseGetStream(handle, &stream));
    CHECK_CUDA( cudaMallocAsync((void**) &this->d_cooOffsets,
        (this->nnz) * sizeof(int), stream) );
    CHECK_CUSPARSE(cusparseXcsr2coo(handle, this->d_csrOffsets, this->nnz,
        this->num_rows, this->d_cooOffsets, CUSPARSE_INDEX_BASE_ZERO));
    if (!keepCsr)
    {
        CHECK_CUDA( cudaFreeAsync(this->d_csrOffsets, stream) );
        this->d_csrOffsets = NULL;
    }
}

void cuSparseMatrix::toDevice(cudaStream_t &stream)
{
    // First, we transfer Rowptr or RowIdx, depending on the format
    assert(this->h_csrOffsets != NULL || this->h_cooOffsets != NULL);
    if (this->h_csrOffsets != NULL)
    {
        CHECK_CUDA( cudaMallocAsync((void**) &this->d_csrOffsets, 
            (this->num_rows + 1) * sizeof(int), stream));
        CHECK_CUDA( cudaMemcpyAsync(this->d_csrOffsets, this->h_csrOffsets,
            (this->num_rows + 1) * sizeof(int),
            cudaMemcpyHostToDevice, stream) );
        delete [] this->h_csrOffsets;
        this->h_csrOffsets = NULL;
    }
    if (this->h_cooOffsets != NULL)
    {
        CHECK_CUDA( cudaMallocAsync((void**) &this->d_cooOffsets, 
            (this->nnz) * sizeof(int), stream));
        CHECK_CUDA( cudaMemcpyAsync(this->d_cooOffsets, this->h_cooOffsets,
            (this->nnz) * sizeof(int),
            cudaMemcpyHostToDevice, stream) );
        delete [] this->h_cooOffsets;
        this->h_cooOffsets = NULL;
    }

    // Transfer col and values
    assert(this->h_columns != NULL && this->h_values != NULL);

    CHECK_CUDA( cudaMallocAsync((void**) &this->d_columns, 
        (this->nnz) * sizeof(int), stream));
    CHECK_CUDA( cudaMallocAsync((void**) &this->d_values, 
        (this->nnz) * sizeof(float), stream));

    CHECK_CUDA( cudaMemcpyAsync(this->d_columns, this->h_columns, this->nnz * sizeof(int),
        cudaMemcpyHostToDevice, stream) );
    CHECK_CUDA( cudaMemcpyAsync(this->d_values, this->h_values, this->nnz * sizeof(float),
        cudaMemcpyHostToDevice, stream) );    

    // Set Matrix descriptor
    if (this->d_csrOffsets != NULL)
    {
        CHECK_CUSPARSE(cusparseCreateCsr(&this->matDescr, 
            this->num_rows, this->num_cols, this->nnz,
            this->d_csrOffsets, this->d_columns, this->d_values,
            CUSPARSE_INDEX_32I, CUSPARSE_INDEX_32I,
            CUSPARSE_INDEX_BASE_ZERO, CUDA_R_32F)); 
    }

    // Reset ptrs
    delete [] this->h_columns;
    delete [] this->h_values;
    this->h_columns = NULL;
    this->h_values = NULL;
}

void cuSparseMatrix::toHost(cudaStream_t &stream, bool keepOnDevice /*= false*/)
{
    // First, we transfer Rowptr or RowIdx, depending on the format
    assert(this->d_csrOffsets != NULL || this->d_cooOffsets != NULL);
    if (this->d_csrOffsets != NULL)
    {
        this->h_csrOffsets = new int[this->num_rows + 1];
        CHECK_CUDA( cudaMemcpyAsync(this->h_csrOffsets, this->d_csrOffsets,
            (this->num_rows + 1) * sizeof(int),
            cudaMemcpyDeviceToHost, stream) );
        if (!keepOnDevice)
        {
            CHECK_CUDA( cudaFreeAsync(this->d_csrOffsets, stream) );
            this->d_csrOffsets = NULL;
        }
    }

    if (this->d_cooOffsets != NULL)
    {
        this->h_cooOffsets = new int[this->nnz];
        CHECK_CUDA( cudaMemcpyAsync(this->h_cooOffsets, this->d_cooOffsets,
            (this->nnz) * sizeof(int),
            cudaMemcpyDeviceToHost, stream) );
        if (!keepOnDevice)
        {
            CHECK_CUDA( cudaFreeAsync(this->d_cooOffsets, stream) );
            this->d_cooOffsets = NULL;
        }

    }

    // Transfer col and values
    assert(this->d_columns != NULL && this->d_values != NULL);

    this->h_columns = new int[this->nnz];
    this->h_values = new float[this->nnz];

    CHECK_CUDA( cudaMemcpyAsync(this->h_columns, this->d_columns, this->nnz * sizeof(int),
            cudaMemcpyDeviceToHost, stream) );

    CHECK_CUDA( cudaMemcpyAsync(this->h_values, this->d_values, this->nnz * sizeof(float),
            cudaMemcpyDeviceToHost, stream) );

    // Free CUDA memory
    if (!keepOnDevice)
    {
        if (this->matDescr != 0) CHECK_CUSPARSE( cusparseDestroySpMat(this->matDescr) );
        CHECK_CUDA( cudaFreeAsync(this->d_columns, stream) );
        CHECK_CUDA( cudaFreeAsync(this->d_values, stream) );
        this->d_columns = NULL;
        this->d_values = NULL;
    }

    // Reset ptrs
    this->matDescr = 0;
}
void cuSparseMatrix::freeDeviceMem(cudaStream_t &stream)
{
    if (this->d_csrOffsets != NULL)
    {
        CHECK_CUDA( cudaFreeAsync(this->d_csrOffsets, stream) );
        this->d_csrOffsets = NULL;
    }
    if (this->d_cooOffsets != NULL)
    {
        CHECK_CUDA( cudaFreeAsync(this->d_cooOffsets, stream) );
        this->d_cooOffsets = NULL;
    }
    if (this->d_columns != NULL)
    {
        CHECK_CUDA( cudaFreeAsync(this->d_columns, stream) );
        this->d_columns = NULL;
    }
    if (this->d_values != NULL)
    {
        CHECK_CUDA( cudaFreeAsync(this->d_values, stream) );
        this->d_values = NULL;
    }
    if (this->matDescr != 0)
    {
        CHECK_CUSPARSE( cusparseDestroySpMat(this->matDescr) );
        this->matDescr = 0;
    }
}


cuSparseMatrix::~cuSparseMatrix()
{
    if (this->d_csrOffsets != NULL)
    {
        CHECK_CUDA( cudaFree(this->d_csrOffsets) );
        this->d_csrOffsets = NULL;
    }
    if (this->d_cooOffsets != NULL)
    {
        CHECK_CUDA( cudaFree(this->d_cooOffsets) );
        this->d_cooOffsets = NULL;
    }
    if (this->d_columns != NULL)
    {
        CHECK_CUDA( cudaFree(this->d_columns) );
        this->d_columns = NULL;
    }
    if (this->d_values != NULL)
    {
        CHECK_CUDA( cudaFree(this->d_values) );
        this->d_values = NULL;
    }
    if (this->matDescr != 0)
    {
        CHECK_CUSPARSE( cusparseDestroySpMat(this->matDescr) );
        this->matDescr = 0;
    }
    if (this->h_csrOffsets != NULL)
    {
        delete[] this->h_csrOffsets;
    }
    if (this->h_cooOffsets != NULL)
    {
        delete[] this->h_cooOffsets;
    }
    if (this->h_columns != NULL)
    {
        delete[] this->h_columns;
    }
    if (this->h_values != NULL)
    {
        delete[] this->h_values;
    }
}

void cuSparseMatrix::print() const
{
    std::cout << "Shape: " << this->num_rows << " * " << this->num_cols << std::endl;
    std::cout << "NNZ: " << this->nnz << std::endl;
    std::cout << "Values:"<< std::endl;
    if (this->nnz >0 && (this->h_cooOffsets == NULL || this->h_columns == NULL || this->h_values == NULL))
    {
        std::cout << "  Error! This matrix is not ready for printing." << std::endl;
        return;
    }
    if (nnz >0)
    {
        for (int i=0; i<this->nnz; i++)
        {
            std::cout << "  (" << this->h_cooOffsets[i] << ", " 
                << this->h_columns[i] << ") "
                << this->h_values[i];
            if (i%4 == 3)
            {
                std::cout << std::endl;
            }
            else
            {
                std::cout << "    ";
            }
        }
        if (this->nnz%4 != 0) std::cout << std::endl;
    }
    else
    {
        std::cout << "  (None)" << std::endl;
    }

    return;
}