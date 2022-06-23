#include "DBCore.hpp"

using namespace FlowMatrix;

DBCore::DBCore(const std::string &dbPath, const std::string &traceName)
{
#ifdef TIME_MEASUREMENT
    this->resetTimer();
#endif // TIME_MEASUREMENT

    has_stream = false;
    sql_cmd_buffer = (char*) malloc(1024);
    int res = sqlite3_open_v2(dbPath.c_str(), &this->db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_FULLMUTEX, NULL);
    if (res)
    {
        std::cout<< "[E] Unable to open database "<< traceName << "!" << std::endl;
        DIE();
    }

    this->table_name = traceName;

    // Create Trace Table
    std::string create_table_cmd =  
        "CREATE TABLE IF NOT EXISTS " + this->table_name +
        "(   start_pos INTEGER NOT NULL,        \
            end_pos INTEGER NOT NULL,           \
            rawbytes TEXT,                      \
            asm TEXT,                           \
            shape INTERGER,                     \
            nnz INTERGER NOT NULL,              \
            row BLOB,                           \
            column BLOB,                        \
            value BLOB,                         \
            valid_indices BLOB,                 \
            PRIMARY KEY (start_pos, end_pos)    \
        );";
    char *errMsg = 0;
    int rc = sqlite3_exec(this->db, create_table_cmd.c_str(), NULL, NULL, &errMsg);
    if (rc != SQLITE_OK) {
        LOG("[E] Error at creating table: %s.\n", errMsg);
        sqlite3_free(errMsg);
        sqlite3_close(db);
        this->db = NULL;
        DIE();
    }

    // Create syscall table
    create_table_cmd =  
        "CREATE TABLE IF NOT EXISTS " + 
        this->table_name + "_sys " +
        "(  instr_id INTERGER NOT NULL,         \
            syscall_id INTERGER NOT NULL,       \
            name TEXT,                          \
            rvalue INTERGER,                    \
            nargs INTERGER,                     \
            args BLOB,                          \
            PRIMARY KEY (instr_id)              \
        );";
    rc = sqlite3_exec(this->db, create_table_cmd.c_str(), NULL, NULL, &errMsg);
    if (rc != SQLITE_OK) {
        LOG("[E] Error at creating table: %s.\n", errMsg);
        sqlite3_free(errMsg);
        sqlite3_close(db);
        this->db = NULL;
        DIE();
    }

    // Create syscall table
    create_table_cmd =  
        "CREATE TABLE IF NOT EXISTS " + 
        this->table_name + "_vstate " +
        "(  start INTERGER NOT NULL,            \
            size  INTERGER NOT NULL,            \
            name  TEXT,                         \
            id    INTERGER,                     \
            PRIMARY KEY (start)                 \
        );";
    rc = sqlite3_exec(this->db, create_table_cmd.c_str(), NULL, NULL, &errMsg);
    if (rc != SQLITE_OK) {
        LOG("[E] Error at creating table: %s.\n", errMsg);
        sqlite3_free(errMsg);
        sqlite3_close(db);
        this->db = NULL;
        DIE();
    }

    // Init Transaction Buffer
    buffer_idx = 0;
}

DBCore::DBCore(const std::string &dbPath)
{
#ifdef TIME_MEASUREMENT
    // Counter and timer
    this->load_count = 0;
    this->store_count = 0;
    this->load_fail_count = 0;
    this->offload_count = 0;
    this->onload_count = 0;
    this->load_timer = std::chrono::duration<double>::zero();
    this->load_fail_timer = std::chrono::duration<double>::zero();
    this->store_timer = std::chrono::duration<double>::zero();
    this->offload_timer = std::chrono::duration<double>::zero();
    this->onload_timer = std::chrono::duration<double>::zero();
#endif // TIME_MEASUREMENT

    this->table_name = "";
    has_stream = false;
    sql_cmd_buffer = (char*) malloc(1024);    
    buffer_idx = 0;
    int res = sqlite3_open_v2(dbPath.c_str(), &this->db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_FULLMUTEX, NULL);
    if (res)
    {
        std::cout<< "[E] Unable to open database "<< table_name << "!" << std::endl;
        DIE();
    }
}
std::string DBCore::ListAllTraces()
{
    sqlite3_stmt *stmt;

    sprintf(this->sql_cmd_buffer,
        "SELECT name FROM sqlite_master WHERE type='table';"); 
    if (SQLITE_OK != sqlite3_prepare(this->db, this->sql_cmd_buffer, strlen(this->sql_cmd_buffer), &stmt, NULL))
    {
        std::cout <<"Failed to prepare: " << sqlite3_errmsg(this->db) << std::endl;
        DIE();
    }
    int rc = sqlite3_step(stmt);
    if (SQLITE_ROW != rc && SQLITE_DONE != rc)
    {
        std::cout <<"Failed to get table list: " << sqlite3_errmsg(this->db) << std::endl;
        DIE();
    }
    if (SQLITE_DONE == rc)
    {
        return "(None)";
    }
    int traceCount = 0;
    std::string rstr = "";
    do
    {
        std::string thisTableName((const char*)sqlite3_column_text(stmt, 0));
        if (thisTableName.find("_vstate") != std::string::npos) continue;
        if (thisTableName.find("_sys") != std::string::npos) continue;
        traceCount++;
        rstr += "[";
        rstr += std::to_string(traceCount);
        rstr += "] ";
        rstr += thisTableName;
        if (traceCount % 4 == 0)
        {
            rstr += "\n";
        }
        else
        {
            rstr += "\t\t";
        }
    } while(SQLITE_ROW == sqlite3_step(stmt));
    rstr+="\n";

    sqlite3_finalize(stmt);

    return rstr;
}

#ifdef TIME_MEASUREMENT
void DBCore::resetTimer()
{
    // Counter and timer
    this->load_count = 0;
    this->store_count = 0;
    this->load_fail_count = 0;
    this->offload_count = 0;
    this->onload_count = 0;
    this->load_timer = std::chrono::duration<double>::zero();
    this->load_fail_timer = std::chrono::duration<double>::zero();
    this->store_timer = std::chrono::duration<double>::zero();
    this->offload_timer = std::chrono::duration<double>::zero();
    this->onload_timer = std::chrono::duration<double>::zero();
}

void DBCore::printTimer()
{
    // Print counter
    if (onload_count > 0)
        std::cout << "GPU Onload:\t" << this->onload_count << "\tTime: " << this->onload_timer.count() << std::endl;
    if (offload_count > 0)
        std::cout << "GPU Offload:\t" << this->offload_count << "\tTime: " << this->offload_timer.count() << std::endl;
    if (load_count > 0)
        std::cout << "DB Load:\t" << this->load_count << "\tTime: " << this->load_timer.count() << std::endl;
    if (load_fail_count > 0)
        std::cout << "DB Failed Load:\t" << this->load_fail_count << "\tTime: " << this->load_fail_timer.count() << std::endl;
    if (store_count > 0)
        std::cout << "DB Store:\t" << this->store_count << "\tTime: " << this->store_timer.count() << std::endl;
}
#endif // TIME_MEASUREMENT

DBCore::~DBCore()
{
    this->Commit();
    free(sql_cmd_buffer);
    if (this->db != NULL)
    {
        sqlite3_close(this->db);
    }
}

void DBCore::RemoveAllCachedRules()
{
    LOG("[I] Removing all intermedia results from database.");
    sprintf(this->sql_cmd_buffer,
        "DELETE FROM %s WHERE (start_pos <> end_pos);",
        this->table_name.c_str()); 
    char *errMsg = 0;
    int rc = sqlite3_exec(this->db, this->sql_cmd_buffer, NULL, NULL, &errMsg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Error at removing cached rules: %s.\n", errMsg);
        sqlite3_free(errMsg);
        sqlite3_close(db);
        this->db = NULL;
        DIE();
    }
}

void DBCore::Store(int left, int right, 
        int matrix_size, int nnz, int* row, int* col, float* value, 
        int num_valid_indices, const int *valid_indices,
        std::string rawbytes, std::string asm_str, bool isTranscation)
{
    sqlite3_stmt *stmt;

    if (nnz <= 0 && num_valid_indices==0)
    {
        sprintf(this->sql_cmd_buffer,
            "INSERT OR IGNORE INTO %s                                   \
                (start_pos, end_pos, rawbytes,                          \
                 asm, shape, nnz)                                       \
            VALUES                                                      \
                (%d, %d, ?, ?, %d, %d);", 
            this->table_name.c_str(), left, right, matrix_size, nnz); 
        if (SQLITE_OK != sqlite3_prepare(db, this->sql_cmd_buffer, strlen(this->sql_cmd_buffer), &stmt, NULL))
        {
            std::cout <<"Failed to prepare row: " << sqlite3_errmsg(this->db) << std::endl;
            DIE();
        }
        if (SQLITE_OK != sqlite3_bind_text(stmt, 1, rawbytes.c_str(), rawbytes.length(), SQLITE_STATIC) ||
            SQLITE_OK != sqlite3_bind_text(stmt, 2, asm_str.c_str(),  asm_str.length(),  SQLITE_TRANSIENT))
            {
                std::cout <<"Failed to bind text row: " << sqlite3_errmsg(this->db) << std::endl;
                DIE();
            }
        if (SQLITE_DONE != sqlite3_step(stmt))
            {
                std::cout <<"Failed to step: " << sqlite3_errmsg(this->db) << std::endl;
                DIE();
            }
        if (!isTranscation) sqlite3_finalize(stmt);
        return;
    }

    if (nnz == 0 && num_valid_indices!=0)
    {
        sprintf(this->sql_cmd_buffer,
            "INSERT OR IGNORE INTO %s                   \
                (start_pos, end_pos, rawbytes, asm,     \
                    shape, nnz,                         \
                    valid_indices)                      \
            VALUES                                      \
                (%d, %d, ?, ?, %d, %d, ?);", 
        this->table_name.c_str(), left, right, matrix_size, nnz);                   
        if (SQLITE_OK != sqlite3_prepare(this->db, this->sql_cmd_buffer, strlen(this->sql_cmd_buffer), &stmt, NULL))
        {
            std::cout <<"Failed to prepare: " << sqlite3_errmsg(this->db) << std::endl;
            DIE();
        }

        if (SQLITE_OK != sqlite3_bind_text(stmt, 1, rawbytes.c_str(), rawbytes.length(), SQLITE_STATIC) ||
            SQLITE_OK != sqlite3_bind_text(stmt, 2, asm_str.c_str(),  asm_str.length(),  SQLITE_TRANSIENT))
            {
                std::cout <<"Failed to bind text row: " << sqlite3_errmsg(this->db) << std::endl;
                DIE();
            }

        if (SQLITE_OK != sqlite3_bind_blob(stmt, 3, valid_indices, sizeof(int)*num_valid_indices, SQLITE_STATIC))
        {
            std::cout <<"Failed to bind rows: " << sqlite3_errmsg(this->db) << std::endl;
            DIE();
        }

        if (SQLITE_DONE != sqlite3_step(stmt))
        {
            std::cout <<"Failed to execute: " << sqlite3_errmsg(this->db) << std::endl;
            DIE();
        }

        if (!isTranscation) sqlite3_finalize(stmt);
        return;
    }

    if (nnz > 0 && num_valid_indices > 0)
    {
        sprintf(this->sql_cmd_buffer,
            "INSERT OR IGNORE INTO %s                   \
                (start_pos, end_pos, rawbytes, asm,     \
                    shape, nnz,                         \
                    row, column, value, valid_indices)  \
            VALUES                                      \
                (%d, %d, ?, ?, %d, %d, ?, ?, ?, ?);", 
            this->table_name.c_str(), left, right, matrix_size, nnz);                   
        if (SQLITE_OK != sqlite3_prepare(this->db, this->sql_cmd_buffer, strlen(this->sql_cmd_buffer), &stmt, NULL))
        {
            std::cout <<"Failed to prepare: " << sqlite3_errmsg(this->db) << std::endl;
            DIE();
        }

        if (SQLITE_OK != sqlite3_bind_text(stmt, 1, rawbytes.c_str(), rawbytes.length(), SQLITE_STATIC) ||
            SQLITE_OK != sqlite3_bind_text(stmt, 2, asm_str.c_str(),  asm_str.length(),  SQLITE_TRANSIENT))
            {
                std::cout <<"Failed to bind text row: " << sqlite3_errmsg(this->db) << std::endl;
                DIE();
            }

        if (SQLITE_OK != sqlite3_bind_blob(stmt, 3, row, sizeof(int)*nnz, SQLITE_STATIC) ||
            SQLITE_OK != sqlite3_bind_blob(stmt, 4, col, sizeof(int)*nnz, SQLITE_STATIC) ||
            SQLITE_OK != sqlite3_bind_blob(stmt, 5, value, sizeof(float)*nnz, SQLITE_STATIC) ||
            SQLITE_OK != sqlite3_bind_blob(stmt, 6, valid_indices, sizeof(int)*num_valid_indices, SQLITE_STATIC))
        {
            std::cout <<"Failed to bind rows: " << sqlite3_errmsg(this->db) << std::endl;
            DIE();
        }

        if (SQLITE_DONE != sqlite3_step(stmt))
        {
            std::cout <<"Failed to execute: " << sqlite3_errmsg(this->db) << std::endl;
            DIE();
        }

        if (!isTranscation) sqlite3_finalize(stmt);
        return;
    }

    // Should not reach here
    LOG("[E] Invalid nnz (%d) and num_valid_indices (%d)", nnz, num_valid_indices);
    DIE();
}

void DBCore::StoreFromDevice(cusparseHandle_t &handle,
        int left, int right, 
        cuSparseMatrix *matrix_ptr, 
        int num_valid_indices, const int *valid_indices)
{
    if (matrix_ptr->nnz > 0)
    {
        #ifdef TIME_MEASUREMENT
            auto begin_time = std::chrono::high_resolution_clock::now();
        #endif

        matrix_ptr->toCoo(handle, false);

        #ifdef TIME_MEASUREMENT
            auto end_time = std::chrono::high_resolution_clock::now();
            this->offload_count ++;
            this->offload_timer += end_time - begin_time;
        #endif
        if (!has_stream)
        {
            CHECK_CUSPARSE(cusparseGetStream(handle, &this->stream));
            has_stream = true;
        }
    }

    matrix_storage_row_t &storage_row = this->transaction_buffer[this->buffer_idx];
    storage_row.left = left;
    storage_row.right = right;
    storage_row.matrix_ptr = matrix_ptr;
    storage_row.num_valid_indices = num_valid_indices;
    storage_row.valid_indices = valid_indices;
    this->buffer_idx++;
    if (this->buffer_idx >= TRANSACTION_BUFFER_SIZE) 
    {
        this->Commit();
    }

}

void DBCore::Commit()
{
    if (this->buffer_idx == 0) return;

    // Begin a transaction
    char *errMsg = 0;
    int rc = sqlite3_exec(this->db, "BEGIN TRANSACTION;", NULL, NULL, &errMsg);
    if (UNLIKELY(rc != SQLITE_OK)) {
        LOG("[E] Error at starting a transaction: %s.\n", errMsg);
        sqlite3_free(errMsg);
        sqlite3_close(db);
        this->db = NULL;
        DIE();
    }

#ifdef DEBUG
    LOG("[I] Write %d rows to database.", this->buffer_idx);
#endif

    // Store buffer
    for (unsigned int i = 0; i<this->buffer_idx; i++)
    {
        auto &storage_row = this->transaction_buffer[i];
        
        if (storage_row.matrix_ptr->nnz > 0)
        {
            assert(this->has_stream);

            #ifdef TIME_MEASUREMENT
                auto begin_time = std::chrono::high_resolution_clock::now();
            #endif

            storage_row.matrix_ptr->toHost(this->stream);

            #ifdef TIME_MEASUREMENT
                auto end_time = std::chrono::high_resolution_clock::now();
                this->offload_timer += end_time - begin_time;
            #endif
        }

        #ifdef TIME_MEASUREMENT
            auto begin_time = std::chrono::high_resolution_clock::now();
        #endif

        this->Store(storage_row.left, storage_row.right, 
            storage_row.matrix_ptr->num_rows, 
            storage_row.matrix_ptr->nnz,
            storage_row.matrix_ptr->h_cooOffsets, 
            storage_row.matrix_ptr->h_columns, 
            storage_row.matrix_ptr->h_values,
            storage_row.num_valid_indices, 
            storage_row.valid_indices,
            "", "",
            true);
            
        #ifdef TIME_MEASUREMENT
            auto end_time = std::chrono::high_resolution_clock::now();
            this->store_count ++;
            this->store_timer += end_time - begin_time;
        #endif
        // Free ptrs
        delete storage_row.matrix_ptr;
        delete [] storage_row.valid_indices;
    }

    // End this transcation
    rc = sqlite3_exec(this->db, "END TRANSACTION;", NULL, NULL, &errMsg);
    if (UNLIKELY(rc != SQLITE_OK)) {
        LOG("[E] Error at ending a transaction: %s.\n", errMsg);
        sqlite3_free(errMsg);
        sqlite3_close(db);
        this->db = NULL;
        DIE();
    }

    // Reset buffer size
    this->buffer_idx = 0;
}

cuSparseMatrix *DBCore::LoadToDevice(cusparseHandle_t &handle, int left, int right, int &num_valid_indices, int *&valid_indices)
{
    sqlite3_stmt *stmt;
    valid_indices = 0;
    valid_indices = NULL;

    cuSparseMatrix *matrix_ptr = new cuSparseMatrix();

#ifdef TIME_MEASUREMENT
    auto begin_time = std::chrono::high_resolution_clock::now();
#endif

    sprintf(this->sql_cmd_buffer, 
        "SELECT                                     \
            shape, nnz,                             \
            row, column, value,                     \
            valid_indices                           \
        FROM %s                                     \
        WHERE                                       \
            start_pos = %d and end_pos = %d;",
        this->table_name.c_str(), left, right);    

    if (SQLITE_OK != sqlite3_prepare(this->db, this->sql_cmd_buffer, strlen(this->sql_cmd_buffer), &stmt, NULL))
    {
        std::cout <<"Failed to prepare: " << sqlite3_errmsg(this->db) << std::endl;
        DIE();
    }

    auto rc = sqlite3_step(stmt);
    if (SQLITE_ROW != rc && SQLITE_DONE != rc)
    {
        std::cout <<"Failed to execute: " << sqlite3_errmsg(this->db) << std::endl;
        DIE();
    }
    else  if (SQLITE_DONE == rc)
    {
        // This rule is not in database
        {
            #ifdef TIME_MEASUREMENT
                auto end_time = std::chrono::high_resolution_clock::now();
                this->load_fail_count ++;
                this->load_fail_timer += end_time - begin_time;
            #endif
        }
        return NULL;
    }

    int shape = 0;
    int nnz = 0;
    int *row = NULL;
    int *column = NULL;
    float *value = NULL;

    
    int loop_cnt = 0;
    do 
    {   
        // Shape, nnz
        shape = sqlite3_column_int(stmt, 0);
        nnz = sqlite3_column_int(stmt, 1);
        
        // Valid_indices
        unsigned int blob_size = sqlite3_column_bytes(stmt, 5);
        assert(blob_size % sizeof(int) == 0);
        num_valid_indices = blob_size/sizeof(int);
        if (num_valid_indices <= 0)
        {
            if (nnz <= 0)
            {
                {
                    #ifdef TIME_MEASUREMENT
                        auto end_time = std::chrono::high_resolution_clock::now();
                        this->load_count ++;
                        this->load_timer += end_time - begin_time;
                    #endif
                }
                matrix_ptr->fromDB(handle, shape, shape, nnz, NULL, NULL, NULL);
                return matrix_ptr;     
            }
            else
            {
                // Has nnz but no indices?
                LOG("[E] Rule without indices.");
                DIE();
            }
        } 
        const void *valid_indices_tmp = sqlite3_column_blob(stmt, 5);
        valid_indices = new int[num_valid_indices];
        memcpy(valid_indices, valid_indices_tmp, blob_size);

        // Row, Col, Value
        if (nnz <= 0)
        {
            {
                #ifdef TIME_MEASUREMENT
                    auto end_time = std::chrono::high_resolution_clock::now();
                    this->load_count ++;
                    this->load_timer += end_time - begin_time;
                #endif
            }
            matrix_ptr->fromDB(handle, shape, shape, nnz, NULL, NULL, NULL);
            return matrix_ptr;
        }

        blob_size = sqlite3_column_bytes(stmt, 2);
        assert(nnz * sizeof(int) == blob_size);
        const void *row_tmp = sqlite3_column_blob(stmt, 2);
        row = new int[nnz];
        memcpy(row, row_tmp, blob_size);

        blob_size = sqlite3_column_bytes(stmt, 3);
        assert(nnz * sizeof(int) == blob_size);
        const void *column_tmp = sqlite3_column_blob(stmt, 3);
        column = new int[nnz];
        memcpy(column, column_tmp, blob_size);

        blob_size = sqlite3_column_bytes(stmt, 4);
        assert(nnz * sizeof(float) == blob_size);
        const void *value_tmp = sqlite3_column_blob(stmt, 4);
        value = new float[nnz];
        memcpy(value, value_tmp, blob_size);

        // Update loop cnt
        loop_cnt++;
    } while(SQLITE_ROW == sqlite3_step(stmt));
    assert(loop_cnt == 1);
    sqlite3_finalize(stmt);

#ifdef TIME_MEASUREMENT
    auto end_time = std::chrono::high_resolution_clock::now();
    this->load_count ++;
    this->load_timer += end_time - begin_time;
#endif

#ifdef TIME_MEASUREMENT
    begin_time = std::chrono::high_resolution_clock::now();
#endif

    matrix_ptr->fromDB(handle, shape, shape, nnz, row, column, value);

#ifdef TIME_MEASUREMENT
    end_time = std::chrono::high_resolution_clock::now();
    this->onload_count ++;
    this->onload_timer += end_time - begin_time;
#endif


    delete [] row;
    delete [] column;
    delete [] value;

    return matrix_ptr;
}

int DBCore::GetTraceSize()
{
    sqlite3_stmt *stmt;

    sprintf(this->sql_cmd_buffer,
        "SELECT MAX(start_pos) from %s where (start_pos = end_pos);",
        this->table_name.c_str()); 
    if (SQLITE_OK != sqlite3_prepare(this->db, this->sql_cmd_buffer, strlen(this->sql_cmd_buffer), &stmt, NULL))
    {
        std::cout <<"Failed to prepare: " << sqlite3_errmsg(this->db) << std::endl;
        DIE();
    }
    if (SQLITE_ROW != sqlite3_step(stmt))
    {
        std::cout <<"Failed to get trace size: " << sqlite3_errmsg(this->db) << std::endl;
        DIE();
    }
    int rvalue = sqlite3_column_int(stmt, 0);
    assert(SQLITE_DONE == sqlite3_step(stmt));
    sqlite3_finalize(stmt);

    return rvalue;
}

int DBCore::GetSyscallId(const int instr_id)
{
    sqlite3_stmt *stmt;

    std::string syscall_table_name = this->table_name + "_sys";
    sprintf(this->sql_cmd_buffer,
        "SELECT COUNT(*) from %s WHERE (instr_id <= %d)",
        syscall_table_name.c_str(), instr_id); 
    if (SQLITE_OK != sqlite3_prepare(this->db, this->sql_cmd_buffer, strlen(this->sql_cmd_buffer), &stmt, NULL))
    {
        std::cout <<"Failed to prepare: " << sqlite3_errmsg(this->db) << std::endl;
        DIE();
    }
    if (SQLITE_ROW != sqlite3_step(stmt))
    {
        std::cout <<"Failed to get trace size: " << sqlite3_errmsg(this->db) << std::endl;
        DIE();
    }
    int rvalue = sqlite3_column_int(stmt, 0);
    assert(SQLITE_DONE == sqlite3_step(stmt));
    sqlite3_finalize(stmt);

    return rvalue;
}

bool DBCore::GetInfluenceRangeByInstrIdx(const int instr_id, int &length, int *&indices)
{
    sqlite3_stmt *stmt;

    sprintf(this->sql_cmd_buffer, 
        "SELECT                                     \
            valid_indices                           \
        FROM %s                                     \
        WHERE                                       \
            start_pos = %d and end_pos = %d;",
        this->table_name.c_str(), instr_id, instr_id);    

    if (SQLITE_OK != sqlite3_prepare(this->db, this->sql_cmd_buffer, strlen(this->sql_cmd_buffer), &stmt, NULL))
    {
        std::cout <<"Failed to prepare: " << sqlite3_errmsg(this->db) << std::endl;
        DIE();
    }

    auto rc = sqlite3_step(stmt);
    if (SQLITE_ROW != rc && SQLITE_DONE != rc)
    {
        std::cout <<"Failed to execute: " << sqlite3_errmsg(this->db) << std::endl;
        DIE();
    }
    else  if (SQLITE_DONE == rc)
    {
        // Rule not in db
        return false;
    }

    // Valid_indices
    unsigned int blob_size = sqlite3_column_bytes(stmt, 0);
    length = blob_size/sizeof(int);
    const void *valid_indices_tmp = sqlite3_column_blob(stmt, 0);
    indices = new int[length];
    memcpy(indices, valid_indices_tmp, blob_size);

    assert(SQLITE_DONE == sqlite3_step(stmt));
    sqlite3_finalize(stmt);
    return true;
}

int DBCore::GetSyscallsByname(std::string syscallName, syscall_t*&syscalls, int start, int end, int num /*=0*/)
{
    sqlite3_stmt *stmt;
    std::string syscall_table_name = this->table_name + "_sys";

    assert(num>=0);

    std::for_each(syscallName.begin(), syscallName.end(), [](char & c){
        c = ::tolower(c);
    });
    sprintf(this->sql_cmd_buffer,
        "SELECT                         \
            COUNT(*)                    \
        FROM %s                         \
        WHERE                           \
            name LIKE \"%s\"            \
            AND                         \
            instr_id >= %d              \
            AND                         \
            instr_id <= %d;",      
        syscall_table_name.c_str(), syscallName.c_str(), start, end); 

    if (SQLITE_OK != sqlite3_prepare(this->db, this->sql_cmd_buffer, strlen(this->sql_cmd_buffer), &stmt, NULL))
    {
        std::cout <<"Failed to prepare: " << sqlite3_errmsg(this->db) << std::endl;
        DIE();
    }
    if (SQLITE_ROW != sqlite3_step(stmt))
    {
        std::cout <<"Failed to get syscall number: " << sqlite3_errmsg(this->db) << std::endl;
        DIE();
    }
    int count = sqlite3_column_int(stmt, 0);
    assert(SQLITE_DONE == sqlite3_step(stmt));
    sqlite3_finalize(stmt);

    if (count == 0) return count;

    syscalls = new syscall_t[count];
    if (num == 0)
    {
    sprintf(this->sql_cmd_buffer,
        "SELECT                         \
            instr_id, syscall_id, name, \
            rvalue, nargs, args         \
        FROM %s                         \
        WHERE                           \
            name LIKE \"%s\"            \
            AND                         \
            instr_id >= %d              \
            AND                         \
            instr_id <= %d;",
        syscall_table_name.c_str(), syscallName.c_str(), start, end);    
    }
    else
    {
        sprintf(this->sql_cmd_buffer,
            "SELECT                         \
                instr_id, syscall_id, name, \
                rvalue ,nargs, args         \
            FROM %s                         \
            WHERE                           \
                name LIKE \"%s\"            \
                AND                         \
                instr_id >= %d              \
                AND                         \
                instr_id <= %d              \
            LIMIT 1                         \
            OFFSET %d;",
            syscall_table_name.c_str(), syscallName.c_str(), start, end, num-1); 
    }      

    if (SQLITE_OK != sqlite3_prepare(this->db, this->sql_cmd_buffer, strlen(this->sql_cmd_buffer), &stmt, NULL))
    {
        std::cout <<"Failed to prepare: " << sqlite3_errmsg(this->db) << std::endl;
        DIE();
    }
    if (SQLITE_ROW != sqlite3_step(stmt) && count>0)
    {
        std::cout <<"Failed to get syscall number: " << sqlite3_errmsg(this->db) << std::endl;
        DIE();
    }

    int idx = 0;
    do
    {
        syscall_t &this_syscall = syscalls[idx];
        
        this_syscall.instr_id = sqlite3_column_int(stmt, 0);
        assert(this_syscall.instr_id > 0);

        this_syscall.syscall_id = sqlite3_column_int(stmt, 1);

        const char *syscallname_tmp = (const char*)sqlite3_column_text(stmt, 2);
        strncpy( this_syscall.sys_name, syscallname_tmp, sizeof(this_syscall.sys_name) );

        this_syscall.rvalue = sqlite3_column_int(stmt, 3);
        this_syscall.nargs = sqlite3_column_int(stmt, 4);
        assert(this_syscall.nargs <= 6);

        auto blob_size = sqlite3_column_bytes(stmt, 5);
        assert((int)(this_syscall.nargs * sizeof(uint64_t)) == blob_size);
        const void *args_tmp = sqlite3_column_blob(stmt, 5);
        memcpy(this_syscall.args, args_tmp, blob_size);

        idx++;
    } while (SQLITE_ROW == sqlite3_step(stmt));
    sqlite3_finalize(stmt);

    if (num==0)
        assert(idx == count);
    else
        if (count>0) assert(idx == 1);
    
    return idx;
}

std::map<int, std::string> *DBCore::GetOffsetName(std::set<int>&offset)
{
    std::string vstate_table_name = this->table_name + "_vstate";
    std::map<int, std::string> *rmap = new std::map<int, std::string>();
    sqlite3_stmt *stmt;

    // Yes, I know this is slow :(
    // TODO: Optimization the symbol query from db.
    LOG("[I] Prepare for %lu printing symbols from database. This may take a while.", offset.size());

    for (const int &this_offset : offset)
    {   
        sprintf(this->sql_cmd_buffer,
            "SELECT                         \
                start, name                 \
            FROM %s                         \
            WHERE                           \
                start <= %d                 \
                AND                         \
                start+size > %d;",
            vstate_table_name.c_str(), this_offset, this_offset);
        if (SQLITE_OK != sqlite3_prepare(this->db, this->sql_cmd_buffer, strlen(this->sql_cmd_buffer), &stmt, NULL))
        {
            std::cout <<"Failed to prepare: " << sqlite3_errmsg(this->db) << std::endl;
            DIE();
        }
        int rc = sqlite3_step(stmt);
        if (SQLITE_DONE == rc)
        {
            std::cout << this_offset << " not found in db." << std::endl;
            DIE();
        }
        if (SQLITE_ROW != rc)
        {
            std::cout <<"Failed to get: " << sqlite3_errmsg(this->db) << std::endl;
            DIE();
        }
        int start = (sqlite3_column_int(stmt, 0));
        std::string offset_name((const char *)sqlite3_column_text(stmt, 1));
        assert(SQLITE_DONE == sqlite3_step(stmt));
        sqlite3_finalize(stmt);
        if(offset_name.find("0x") == std::string::npos)
        {
            // Mark reg offset
            offset_name += "[";
            offset_name += std::to_string(this_offset - start);
            offset_name += "]";
        }
        rmap->insert(std::pair<int,std::string>(this_offset, offset_name));

    }
    return rmap;
}


std::vector<int> DBCore::GetInstrIdxByASM(std::string asm_str, int start, int end)
{
    std::vector<int> rvector;
    sqlite3_stmt *stmt;
    sprintf(this->sql_cmd_buffer,
        "SELECT                         \
            start_pos                   \
        FROM %s                         \
        WHERE                           \
            asm LIKE \"%s%%\"           \
            AND start_pos = end_pos     \
            AND start_pos >= %d         \
            AND start_pos <= %d;",
    this->table_name.c_str(), asm_str.c_str(), start, end);
    if (SQLITE_OK != sqlite3_prepare(this->db, this->sql_cmd_buffer, strlen(this->sql_cmd_buffer), &stmt, NULL))
    {
        std::cout <<"Failed to prepare: " << sqlite3_errmsg(this->db) << std::endl;
        DIE();
    }
    int rc = sqlite3_step(stmt);
    if (SQLITE_DONE == rc)
    {
        return rvector;
    }
    if (SQLITE_ROW != rc)
    {
        std::cout <<"Failed to get: " << sqlite3_errmsg(this->db) << std::endl;
        DIE();
    }
    do
    { 
        rvector.push_back(sqlite3_column_int(stmt, 0));
    }
    while(SQLITE_ROW == sqlite3_step(stmt));
    sqlite3_finalize(stmt);
    return rvector;
}


void DBCore::SetMemInfluence(uint64_t base, uint64_t size, std::set<int> *influence_set_ptr)
{
    sqlite3_stmt *stmt;

    std::string vstate_table_name = this->table_name + "_vstate";
    sprintf(this->sql_cmd_buffer,
        "SELECT                         \
            start                       \
        FROM %s                         \
        WHERE                           \
            id >= %lu AND id < %lu      \
        LIMIT 1",
        vstate_table_name.c_str(), base, base+size);  

    if (SQLITE_OK != sqlite3_prepare(this->db, this->sql_cmd_buffer, strlen(this->sql_cmd_buffer), &stmt, NULL))
    {
        std::cout <<"Failed to prepare: " << sqlite3_errmsg(this->db) << std::endl;
        DIE();
    }
    int rc = sqlite3_step(stmt);
    if (SQLITE_DONE == rc)
    {
        LOG("[W] Memory [%p-%p] not found in db.", (void *)base, (void *)(base+size-1));
        sqlite3_finalize(stmt);
        return;
    }
    if (SQLITE_ROW != rc)
    {
        std::cout <<"Failed to get: " << sqlite3_errmsg(this->db) << std::endl;
        DIE();
    }
    int start = (sqlite3_column_int(stmt, 0));
    assert(SQLITE_DONE == sqlite3_step(stmt));
    sqlite3_finalize(stmt);

    sprintf(this->sql_cmd_buffer,
        "SELECT                         \
            start                       \
        FROM %s                         \
        WHERE                           \
            id >= %lu AND id < %lu      \
        ORDER BY id DESC                \
        LIMIT 1",
        vstate_table_name.c_str(), base, base+size);  

    if (SQLITE_OK != sqlite3_prepare(this->db, this->sql_cmd_buffer, strlen(this->sql_cmd_buffer), &stmt, NULL))
    {
        std::cout <<"Failed to prepare: " << sqlite3_errmsg(this->db) << std::endl;
        DIE();
    }
    rc = sqlite3_step(stmt);
    if (SQLITE_DONE == rc)
    {
        std::cout <<"Memory 0x" << std::hex << base+size-1 << " not found in db." << std::endl;
        DIE()
    }
    if (SQLITE_ROW != rc)
    {
        std::cout <<"Failed to get: " << sqlite3_errmsg(this->db) << std::endl;
        DIE();
    }
    int end = (sqlite3_column_int(stmt, 0));
    assert(SQLITE_DONE == sqlite3_step(stmt));
    sqlite3_finalize(stmt);

    assert(end >= start);
    for (int i=start; i<=end; i++)
    {
        influence_set_ptr->insert(i);
    }

#ifdef DEBUG
    if (uint64_t(end - start + 1) < size)
    {
        LOG("[W] %lu expected, but only %d found in db.", size, end-start+1);
    }
#endif

    return;
}


std::string DBCore::PrintSyscallByIdx(const int syscall_idx)
{
    assert(syscall_idx >= 0);

    sqlite3_stmt *stmt;
    std::string syscall_table_name = this->table_name + "_sys";
    if (syscall_idx == 0)
    {
        sprintf(this->sql_cmd_buffer,
            "SELECT                         \
                instr_id, syscall_id, name, \
                rvalue, nargs, args         \
            FROM %s;",
            syscall_table_name.c_str());   
    }
    else
    {
        sprintf(this->sql_cmd_buffer,
            "SELECT                         \
                instr_id, syscall_id, name, \
                rvalue, nargs, args         \
            FROM %s                         \
            LIMIT 1                         \
            OFFSET %d;",
            syscall_table_name.c_str(), syscall_idx-1);          
    }
    if (SQLITE_OK != sqlite3_prepare(this->db, this->sql_cmd_buffer, strlen(this->sql_cmd_buffer), &stmt, NULL))
    {
        std::cout <<"Failed to prepare: " << sqlite3_errmsg(this->db) << std::endl;
        DIE();
    }
    auto rc = sqlite3_step(stmt);
    if (SQLITE_DONE == rc)
    {
        if (syscall_idx == 0)
            return("No syscall found!\n");
        else
            return("Syscall index too large!\n");
    }
    if (SQLITE_ROW != rc)
    {
        std::cout <<"Failed to get trace size: " << sqlite3_errmsg(this->db) << std::endl;
        DIE();
    }
    int this_syscall_idx = (syscall_idx==0) ? 1 : syscall_idx;
    std::stringstream ss;
    
    do
    {
        syscall_t this_syscall;
        
        this_syscall.instr_id = sqlite3_column_int(stmt, 0);
        assert(this_syscall.instr_id > 0);

        this_syscall.syscall_id = sqlite3_column_int(stmt, 1);

        const char *syscallname_tmp = (const char*)sqlite3_column_text(stmt, 2);
        strncpy( this_syscall.sys_name, syscallname_tmp, sizeof(this_syscall.sys_name) );

        this_syscall.rvalue = sqlite3_column_int(stmt, 3);
        this_syscall.nargs = sqlite3_column_int(stmt, 4);
        assert(this_syscall.nargs <= 6);

        auto blob_size = sqlite3_column_bytes(stmt, 5);
        assert((int)(this_syscall.nargs * sizeof(uint64_t)) == blob_size);
        const void *args_tmp = sqlite3_column_blob(stmt, 5);
        memcpy(this_syscall.args, args_tmp, blob_size);

        ss << std::right << std::setw(7) << "[" + std::to_string(this_syscall_idx) + "]@";
        ss << std::left << std::setw(8) << std::to_string(this_syscall.instr_id);
        ss << std::left << std::setw(20) << std::string(this_syscall.sys_name)+"("+std::to_string(this_syscall.syscall_id)+")";
        if (this_syscall.rvalue == (uint64_t)-1)
            ss << " -1";
        else
            ss << " 0x" << std::hex << this_syscall.rvalue;
        ss << " = " <<this_syscall.sys_name << "(";
        for (uint64_t arg_id=0; arg_id<this_syscall.nargs; arg_id++)
        {
            ss << "0x" << std::hex << this_syscall.args[arg_id];
            if (arg_id+1 != this_syscall.nargs) ss << ", ";
        }
        ss << ")" << std::dec <<std::endl;

        this_syscall_idx++;
    } while (SQLITE_ROW == sqlite3_step(stmt));

    sqlite3_finalize(stmt);
    return ss.str();
}


void DBCore::StoreSyscall(syscall_t &syscall)
{
    sqlite3_stmt *stmt;
    std::string syscall_table_name = this->table_name + "_sys";
    sprintf(this->sql_cmd_buffer,
        "INSERT OR IGNORE INTO %s                   \
            (instr_id, syscall_id, name, nargs,     \
                args, rvalue)                       \
        VALUES                                      \
            (%d, %d, ?, %lu, ?, %lu);", 
        syscall_table_name.c_str(), syscall.instr_id, syscall.syscall_id, syscall.nargs, syscall.rvalue);                   
    if (SQLITE_OK != sqlite3_prepare(this->db, this->sql_cmd_buffer, strlen(this->sql_cmd_buffer), &stmt, NULL))
    {
        std::cout <<"Failed to prepare: " << sqlite3_errmsg(this->db) << std::endl;
        DIE();
    }

    if (SQLITE_OK != sqlite3_bind_text(stmt, 1, syscall.sys_name, strlen(syscall.sys_name), SQLITE_STATIC))
    {
        std::cout <<"Failed to bind text row: " << sqlite3_errmsg(this->db) << std::endl;
        DIE();
    }

    if (SQLITE_OK != sqlite3_bind_blob(stmt, 2, syscall.args, sizeof(uint64_t)*syscall.nargs, SQLITE_STATIC))
    {
        std::cout <<"Failed to bind rows: " << sqlite3_errmsg(this->db) << std::endl;
        DIE();
    }

    if (SQLITE_DONE != sqlite3_step(stmt))
    {
        std::cout <<"Failed to execute: " << sqlite3_errmsg(this->db) << std::endl;
        DIE();
    }

    sqlite3_finalize(stmt);
    return;
}

void DBCore::StoreVState(VState &vstate)
{
    sqlite3_stmt *stmt;
    std::string vstate_table_name = this->table_name + "_vstate";

    // Begin a transaction
    char *errMsg = 0;
    int rc = sqlite3_exec(this->db, "BEGIN TRANSACTION;", NULL, NULL, &errMsg);
    if (UNLIKELY(rc != SQLITE_OK)) {
        LOG("[E] Error at starting a transaction: %s.\n", errMsg);
        sqlite3_free(errMsg);
        sqlite3_close(db);
        this->db = NULL;
        DIE();
    }

    // Insert
    for (auto it = vstate.offset2id_map.begin(); it != vstate.offset2id_map.end(); it++)
    {
        auto start_offset = it->first;
        auto id = it->second;
        auto next_it = std::next(it);
        
        // LOG("ID: %lu @ %lu", id, start_offset);

        uint64_t this_size;
        if (next_it == vstate.offset2id_map.end())
            this_size = (vstate.size-1) - start_offset + 1;
        else
            this_size = next_it->first - start_offset;
        std::string this_name = VState::GetVStateNameByID(id);

        sprintf(this->sql_cmd_buffer,
            "INSERT OR IGNORE INTO %s                   \
                (start, size, name, id)                 \
            VALUES                                      \
                (%lu, %lu, ?, %lu);",
            vstate_table_name.c_str(), start_offset, this_size, id);            
        if (SQLITE_OK != sqlite3_prepare(this->db, this->sql_cmd_buffer, strlen(this->sql_cmd_buffer), &stmt, NULL))
        {
            std::cout <<"Failed to prepare: " << sqlite3_errmsg(this->db) << std::endl;
            DIE();
        }

        if (SQLITE_OK != sqlite3_bind_text(stmt, 1, this_name.c_str(), this_name.length(), SQLITE_STATIC))
        {
            std::cout <<"Failed to bind text row: " << sqlite3_errmsg(this->db) << std::endl;
            DIE();
        }
        if (SQLITE_DONE != sqlite3_step(stmt))
        {
            std::cout <<"Failed to execute: " << sqlite3_errmsg(this->db) << std::endl;
            DIE();
        }
    }

    // End this transcation
    rc = sqlite3_exec(this->db, "END TRANSACTION;", NULL, NULL, &errMsg);
    if (UNLIKELY(rc != SQLITE_OK)) {
        LOG("[E] Error at ending a transaction: %s.\n", errMsg);
        sqlite3_free(errMsg);
        sqlite3_close(db);
        this->db = NULL;
        DIE();
    }

    return;
}

void DBCore::StartTransaction()
{
    char *errMsg = 0;
    int rc = sqlite3_exec(this->db, "BEGIN TRANSACTION;", NULL, NULL, &errMsg);
    if (UNLIKELY(rc != SQLITE_OK)) {
        LOG("[E] Error at starting a transaction: %s.\n", errMsg);
        sqlite3_free(errMsg);
        sqlite3_close(db);
        this->db = NULL;
        DIE();
    }
}

void DBCore::EndTransaction()
{
    char *errMsg = 0;
    int rc = sqlite3_exec(this->db, "END TRANSACTION;", NULL, NULL, &errMsg);
    if (UNLIKELY(rc != SQLITE_OK)) {
        LOG("[E] Error at ending a transaction: %s.\n", errMsg);
        sqlite3_free(errMsg);
        sqlite3_close(db);
        this->db = NULL;
        DIE();
    }
}

std::string DBCore::PrintTrace(const int instr_id, const int len /*= -1*/)
{
    std::stringstream ss;
    int traceSize = this->GetTraceSize();
    int length, start_pos;
    if (len == -1)
    {
        start_pos = std::max(1, instr_id-3);
        length = 7 + instr_id - start_pos;
    }
    else
    {
        length = len;
        start_pos = instr_id;
    }

    for (int i=0; i<length; i++)
    {
        int this_instr_id = start_pos+i;
        if (this_instr_id > traceSize) 
            break;
        
        sqlite3_stmt *stmt;
        sprintf(this->sql_cmd_buffer,
            "SELECT rawbytes, asm FROM %s WHERE (start_pos = %d and end_pos = %d);",
            this->table_name.c_str(), this_instr_id, this_instr_id); 
        if (SQLITE_OK != sqlite3_prepare(this->db, this->sql_cmd_buffer, strlen(this->sql_cmd_buffer), &stmt, NULL))
        {
            std::cout <<"Failed to prepare: " << sqlite3_errmsg(this->db) << std::endl;
            DIE();
        }
        int rc = sqlite3_step(stmt);
        if (rc == SQLITE_DONE)
        {
            ss << std::setw(9) << this_instr_id << ":";
            ss << std::left << std::setw(30) << "(No record)" << std::endl;
            continue;
        }
        if (SQLITE_ROW != rc)
        {
            std::cout <<"Failed to get row: " << sqlite3_errmsg(this->db) << std::endl;
            DIE();
        }
        std::string rawbytes_str((const char*)sqlite3_column_text(stmt, 0));
        std::string asm_str((const char*)sqlite3_column_text(stmt, 1));
        assert(SQLITE_DONE == sqlite3_step(stmt));
        sqlite3_finalize(stmt);
        if (len == -1 && this_instr_id == instr_id)
            ss << std::right << std::setw(8) << std::string("==>") + std::to_string(this_instr_id);
        else
            ss << std::right << std::setw(8) << this_instr_id;
        ss << ":\t" << std::left << std::setw(15) <<  rawbytes_str;
        ss << std::setw(32) << asm_str << std::endl;
    }
    return ss.str();
}