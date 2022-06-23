#ifndef FLOWMATRIX_SYSCALL_HOOK_HPP
#define FLOWMATRIX_SYSCALL_HOOK_HPP

#include <trace.hpp>
#include <sys/socket.h>
#include <trace.hpp>

#define SYSCALL_READ        0
#define SYSCALL_WRITE       1
#define SYSCALL_MMAP        9
#define SYSCALL_PREAD64     17
#define SYSCALL_PWRITE64    18
#define SYSCALL_SENDTO      44
#define SYSCALL_RECVFROM    45
#define SYSCALL_SENDMSG     46
#define SYSCALL_RECVMSG     47
#define SYSCALL_RECVMMSG    299
#define SYSCALL_SENDMMSG    307

using FlowMatrix::syscall_t;
namespace FlowMatrix{
bool get_syscall_buffer(
    const syscall_t &this_syscall, 
    uint64_t &buffer_base, 
    uint64_t &buffer_size,
    bool &isSource)
{
    buffer_base = (uint64_t) -1;
    buffer_size = (uint64_t) -1;
    switch (this_syscall.syscall_id)
    {
        // Source syscalls
        case SYSCALL_READ:
        case SYSCALL_RECVFROM:
        case SYSCALL_RECVMSG:
        case SYSCALL_PREAD64:
            isSource = true;
            if (this_syscall.rvalue != (uint64_t) -1)
            {
                buffer_base = this_syscall.args[1];
                buffer_size = this_syscall.rvalue;
            }
            return true;            
        case SYSCALL_RECVMMSG:
            isSource = true;
            if (this_syscall.rvalue != (uint64_t) -1)
            {
                buffer_base = this_syscall.args[1];
                buffer_size = this_syscall.rvalue * sizeof(struct mmsghdr);
            }
            return true;            
        case SYSCALL_MMAP:
            isSource = true;
            if (this_syscall.rvalue != (uint64_t) -1)
            {
                buffer_base = this_syscall.rvalue;
                buffer_size = this_syscall.args[1];
            }
            return true; 
            
        // Sink syscalls
        case SYSCALL_WRITE:
        case SYSCALL_SENDTO:
        case SYSCALL_SENDMSG: 
        case SYSCALL_PWRITE64:
            isSource = false;
            if (this_syscall.rvalue != (uint64_t) -1)
            {
                buffer_base = this_syscall.args[1];
                buffer_size = this_syscall.rvalue;
            }
            return true;   
        case SYSCALL_SENDMMSG:
            isSource = false;
            if (this_syscall.rvalue != (uint64_t) -1)
            {
                buffer_base = this_syscall.args[1];
                buffer_size = this_syscall.rvalue * sizeof(struct mmsghdr);
            }
            return true;
        
        // Not implemented syscalls :(
        default:
            return false;
    }
}
}       // Namespace FlowMatrix
#endif  // FLOWMATRIX_SYSCALL_HOOK_HPP