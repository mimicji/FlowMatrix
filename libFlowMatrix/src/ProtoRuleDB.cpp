#include "ProtoRuleDB.hpp"

namespace FlowMatrix
{

ProtoRuleDB::ProtoRuleDB(const std::string &dbPath, const std::string arch)
{
    // Check ProtoBuf Version
    GOOGLE_PROTOBUF_VERIFY_VERSION;

    // Set path
    this->path = dbPath;
    this->arch = arch;
}

ProtoRuleDB::~ProtoRuleDB()
{
    // Shut down ProtoBuf lib
    google::protobuf::ShutdownProtobufLibrary();
}

int ProtoRuleDB::loadRule(const std::string raw_bytes, acorn_obj::TaintRule &rule)
{
    std::string filePath;
    filePath = this->path+"/"+this->arch+"/"+raw_bytes+".bin";
    std::fstream input(filePath, std::ios::in | std::ios::binary);
    if ( (!input) || !rule.ParseFromIstream(&input)) 
    {
        LOG("[W] Fail to load rule %s with arch %s", raw_bytes.c_str(), arch.c_str());
        input.close();
        return -1;
    }
    input.close();
    return 0;
}




} // End of namespace FlowMatrix