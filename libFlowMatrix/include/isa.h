/* KH: This is an auto-generated file from Squirrel ISA. Do Not edit. */
#ifndef _ISA_H_
#define _ISA_H_

#include <string>

bool isMemRead(const unsigned int memVal);
unsigned int getMemSize(const unsigned int memVal);
unsigned int getMemSlot(const unsigned int memVal);
bool isMemValue(const unsigned int memVal);
std::string mem2str(const unsigned int memVal);
int getRegName(const std::string arch, const unsigned int reg_id, std::string &reg_name);
int getRegSize(const std::string arch, const unsigned int reg_id, unsigned int &reg_size);
int getRegId(const std::string arch, const std::string reg_name, unsigned int &reg_id);
int getEncloseReg(const std::string arch, const unsigned int reg_id, unsigned int &enclose_reg_id);
int getRegId_peekapoo(const std::string arch, const std::string reg_name, unsigned int &reg_id);

#endif