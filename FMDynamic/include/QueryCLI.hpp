#ifndef FLOWMATRIX_QUERY_CLI_HPP
#define FLOWMATRIX_QUERY_CLI_HPP

#include <cli/cli.h>
#include <cli/clifilesession.h>
#include <FMQuery.hpp>
#include <QueryCLI.hpp>
#include <SyscallHook.hpp>

#define FLOAT_THRESHOLD (0.5f)

/********************************/
/*         Structures           */
/********************************/
typedef enum _vars_type_t : unsigned char
{ 
    SYSCALL = 1, 
    INSTR = 2, 
    REGISTER = 3,
    MEMORY = 4,
} vars_type_t;

/********************************/
/*       Utility Functions      */
/********************************/
void WorkOn(std::ostream& out, std::string &_traceName);
void RemoveCache(std::ostream& out);
void InitQueryRange(std::ostream &out, int start, int end);
void PrepareQuery(std::ostream &out, int level);
void QueryInRange(
    std::ostream& out, 
    std::string _range,
    std::string _src_type,
    std::string _srcs,
    std::string _direction,
    std::string _dest_type,
    std::string _dests);
void Query(
    std::ostream& out, 
    std::string _src_type,
    std::string _srcs,
    std::string _direction,
    std::string _dest_type,
    std::string _dests);

/********************************/
/*    Trace-Related Functions   */
/********************************/
void TracePrint(std::ostream &out, const int pos);
void TracePrintRange(std::ostream &out, const int pos, const int length);
void SyscallPrintAll(std::ostream &out);
void SyscallPrint(std::ostream &out, int index);

/********************************/
/*      Setting Functions       */
/********************************/
void SetStorageLevel(std::ostream& out, int level);
void SetDB(std::ostream& out, std::string &_dbPath);

/********************************/
/*    Print Info Functions      */
/********************************/
void WhichDB(std::ostream& out);
void ListTraces(std::ostream& out);
void TraceSize(std::ostream& out);
void PrintLastResult(std::ostream& out);
void PrintSingleInstr(std::ostream& out, int instr_id);

/********************************/
/*       Helper Functions       */
/********************************/
void UsageMenu(std::ostream& out, std::string cmd);
void SubQueryHelper(std::ostream& out);
void QueryHelper(std::ostream& out);
void QueryInRangeHelper(std::ostream& out);

/********************************/
/*    Query Assist Functions    */
/********************************/
void MakeSourceSinkSet(std::map<int, std::set<int>*> &source_sink_list, int instr_id, int len, int* indices);
int AddSyscallsToSourceSinkSet(std::map<int, std::set<int>*> &source_sink_list, int num_syscalls, syscall_t* syscalls);
void ParseVars(vars_type_t type, std::string vars, int start, int end, std::map<int, std::set<int>*> &source_sink_list);

/********************************/
/*      Checker Functions       */
/********************************/
bool _Check_DB(std::ostream& out);
bool _Check_Trace(std::ostream& out);
bool _Check_Query(std::ostream& out);

#endif