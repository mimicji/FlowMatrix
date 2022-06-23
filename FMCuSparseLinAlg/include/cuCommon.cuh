#ifndef __FLOWMATRIX_UTILS_CU_COMMON_H_
#define __FLOWMATRIX_UTILS_CU_COMMON_H_

#include <cuda_runtime_api.h> 
#include <cusparse.h>  
#include <cuda_fp16.h>       
#include <stdio.h>            
#include <stdlib.h> 
#include <assert.h>        
#include <iostream>
#include <chrono>

// #define DEBUG
#define TIME_MEASUREMENT

#define BUFFER_INIT_SIZE 204800000

#ifndef PYTHON_FLOWMATRIX
    typedef int32_t index_type_t;
    typedef float value_type_t;
    #define COMPUTE_TYPE CUDA_R_32F
    #define float2value(f) (f)
    #define value2float(h) (h)
#else
    // typedef int64_t index_type_t;
    // typedef float value_type_t;
    // #define COMPUTE_TYPE CUDA_R_32F
    // #define float2value(f) (f)
    // #define value2float(h) (h)
#endif

#define LIKELY(x) __builtin_expect(!!(x), 1)
#define UNLIKELY(x) __builtin_expect(!!(x), 0)
#define CHECK_CUDA(call)                                                       \
{                                                                              \
    const cudaError_t error = call;                                            \
    if (UNLIKELY(error != cudaSuccess))                                        \
    {                                                                          \
        fprintf(stderr, "Error: %s:%d, ", __FILE__, __LINE__);                 \
        fprintf(stderr, "code: %d, reason: %s\n", error,                       \
                cudaGetErrorString(error));                                    \
    }                                                                          \
}


#define CHECK_CUSPARSE(call)                                                   \
{                                                                              \
    cusparseStatus_t err;                                                      \
    if (UNLIKELY((err = (call)) != CUSPARSE_STATUS_SUCCESS))                   \
    {                                                                          \
        fprintf(stderr, "Got error %d (%s) at %s:%d\n", err,                   \
            cusparseGetErrorName(err), __FILE__, __LINE__);                    \
        cudaError_t cuda_err = cudaGetLastError();                             \
        if (cuda_err != cudaSuccess)                                           \
        {                                                                      \
            fprintf(stderr, "  CUDA error \"%s\" also detected\n",             \
                    cudaGetErrorString(cuda_err));                             \
        }                                                                      \
        exit(1);                                                               \
    }                                                                          \
}

inline bool CHECK_CUSPARSE_NO_EXIT(const cusparseStatus_t &err)
{
    if (UNLIKELY((err != CUSPARSE_STATUS_SUCCESS)))                   
    {                                                                          
        fprintf(stderr, "Got error %d (%s) at %s:%d\n", err,                   
            cusparseGetErrorName(err), __FILE__, __LINE__);                    
        cudaError_t cuda_err = cudaGetLastError();                             
        if (cuda_err != cudaSuccess)                                           
        {                                                                      
            fprintf(stderr, "  CUDA error \"%s\" also detected\n",             
                    cudaGetErrorString(cuda_err));                             
        }
        return false;                                                                
    }
    return true;                                                          
}

#endif // __FLOWMATRIX_UTILS_CU_COMMON_H_