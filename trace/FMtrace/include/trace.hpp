#ifndef __FLOWMATRIX_TRACE_H__
#define __FLOWMATRIX_TRACE_H__

#define RAW_BYTE_MAX_LEN 16
#define MEM_REF_MAX_NUM 4
#define SYS_CALL_NAME_MAX_LEN 32
#define BUFFER_SIZE 8192

#include <common.hpp>

namespace FlowMatrix{

typedef struct {
    uint32_t instr_id;
    uint32_t instr_len;
    uint32_t mem_ref_num;
    uint64_t rflags;
    uint8_t raw_bytes[RAW_BYTE_MAX_LEN];
    uint64_t mem_ref_addr[MEM_REF_MAX_NUM];

    void Print();
} instr_t;

typedef struct {
    uint32_t instr_id;
    uint32_t syscall_id;
    uint64_t rvalue;
    uint64_t nargs;
    uint64_t args[6];
    char sys_name[SYS_CALL_NAME_MAX_LEN];

    void Print();
} syscall_t;

typedef struct _mem_ref_t {
    uint64_t mem_ref_addr;

    bool operator < (const _mem_ref_t &ref) const{
        return (this->mem_ref_addr < ref.mem_ref_addr);
    }

    void Print();
} mem_ref_t;

template<class FrameType>
class TraceHandle
{
private:
    FILE *trace_file;
    FrameType *buffer, *buffer_head;
    size_t buffer_size;
    bool is_read_mode;

    size_t FlushBuffer();
    size_t LoadBuffer();

public:
    void CreateTrace(const std::string &trace_path);
    void LoadTrace(const std::string &trace_path);
    void CloseTrace();

    void WriteTrace(const FrameType &frame);
    size_t ReadNext(FrameType &frame);

    TraceHandle();
    ~TraceHandle();
};

template class TraceHandle<instr_t>;
template class TraceHandle<syscall_t>;
template class TraceHandle<mem_ref_t>;


typedef struct {
    TraceHandle<instr_t> instr_trace;
    TraceHandle<syscall_t> syscall_trace;
    TraceHandle<mem_ref_t> mem_ref_set;
} trace_handle_t;

class Trace
{
public:
    // TODO: Support mmap-style File I/O to gain further speedup.
    static trace_handle_t *CreateTrace(const std::string &trace_name);
    static trace_handle_t *LoadTrace(const std::string &trace_name);
    static void CloseTrace(trace_handle_t *trace_handle);
};

}; // End of FlowMatrix namespace

#endif