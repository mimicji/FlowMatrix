#include <FMQuery.hpp>

using namespace FlowMatrix;

namespace
{
int highestPowerOf2(int x)
{
    assert(x >= 0);
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;
    return x ^ (x >> 1);
}
}
int AlignAndMultiply(
    CuSparseCore *sp_core,
    cuSparseMatrix &MatrixA,
    int num_valid_indices_A,
    int *valid_indices_A,
    cuSparseMatrix &MatrixB,
    int num_valid_indices_B,
    int *valid_indices_B,
    cuSparseMatrix *&productMatrix_ptr,
    int &superset_num_indices,
    int *&superset_indices,
    int left,           // Only for debugging
    int right)          // Only for debugging
{
    int rvalue = 0;

    sp_core->sync();
    // Get superset and difference set
    int num_missing_indices_1; 
    int *missing_indices_1;
    int num_missing_indices_2; 
    int *missing_indices_2;
    int *superset_indices_; // Non-const ptr
    FMRule::getSuperSet(
        num_valid_indices_A, valid_indices_A,
        num_valid_indices_B, valid_indices_B,
        superset_num_indices, superset_indices_,
        num_missing_indices_1, missing_indices_1,
        num_missing_indices_2, missing_indices_2);
    superset_indices = superset_indices_; // Set ptr

    // Prepare Extended Mat A and Extended Mat B within two streams

    // Extend mat A
    cuSparseMatrix matA_extend;
    cuSparseMatrix *extended_matA_ptr;
    if (num_missing_indices_1 == 0)
    {
        // No need to extend
        extended_matA_ptr = &MatrixA;
    }
    else
    {
        matA_extend.setMainDiagonal(sp_core->handle, MatrixA.num_rows, MatrixA.num_cols, num_missing_indices_1, missing_indices_1, 1.0f);
        if (MatrixA.nnz <= 0)
        {
            extended_matA_ptr = &matA_extend;
        }
        else
        {
// #ifdef DEBUG
//             LOG("[I] Before for mat A add...");
// #endif
            extended_matA_ptr = sp_core->add(MatrixA, matA_extend, 1.0, 1.0, false);
// #ifdef DEBUG
//             LOG("[I] After for mat A add...");
// #endif
        }
    }

    // Extend mat B

    cuSparseMatrix matB_extend;
    cuSparseMatrix *extended_matB_ptr;
    if (num_missing_indices_2 == 0)
    {
        // No need to extend
        extended_matB_ptr = &MatrixB;
    }
    else
    {
        matB_extend.setMainDiagonal(sp_core->associate_handle, MatrixB.num_rows, MatrixB.num_cols, num_missing_indices_2, missing_indices_2, 1.0f);
        if (MatrixB.nnz <= 0)
        {
            extended_matB_ptr = &matB_extend;

        }
        else
        {
// #ifdef DEBUG
//             LOG("[I] Before for mat B add...");
//         if (left == 125324 && right == 125327)
//         {   
//             cuSparseMatrix *mat1 = &MatrixB;
//             cuSparseMatrix *mat2 = &matB_extend;
//             std::cout << "Left: \n";
//             mat1->toCoo(sp_core->handle);
//             mat1->toHost(sp_core->stream);
//             mat1->print();
//             std::cout << std::endl;
//             LOG("[D] Right: ");
//             mat2->toCoo(sp_core->handle);
//             mat2->toHost(sp_core->stream);
//             mat2->print();
//             DIE();
//         }
// #endif
            extended_matB_ptr = sp_core->add(MatrixB, matB_extend, 1.0, 1.0, true); 
// #ifdef DEBUG   
//             LOG("[I] After for mat B add...");
// #endif
        }
    }

    // Get product matrix
    if ((num_missing_indices_1 == 0 && MatrixA.nnz == 0) ||
        (num_missing_indices_2 == 0 && MatrixB.nnz == 0) ||
        (extended_matA_ptr->nnz == 0)                    ||         // This may happen when add() failed.
        (extended_matB_ptr->nnz == 0))
    {
        rvalue = 1; // Weird.
        productMatrix_ptr = new cuSparseMatrix(MatrixA.num_rows, MatrixA.num_cols);
        productMatrix_ptr->nnz = 0;
    }
    else
    {   
        sp_core->sync();

        // {   
        //     std::cout << "Superset: " << superset_num_indices<<std::endl;
        //     for (int i=0; i<superset_num_indices; i++)
        //     {
        //         std::cout << superset_indices[i] << " ";
        //     }
        //     std::cout << std::endl;
        //     std::cout << "Miss:" << std::endl; 
        //     for (int i=0; i<num_missing_indices_1; i++)
        //     {
        //         std::cout << missing_indices_1[i] << " ";
        //     }
        //     std::cout << std::endl;
        //     cuSparseMatrix *mat1 = extended_matA_ptr;
        //     cuSparseMatrix *mat2 = extended_matB_ptr;
        //     std::cout << "Left: nnz = " << mat1->nnz << std::endl;
        //     for (int i=0; i<num_valid_indices_A; i++)
        //     {
        //         std::cout << valid_indices_A[i] << " ";
        //     }
        //     std::cout << std::endl;
        //     mat1->toCoo(sp_core->handle);
        //     mat1->toHost(sp_core->stream);
        //     mat1->print();
        //     std::cout << std::endl;
        //     std::cout << "Right: nnz = " << mat2->nnz << std::endl;
        //     mat2->toCoo(sp_core->handle);
        //     mat2->toHost(sp_core->stream);
        //     mat2->print();
        //     DIE();
        // }

        productMatrix_ptr = (sp_core->*(sp_core->spgemm))(*extended_matA_ptr, *extended_matB_ptr, 1.0f);
    }

    // Normal free would also free cuda memory.
    // But we explicitly free cuda memory in an asynchronous way.
    if (num_missing_indices_1 > 0)
        matA_extend.freeDeviceMem(sp_core->stream);
    if (num_missing_indices_2 > 0)
        matB_extend.freeDeviceMem(sp_core->stream);
    if (MatrixA.nnz > 0 && num_missing_indices_1 != 0) 
        extended_matA_ptr->freeDeviceMem(sp_core->stream);
    if (MatrixB.nnz > 0 && num_missing_indices_2 != 0) 
        extended_matB_ptr->freeDeviceMem(sp_core->stream);
    
    // Free Matrix ptrs
    if (MatrixA.nnz > 0 && num_missing_indices_1 != 0) 
        delete extended_matA_ptr;
    if (MatrixB.nnz > 0 && num_missing_indices_2 != 0) 
        delete extended_matB_ptr;

    // Free CPU buffers
    if (num_missing_indices_1 > 0)
        delete [] missing_indices_1;
    if (num_missing_indices_2 > 0)
        delete [] missing_indices_2;

    return rvalue;
}

bool RecursiveBuildSubTree(
    CuSparseCore *sp,
    DBCore *db,
    const int left,
    const int right,
    const int num_not_stored_layer,
    cuSparseMatrix *&matrix_ptr,
    int &num_indices,
    int *&indices)
{
    const int range = right - left + 1;

    // Invalid range
    if (UNLIKELY(range<=0)) return NULL;

    // Check if in DB or not
    matrix_ptr = db->LoadToDevice(sp->handle, left, right, num_indices, indices);
    if (matrix_ptr != NULL)
    {
        // Directly load from DB
        #ifdef DEBUG
            LOG("[D] Load (%d,%d) from DB.", left, right);
        #endif
        return true;
    }

    // Is this a leaf node?
    if (range == 1)
    {
        // This is a leaf node, but not in database :(
        // We can't do anything about this. Warn user.
        LOG("[W] Rule for %d not found. Ignore.", left);
        return false;
    }

    // This is not a leaf. We build tree based on pow of 2
    const int mid = left + highestPowerOf2(range-1);

    // Get data flow sum from left child
    cuSparseMatrix *left_child_mat_ptr = NULL;
    int left_num_indices = 0;
    int *left_indices = NULL;
    bool isLeftFromDB = RecursiveBuildSubTree(sp, db, left, mid-1, num_not_stored_layer, left_child_mat_ptr, left_num_indices, left_indices);

    // Get data flow sum from right child
    cuSparseMatrix *right_child_mat_ptr = NULL;
    int right_num_indices = 0;
    int *right_indices = NULL;
    bool isRightFromDB = RecursiveBuildSubTree(sp, db, mid, right, num_not_stored_layer, right_child_mat_ptr, right_num_indices, right_indices);

    // Check validity
    if (left_child_mat_ptr == NULL || left_num_indices == 0)
    {
        // Left child is not valid. Retrun right child. Even if right is either not valid
        matrix_ptr = right_child_mat_ptr;
        num_indices = right_num_indices;
        indices = right_indices;
        return isRightFromDB;
    }
    if (right_child_mat_ptr == NULL || right_num_indices == 0)
    {
        // Right child is not valid. Retrun left child.
        matrix_ptr = left_child_mat_ptr;
        num_indices = left_num_indices;
        indices = left_indices;
        return isLeftFromDB;
    }

    // Multiple left and right
#ifdef DEBUG
    LOG("[D] Multiply left %d and right %d.", left, right);
#endif

    int *superset_indices = NULL;
    int rvalue = 
        AlignAndMultiply(sp, 
            *left_child_mat_ptr, left_num_indices, left_indices,
            *right_child_mat_ptr, right_num_indices, right_indices,
            matrix_ptr, num_indices, superset_indices, left , right);
    indices = superset_indices;
    if (rvalue == 1)
    {
        LOG("[W] Empty dataflow observed at (%d, %d) * (%d, %d)", left, mid-1, mid, right);
    } 

    // Store children if applicable
    if (range > 1<<num_not_stored_layer && !isLeftFromDB)
    {
        // Pass ptrs to DBCore. It would help us free.
        db->StoreFromDevice(sp->handle, left, mid-1, left_child_mat_ptr, left_num_indices, left_indices);
    }
    else
    {
        // Free children
        delete left_child_mat_ptr;
        delete [] left_indices;
    }

    if (range > 1<<num_not_stored_layer && !isRightFromDB)
    {
        // Pass ptrs to DBCore. It would help us free.
        db->StoreFromDevice(sp->handle, mid, right, right_child_mat_ptr, right_num_indices, right_indices);
    }
    else
    {
        // Free children
        delete [] right_indices;
        delete right_child_mat_ptr;
    }

    // Return
    return false;
}

// Build tree within single thread
void BuildSubTree(
    std::string dbPath, 
    std::string traceName,
    const int left, 
    const int right, 
    const int num_not_stored_layer,
    bool always_use_safe_spgemm)
{
    // Init CUDA library for this thread
    CuSparseCore *sp_core = new CuSparseCore(always_use_safe_spgemm);
    DBCore *db = new DBCore(dbPath, traceName);

    cuSparseMatrix *root_mat_ptr = NULL;
    int root_num_indices = 0;
    int *root_indices = NULL;
    bool isLoadedFromDB = RecursiveBuildSubTree(sp_core, db, left, right, num_not_stored_layer, root_mat_ptr, root_num_indices, root_indices);
    
    // root_mat_ptr->toCoo(sp_core->handle);
    // root_mat_ptr->toHost(sp_core->stream);
    // root_mat_ptr->print();
    // DIE();

    if ((right - left + 1 > (1<<(num_not_stored_layer+1))) && !isLoadedFromDB)
    {
        #ifdef DEBUG
            LOG("[D] Append (%d, %d) to DB buffer.", left, right);
        #endif
        db->StoreFromDevice(sp_core->handle, left, right, root_mat_ptr, root_num_indices, root_indices);
    }
    else
    {
        delete [] root_indices;
        root_mat_ptr->freeDeviceMem(sp_core->stream);
        delete root_mat_ptr;
    }

    // Destory CUDA library wrapper
    delete db;
    delete sp_core;
}

std::vector<std::pair<int, int>> returned_pairs;
cuSparseMatrix *RecursiveQuerySubTree(
    CuSparseCore *sp, 
    DBCore *db, 
    int target_start, 
    int target_end, 
    int current_left, 
    int current_right, 
    int &this_node_num_indices, 
    int *&this_node_indices)
{
    assert(current_left<=current_right);

    // LOG("Subquery: (%d,%d)", current_left, current_right);

    this_node_num_indices = 0;
    this_node_indices = NULL;
    
    if (target_start <= current_left && target_end >= current_right)
    {

        // LOG("Return (%d,%d)", current_left, current_right);


        // Check if in DB or not
        cuSparseMatrix *matrix_ptr = db->LoadToDevice(sp->handle, current_left, current_right, this_node_num_indices, this_node_indices);
        if (matrix_ptr != NULL)
        {
            return matrix_ptr;
        }
        else
        {
            if (current_left == current_right)
            {
                // Leaf node is missing. can't help
                return NULL;
            } 
            else
            {
                RecursiveBuildSubTree(sp, db, current_left, current_right, 29, matrix_ptr, this_node_num_indices, this_node_indices);
                return matrix_ptr;
            }
        }
    }

    // Overlapped!
    const int range = current_right - current_left + 1;
    const int mid = current_left + highestPowerOf2(range-1);
    // LOG("Overlapping @ (%d,%d) with next mid=%d", current_left, current_right, mid);

    // Get data flow sum from left child
    cuSparseMatrix *left_child_mat_ptr = NULL;
    int left_num_indices = 0;
    int *left_indices = NULL;
    if (mid-1 >= target_start)
    {
        left_child_mat_ptr = RecursiveQuerySubTree(sp, db, target_start, target_end, current_left, mid-1, left_num_indices, left_indices);
    }

    // Get data flow sum from right child
    cuSparseMatrix *right_child_mat_ptr = NULL;
    int right_num_indices = 0;
    int *right_indices = NULL;
    // LOG("Overlapping @ (%d,%d) with next mid=%d while target end = %d", current_left, current_right, mid, target_end);
    if (mid <= target_end)
    {
        right_child_mat_ptr = RecursiveQuerySubTree(sp, db, target_start, target_end, mid, current_right, right_num_indices, right_indices);
    }

    if (left_child_mat_ptr == NULL || left_num_indices == 0)
    {
        this_node_num_indices = right_num_indices;
        this_node_indices = right_indices;
        if (left_child_mat_ptr != NULL) delete left_child_mat_ptr;
        return right_child_mat_ptr;
    }
    if (right_child_mat_ptr == NULL || right_num_indices == 0)
    {
        this_node_num_indices = left_num_indices;
        this_node_indices = left_indices;
        if (right_child_mat_ptr != NULL) delete right_child_mat_ptr;
        return left_child_mat_ptr;
    }

    // Sum childrens
    int *superset_indices = NULL;
    cuSparseMatrix *matrix_ptr = NULL; 

// #ifdef DEBUG
    // LOG("Multi: (%d, %d)*(%d, %d) for (%d, %d)", 
    //     current_left, mid-1, 
    //     mid, current_right, target_start, target_end);
// #endif

    AlignAndMultiply(sp, 
        *left_child_mat_ptr, left_num_indices, left_indices,
        *right_child_mat_ptr, right_num_indices, right_indices,
        matrix_ptr, this_node_num_indices, superset_indices);
    this_node_indices = superset_indices;

    // Free
    delete [] left_indices;
    delete [] right_indices;
    delete left_child_mat_ptr;
    delete right_child_mat_ptr;

    // Normal Return
    return matrix_ptr;
}

cuSparseMatrix *FMQuery::Query(int left, int right)
{
    // Basic check
    // LOG("Query (%d,%d)",left, right);
    if (!(left>=this->startPos && right <= this->endPos))
    {
        printf("Query (%d, %d) from (%d, %d) is invalid.\n", left, right, this->startPos, this->endPos);
        DIE();
    }
    assert(left<=right);

    cuSparseMatrix *root_mat_ptr = NULL;
    int root_num_indices = 0;
    int *root_indices = NULL;
    root_mat_ptr = RecursiveQuerySubTree(sp_core, db, left, right, this->startPos, this->endPos, root_num_indices, root_indices);
    
    root_mat_ptr->toCoo(sp_core->handle, true);
    root_mat_ptr->toHost(sp_core->stream, true);
    sp_core->sync();
        
    delete [] root_indices;

    return root_mat_ptr;
}


int FMQuery::GetTaskScale()
{
    return (this->endPos - this->startPos + 1);
}

// Lock-free multi-thread tree construction
void FMQuery::BuildTree(int num_not_stored_layer /*= 0*/, int thread_num /*=THREAD_NUM_LIMIT*/, bool always_use_safe_spgemm /* =0 */)
{
    if (num_not_stored_layer <0)
        num_not_stored_layer = 29;
    num_not_stored_layer = std::min(num_not_stored_layer, 29);

    if (thread_num != 1)
    {
        std::cout << "Multi-thread is currently not supported.\n";
        thread_num = 1;
    }

    BuildSubTree(this->dbPath, this->traceName, this->startPos, this->endPos, num_not_stored_layer, always_use_safe_spgemm);
    return;

    this->thread_num = highestPowerOf2(thread_num);
    int taskSize = 1;
    while (taskSize * this->thread_num < this->GetTaskScale())
    {
        taskSize *= 2;
    }

    // Multi-thread
    std::thread threads[this->thread_num];
    int current_left = this->startPos;
    for (int thread_idx=0; thread_idx<this->thread_num; ++thread_idx)
    {
        int current_right = std::min(current_left+taskSize-1, this->endPos);
        threads[thread_idx] = std::thread(&BuildSubTree, this->dbPath, this->traceName, current_left, current_right, num_not_stored_layer, always_use_safe_spgemm);
        LOG("[I] Thread %d build tree for [%d, %d].", thread_idx, current_left, current_right);
        current_left = current_right+1;
    }
    
    // Wait for threads
    for (auto &t : threads) t.join();

    // Normal return
    return;
}

FMQuery::FMQuery(std::string dbPath, std::string traceName, int traceLen, int startPos /* =1 */, int endPos /* =0 */, bool always_use_safe_spgemm /*= true*/)
{
    this->dbPath = dbPath;
    this->traceName = traceName;

    this->traceLen = traceLen;
    this->startPos = startPos;
    this->endPos = (endPos < startPos) ? this->traceLen : endPos;

    // Init CUDA library for this thread
    this->sp_core = new CuSparseCore(always_use_safe_spgemm);
    this->db = new DBCore(dbPath, traceName);
}

FMQuery::~FMQuery()
{
    // Destory CUDA library wrapper
    delete db;
    delete sp_core;
}

