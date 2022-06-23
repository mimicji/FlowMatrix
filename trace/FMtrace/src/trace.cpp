#include <trace.hpp>

template<class FrameType>
size_t FlowMatrix::TraceHandle<FrameType>::FlushBuffer()
{
    size_t rvalue = fwrite(this->buffer,sizeof(FrameType),this->buffer_size,this->trace_file);
    assert(rvalue == this->buffer_size);
    this->buffer_head = this->buffer;
    this->buffer_size = 0;
    return rvalue;
}

template<class FrameType>
size_t FlowMatrix::TraceHandle<FrameType>::LoadBuffer()
{
    this->buffer_size = fread(this->buffer,sizeof(FrameType),BUFFER_SIZE,this->trace_file);
    this->buffer_head = this->buffer;
    return this->buffer_size;
}

template<class FrameType>
void FlowMatrix::TraceHandle<FrameType>::CreateTrace(const std::string &trace_path)
{
    this->is_read_mode = false;
    this->trace_file = fopen(trace_path.c_str(), "wb");
    if (this->trace_file == NULL)
    {
        LOG("[E] Unable to open %s. Terminate.", trace_path.c_str());
        DIE();
    }
    this->buffer = new FrameType[BUFFER_SIZE];
    this->buffer_head = this->buffer;
    this->buffer_size = 0;
}

template<class FrameType>
void FlowMatrix::TraceHandle<FrameType>::LoadTrace(const std::string &trace_path)
{
    this->is_read_mode = true;
    this->trace_file = fopen(trace_path.c_str(), "rb");
    if (this->trace_file == NULL)
    {
        LOG("[E] Unable to open %s. Terminate.", trace_path.c_str());
        DIE();
    }
    this->buffer = new FrameType[BUFFER_SIZE];
    this->buffer_head = this->buffer;
    this->buffer_size = 0;
}

template<class FrameType>
void FlowMatrix::TraceHandle<FrameType>::CloseTrace()
{
    if (this->buffer!=NULL)
    {
        // Flush the write buffer in write mode
        if (!this->is_read_mode) this->FlushBuffer();
        // Free buffer
        delete[] this->buffer;
    }

    // Close file
    if (this->trace_file!=NULL)
    {
        fclose(this->trace_file);
    }

    // Set every ptr to NULL
    this->buffer = NULL;
    this->buffer_head = NULL;
    this->trace_file = NULL;
}

template<class FrameType>
void FlowMatrix::TraceHandle<FrameType>::WriteTrace(const FrameType &frame)
{
    std::memcpy(this->buffer_head, &frame, sizeof(FrameType));
    this->buffer_head++;
    this->buffer_size++;
    if (buffer_size >= BUFFER_SIZE) this->FlushBuffer();
}

template<class FrameType>
size_t FlowMatrix::TraceHandle<FrameType>::ReadNext(FrameType &frame)
{
    if (this->buffer + this->buffer_size == this->buffer_head)
    {
        // Buffer is empty. Load more from file.
        this->LoadBuffer();
        if (this->buffer_size == 0)
        {
            // EOF. Return with 0;
            return 0;
        }
    }
    std::memcpy(&frame, this->buffer_head, sizeof(FrameType));
    this->buffer_head++;
    return 1;
}

template<class FrameType>
FlowMatrix::TraceHandle<FrameType>::TraceHandle()
{
    // do nothing but init with 0
    this->buffer = NULL;
    this->buffer_head = NULL;
    this->trace_file = NULL;
    this->buffer_size = 0;
    this->is_read_mode = false;
}

template<class FrameType>
FlowMatrix::TraceHandle<FrameType>::~TraceHandle()
{
    // Safe deconstruction. Just in case users forgot to call CloseTrace()
    this->CloseTrace();
}


FlowMatrix::trace_handle_t *FlowMatrix::Trace::CreateTrace(const std::string &trace_name)
{
    mkdir(trace_name.c_str(), 0777);
    trace_handle_t *handle = new(trace_handle_t);
    std::string instr_trace_name = trace_name + "/instr.fmtrace";
    std::string sys_trace_name = trace_name + "/sys.fmtrace";
    std::string mem_ref_trace_name = trace_name + "/mem_ref.fmtrace";
    std::string log_path = trace_name + "/log.txt";
    handle->instr_trace.CreateTrace(instr_trace_name);
    handle->syscall_trace.CreateTrace(sys_trace_name);
    handle->mem_ref_set.CreateTrace(mem_ref_trace_name);
    assert(freopen(log_path.c_str(),"w", stderr)!=NULL);
    return handle;
}

FlowMatrix::trace_handle_t *FlowMatrix::Trace::LoadTrace(const std::string &trace_name)
{
    trace_handle_t *handle = new(trace_handle_t);
    std::string instr_trace_name = trace_name + "/instr.fmtrace";
    std::string sys_trace_name = trace_name + "/sys.fmtrace";
    std::string mem_ref_trace_name = trace_name + "/mem_ref.fmtrace";
    std::string log_path = trace_name + "/log.txt";
    handle->instr_trace.LoadTrace(instr_trace_name);
    handle->syscall_trace.LoadTrace(sys_trace_name);
    handle->mem_ref_set.LoadTrace(mem_ref_trace_name);
    assert(freopen(log_path.c_str(),"w", stderr)!=NULL);
    return handle;
}

void FlowMatrix::Trace::CloseTrace(trace_handle_t *trace_handle)
{
    trace_handle->instr_trace.CloseTrace();
    trace_handle->syscall_trace.CloseTrace();
    delete trace_handle;
}

void FlowMatrix::instr_t::Print()
{
    std::cout<<"["<<this->instr_id<<"] ";
    std::string rawbyte_str = uint8_to_hex_string(this->raw_bytes, this->instr_len);
    std::cout<<rawbyte_str<<"\t";
    std::string asm_str;
    disassemble_one_instr_x86(this->raw_bytes, this->instr_len, asm_str);
    std::cout << asm_str <<" (rflags=0x"<<std::hex<<this->rflags<<std::dec<<")"<<std::endl;
    if (this->mem_ref_num>0)
    {
        std::cout<<std::hex;
        for (uint32_t idx=0; idx<this->mem_ref_num; idx++)
        {
            std::cout<<"  Mem[" << idx<< "]: 0x"<<this->mem_ref_addr[idx]<<std::endl;
        }
        std::cout<<std::dec;
    }
}

void FlowMatrix::syscall_t::Print()
{
    std::cout<<"["<<this->instr_id<<"]Syscall "<<this->sys_name 
             << "(" <<this->syscall_id << ")" << std::endl;
    for (uint64_t idx=0; idx<this->nargs; idx++)
    {
        std::cout<<"  Arg " << idx <<": "<<this->args[idx]<<std::endl;
    } 
}

void FlowMatrix::mem_ref_t::Print()
{
    std::cout<<"0x" << std::hex << this->mem_ref_addr << std::dec << std::endl;
}

