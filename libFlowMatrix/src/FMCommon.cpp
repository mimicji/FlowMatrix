#include <FMCommon.hpp>

const std::string currentDateTime() {
    time_t     now = time(0);
    struct tm  tstruct;
    char       buf[80];
    tstruct = *localtime(&now);
    strftime(buf, sizeof(buf), "%Y-%m-%d.%X ", &tstruct);

    return buf;
}

void printProgress(double percentage) {
    int val = (int) (percentage * 100);
    int lpad = (int) (percentage * PBWIDTH);
    int rpad = PBWIDTH - lpad;
    printf("\r%3d%% [%.*s%*s]", val, lpad, PBSTR, rpad, "");
    fflush(stdout);
}

std::string uint8_to_hex_string(const uint8_t *uint8_arrary, const uint64_t size)
{
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for (uint64_t i = 0; i < size; i++) {
        ss << std::hex << std::setw(2) << static_cast<int>(uint8_arrary[i]);
    }
    return ss.str();
}

/* Disassemble and print one instruction from buffer. Return size of this instruction */
int disassemble_one_instr_x86(uint8_t *raw_bytes, uint32_t len, std::string &result_asm, size_t addr /* = 0x0 */) 
{
    csh handle;
	cs_insn *insn;
	size_t count;
    std::stringstream ss;


	if (cs_open(CS_ARCH_X86, CS_MODE_64, &handle) != CS_ERR_OK)
		return -1;
	count = cs_disasm(handle, raw_bytes, len, addr, 0, &insn);
	if (count > 0) {
        ss << std::hex<<insn[0].mnemonic << "\t" << insn[0].op_str;
		cs_free(insn, count);
        cs_close(&handle);
        result_asm =ss.str();
        return 1;
	} 
    else
	{
        std::string raw_byte_str = uint8_to_hex_string(raw_bytes, len);

        // Fail to disassemble. Alert.
        LOG("[W] Failed to disassemble given code %s!", raw_byte_str.c_str());
	    cs_close(&handle);
        return 0;
    }
}
