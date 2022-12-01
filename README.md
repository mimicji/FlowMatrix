# FlowMatrix
A query-style tool for GPU-assisted offline information-flow analysis.

## Cite
> Kaihang Ji, Jun Zeng, Yuancheng Jiang, and Zhenkai Liang, Zheng Leong Chua, Prateek Saxena and Abhik Roychoudhury, *FlowMatrix: GPU-Assisted Information-Flow Analysis through Matrix-Based Representation*.  In *USENIX Security Symposium*, 2022.

## Dependencies
* [GCC](https://gcc.gnu.org/) >= 7.5
* [CUDA Toolkit](https://developer.nvidia.com/cuda-toolkit) >= 11.3
* [Capstone](https://www.capstone-engine.org) >= 4.0
* [SQLite](https://www.sqlite.org/index.html) for data storage and management.
* [Protocol Buffers](https://developers.google.com/protocol-buffers) for rule processing.
* [CLI](https://github.com/daniele77/cli) for CLI interface.
* [TaintInduce](https://www.ndss-symposium.org/wp-content/uploads/2019/02/ndss2019_05A-2_Chua_paper.pdf) for taint rule generation.
## Installation
Simply build with make at the project root directory, use:

    $ make -j
## Usage
    $ ./bin/QueryCLI <path_to_database>
### 1. Examples
Load a database:

    $ ./bin/QueryCLI ./examples.db
    
Example of sample1: data flows of two 'mov' instructions. Check [sample1](examples/sample1/) for more details.

    FMQuery> WorkOn sample1
    
    FMQuery> Query INSTR 1 TO INSTR 2

Example of sample2: data flows between buffer read and buffer write. Check [sample2](examples/sample2/) for more details.

    FMQuery> WorkOn sample2
    
    FMQuery> Query SYSCALL read TO SYSCALL write
    
### 2. Advanced System Call Query
Query data flows among all common source syscalls to all common sink syscalls:

    FMQuery> Query SYSCALL read,recv*,mmap TO SYSCALL write,send*
    
That might print multiple records as it performs a n-to-m query. You might want to narrow down the query range by first checking the system call trace:

    FMQuery> SyscallPrintAll
    ...
    [40]@119916  read(0)              0x22a = read(0x8, 0x7fff9479ae20, 0x400)
    ...
    [47]@184288  write(1)             0x22a = write(0x8, 0x7fff9479ae20, 0x22a)
    ...
    
"119916" and "184288" are their instruction index respectively. Thus, you can perfrom query upon a subset of the trace:

    FMQuery> QueryInRange (119916,184288) SYSCALL read,recv*,mmap TO SYSCALL write,send*

Or, just query the data flow between these two syscall instructions:

    FMQuery> Query INSTR 119916 TO INSTR 184288    

### 3. Advanced Instruction Query
System call queries can be combained with instruction queries.

For example, query data flows from all common source syscalls to all return instructions 
(usually for checking whether RIP is influenced by any source syscall): 

    FMQuery> Query SYSCALL read,recv* TO INSTR ret

Also, you may print instruction traces for better understanding (for example, the 999th instruction in Sample2 is a RET instruction):

    FMQuery> TracePrint 999
       996:       488d0490       lea      rax, qword ptr [rax + rdx*4]
       997:       898ff4020000   mov      dword ptr [rdi + 0x2f4], ecx
       998:       48898708030000 mov      qword ptr [rdi + 0x308], rax
    ==>999:       c3             ret
      1000:       488d0521892200 lea      rax, qword ptr [rip + 0x228921]
      1001:       48890542892200 mov      qword ptr [rip + 0x228942], rax
      1002:       488d0523dfffff lea      rax, qword ptr [rip - 0x20dd]
      1003:       4889054c8c2200 mov      qword ptr [rip + 0x228c4c], rax
      1004:       488d0585902200 lea      rax, qword ptr [rip + 0x229085]
      1005:       488905468c2200 mov      qword ptr [rip + 0x228c46], rax

For details about the dataflows of a specified instruction, please use command "PrintOne":

    FMQuery> PrintOne 999
    0x7fff9479b288 -> RIP[0]  0x7fff9479b288
    0x7fff9479b289 -> RIP[1]  0x7fff9479b289
    0x7fff9479b28a -> RIP[2]  0x7fff9479b28a
    0x7fff9479b28b -> RIP[3]  0x7fff9479b28b
    0x7fff9479b28c -> RIP[4]  0x7fff9479b28c
    0x7fff9479b28d -> RIP[5]  0x7fff9479b28d
    0x7fff9479b28e -> RIP[6]  0x7fff9479b28e
    0x7fff9479b28f -> RIP[7]  0x7fff9479b28f
    RSP[0] -> RSP[0]
    RSP[1] -> RSP[1]  RSP[2]  RSP[3]  RSP[4]  RSP[5]  RSP[6]  RSP[7]
    RSP[2] -> RSP[2]  RSP[3]  RSP[4]  RSP[5]  RSP[6]  RSP[7]
    RSP[3] -> RSP[3]  RSP[4]  RSP[5]  RSP[6]  RSP[7]
    RSP[4] -> RSP[4]  RSP[5]  RSP[6]  RSP[7]
    RSP[5] -> RSP[5]  RSP[6]  RSP[7]
    RSP[6] -> RSP[6]  RSP[7]
    RSP[7] -> RSP[7]

### 4. Forward analysis and Backward analysis
In some cases, analysts wonder the dataflow at certain location. 

For example, in our case study, we wonder whether the return value of a specified function affects 
the final sink or not. Normal query doesn't satisfy our need because a RET instruction itself doesn't 
include return value (RAX register) in its variable state, as shown above. In this case, we need a
backward query:

    FMQuery> Query INSTR <RET_INSTR_IDX> BACKWARD INSTR <SINK_INSTR_IDX>
    ...
    RAX[0] -> xxx
    ...
    
*<RET_INSTR_IDX> and <SINK_INSTR_IDX> are the indices of a RET instruction and the final sink instruction respectively. 
Please refer to our paper for details of this case study.

Forward analysis is powerful and frequently used in our experiments including case studies, ditto. 
Here we give an example of Out-Of-Bounds(OOB) write analysis.

Senario OOB write: A buffer overflow occurs when the program copies data from a input file by memcpy(). 

    FMQuery> Query SYSCALL read[2] FORWARD INSTR call[53]   # memcpy()    
    ...
    RAX -> ... RDX ...
    ...
    
Also, the return value (RAX) of a READ syscall is the num of read bytes. 
It affects the third argument (RDX) of memcpy(), which is number of bytes to copy.
Then we verify dataflows from read syscall to the memcpy dst buffer and the existence of OOB.

    FMQuery> Query SYSCALL read[2] FORWARD INSTR ret[49]   # return from memcpy()    
    ...
    0x7fff9479b048 -> 0x7faebe7e53c0 ...
    0x7fff9479b049 -> 0x7faebe7e53c1 ...   # Overflow!
    ...

## Query Usage
Query Usage:

    Query <Vars1Type> <Vars1> <Direction> <Vars2Type> <Vars2>
    Arguments:
        Vars1Type, Vars2Type: {S(yscall), I(nstr), R(egister), M(emory)}.
        Vars1, Vars2: 
            (if Type is SYSCALL) : {SyscallName, SyscallName[id]}. E.g., "read", "read[1]", "read,recv*", "recv*"
            (if Type is INSTR)   : {InstrIndexInTrace, Instr}. E.g., "1", "ret", "push,pop", "mov*rax*"
            (if Type is REGISTER): {RegisterName}. E.g., "rip", "xmm1,xmm2,xmm3,xmm4"
            (if Type is MEMORY)  : {MemAddr}. E.g., "0x800000", "0x800000-0x801000", "0x0-0x1,0x4-0x5,0x8"
        Direction: {T(o), F(orward), B(ackward)}
            TO      : Query dataflows from Vars1 to Vars2
            FORWARD : Pefrom forward analysis. Query dataflows from Vars1 to positions of Vars2
            BACKWARD: Pefrom backward analysis. Query dataflows from positions of Vars1 to Vars2
    *All IDs start from 1. '*' is the wildcard for advanced matching.

QueryInRange Usage:

    QueryInRange <Range> <Vars1Type> <Vars1> <Direction> <Vars2Type> <Vars2>
    Arguments:
        Range: (src,dest).
        (Others are the same with Query command.)

## Usage for all commands
     FMQuery> help
     - help
            This help message
     - exit
            Quit the session
     - WhichDB
            Show which database is worked on.
     - ListTraces
            List all traces in database.
     - WorkOn <string>
            Select which trace to work on.
     - TraceSize
            Show size of the current trace.
     - PrepareQuery <int> <int>
            Pre-compute intermedia query results for later query on range (start, end).
     - Query <string> <string> <string> <string> <string>
            Perform data flow query. E.g. Query SYSCALL read,recv TOWARDS REG rip
     - QueryInRange <string> <string> <string> <string> <string> <string>
            Perform data flow query with-in a sub range
     - MultiplyTwo <int> <int>
            Multiply specified two instrutions.
     - TracePrint <int>
            Print one instruction from trace by instruction index
     - TracePrintRange <int> <int>
            Print a list of instructions.
     - PrintOne <int>
            Print data flows of one instruction by index.
     - SyscallPrintAll
            List all syscalls in trace.
     - SyscallPrint <int>
            Print one syscall in trace by its index.
     - LastResult
            Print the raw matrix of the last one-to-one query.
     - Usage <string>
            Show usage of the specified command.
     - Settings
            (menu)
