#ifndef __LIBFLOWMATRIX_PROTORULEDB_H_
#define __LIBFLOWMATRIX_PROTORULEDB_H_

#include "acorns_obj.pb.h"
#include "FMCommon.hpp"
#include "isa.h"

namespace FlowMatrix{

class ProtoRuleDB
{
private:
    std::string path;
    std::string arch;

public:
    // Constructor and destructor
    ProtoRuleDB(const std::string &dbPath, const std::string arch);
    ~ProtoRuleDB();

    // Rule utilities
    int loadRule(const std::string raw_bytes, acorn_obj::TaintRule &rule);


};

} // End of namespace FlowMatrix

#endif // __LIBFLOWMATRIX_PROTORULEDB_H_