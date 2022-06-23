#include <QueryCLI.hpp>


using namespace cli;
using namespace std;
using namespace FlowMatrix;


std::string dbPath, traceName = "";
DBCore *db = NULL;
FMQuery *query = NULL;
cuSparseMatrix *lastResult = NULL;
int process_limit = 1;

/************************************/
/*            Configs               */
/************************************/
int storageLevel = 1;

bool _Check_DB(std::ostream& out)
{
    if (db == NULL)
    {
        if (dbPath != "")
        {
            db = new DBCore(dbPath);
        }
        else
        {
            out << "DB is not set. Call \"SetDB\" to set DB first.\n";
            return false;
        }
    }
    return true;
}

bool _Check_Trace(std::ostream& out)
{
    if (traceName != "")
    {
        return true;
    }
    else
    {
        out << "Trace is not selected. Call \"WorkOn\" to select a trace first.\n";
        return false;
    }
}

bool _Check_Query(std::ostream& out)
{
    if (query != NULL)
    {
        return true;
    }
    else
    {
        out << "FMQuery range is not set. Please set first.\n";
        return false;
    }
}

void SetStorageLevel(std::ostream& out, int level)
{
    storageLevel = level;
    out << "Set as " << storageLevel;
}

void SetDB(std::ostream& out, std::string &_dbPath)
{
    dbPath = _dbPath;
    db = new DBCore(_dbPath);
    out << "Set as " << dbPath << ".\n";
}

void WhichDB(std::ostream& out)
{
    out << "Current DB: " <<  dbPath << ".\n";
}

void ListTraces(std::ostream& out)
{
    if (_Check_DB(out)) 
        out << db->ListAllTraces(); 
}

void WorkOn(std::ostream& out, std::string &_traceName)
{
    if (_Check_DB(out)) 
    {
        std::string allTraces = db->ListAllTraces(); 
        if (allTraces.find(traceName) != std::string::npos)
        {
            traceName = _traceName;
            db->table_name = _traceName;
            out << "Work on: " << _traceName << ".\n";
        }
        else
        {
            out << _traceName << " not found in database.\n";
        }
    }
    InitQueryRange(out, 1, db->GetTraceSize());
}

void TraceSize(std::ostream& out)
{
    if (_Check_DB(out) && _Check_Trace(out))
        out << db->GetTraceSize() << "\n"; 
}

void RemoveCache(std::ostream& out)
{
    if (_Check_DB(out)) 
    {
        db->RemoveAllCachedRules();
        out << "Done.\n";
    }
}

void InitQueryRange(std::ostream &out, int start, int end)
{
    if (query != NULL)
    {
        delete query;
        query = NULL;
        db = NULL;
    }

    if (db == NULL)
    {
        if (dbPath=="") 
        {
            out << "DB is not set. Call \"SetDB\" to set DB first.\n";
            return;
        }
        if (traceName == "")
        {
            out << "Trace is not selected. Call \"WorkOn\" to select a trace first.\n";
            return;
        }
        db = new DBCore(dbPath, traceName);
    }

    int trace_len = db->GetTraceSize();
    
    if (end <= 0)
    {
        end = trace_len;
    }

    if (start>end || start <= 0 || end > trace_len)
    {
        out << "Invalid start and end.";
        return;
    }
 
    query = new FMQuery(dbPath, traceName, trace_len, start, end);
}

void PrepareQuery(std::ostream &out, int level)
{
    if (_Check_Query(out))
    {
        delete db;

        if (process_limit == 1)
        {
            #ifdef TIME_MEASUREMENT
                query->sp_core->resetTimer();
                query->db->resetTimer();
            #endif
            // No worker. Process in this thread.
            query->BuildTree(level, 1, true);
            #ifdef TIME_MEASUREMENT
                query->sp_core->printTimer();
                query->db->printTimer();
            #endif
        }
        else
        {
            // Multi-processing. Fork and exec
            int pid = fork();
            // TODO
        }
        db = new DBCore(dbPath, traceName);
        out << "Done.\n";
    }
}

void TracePrint(std::ostream &out, const int pos)
{
    if (_Check_DB(out) && _Check_Trace(out))
    {
        out << db->PrintTrace(pos, -1);
    }
}

void TracePrintRange(std::ostream &out, const int pos, const int length)
{
    if (_Check_DB(out) && _Check_Trace(out))
    {
        out << db->PrintTrace(pos, length);
    }
}

void SyscallPrintAll(std::ostream &out)
{
    if (_Check_DB(out) && _Check_Trace(out))
    {
        out << db->PrintSyscallByIdx(0);
    }
}

void SyscallPrint(std::ostream &out, int index)
{
    if (_Check_DB(out) && _Check_Trace(out))
    {
        out << db->PrintSyscallByIdx(index);
    }
}

void SubQueryHelper(std::ostream& out)
{
    out << "    Vars1Type, Vars2Type: {S(yscall), I(nstr), R(egister), M(emory)}.\n";
    out << "    Vars1, Vars2: \n";
    out << "        (if Type is SYSCALL) : {SyscallName, SyscallName[id]}. E.g., \"read\", \"read[1]\", \"read,recv*\", \"recv*\"" << std::endl;
    out << "        (if Type is INSTR)   : {InstrIndexInTrace, Instr}. E.g., \"1\", \"ret\", \"push,pop\", \"mov*rax*\"" << std::endl;
    out << "        (if Type is REGISTER): {RegisterName}. E.g., \"rip\", \"xmm1,xmm2,xmm3,xmm4\"" << std::endl;
    out << "        (if Type is MEMORY)  : {MemAddr}. E.g., \"0x800000\", \"0x800000-0x801000\", \"0x0-0x1,0x4-0x5,0x8\"" << std::endl;
    out << "    Direction: {T(o), F(orward), B(ackward)}\n";
    out << "        TO      : Query dataflows from Vars1 to Vars2\n";
    out << "        FORWARD : Pefrom forward analysis. Query dataflows from Vars1 to positions of Vars2\n";
    out << "        BACKWARD: Pefrom backward analysis. Query dataflows from positions of Vars1 to Vars2\n";
    out << "*All IDs start from 1. '*' is the wildcard for advanced matching.\n";
}

void QueryHelper(std::ostream& out)
{
    out << "Query Usage:\n";
    out << "    Query <Vars1Type> <Vars1> <Direction> <Vars2Type> <Vars2>\n";
    out << "Arguments:\n";
    SubQueryHelper(out);
}

void QueryInRangeHelper(std::ostream& out)
{
    out << "QueryInRange Usage:\n";
    out << "    QueryInRange <Range> <Vars1Type> <Vars1> <Direction> <Vars2Type> <Vars2>\n";
    out << "Arguments:\n";
    out << "    Range: (src,dest).\n";
    SubQueryHelper(out);
}

void MakeSourceSinkSet(std::map<int, std::set<int>*> &source_sink_list, int instr_id, int len, int* indices)
{
    auto it = source_sink_list.find(instr_id);
    if (it == source_sink_list.end())
    {
        std::set<int> *indice_set = new std::set<int>();
        indice_set->insert(indices, indices+len);
        source_sink_list.insert(std::make_pair(instr_id, indice_set));
    } 
    else
    {
        std::set<int> *indice_set = it->second;
        indice_set->insert(indices, indices+len);
    }
    delete [] indices;
}

int AddSyscallsToSourceSinkSet(std::map<int, std::set<int>*> &source_sink_list, int num_syscalls, syscall_t* syscalls)
{
    int valid_cnt = 0;
    for (int i=0; i<num_syscalls; i++)
    {
        syscall_t &this_syscall = syscalls[i];

        // Syscall hooks
        uint64_t hooked_buffer_base = (uint64_t) -1;
        uint64_t hooked_buffer_size = (uint64_t) -1;
        bool isSource;
        if (!get_syscall_buffer(this_syscall, hooked_buffer_base, hooked_buffer_size, isSource) ||
            hooked_buffer_base == (uint64_t) -1 || 
            hooked_buffer_size == (uint64_t) -1 ||
            hooked_buffer_size == 0)
        {
            continue;
        }
            
        valid_cnt++;
        // Create set
        auto it = source_sink_list.find(this_syscall.instr_id);
        if (it == source_sink_list.end())
        {
            std::set<int> *indice_set = new std::set<int>();
            source_sink_list.insert(std::make_pair(this_syscall.instr_id, indice_set));
            it = source_sink_list.find(this_syscall.instr_id);
            assert(it != source_sink_list.end());
        } 
        std::set<int> *indice_set = it->second;

        // Update indice set
        db->SetMemInfluence(hooked_buffer_base, hooked_buffer_size, indice_set);
    }
    if (num_syscalls>0) delete [] syscalls;
    return valid_cnt;
}

void ParseVars(vars_type_t type, std::string vars, int start, int end, std::map<int, std::set<int>*> &source_sink_list)
{
    std::string delimiter = ",";

    if (type==vars_type_t::INSTR)
    {
        size_t pos = 0;
        bool loop_end = (vars!="") ? false : true;
        while (!loop_end) {
            std::string token; 
            if ((pos = vars.find(delimiter)) != std::string::npos)
            {
                token = vars.substr(0, pos);
                vars.erase(0, pos + delimiter.length());
            }
            else
            {
                token = vars;
                loop_end = true;
            }

            int instr_idx = atoi(token.c_str());
            if (instr_idx == 0)
            {
                // This is an asm search
                std::vector<int> result = db->GetInstrIdxByASM(token, start, end);
                int valid_instr_count = 0;
                for (auto instrId : result)
                {
                    int indice_len = 0;
                    int *indice = NULL;
                    if (db->GetInfluenceRangeByInstrIdx(instrId, indice_len, indice))
                    {
                        valid_instr_count++;
                        MakeSourceSinkSet(source_sink_list, instrId, indice_len, indice);
                    }
                }
                std::replace(token.begin(), token.end(), '%', '*'); 
                std::cout << "  Found " << result.size() << " " << token <<" instructions. " << valid_instr_count << " of them are accepted."<< std::endl; 
            }
            else
            {
                // This is an instr idx
                int indice_len = 0;
                int *indice = NULL;                
                if (db->GetInfluenceRangeByInstrIdx(instr_idx, indice_len, indice))
                {
                    MakeSourceSinkSet(source_sink_list, instr_idx, indice_len, indice);
                }
            }
        }
        return;
    }
    if (type==vars_type_t::SYSCALL)
    {
        size_t pos = 0;
        bool loop_end = (vars!="") ? false : true;
        while (!loop_end) {
            std::string token; 
            if ((pos = vars.find(delimiter)) != std::string::npos)
            {
                token = vars.substr(0, pos);
                vars.erase(0, pos + delimiter.length());
            }
            else
            {
                token = vars;
                loop_end = true;
            }

            size_t pos_of_left_bracket;
            if ((pos_of_left_bracket = token.find("[")) != std::string::npos)
            {
                // Input such as: "read[1]"
                std::string syscall_name = token.substr(0, pos_of_left_bracket);
                token.erase(0, pos_of_left_bracket + 1);

                int syscall_index = -1;
                std::string right_bracket;
                std::stringstream ss(token);
                ss >> syscall_index >> right_bracket;
                if (right_bracket != "]" || syscall_index == -1)
                {
                    std::cout << "Error when parsinig syscall vars." << std::endl;
                    return;
                }
                syscall_t *this_syscall;
                auto num_syscall =db->GetSyscallsByname(syscall_name, this_syscall, start, end, syscall_index);
                if (num_syscall > 0)
                {
                    for (int i_syscall=0; i_syscall<num_syscall; i_syscall++)
                    {
                        int indice_len = 0;
                        int *indice = NULL;                
                        if (db->GetInfluenceRangeByInstrIdx(this_syscall[i_syscall].instr_id, indice_len, indice))
                        {
                            MakeSourceSinkSet(source_sink_list, this_syscall[i_syscall].instr_id, indice_len, indice);
                        }
                    }
                    delete [] this_syscall;                
                }
            }
            else
            {
                // Input such as: "read"
                syscall_t *this_syscall;
                auto num_syscall = db->GetSyscallsByname(token, this_syscall, start, end);
                if (num_syscall > 0)
                {
                    for (int i_syscall=0; i_syscall<num_syscall; i_syscall++)
                    {
                        int indice_len = 0;
                        int *indice = NULL;                
                        if (db->GetInfluenceRangeByInstrIdx(this_syscall[i_syscall].instr_id, indice_len, indice))
                        {
                            MakeSourceSinkSet(source_sink_list, this_syscall[i_syscall].instr_id, indice_len, indice);
                        }
                    }
                    delete [] this_syscall;
                }

                std::replace(token.begin(), token.end(), '%', '*'); 
                std::cout << "  Found " << num_syscall << " " << token << " syscalls. " <<std::endl; 
            }
        }
        return;
    }

    // Disable REG and MEM query
    if (type==vars_type_t::REGISTER)
    {
        throw "This version doesn't support REG query. It will be available in our Realse version.";
    }
    if (type==vars_type_t::MEMORY)
    {
        throw "This version doesn't support MEM query. It will be available in our Realse version.";
    }
    
    // Should not reach here
    LOG("[E] Unknown VarsType");
    DIE();
    return;
}

void QueryInRange(
    std::ostream& out, 
    std::string _range,
    std::string _src_type,
    std::string _srcs,
    std::string _direction,
    std::string _dest_type,
    std::string _dests)
{
    // Base check
    if (!_Check_Query(out)) return;

    // Clean up last result
    if (lastResult != NULL)
    {
        delete lastResult;
        lastResult = NULL;
    }

    std::string src_type, srcs, dest_type, dests;
    bool isForward = false;
    bool isBackward = false;
    // Direction parser
    {
        std::for_each(_direction.begin(), _direction.end(), [](char & c){
            c = ::tolower(c);
        });
        src_type = _src_type;
        srcs = _srcs;
        dest_type = _dest_type;
        dests = _dests;
        bool direction_has_set = false;
        if (_direction == "to" || _direction == "t")
        {
            direction_has_set = true;
        }
        if (_direction == "forward" || _direction == "f")
        {
            isForward = true;
            direction_has_set = true; 
        }
        if (_direction == "backward" || _direction == "b")
        {
            isBackward = true;
            direction_has_set = true;   
        }
        if (!direction_has_set) 
        {
            out << "Unknown Direction.\n";
            return;
        }
    }

    // Range parser
    int subRangeStart = query->startPos;
    int subRangeEnd = query->endPos;
    if (_range!="")
    {
        // "(1,12345)"
        char left_bracket, comma, right_bracket;
        std::stringstream ss(_range);
        ss >> left_bracket >> subRangeStart >> comma >> subRangeEnd >> right_bracket;
        if (left_bracket!='(' || right_bracket!=')' || !(comma==',' || comma=='~'))
        {
            out << "Unknown Range.\n";
            return;
        }
        if (subRangeStart > subRangeEnd || subRangeStart < query->startPos || subRangeEnd > query->endPos)
        {
            out << "Invalid Range.\n";
            return;
        }
    }

    // VarsType parser
    vars_type_t srcType, destType;
    {
        std::for_each(src_type.begin(), src_type.end(), [](char & c){
            c = ::tolower(c);
        });
        std::for_each(dest_type.begin(), dest_type.end(), [](char & c){
            c = ::tolower(c);
        });
        if (src_type=="syscall" || src_type=="s") srcType = vars_type_t::SYSCALL;
        else if (src_type=="instr" || src_type=="instruction" || src_type=="i") srcType = vars_type_t::INSTR;
        else if (src_type=="register" || src_type=="reg" || src_type=="r") srcType = vars_type_t::REGISTER;
        else if (src_type=="memory" || src_type=="mem" || src_type=="m") srcType = vars_type_t::MEMORY;
        else
        {
            out << "Unknown Source Type.\n";
            return;
        }
        if (dest_type=="syscall"|| dest_type=="s") destType = vars_type_t::SYSCALL;
        else if (dest_type=="instr" || dest_type=="instruction"|| dest_type=="i") destType = vars_type_t::INSTR;
        else if (dest_type=="register" || dest_type=="reg"|| dest_type=="r") destType = vars_type_t::REGISTER;
        else if (dest_type=="memory" || dest_type=="mem"|| dest_type=="m") destType = vars_type_t::MEMORY;
        else
        {
            out << "Unknown Dest Type.\n";
            return;
        }
    }

    // Parse Vars
    std::replace(srcs.begin(), srcs.end(), '*', '%');       // replace all '*' to '%'
    std::replace(dests.begin(), dests.end(), '*', '%'); 
    std::for_each(srcs.begin(), srcs.end(), [](char & c){   // Lower case
        c = ::tolower(c);
    });
    std::for_each(dests.begin(), dests.end(), [](char & c){
        c = ::tolower(c);
    });
    std::map<int, std::set<int>*> sourceList, destList;
    ParseVars(srcType, srcs, subRangeStart, subRangeEnd, sourceList);
    ParseVars(destType, dests, subRangeStart, subRangeEnd, destList);

    if (sourceList.size()==0)
    {
        out << "No valid source selected.\n";
        return; 
    }
    if (destList.size()==0)
    {
        out << "No valid dest selected.\n";
        return; 
    }
    const bool isOne2OneQuery = (sourceList.size()==1 && destList.size()==1);

    // TODO: Optimze multi-source multi-sink query
    int record_cnt = 0;
    for (auto source = sourceList.begin(); source!=sourceList.end(); source++)
    {
        int source_instr_id = source->first;
        auto source_set = source->second;
        for (auto sink = destList.begin(); sink!=destList.end(); sink++)
        {
            int sink_instr_id = sink->first;
            auto sink_set = sink->second;
            
            // Do nothing if source >= sink
            if (source_instr_id >= sink_instr_id) continue;


            #ifdef TIME_MEASUREMENT
                query->sp_core->resetTimer();
                query->db->resetTimer();
            #endif

            // We do NOT query (source, sink)! Query (source+1, sink) instead.
            // This helps us to have a clear view from vstate of source to the sink.
            // Otherwise, instruction such as "xor rax, rax" could never be a source
            //  as it contains empty dataflows.
            cuSparseMatrix *result_mat = query->Query(source_instr_id + 1, sink_instr_id);
            assert(result_mat != NULL);

            #ifdef TIME_MEASUREMENT
                query->sp_core->printTimer();
                query->db->printTimer();
            #endif

            if (isBackward && isOne2OneQuery)
            {
                source_set->insert(result_mat->h_cooOffsets, result_mat->h_cooOffsets+result_mat->nnz);
            }

            if (isForward && isOne2OneQuery)
            {
                sink_set->insert(result_mat->h_columns, result_mat->h_columns+result_mat->nnz);
            }

            std::map<int, std::string> *id_name_map = NULL;
            std::map<std::string, std::vector<std::string>> print_map;
            if (isOne2OneQuery)
            {
                std::set<int> merged_idx_set;
                std::merge(source_set->begin(), source_set->end(),
                    sink_set->begin(), sink_set->end(),
                    std::inserter(merged_idx_set, merged_idx_set.begin()));
                id_name_map = db->GetOffsetName(merged_idx_set);
            }
            bool has_influence = false;
            for (int i = 0; i<result_mat->nnz; i++)
            {
                int this_src = result_mat->h_cooOffsets[i];
                int this_dst = result_mat->h_columns[i];
                float this_value = result_mat->h_values[i];
                assert(this_value >= 0);
                if (this_value <= FLOAT_THRESHOLD) continue;
                if (source_set->find(this_src) != source_set->end() &&
                    sink_set->find(this_dst) != sink_set->end())
                {
                    if (!has_influence)
                    {
                        out << "[" << ++record_cnt << "] ";
                        out << source_instr_id << " -> " << sink_instr_id << std::endl;
                        has_influence = true;
                    }
                    if (!isOne2OneQuery) break;
                    auto &src_name = id_name_map->find(this_src)->second;
                    auto &dest_name = id_name_map->find(this_dst)->second;
                    auto print_map_it = print_map.find(src_name);
                    if (print_map_it!=print_map.end())
                    {
                        print_map_it->second.push_back(dest_name);
                    }
                    else
                    {
                        std::vector<std::string> new_vector;
                        new_vector.push_back(dest_name);
                        print_map.insert(std::pair<std::string, std::vector<std::string>>(src_name, new_vector));
                    }
                }
            }
            if (isOne2OneQuery)
            {
                if (!has_influence)
                {
                    out << source_instr_id << " does NOT affect " << sink_instr_id << std::endl;
                }
                else
                {
                    for (auto const& it : print_map)
                    {
                        out << "  " << it.first << " -> ";
                        for (auto const& dest : it.second)
                            out << dest << " ";
                        out << std::endl;
                    }

                }
                lastResult = result_mat;
            }
            else
            {
                if (isForward || isBackward)
                {
                    out << "Forward Mode and Backward Mode only support one-to-one query.\n";
                }
                delete result_mat;
            }            
            if (id_name_map != NULL) delete id_name_map;
        }
    }
}

void Query(
    std::ostream& out, 
    std::string _src_type,
    std::string _srcs,
    std::string _direction,
    std::string _dest_type,
    std::string _dests)
{
    QueryInRange(out, "", _src_type, _srcs, _direction, _dest_type, _dests);
    return;
}

void MultiplyTwo(std::ostream& out, int first_ins, int second_ins)
{
    int ind1_len, ind2_len, super_len;
    int *indice1, *indice2, *super_indices;
    auto mat1_ptr = db->LoadToDevice(query->sp_core->handle, first_ins, first_ins, ind1_len, indice1);
    auto mat2_ptr = db->LoadToDevice(query->sp_core->handle, second_ins, second_ins, ind2_len, indice2);
    cuSparseMatrix *productMatrix_ptr = NULL;

    AlignAndMultiply( query->sp_core, 
        *mat1_ptr, ind1_len, indice1,
        *mat2_ptr, ind2_len, indice2,
        productMatrix_ptr, super_len, super_indices, 0, 0);
    assert(productMatrix_ptr!=NULL);

    productMatrix_ptr->toCoo(query->sp_core->handle);
    productMatrix_ptr->toHost(query->sp_core->stream);
    query->sp_core->sync();
    std::set<int> indice_set;
    indice_set.insert(super_indices, super_indices+super_len);
    auto indice_dict = db->GetOffsetName(indice_set);
    std::map<std::string, std::vector<std::string>> print_map;
    for (int i=0; i<productMatrix_ptr->nnz; i++)
    {
        std::string source_name = indice_dict->find(productMatrix_ptr->h_cooOffsets[i])->second;
        std::string dest_name = indice_dict->find(productMatrix_ptr->h_columns[i])->second;
        auto print_map_it = print_map.find(source_name);
        if ( print_map_it != print_map.end())
        {
            print_map_it->second.push_back(dest_name + "(" + to_string(productMatrix_ptr->h_values[i]) + ")");
        }
        else
        {
            std::vector<std::string> tmp_vector;
            tmp_vector.push_back(dest_name + "(" + to_string(productMatrix_ptr->h_values[i]) + ")");
            print_map.insert(std::make_pair(source_name, tmp_vector));
        }
    }

    for (auto &it : print_map)
    {
        out << it.first << " -> ";
        for (size_t i=0; i<it.second.size(); i++)
        {
            out << it.second[i] << "  ";
            if (i%8 == 7) out << std::endl << "\t";
        }
        out << std::endl;
    }

    delete productMatrix_ptr;
    delete indice_dict;
    delete [] indice1;
    delete [] indice2;
    delete [] super_indices;
}


void PrintSingleInstr(std::ostream& out, int instr_id)
{
    int ind_len;
    int *indice;
    std::set<int> indice_set;
    auto mat_ptr = db->LoadToDevice(query->sp_core->handle, instr_id, instr_id, ind_len, indice);
    mat_ptr->toCoo(query->sp_core->handle);
    mat_ptr->toHost(query->sp_core->stream);
    query->sp_core->sync();
    indice_set.insert(indice, indice+ind_len);
    auto indice_dict = db->GetOffsetName(indice_set);
    std::map<std::string, std::vector<std::string>> print_map;
    for (int i=0; i<mat_ptr->nnz; i++)
    {
        std::string source_name = indice_dict->find(mat_ptr->h_cooOffsets[i])->second;
        std::string dest_name = indice_dict->find(mat_ptr->h_columns[i])->second;
        auto print_map_it = print_map.find(source_name);
        assert(mat_ptr->h_values[i] >= 0.95f && mat_ptr->h_values[i] <= 1.05f);
        if ( print_map_it != print_map.end())
        {
            print_map_it->second.push_back(dest_name);
        }
        else
        {
            std::vector<std::string> tmp_vector;
            tmp_vector.push_back(dest_name);
            print_map.insert(std::make_pair(source_name, tmp_vector));
        }
    }

    for (auto &it : print_map)
    {
        out << it.first << " -> ";
        for (size_t i=0; i<it.second.size(); i++)
        {
            out << it.second[i] << "  ";
            if (i%8 == 7) out << std::endl << "\t";
        }
        out << std::endl;
    }

    delete mat_ptr;
    delete indice_dict;
    delete [] indice;
}


void PrintLastResult(std::ostream& out)
{
    if (lastResult == NULL)
    {
        out << "No stored result.\n"; 
    }
    else
    {
        lastResult->print();
    }
}

void SetProcessLimit(std::ostream& out, int _process_limit)
{
    process_limit = std::min(16, _process_limit);
    process_limit = std::max(1, process_limit);
    out << "Set max number of workers as " << process_limit << std::endl;
    return;
}

void UsageMenu(std::ostream& out, std::string cmd)
{
    std::for_each(cmd.begin(), cmd.end(), [](char & c){
        c = ::tolower(c);
    });
    if (cmd == "query")
    {
        QueryHelper(out);
        return;
    }
    if (cmd == "queryinrange")
    {
        QueryInRangeHelper(out);
        return;
    }
    out << "No document for given command :(\n";
}

int main(int argc, char** argv)
{
    if (argc == 2)
    {
        dbPath = std::string(argv[1]);
        db = new DBCore(dbPath);
    }

    SetColor();
    auto rootMenu = make_unique< Menu >( "FMQuery" );
    {
        rootMenu -> Insert( 
            "WhichDB", 
            [](std::ostream& out) { WhichDB(out);}, 
            "Show which database is worked on." );

        rootMenu -> Insert( 
            "ListTraces", 
            [](std::ostream& out) { ListTraces(out);}, 
            "List all traces in database." );

        rootMenu -> Insert(
            "WorkOn",
            [](std::ostream& out, std::string _traceName){ WorkOn(out, _traceName); },
            "Select which trace to work on." );

        rootMenu -> Insert( 
            "TraceSize", 
            [](std::ostream& out) { TraceSize(out);}, 
            "Show size of the current trace." );

        rootMenu -> Insert( 
            "PrepareQuery", 
            [](std::ostream& out, int start, int end) { InitQueryRange(out, start, end); PrepareQuery(out, storageLevel);}, 
            "Pre-compute intermedia query results for later query on range (start, end)." );

        rootMenu -> Insert(
            "Query",
            [](std::ostream& out, 
                std::string src_type,
                std::string srcs,
                std::string direction,
                std::string dest_type,
                std::string dests) 
                { Query(out, src_type, srcs, direction, dest_type, dests);}, 
            "Perform data flow query. E.g. Query SYSCALL read,recv* TO SYSCALL write" );

        rootMenu -> Insert(
            "QueryInRange",
            [](std::ostream& out,
                std::string range,
                std::string src_type,
                std::string srcs,
                std::string direction,
                std::string dest_type,
                std::string dests ) 
                { QueryInRange(out, range, src_type, srcs, direction, dest_type, dests);}, 
            "Perform data flow query with-in a sub range" );
        rootMenu -> Insert(
            "MultiplyTwo",
            [](std::ostream& out, int first, int second) 
                { MultiplyTwo(out, first, second);}, 
            "Multiply specified two instrutions." );


        rootMenu -> Insert(
            "TracePrint",
            [](std::ostream& out, int pos){ TracePrint(out, pos); },
            "Print one instruction from trace by instruction index" );
        rootMenu -> Insert(
            "TracePrintRange",
            [](std::ostream& out, int pos, int len){ TracePrintRange(out, pos, len); },
            "Print a list of instructions." );
        rootMenu -> Insert(
            "PrintOne",
            [](std::ostream& out, int index){ PrintSingleInstr(out, index);},
            "Print data flows of one instruction by index.");

        rootMenu -> Insert(
            "SyscallPrintAll",
            [](std::ostream& out){ SyscallPrintAll(out); },
            "List all syscalls in trace." );
        rootMenu -> Insert(
            "SyscallPrint",
            [](std::ostream& out, int index){ SyscallPrint(out, index); },
            "Print one syscall in trace by its index." );
        rootMenu -> Insert(
            "LastResult",
            [](std::ostream& out){ PrintLastResult(out);},
            "Print the raw matrix of the last one-to-one query.");
        rootMenu -> Insert(
            "Usage",
            [](std::ostream& out, std::string cmd){ UsageMenu(out, cmd);},
            "Show usage of the specified command.");
    }
    auto SettingsMenu = make_unique< Menu >( "Settings" );
    {
        SettingsMenu -> Insert( 
            "SetDB", 
            [](std::ostream& out, std::string _dbPath) { SetDB(out, _dbPath);}, 
            "Set the path to DB." );
        SettingsMenu -> Insert( 
            "SetStorageLevel", 
            [](std::ostream& out, int level) { SetStorageLevel(out, level);}, 
            "Print Query usage." );
        SettingsMenu -> Insert( 
            "SetProcessLimit", 
            [](std::ostream& out, int _process_limit) { SetProcessLimit(out, _process_limit);}, 
            "Change the maxium number of workers in query tree construction." );
        rootMenu -> Insert( std::move(SettingsMenu));
    }

    Cli cli( std::move(rootMenu) );
    cli.ExitAction( [](auto& out){ out << ""; } );

    CliFileSession input(cli);
    input.Start();

    return 0;
}