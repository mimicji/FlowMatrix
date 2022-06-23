#include <matrix_base.hpp>

MatrixBase::MatrixBase()
{
    isOnDevice = false;
    
    row = nullptr;
    col = nullptr;
    val = nullptr;
}

int MatrixBase::to_csr()
{
    // Condition check
    if (isOnDevice) return -1;
    if (isCSR) return -2;
    
    // Convert!
    const int row_ptr_size = m + 1;
    int *csr_row = new int[row_ptr_size];
    memset(csr_row, 0, sizeof(int)*row_ptr_size);
    for (int i = 0; i < nnz; i++)
        csr_row[row[i] + 1]++;
    for (int i = 0; i < m; i++)
        csr_row[i + 1] += csr_row[i];

    // Free the old row and override
    delete [] row;
    row = csr_row;

    // Set propety and normal return 
    isCSR = true;
    return 0;
}

int MatrixBase::to_coo()
{
    // Condition check
    if (isOnDevice) return -1;
    if (!isCSR) return -2;
    
    // Convert!
    int *coo_row = new int[nnz];
    memset(coo_row, 0, sizeof(int)*nnz);
    int row_index = 0;
    for (int i = 0; i < nnz; i++)
    {
        while(row[row_index+1]<=i) row_index++;
        coo_row[i] = row_index;
    }

    // Assertion check. Should never happen if the input CSR is valid.
    if ((row_index+1 != m) ||
        (row[row_index+1] != nnz))
        return -3;

    // Free the old row and override
    delete [] row;
    row = coo_row;

    // Set propety and normal return 
    isCSR = false;
    return 0;
}

void MatrixBase::print()
{
    if (isOnDevice)
    {
        LOG("[E]Cannot print on-device matrix!");
        return;
    }

    std::cout << "Matrix Size: " << m << " * " << n << std::endl;
    std::cout << "NNZ: " << nnz << std::endl;
    
    // Print Row
    if (isCSR) 
    {
        std::cout << "CSR format:" << std::endl;
        std::cout << "  Row = [";
        for (int i=0; i<m; i++)
        {
            std::cout << row[i] << ",";
        }
        std::cout << row[m] << "]" << std::endl;
    }
    else
    {
        std::cout << "COO format:" << std::endl;
        std::cout << "  Row = [";
        for (int i=0; i<nnz; i++)
        {
            std::cout << row[i] << ",";
        }
        std::cout << "\b]" << std::endl;
    }

    // Print Column
    std::cout << "  Col = [";
    for (int i=0; i<nnz; i++)
    {
        std::cout << col[i] << ",";
    }
    std::cout << "\b]" << std::endl;

    // Print Value
    if (!hasVal)
    {
        std::cout << "  Value = NA " << std::endl;
    }
    else
    {
        std::cout << "  Value = [" << std::setprecision(2);
        for (int i=0; i<nnz; i++)
        {
            std::cout << val[i] << ",";
        }
        std::cout << "\b]" << std::endl;        
    }
}

// Returns how many files have been written to disk.
int MatrixBase::store4py(const std::string &filename_prefix) const
{
    // Do nothing to an on-device matrix
    if (isOnDevice)
    {
        LOG("[E]Cannot store on-device matrix!");
        return 0;
    }

    // Write row
    if (isCSR)
    {
        std::string row_file_name = filename_prefix + ".fm_row";
        auto rptr = fopen(row_file_name.c_str(),"wb");
        fwrite(row, sizeof(int), m, rptr);
        fclose(rptr);
    }
    else
    {
        std::string row_file_name = filename_prefix + ".fm_row_coo";
        auto rptr = fopen(row_file_name.c_str(),"wb");
        fwrite(row, sizeof(int), nnz, rptr);
        fclose(rptr);
    }

    // Write column
    {
        std::string col_file_name = filename_prefix + ".fm_col";
        auto cptr = fopen(col_file_name.c_str(),"wb");
        fwrite(col, sizeof(int), nnz, cptr);
        fclose(cptr);
    }

    // Write val
    if (hasVal)
    {
        std::string val_file_name = filename_prefix + ".fm_val";
        auto vptr = fopen(val_file_name.c_str(),"wb");
        fwrite(val, sizeof(int), nnz, vptr);
        fclose(vptr);
        return 3;
    }
    else
    {
        return 2;
    }
}