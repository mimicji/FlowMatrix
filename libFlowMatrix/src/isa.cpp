/* KH: This is an auto-generated file from Squirrel ISA. Do Not edit. */

#include "isa.h"


// Check memory operation type. Return true if read, false if write.
bool isMemRead(const unsigned int memVal) {
	return bool(memVal & 32768);
}

// Check the slot type. Return true if Value, false if Addr.
bool isMemValue(const unsigned int memVal) {
	return bool(memVal & 16384);
}

/* Return mem size in bytes */
unsigned int getMemSize(const unsigned int memVal) {
	return memVal >> 16;
}

unsigned int getMemSlot(const unsigned int memVal) {
	return memVal & 16383;
}

std::string mem2str(const unsigned int memVal) {
	unsigned int slot_id = getMemSlot(memVal);
	unsigned int mem_size = getMemSize(memVal);
	std::string access = isMemRead(memVal) ? "READ": "WRITE";
	std::string slot_type = isMemValue(memVal) ? "VALUE": "ADDR";
	std::string mem_str = "MEM_" + access + "_" + std::to_string(slot_id) + "_" + slot_type + "_" + std::to_string(mem_size);
	return mem_str;
}

// Provide the register name. Return -1 if reg not found.
int getRegName(const std::string arch, const unsigned int reg_id, std::string &reg_name) {
	if (arch == "X86" || arch == "AMD64") {
		switch(reg_id) {
			case 1:
				reg_name = "AH";
				return 0;
			case 2:
				reg_name = "AL";
				return 0;
			case 3:
				reg_name = "BH";
				return 0;
			case 4:
				reg_name = "BL";
				return 0;
			case 5:
				reg_name = "BPL";
				return 0;
			case 6:
				reg_name = "CH";
				return 0;
			case 7:
				reg_name = "CL";
				return 0;
			case 8:
				reg_name = "CS";
				return 0;
			case 9:
				reg_name = "DH";
				return 0;
			case 10:
				reg_name = "DIL";
				return 0;
			case 11:
				reg_name = "DL";
				return 0;
			case 12:
				reg_name = "DS";
				return 0;
			case 13:
				reg_name = "SIL";
				return 0;
			case 14:
				reg_name = "SPL";
				return 0;
			case 15:
				reg_name = "AX";
				return 0;
			case 16:
				reg_name = "BX";
				return 0;
			case 17:
				reg_name = "CX";
				return 0;
			case 18:
				reg_name = "DX";
				return 0;
			case 19:
				reg_name = "DI";
				return 0;
			case 20:
				reg_name = "BP";
				return 0;
			case 21:
				reg_name = "SI";
				return 0;
			case 22:
				reg_name = "SP";
				return 0;
			case 23:
				reg_name = "EAX";
				return 0;
			case 24:
				reg_name = "EBP";
				return 0;
			case 25:
				reg_name = "EBX";
				return 0;
			case 26:
				reg_name = "ECX";
				return 0;
			case 27:
				reg_name = "EDI";
				return 0;
			case 28:
				reg_name = "EDX";
				return 0;
			case 29:
				reg_name = "EFLAGS";
				return 0;
			case 30:
				reg_name = "EIP";
				return 0;
			case 31:
				reg_name = "EIZ";
				return 0;
			case 32:
				reg_name = "ES";
				return 0;
			case 33:
				reg_name = "ESI";
				return 0;
			case 34:
				reg_name = "ESP";
				return 0;
			case 35:
				reg_name = "FPSW";
				return 0;
			case 36:
				reg_name = "FS";
				return 0;
			case 37:
				reg_name = "GS";
				return 0;
			case 38:
				reg_name = "IP";
				return 0;
			case 39:
				reg_name = "RAX";
				return 0;
			case 40:
				reg_name = "RBP";
				return 0;
			case 41:
				reg_name = "RBX";
				return 0;
			case 42:
				reg_name = "RCX";
				return 0;
			case 43:
				reg_name = "RDI";
				return 0;
			case 44:
				reg_name = "RDX";
				return 0;
			case 45:
				reg_name = "RIP";
				return 0;
			case 46:
				reg_name = "RIZ";
				return 0;
			case 47:
				reg_name = "RSI";
				return 0;
			case 48:
				reg_name = "RSP";
				return 0;
			case 49:
				reg_name = "SS";
				return 0;
			case 50:
				reg_name = "CR0";
				return 0;
			case 51:
				reg_name = "CR1";
				return 0;
			case 52:
				reg_name = "CR2";
				return 0;
			case 53:
				reg_name = "CR3";
				return 0;
			case 54:
				reg_name = "CR4";
				return 0;
			case 55:
				reg_name = "CR5";
				return 0;
			case 56:
				reg_name = "CR6";
				return 0;
			case 57:
				reg_name = "CR7";
				return 0;
			case 58:
				reg_name = "CR8";
				return 0;
			case 59:
				reg_name = "CR9";
				return 0;
			case 60:
				reg_name = "CR10";
				return 0;
			case 61:
				reg_name = "CR11";
				return 0;
			case 62:
				reg_name = "CR12";
				return 0;
			case 63:
				reg_name = "CR13";
				return 0;
			case 64:
				reg_name = "CR14";
				return 0;
			case 65:
				reg_name = "CR15";
				return 0;
			case 66:
				reg_name = "DR0";
				return 0;
			case 67:
				reg_name = "DR1";
				return 0;
			case 68:
				reg_name = "DR2";
				return 0;
			case 69:
				reg_name = "DR3";
				return 0;
			case 70:
				reg_name = "DR4";
				return 0;
			case 71:
				reg_name = "DR5";
				return 0;
			case 72:
				reg_name = "DR6";
				return 0;
			case 73:
				reg_name = "DR7";
				return 0;
			case 74:
				reg_name = "FP0";
				return 0;
			case 75:
				reg_name = "FP1";
				return 0;
			case 76:
				reg_name = "FP2";
				return 0;
			case 77:
				reg_name = "FP3";
				return 0;
			case 78:
				reg_name = "FP4";
				return 0;
			case 79:
				reg_name = "FP5";
				return 0;
			case 80:
				reg_name = "FP6";
				return 0;
			case 81:
				reg_name = "FP7";
				return 0;
			case 82:
				reg_name = "K0";
				return 0;
			case 83:
				reg_name = "K1";
				return 0;
			case 84:
				reg_name = "K2";
				return 0;
			case 85:
				reg_name = "K3";
				return 0;
			case 86:
				reg_name = "K4";
				return 0;
			case 87:
				reg_name = "K5";
				return 0;
			case 88:
				reg_name = "K6";
				return 0;
			case 89:
				reg_name = "K7";
				return 0;
			case 90:
				reg_name = "MM0";
				return 0;
			case 91:
				reg_name = "MM1";
				return 0;
			case 92:
				reg_name = "MM2";
				return 0;
			case 93:
				reg_name = "MM3";
				return 0;
			case 94:
				reg_name = "MM4";
				return 0;
			case 95:
				reg_name = "MM5";
				return 0;
			case 96:
				reg_name = "MM6";
				return 0;
			case 97:
				reg_name = "MM7";
				return 0;
			case 98:
				reg_name = "R8";
				return 0;
			case 99:
				reg_name = "R9";
				return 0;
			case 100:
				reg_name = "R10";
				return 0;
			case 101:
				reg_name = "R11";
				return 0;
			case 102:
				reg_name = "R12";
				return 0;
			case 103:
				reg_name = "R13";
				return 0;
			case 104:
				reg_name = "R14";
				return 0;
			case 105:
				reg_name = "R15";
				return 0;
			case 106:
				reg_name = "ST0";
				return 0;
			case 107:
				reg_name = "ST1";
				return 0;
			case 108:
				reg_name = "ST2";
				return 0;
			case 109:
				reg_name = "ST3";
				return 0;
			case 110:
				reg_name = "ST4";
				return 0;
			case 111:
				reg_name = "ST5";
				return 0;
			case 112:
				reg_name = "ST6";
				return 0;
			case 113:
				reg_name = "ST7";
				return 0;
			case 114:
				reg_name = "XMM0";
				return 0;
			case 115:
				reg_name = "XMM1";
				return 0;
			case 116:
				reg_name = "XMM2";
				return 0;
			case 117:
				reg_name = "XMM3";
				return 0;
			case 118:
				reg_name = "XMM4";
				return 0;
			case 119:
				reg_name = "XMM5";
				return 0;
			case 120:
				reg_name = "XMM6";
				return 0;
			case 121:
				reg_name = "XMM7";
				return 0;
			case 122:
				reg_name = "XMM8";
				return 0;
			case 123:
				reg_name = "XMM9";
				return 0;
			case 124:
				reg_name = "XMM10";
				return 0;
			case 125:
				reg_name = "XMM11";
				return 0;
			case 126:
				reg_name = "XMM12";
				return 0;
			case 127:
				reg_name = "XMM13";
				return 0;
			case 128:
				reg_name = "XMM14";
				return 0;
			case 129:
				reg_name = "XMM15";
				return 0;
			case 130:
				reg_name = "XMM16";
				return 0;
			case 131:
				reg_name = "XMM17";
				return 0;
			case 132:
				reg_name = "XMM18";
				return 0;
			case 133:
				reg_name = "XMM19";
				return 0;
			case 134:
				reg_name = "XMM20";
				return 0;
			case 135:
				reg_name = "XMM21";
				return 0;
			case 136:
				reg_name = "XMM22";
				return 0;
			case 137:
				reg_name = "XMM23";
				return 0;
			case 138:
				reg_name = "XMM24";
				return 0;
			case 139:
				reg_name = "XMM25";
				return 0;
			case 140:
				reg_name = "XMM26";
				return 0;
			case 141:
				reg_name = "XMM27";
				return 0;
			case 142:
				reg_name = "XMM28";
				return 0;
			case 143:
				reg_name = "XMM29";
				return 0;
			case 144:
				reg_name = "XMM30";
				return 0;
			case 145:
				reg_name = "XMM31";
				return 0;
			case 146:
				reg_name = "YMM0";
				return 0;
			case 147:
				reg_name = "YMM1";
				return 0;
			case 148:
				reg_name = "YMM2";
				return 0;
			case 149:
				reg_name = "YMM3";
				return 0;
			case 150:
				reg_name = "YMM4";
				return 0;
			case 151:
				reg_name = "YMM5";
				return 0;
			case 152:
				reg_name = "YMM6";
				return 0;
			case 153:
				reg_name = "YMM7";
				return 0;
			case 154:
				reg_name = "YMM8";
				return 0;
			case 155:
				reg_name = "YMM9";
				return 0;
			case 156:
				reg_name = "YMM10";
				return 0;
			case 157:
				reg_name = "YMM11";
				return 0;
			case 158:
				reg_name = "YMM12";
				return 0;
			case 159:
				reg_name = "YMM13";
				return 0;
			case 160:
				reg_name = "YMM14";
				return 0;
			case 161:
				reg_name = "YMM15";
				return 0;
			case 162:
				reg_name = "YMM16";
				return 0;
			case 163:
				reg_name = "YMM17";
				return 0;
			case 164:
				reg_name = "YMM18";
				return 0;
			case 165:
				reg_name = "YMM19";
				return 0;
			case 166:
				reg_name = "YMM20";
				return 0;
			case 167:
				reg_name = "YMM21";
				return 0;
			case 168:
				reg_name = "YMM22";
				return 0;
			case 169:
				reg_name = "YMM23";
				return 0;
			case 170:
				reg_name = "YMM24";
				return 0;
			case 171:
				reg_name = "YMM25";
				return 0;
			case 172:
				reg_name = "YMM26";
				return 0;
			case 173:
				reg_name = "YMM27";
				return 0;
			case 174:
				reg_name = "YMM28";
				return 0;
			case 175:
				reg_name = "YMM29";
				return 0;
			case 176:
				reg_name = "YMM30";
				return 0;
			case 177:
				reg_name = "YMM31";
				return 0;
			case 178:
				reg_name = "ZMM0";
				return 0;
			case 179:
				reg_name = "ZMM1";
				return 0;
			case 180:
				reg_name = "ZMM2";
				return 0;
			case 181:
				reg_name = "ZMM3";
				return 0;
			case 182:
				reg_name = "ZMM4";
				return 0;
			case 183:
				reg_name = "ZMM5";
				return 0;
			case 184:
				reg_name = "ZMM6";
				return 0;
			case 185:
				reg_name = "ZMM7";
				return 0;
			case 186:
				reg_name = "ZMM8";
				return 0;
			case 187:
				reg_name = "ZMM9";
				return 0;
			case 188:
				reg_name = "ZMM10";
				return 0;
			case 189:
				reg_name = "ZMM11";
				return 0;
			case 190:
				reg_name = "ZMM12";
				return 0;
			case 191:
				reg_name = "ZMM13";
				return 0;
			case 192:
				reg_name = "ZMM14";
				return 0;
			case 193:
				reg_name = "ZMM15";
				return 0;
			case 194:
				reg_name = "ZMM16";
				return 0;
			case 195:
				reg_name = "ZMM17";
				return 0;
			case 196:
				reg_name = "ZMM18";
				return 0;
			case 197:
				reg_name = "ZMM19";
				return 0;
			case 198:
				reg_name = "ZMM20";
				return 0;
			case 199:
				reg_name = "ZMM21";
				return 0;
			case 200:
				reg_name = "ZMM22";
				return 0;
			case 201:
				reg_name = "ZMM23";
				return 0;
			case 202:
				reg_name = "ZMM24";
				return 0;
			case 203:
				reg_name = "ZMM25";
				return 0;
			case 204:
				reg_name = "ZMM26";
				return 0;
			case 205:
				reg_name = "ZMM27";
				return 0;
			case 206:
				reg_name = "ZMM28";
				return 0;
			case 207:
				reg_name = "ZMM29";
				return 0;
			case 208:
				reg_name = "ZMM30";
				return 0;
			case 209:
				reg_name = "ZMM31";
				return 0;
			case 210:
				reg_name = "R8B";
				return 0;
			case 211:
				reg_name = "R9B";
				return 0;
			case 212:
				reg_name = "R10B";
				return 0;
			case 213:
				reg_name = "R11B";
				return 0;
			case 214:
				reg_name = "R12B";
				return 0;
			case 215:
				reg_name = "R13B";
				return 0;
			case 216:
				reg_name = "R14B";
				return 0;
			case 217:
				reg_name = "R15B";
				return 0;
			case 218:
				reg_name = "R8D";
				return 0;
			case 219:
				reg_name = "R9D";
				return 0;
			case 220:
				reg_name = "R10D";
				return 0;
			case 221:
				reg_name = "R11D";
				return 0;
			case 222:
				reg_name = "R12D";
				return 0;
			case 223:
				reg_name = "R13D";
				return 0;
			case 224:
				reg_name = "R14D";
				return 0;
			case 225:
				reg_name = "R15D";
				return 0;
			case 226:
				reg_name = "R8W";
				return 0;
			case 227:
				reg_name = "R9W";
				return 0;
			case 228:
				reg_name = "R10W";
				return 0;
			case 229:
				reg_name = "R11W";
				return 0;
			case 230:
				reg_name = "R12W";
				return 0;
			case 231:
				reg_name = "R13W";
				return 0;
			case 232:
				reg_name = "R14W";
				return 0;
			case 233:
				reg_name = "R15W";
				return 0;
			case 500:
				reg_name = "ST";
				return 0;
			case 501:
				reg_name = "RFLAGS";
				return 0;
			default:
				return -1;
		}
	}
	return -1;
}
// Provide the register name. Return -1 if reg not found.
int getRegSize(const std::string arch, const unsigned int reg_id, unsigned int &reg_size) {
	if (arch == "X86" || arch == "AMD64") {
		if (reg_id >= 146 && reg_id <= 177)
		{
			reg_size = 256;
			return 0;
		}
		switch(reg_id) {
			case 1:
				reg_size = 8;
				return 0;
			case 2:
				reg_size = 8;
				return 0;
			case 3:
				reg_size = 8;
				return 0;
			case 4:
				reg_size = 8;
				return 0;
			case 5:
				reg_size = 8;
				return 0;
			case 6:
				reg_size = 8;
				return 0;
			case 7:
				reg_size = 8;
				return 0;
			case 8:
				reg_size = 8;
				return 0;
			case 9:
				reg_size = 8;
				return 0;
			case 10:
				reg_size = 8;
				return 0;
			case 11:
				reg_size = 8;
				return 0;
			case 12:
				reg_size = 8;
				return 0;
			case 13:
				reg_size = 8;
				return 0;
			case 14:
				reg_size = 8;
				return 0;
			case 15:
				reg_size = 16;
				return 0;
			case 16:
				reg_size = 16;
				return 0;
			case 17:
				reg_size = 16;
				return 0;
			case 18:
				reg_size = 16;
				return 0;
			case 19:
				reg_size = 16;
				return 0;
			case 20:
				reg_size = 16;
				return 0;
			case 21:
				reg_size = 16;
				return 0;
			case 22:
				reg_size = 16;
				return 0;
			case 23:
				reg_size = 32;
				return 0;
			case 24:
				reg_size = 32;
				return 0;
			case 25:
				reg_size = 32;
				return 0;
			case 26:
				reg_size = 32;
				return 0;
			case 27:
				reg_size = 32;
				return 0;
			case 28:
				reg_size = 32;
				return 0;
			case 29:
				reg_size = 32;
				return 0;
			case 30:
				reg_size = 32;
				return 0;
			case 31:
				reg_size = 32;
				return 0;
			case 32:
				reg_size = 32;
				return 0;
			case 33:
				reg_size = 32;
				return 0;
			case 34:
				reg_size = 32;
				return 0;
			case 39:
				reg_size = 64;
				return 0;
			case 40:
				reg_size = 64;
				return 0;
			case 41:
				reg_size = 64;
				return 0;
			case 42:
				reg_size = 64;
				return 0;
			case 43:
				reg_size = 64;
				return 0;
			case 44:
				reg_size = 64;
				return 0;
			case 45:
				reg_size = 64;
				return 0;
			case 46:
				reg_size = 64;
				return 0;
			case 47:
				reg_size = 64;
				return 0;
			case 48:
				reg_size = 64;
				return 0;
			case 74:
				reg_size = 80;
				return 0;
			case 75:
				reg_size = 80;
				return 0;
			case 76:
				reg_size = 80;
				return 0;
			case 77:
				reg_size = 80;
				return 0;
			case 78:
				reg_size = 80;
				return 0;
			case 79:
				reg_size = 80;
				return 0;
			case 80:
				reg_size = 80;
				return 0;
			case 81:
				reg_size = 80;
				return 0;
			case 98:
				reg_size = 64;
				return 0;
			case 99:
				reg_size = 64;
				return 0;
			case 100:
				reg_size = 64;
				return 0;
			case 101:
				reg_size = 64;
				return 0;
			case 102:
				reg_size = 64;
				return 0;
			case 103:
				reg_size = 64;
				return 0;
			case 104:
				reg_size = 64;
				return 0;
			case 105:
				reg_size = 64;
				return 0;
			case 106:
				reg_size = 80;
				return 0;
			case 107:
				reg_size = 80;
				return 0;
			case 108:
				reg_size = 80;
				return 0;
			case 109:
				reg_size = 80;
				return 0;
			case 110:
				reg_size = 80;
				return 0;
			case 111:
				reg_size = 80;
				return 0;
			case 112:
				reg_size = 80;
				return 0;
			case 113:
				reg_size = 80;
				return 0;
			case 114:
				reg_size = 128;
				return 0;
			case 115:
				reg_size = 128;
				return 0;
			case 116:
				reg_size = 128;
				return 0;
			case 117:
				reg_size = 128;
				return 0;
			case 118:
				reg_size = 128;
				return 0;
			case 119:
				reg_size = 128;
				return 0;
			case 120:
				reg_size = 128;
				return 0;
			case 121:
				reg_size = 128;
				return 0;
			case 122:
				reg_size = 128;
				return 0;
			case 123:
				reg_size = 128;
				return 0;
			case 124:
				reg_size = 128;
				return 0;
			case 125:
				reg_size = 128;
				return 0;
			case 126:
				reg_size = 128;
				return 0;
			case 127:
				reg_size = 128;
				return 0;
			case 128:
				reg_size = 128;
				return 0;
			case 129:
				reg_size = 128;
				return 0;
			case 130:
				reg_size = 128;
				return 0;
			case 131:
				reg_size = 128;
				return 0;
			case 132:
				reg_size = 128;
				return 0;
			case 133:
				reg_size = 128;
				return 0;
			case 134:
				reg_size = 128;
				return 0;
			case 135:
				reg_size = 128;
				return 0;
			case 136:
				reg_size = 128;
				return 0;
			case 137:
				reg_size = 128;
				return 0;
			case 138:
				reg_size = 128;
				return 0;
			case 139:
				reg_size = 128;
				return 0;
			case 140:
				reg_size = 128;
				return 0;
			case 141:
				reg_size = 128;
				return 0;
			case 142:
				reg_size = 128;
				return 0;
			case 143:
				reg_size = 128;
				return 0;
			case 144:
				reg_size = 128;
				return 0;
			case 145:
				reg_size = 128;
				return 0;
			case 210:
				reg_size = 8;
				return 0;
			case 211:
				reg_size = 8;
				return 0;
			case 212:
				reg_size = 8;
				return 0;
			case 213:
				reg_size = 8;
				return 0;
			case 214:
				reg_size = 8;
				return 0;
			case 215:
				reg_size = 8;
				return 0;
			case 216:
				reg_size = 8;
				return 0;
			case 217:
				reg_size = 8;
				return 0;
			case 218:
				reg_size = 32;
				return 0;
			case 219:
				reg_size = 32;
				return 0;
			case 220:
				reg_size = 32;
				return 0;
			case 221:
				reg_size = 32;
				return 0;
			case 222:
				reg_size = 32;
				return 0;
			case 223:
				reg_size = 32;
				return 0;
			case 224:
				reg_size = 32;
				return 0;
			case 225:
				reg_size = 32;
				return 0;
			case 226:
				reg_size = 16;
				return 0;
			case 227:
				reg_size = 16;
				return 0;
			case 228:
				reg_size = 16;
				return 0;
			case 229:
				reg_size = 16;
				return 0;
			case 230:
				reg_size = 16;
				return 0;
			case 231:
				reg_size = 16;
				return 0;
			case 232:
				reg_size = 16;
				return 0;
			case 233:
				reg_size = 16;
				return 0;
			case 500:
				reg_size = 80;
				return 0;
			case 501:
				reg_size = 64;
				return 0;
			default:
				printf("Cannot identify size for %d!\n", reg_id);
				return -1;
		}
	}
	return -1;
}
// Provide the register id by name. Return -1 if reg not found. 
// KH: Pls avoid calling this function frequently! 
int getRegId(const std::string arch, const std::string reg_name, unsigned int &reg_id) {
	if (arch == "X86" || arch == "AMD64") {
		if (__builtin_expect(reg_name=="AH", 0)) {
			reg_id = 1;
			return 0;
		}
		if (__builtin_expect(reg_name=="AL", 0)) {
			reg_id = 2;
			return 0;
		}
		if (__builtin_expect(reg_name=="BH", 0)) {
			reg_id = 3;
			return 0;
		}
		if (__builtin_expect(reg_name=="BL", 0)) {
			reg_id = 4;
			return 0;
		}
		if (__builtin_expect(reg_name=="BPL", 0)) {
			reg_id = 5;
			return 0;
		}
		if (__builtin_expect(reg_name=="CH", 0)) {
			reg_id = 6;
			return 0;
		}
		if (__builtin_expect(reg_name=="CL", 0)) {
			reg_id = 7;
			return 0;
		}
		if (__builtin_expect(reg_name=="CS", 0)) {
			reg_id = 8;
			return 0;
		}
		if (__builtin_expect(reg_name=="DH", 0)) {
			reg_id = 9;
			return 0;
		}
		if (__builtin_expect(reg_name=="DIL", 0)) {
			reg_id = 10;
			return 0;
		}
		if (__builtin_expect(reg_name=="DL", 0)) {
			reg_id = 11;
			return 0;
		}
		if (__builtin_expect(reg_name=="DS", 0)) {
			reg_id = 12;
			return 0;
		}
		if (__builtin_expect(reg_name=="SIL", 0)) {
			reg_id = 13;
			return 0;
		}
		if (__builtin_expect(reg_name=="SPL", 0)) {
			reg_id = 14;
			return 0;
		}
		if (__builtin_expect(reg_name=="AX", 0)) {
			reg_id = 15;
			return 0;
		}
		if (__builtin_expect(reg_name=="BX", 0)) {
			reg_id = 16;
			return 0;
		}
		if (__builtin_expect(reg_name=="CX", 0)) {
			reg_id = 17;
			return 0;
		}
		if (__builtin_expect(reg_name=="DX", 0)) {
			reg_id = 18;
			return 0;
		}
		if (__builtin_expect(reg_name=="DI", 0)) {
			reg_id = 19;
			return 0;
		}
		if (__builtin_expect(reg_name=="BP", 0)) {
			reg_id = 20;
			return 0;
		}
		if (__builtin_expect(reg_name=="SI", 0)) {
			reg_id = 21;
			return 0;
		}
		if (__builtin_expect(reg_name=="SP", 0)) {
			reg_id = 22;
			return 0;
		}
		if (__builtin_expect(reg_name=="EAX", 0)) {
			reg_id = 23;
			return 0;
		}
		if (__builtin_expect(reg_name=="EBP", 0)) {
			reg_id = 24;
			return 0;
		}
		if (__builtin_expect(reg_name=="EBX", 0)) {
			reg_id = 25;
			return 0;
		}
		if (__builtin_expect(reg_name=="ECX", 0)) {
			reg_id = 26;
			return 0;
		}
		if (__builtin_expect(reg_name=="EDI", 0)) {
			reg_id = 27;
			return 0;
		}
		if (__builtin_expect(reg_name=="EDX", 0)) {
			reg_id = 28;
			return 0;
		}
		if (__builtin_expect(reg_name=="EFLAGS", 0)) {
			reg_id = 29;
			return 0;
		}
		if (__builtin_expect(reg_name=="EIP", 0)) {
			reg_id = 30;
			return 0;
		}
		if (__builtin_expect(reg_name=="EIZ", 0)) {
			reg_id = 31;
			return 0;
		}
		if (__builtin_expect(reg_name=="ES", 0)) {
			reg_id = 32;
			return 0;
		}
		if (__builtin_expect(reg_name=="ESI", 0)) {
			reg_id = 33;
			return 0;
		}
		if (__builtin_expect(reg_name=="ESP", 0)) {
			reg_id = 34;
			return 0;
		}
		if (__builtin_expect(reg_name=="FPSW", 0)) {
			reg_id = 35;
			return 0;
		}
		if (__builtin_expect(reg_name=="FS", 0)) {
			reg_id = 36;
			return 0;
		}
		if (__builtin_expect(reg_name=="GS", 0)) {
			reg_id = 37;
			return 0;
		}
		if (__builtin_expect(reg_name=="IP", 0)) {
			reg_id = 38;
			return 0;
		}
		if (__builtin_expect(reg_name=="RAX", 0)) {
			reg_id = 39;
			return 0;
		}
		if (__builtin_expect(reg_name=="RBP", 0)) {
			reg_id = 40;
			return 0;
		}
		if (__builtin_expect(reg_name=="RBX", 0)) {
			reg_id = 41;
			return 0;
		}
		if (__builtin_expect(reg_name=="RCX", 0)) {
			reg_id = 42;
			return 0;
		}
		if (__builtin_expect(reg_name=="RDI", 0)) {
			reg_id = 43;
			return 0;
		}
		if (__builtin_expect(reg_name=="RDX", 0)) {
			reg_id = 44;
			return 0;
		}
		if (__builtin_expect(reg_name=="RIP", 0)) {
			reg_id = 45;
			return 0;
		}
		if (__builtin_expect(reg_name=="RIZ", 0)) {
			reg_id = 46;
			return 0;
		}
		if (__builtin_expect(reg_name=="RSI", 0)) {
			reg_id = 47;
			return 0;
		}
		if (__builtin_expect(reg_name=="RSP", 0)) {
			reg_id = 48;
			return 0;
		}
		if (__builtin_expect(reg_name=="SS", 0)) {
			reg_id = 49;
			return 0;
		}
		if (__builtin_expect(reg_name=="CR0", 0)) {
			reg_id = 50;
			return 0;
		}
		if (__builtin_expect(reg_name=="CR1", 0)) {
			reg_id = 51;
			return 0;
		}
		if (__builtin_expect(reg_name=="CR2", 0)) {
			reg_id = 52;
			return 0;
		}
		if (__builtin_expect(reg_name=="CR3", 0)) {
			reg_id = 53;
			return 0;
		}
		if (__builtin_expect(reg_name=="CR4", 0)) {
			reg_id = 54;
			return 0;
		}
		if (__builtin_expect(reg_name=="CR5", 0)) {
			reg_id = 55;
			return 0;
		}
		if (__builtin_expect(reg_name=="CR6", 0)) {
			reg_id = 56;
			return 0;
		}
		if (__builtin_expect(reg_name=="CR7", 0)) {
			reg_id = 57;
			return 0;
		}
		if (__builtin_expect(reg_name=="CR8", 0)) {
			reg_id = 58;
			return 0;
		}
		if (__builtin_expect(reg_name=="CR9", 0)) {
			reg_id = 59;
			return 0;
		}
		if (__builtin_expect(reg_name=="CR10", 0)) {
			reg_id = 60;
			return 0;
		}
		if (__builtin_expect(reg_name=="CR11", 0)) {
			reg_id = 61;
			return 0;
		}
		if (__builtin_expect(reg_name=="CR12", 0)) {
			reg_id = 62;
			return 0;
		}
		if (__builtin_expect(reg_name=="CR13", 0)) {
			reg_id = 63;
			return 0;
		}
		if (__builtin_expect(reg_name=="CR14", 0)) {
			reg_id = 64;
			return 0;
		}
		if (__builtin_expect(reg_name=="CR15", 0)) {
			reg_id = 65;
			return 0;
		}
		if (__builtin_expect(reg_name=="DR0", 0)) {
			reg_id = 66;
			return 0;
		}
		if (__builtin_expect(reg_name=="DR1", 0)) {
			reg_id = 67;
			return 0;
		}
		if (__builtin_expect(reg_name=="DR2", 0)) {
			reg_id = 68;
			return 0;
		}
		if (__builtin_expect(reg_name=="DR3", 0)) {
			reg_id = 69;
			return 0;
		}
		if (__builtin_expect(reg_name=="DR4", 0)) {
			reg_id = 70;
			return 0;
		}
		if (__builtin_expect(reg_name=="DR5", 0)) {
			reg_id = 71;
			return 0;
		}
		if (__builtin_expect(reg_name=="DR6", 0)) {
			reg_id = 72;
			return 0;
		}
		if (__builtin_expect(reg_name=="DR7", 0)) {
			reg_id = 73;
			return 0;
		}
		if (__builtin_expect(reg_name=="FP0", 0)) {
			reg_id = 74;
			return 0;
		}
		if (__builtin_expect(reg_name=="FP1", 0)) {
			reg_id = 75;
			return 0;
		}
		if (__builtin_expect(reg_name=="FP2", 0)) {
			reg_id = 76;
			return 0;
		}
		if (__builtin_expect(reg_name=="FP3", 0)) {
			reg_id = 77;
			return 0;
		}
		if (__builtin_expect(reg_name=="FP4", 0)) {
			reg_id = 78;
			return 0;
		}
		if (__builtin_expect(reg_name=="FP5", 0)) {
			reg_id = 79;
			return 0;
		}
		if (__builtin_expect(reg_name=="FP6", 0)) {
			reg_id = 80;
			return 0;
		}
		if (__builtin_expect(reg_name=="FP7", 0)) {
			reg_id = 81;
			return 0;
		}
		if (__builtin_expect(reg_name=="K0", 0)) {
			reg_id = 82;
			return 0;
		}
		if (__builtin_expect(reg_name=="K1", 0)) {
			reg_id = 83;
			return 0;
		}
		if (__builtin_expect(reg_name=="K2", 0)) {
			reg_id = 84;
			return 0;
		}
		if (__builtin_expect(reg_name=="K3", 0)) {
			reg_id = 85;
			return 0;
		}
		if (__builtin_expect(reg_name=="K4", 0)) {
			reg_id = 86;
			return 0;
		}
		if (__builtin_expect(reg_name=="K5", 0)) {
			reg_id = 87;
			return 0;
		}
		if (__builtin_expect(reg_name=="K6", 0)) {
			reg_id = 88;
			return 0;
		}
		if (__builtin_expect(reg_name=="K7", 0)) {
			reg_id = 89;
			return 0;
		}
		if (__builtin_expect(reg_name=="MM0", 0)) {
			reg_id = 90;
			return 0;
		}
		if (__builtin_expect(reg_name=="MM1", 0)) {
			reg_id = 91;
			return 0;
		}
		if (__builtin_expect(reg_name=="MM2", 0)) {
			reg_id = 92;
			return 0;
		}
		if (__builtin_expect(reg_name=="MM3", 0)) {
			reg_id = 93;
			return 0;
		}
		if (__builtin_expect(reg_name=="MM4", 0)) {
			reg_id = 94;
			return 0;
		}
		if (__builtin_expect(reg_name=="MM5", 0)) {
			reg_id = 95;
			return 0;
		}
		if (__builtin_expect(reg_name=="MM6", 0)) {
			reg_id = 96;
			return 0;
		}
		if (__builtin_expect(reg_name=="MM7", 0)) {
			reg_id = 97;
			return 0;
		}
		if (__builtin_expect(reg_name=="R8", 0)) {
			reg_id = 98;
			return 0;
		}
		if (__builtin_expect(reg_name=="R9", 0)) {
			reg_id = 99;
			return 0;
		}
		if (__builtin_expect(reg_name=="R10", 0)) {
			reg_id = 100;
			return 0;
		}
		if (__builtin_expect(reg_name=="R11", 0)) {
			reg_id = 101;
			return 0;
		}
		if (__builtin_expect(reg_name=="R12", 0)) {
			reg_id = 102;
			return 0;
		}
		if (__builtin_expect(reg_name=="R13", 0)) {
			reg_id = 103;
			return 0;
		}
		if (__builtin_expect(reg_name=="R14", 0)) {
			reg_id = 104;
			return 0;
		}
		if (__builtin_expect(reg_name=="R15", 0)) {
			reg_id = 105;
			return 0;
		}
		if (__builtin_expect(reg_name=="ST0", 0)) {
			reg_id = 106;
			return 0;
		}
		if (__builtin_expect(reg_name=="ST1", 0)) {
			reg_id = 107;
			return 0;
		}
		if (__builtin_expect(reg_name=="ST2", 0)) {
			reg_id = 108;
			return 0;
		}
		if (__builtin_expect(reg_name=="ST3", 0)) {
			reg_id = 109;
			return 0;
		}
		if (__builtin_expect(reg_name=="ST4", 0)) {
			reg_id = 110;
			return 0;
		}
		if (__builtin_expect(reg_name=="ST5", 0)) {
			reg_id = 111;
			return 0;
		}
		if (__builtin_expect(reg_name=="ST6", 0)) {
			reg_id = 112;
			return 0;
		}
		if (__builtin_expect(reg_name=="ST7", 0)) {
			reg_id = 113;
			return 0;
		}
		if (__builtin_expect(reg_name=="XMM0", 0)) {
			reg_id = 114;
			return 0;
		}
		if (__builtin_expect(reg_name=="XMM1", 0)) {
			reg_id = 115;
			return 0;
		}
		if (__builtin_expect(reg_name=="XMM2", 0)) {
			reg_id = 116;
			return 0;
		}
		if (__builtin_expect(reg_name=="XMM3", 0)) {
			reg_id = 117;
			return 0;
		}
		if (__builtin_expect(reg_name=="XMM4", 0)) {
			reg_id = 118;
			return 0;
		}
		if (__builtin_expect(reg_name=="XMM5", 0)) {
			reg_id = 119;
			return 0;
		}
		if (__builtin_expect(reg_name=="XMM6", 0)) {
			reg_id = 120;
			return 0;
		}
		if (__builtin_expect(reg_name=="XMM7", 0)) {
			reg_id = 121;
			return 0;
		}
		if (__builtin_expect(reg_name=="XMM8", 0)) {
			reg_id = 122;
			return 0;
		}
		if (__builtin_expect(reg_name=="XMM9", 0)) {
			reg_id = 123;
			return 0;
		}
		if (__builtin_expect(reg_name=="XMM10", 0)) {
			reg_id = 124;
			return 0;
		}
		if (__builtin_expect(reg_name=="XMM11", 0)) {
			reg_id = 125;
			return 0;
		}
		if (__builtin_expect(reg_name=="XMM12", 0)) {
			reg_id = 126;
			return 0;
		}
		if (__builtin_expect(reg_name=="XMM13", 0)) {
			reg_id = 127;
			return 0;
		}
		if (__builtin_expect(reg_name=="XMM14", 0)) {
			reg_id = 128;
			return 0;
		}
		if (__builtin_expect(reg_name=="XMM15", 0)) {
			reg_id = 129;
			return 0;
		}
		if (__builtin_expect(reg_name=="XMM16", 0)) {
			reg_id = 130;
			return 0;
		}
		if (__builtin_expect(reg_name=="XMM17", 0)) {
			reg_id = 131;
			return 0;
		}
		if (__builtin_expect(reg_name=="XMM18", 0)) {
			reg_id = 132;
			return 0;
		}
		if (__builtin_expect(reg_name=="XMM19", 0)) {
			reg_id = 133;
			return 0;
		}
		if (__builtin_expect(reg_name=="XMM20", 0)) {
			reg_id = 134;
			return 0;
		}
		if (__builtin_expect(reg_name=="XMM21", 0)) {
			reg_id = 135;
			return 0;
		}
		if (__builtin_expect(reg_name=="XMM22", 0)) {
			reg_id = 136;
			return 0;
		}
		if (__builtin_expect(reg_name=="XMM23", 0)) {
			reg_id = 137;
			return 0;
		}
		if (__builtin_expect(reg_name=="XMM24", 0)) {
			reg_id = 138;
			return 0;
		}
		if (__builtin_expect(reg_name=="XMM25", 0)) {
			reg_id = 139;
			return 0;
		}
		if (__builtin_expect(reg_name=="XMM26", 0)) {
			reg_id = 140;
			return 0;
		}
		if (__builtin_expect(reg_name=="XMM27", 0)) {
			reg_id = 141;
			return 0;
		}
		if (__builtin_expect(reg_name=="XMM28", 0)) {
			reg_id = 142;
			return 0;
		}
		if (__builtin_expect(reg_name=="XMM29", 0)) {
			reg_id = 143;
			return 0;
		}
		if (__builtin_expect(reg_name=="XMM30", 0)) {
			reg_id = 144;
			return 0;
		}
		if (__builtin_expect(reg_name=="XMM31", 0)) {
			reg_id = 145;
			return 0;
		}
		if (__builtin_expect(reg_name=="YMM0", 0)) {
			reg_id = 146;
			return 0;
		}
		if (__builtin_expect(reg_name=="YMM1", 0)) {
			reg_id = 147;
			return 0;
		}
		if (__builtin_expect(reg_name=="YMM2", 0)) {
			reg_id = 148;
			return 0;
		}
		if (__builtin_expect(reg_name=="YMM3", 0)) {
			reg_id = 149;
			return 0;
		}
		if (__builtin_expect(reg_name=="YMM4", 0)) {
			reg_id = 150;
			return 0;
		}
		if (__builtin_expect(reg_name=="YMM5", 0)) {
			reg_id = 151;
			return 0;
		}
		if (__builtin_expect(reg_name=="YMM6", 0)) {
			reg_id = 152;
			return 0;
		}
		if (__builtin_expect(reg_name=="YMM7", 0)) {
			reg_id = 153;
			return 0;
		}
		if (__builtin_expect(reg_name=="YMM8", 0)) {
			reg_id = 154;
			return 0;
		}
		if (__builtin_expect(reg_name=="YMM9", 0)) {
			reg_id = 155;
			return 0;
		}
		if (__builtin_expect(reg_name=="YMM10", 0)) {
			reg_id = 156;
			return 0;
		}
		if (__builtin_expect(reg_name=="YMM11", 0)) {
			reg_id = 157;
			return 0;
		}
		if (__builtin_expect(reg_name=="YMM12", 0)) {
			reg_id = 158;
			return 0;
		}
		if (__builtin_expect(reg_name=="YMM13", 0)) {
			reg_id = 159;
			return 0;
		}
		if (__builtin_expect(reg_name=="YMM14", 0)) {
			reg_id = 160;
			return 0;
		}
		if (__builtin_expect(reg_name=="YMM15", 0)) {
			reg_id = 161;
			return 0;
		}
		if (__builtin_expect(reg_name=="YMM16", 0)) {
			reg_id = 162;
			return 0;
		}
		if (__builtin_expect(reg_name=="YMM17", 0)) {
			reg_id = 163;
			return 0;
		}
		if (__builtin_expect(reg_name=="YMM18", 0)) {
			reg_id = 164;
			return 0;
		}
		if (__builtin_expect(reg_name=="YMM19", 0)) {
			reg_id = 165;
			return 0;
		}
		if (__builtin_expect(reg_name=="YMM20", 0)) {
			reg_id = 166;
			return 0;
		}
		if (__builtin_expect(reg_name=="YMM21", 0)) {
			reg_id = 167;
			return 0;
		}
		if (__builtin_expect(reg_name=="YMM22", 0)) {
			reg_id = 168;
			return 0;
		}
		if (__builtin_expect(reg_name=="YMM23", 0)) {
			reg_id = 169;
			return 0;
		}
		if (__builtin_expect(reg_name=="YMM24", 0)) {
			reg_id = 170;
			return 0;
		}
		if (__builtin_expect(reg_name=="YMM25", 0)) {
			reg_id = 171;
			return 0;
		}
		if (__builtin_expect(reg_name=="YMM26", 0)) {
			reg_id = 172;
			return 0;
		}
		if (__builtin_expect(reg_name=="YMM27", 0)) {
			reg_id = 173;
			return 0;
		}
		if (__builtin_expect(reg_name=="YMM28", 0)) {
			reg_id = 174;
			return 0;
		}
		if (__builtin_expect(reg_name=="YMM29", 0)) {
			reg_id = 175;
			return 0;
		}
		if (__builtin_expect(reg_name=="YMM30", 0)) {
			reg_id = 176;
			return 0;
		}
		if (__builtin_expect(reg_name=="YMM31", 0)) {
			reg_id = 177;
			return 0;
		}
		if (__builtin_expect(reg_name=="ZMM0", 0)) {
			reg_id = 178;
			return 0;
		}
		if (__builtin_expect(reg_name=="ZMM1", 0)) {
			reg_id = 179;
			return 0;
		}
		if (__builtin_expect(reg_name=="ZMM2", 0)) {
			reg_id = 180;
			return 0;
		}
		if (__builtin_expect(reg_name=="ZMM3", 0)) {
			reg_id = 181;
			return 0;
		}
		if (__builtin_expect(reg_name=="ZMM4", 0)) {
			reg_id = 182;
			return 0;
		}
		if (__builtin_expect(reg_name=="ZMM5", 0)) {
			reg_id = 183;
			return 0;
		}
		if (__builtin_expect(reg_name=="ZMM6", 0)) {
			reg_id = 184;
			return 0;
		}
		if (__builtin_expect(reg_name=="ZMM7", 0)) {
			reg_id = 185;
			return 0;
		}
		if (__builtin_expect(reg_name=="ZMM8", 0)) {
			reg_id = 186;
			return 0;
		}
		if (__builtin_expect(reg_name=="ZMM9", 0)) {
			reg_id = 187;
			return 0;
		}
		if (__builtin_expect(reg_name=="ZMM10", 0)) {
			reg_id = 188;
			return 0;
		}
		if (__builtin_expect(reg_name=="ZMM11", 0)) {
			reg_id = 189;
			return 0;
		}
		if (__builtin_expect(reg_name=="ZMM12", 0)) {
			reg_id = 190;
			return 0;
		}
		if (__builtin_expect(reg_name=="ZMM13", 0)) {
			reg_id = 191;
			return 0;
		}
		if (__builtin_expect(reg_name=="ZMM14", 0)) {
			reg_id = 192;
			return 0;
		}
		if (__builtin_expect(reg_name=="ZMM15", 0)) {
			reg_id = 193;
			return 0;
		}
		if (__builtin_expect(reg_name=="ZMM16", 0)) {
			reg_id = 194;
			return 0;
		}
		if (__builtin_expect(reg_name=="ZMM17", 0)) {
			reg_id = 195;
			return 0;
		}
		if (__builtin_expect(reg_name=="ZMM18", 0)) {
			reg_id = 196;
			return 0;
		}
		if (__builtin_expect(reg_name=="ZMM19", 0)) {
			reg_id = 197;
			return 0;
		}
		if (__builtin_expect(reg_name=="ZMM20", 0)) {
			reg_id = 198;
			return 0;
		}
		if (__builtin_expect(reg_name=="ZMM21", 0)) {
			reg_id = 199;
			return 0;
		}
		if (__builtin_expect(reg_name=="ZMM22", 0)) {
			reg_id = 200;
			return 0;
		}
		if (__builtin_expect(reg_name=="ZMM23", 0)) {
			reg_id = 201;
			return 0;
		}
		if (__builtin_expect(reg_name=="ZMM24", 0)) {
			reg_id = 202;
			return 0;
		}
		if (__builtin_expect(reg_name=="ZMM25", 0)) {
			reg_id = 203;
			return 0;
		}
		if (__builtin_expect(reg_name=="ZMM26", 0)) {
			reg_id = 204;
			return 0;
		}
		if (__builtin_expect(reg_name=="ZMM27", 0)) {
			reg_id = 205;
			return 0;
		}
		if (__builtin_expect(reg_name=="ZMM28", 0)) {
			reg_id = 206;
			return 0;
		}
		if (__builtin_expect(reg_name=="ZMM29", 0)) {
			reg_id = 207;
			return 0;
		}
		if (__builtin_expect(reg_name=="ZMM30", 0)) {
			reg_id = 208;
			return 0;
		}
		if (__builtin_expect(reg_name=="ZMM31", 0)) {
			reg_id = 209;
			return 0;
		}
		if (__builtin_expect(reg_name=="R8B", 0)) {
			reg_id = 210;
			return 0;
		}
		if (__builtin_expect(reg_name=="R9B", 0)) {
			reg_id = 211;
			return 0;
		}
		if (__builtin_expect(reg_name=="R10B", 0)) {
			reg_id = 212;
			return 0;
		}
		if (__builtin_expect(reg_name=="R11B", 0)) {
			reg_id = 213;
			return 0;
		}
		if (__builtin_expect(reg_name=="R12B", 0)) {
			reg_id = 214;
			return 0;
		}
		if (__builtin_expect(reg_name=="R13B", 0)) {
			reg_id = 215;
			return 0;
		}
		if (__builtin_expect(reg_name=="R14B", 0)) {
			reg_id = 216;
			return 0;
		}
		if (__builtin_expect(reg_name=="R15B", 0)) {
			reg_id = 217;
			return 0;
		}
		if (__builtin_expect(reg_name=="R8D", 0)) {
			reg_id = 218;
			return 0;
		}
		if (__builtin_expect(reg_name=="R9D", 0)) {
			reg_id = 219;
			return 0;
		}
		if (__builtin_expect(reg_name=="R10D", 0)) {
			reg_id = 220;
			return 0;
		}
		if (__builtin_expect(reg_name=="R11D", 0)) {
			reg_id = 221;
			return 0;
		}
		if (__builtin_expect(reg_name=="R12D", 0)) {
			reg_id = 222;
			return 0;
		}
		if (__builtin_expect(reg_name=="R13D", 0)) {
			reg_id = 223;
			return 0;
		}
		if (__builtin_expect(reg_name=="R14D", 0)) {
			reg_id = 224;
			return 0;
		}
		if (__builtin_expect(reg_name=="R15D", 0)) {
			reg_id = 225;
			return 0;
		}
		if (__builtin_expect(reg_name=="R8W", 0)) {
			reg_id = 226;
			return 0;
		}
		if (__builtin_expect(reg_name=="R9W", 0)) {
			reg_id = 227;
			return 0;
		}
		if (__builtin_expect(reg_name=="R10W", 0)) {
			reg_id = 228;
			return 0;
		}
		if (__builtin_expect(reg_name=="R11W", 0)) {
			reg_id = 229;
			return 0;
		}
		if (__builtin_expect(reg_name=="R12W", 0)) {
			reg_id = 230;
			return 0;
		}
		if (__builtin_expect(reg_name=="R13W", 0)) {
			reg_id = 231;
			return 0;
		}
		if (__builtin_expect(reg_name=="R14W", 0)) {
			reg_id = 232;
			return 0;
		}
		if (__builtin_expect(reg_name=="R15W", 0)) {
			reg_id = 233;
			return 0;
		}
		if (__builtin_expect(reg_name=="ST", 0)) {
			reg_id = 500;
			return 0;
		}
		if (__builtin_expect(reg_name=="RFLAGS", 0)) {
			reg_id = 501;
			return 0;
		}
	}
	return -1;
}

// Provide the register id which contains the given one. Return -1 if reg not found.
int getEncloseReg(const std::string arch, const unsigned int reg_id, unsigned int &enclose_reg_id) {
	if (arch == "X86" || arch == "AMD64") {
		if (reg_id >= 114 && reg_id <= 145)
		{
			enclose_reg_id = reg_id+32;
			return 0;
		}
		switch(reg_id) {
			case 1:
				enclose_reg_id = 39;
				return 0;
			case 2:
				enclose_reg_id = 39;
				return 0;
			case 3:
				enclose_reg_id = 41;
				return 0;
			case 4:
				enclose_reg_id = 41;
				return 0;
			case 5:
				enclose_reg_id = 40;
				return 0;
			case 6:
				enclose_reg_id = 42;
				return 0;
			case 7:
				enclose_reg_id = 42;
				return 0;
			case 8:
				enclose_reg_id = 8;
				return 0;
			case 9:
				enclose_reg_id = 44;
				return 0;
			case 10:
				enclose_reg_id = 43;
				return 0;
			case 11:
				enclose_reg_id = 44;
				return 0;
			case 12:
				enclose_reg_id = 12;
				return 0;
			case 13:
				enclose_reg_id = 47;
				return 0;
			case 14:
				enclose_reg_id = 48;
				return 0;
			case 15:
				enclose_reg_id = 39;
				return 0;
			case 16:
				enclose_reg_id = 41;
				return 0;
			case 17:
				enclose_reg_id = 42;
				return 0;
			case 18:
				enclose_reg_id = 44;
				return 0;
			case 19:
				enclose_reg_id = 43;
				return 0;
			case 20:
				enclose_reg_id = 40;
				return 0;
			case 21:
				enclose_reg_id = 47;
				return 0;
			case 22:
				enclose_reg_id = 48;
				return 0;
			case 23:
				enclose_reg_id = 39;
				return 0;
			case 24:
				enclose_reg_id = 40;
				return 0;
			case 25:
				enclose_reg_id = 41;
				return 0;
			case 26:
				enclose_reg_id = 42;
				return 0;
			case 27:
				enclose_reg_id = 43;
				return 0;
			case 28:
				enclose_reg_id = 44;
				return 0;
			case 29:
				enclose_reg_id = 501;
				return 0;
			case 30:
				enclose_reg_id = 45;
				return 0;
			case 31:
				enclose_reg_id = 31;
				return 0;
			case 32:
				enclose_reg_id = 32;
				return 0;
			case 33:
				enclose_reg_id = 47;
				return 0;
			case 34:
				enclose_reg_id = 48;
				return 0;
			case 38:
				enclose_reg_id = 45;
				return 0;
			case 39:
				enclose_reg_id = 39;
				return 0;
			case 40:
				enclose_reg_id = 40;
				return 0;
			case 41:
				enclose_reg_id = 41;
				return 0;
			case 42:
				enclose_reg_id = 42;
				return 0;
			case 43:
				enclose_reg_id = 43;
				return 0;
			case 44:
				enclose_reg_id = 44;
				return 0;
			case 45:
				enclose_reg_id = 45;
				return 0;
			case 46:
				enclose_reg_id = 46;
				return 0;
			case 47:
				enclose_reg_id = 47;
				return 0;
			case 48:
				enclose_reg_id = 48;
				return 0;
			case 74:
				enclose_reg_id = 74;
				return 0;
			case 75:
				enclose_reg_id = 75;
				return 0;
			case 76:
				enclose_reg_id = 76;
				return 0;
			case 77:
				enclose_reg_id = 77;
				return 0;
			case 78:
				enclose_reg_id = 78;
				return 0;
			case 79:
				enclose_reg_id = 79;
				return 0;
			case 80:
				enclose_reg_id = 80;
				return 0;
			case 81:
				enclose_reg_id = 81;
				return 0;
			case 90:
				enclose_reg_id = 74;
				return 0;
			case 91:
				enclose_reg_id = 75;
				return 0;
			case 92:
				enclose_reg_id = 76;
				return 0;
			case 93:
				enclose_reg_id = 77;
				return 0;
			case 94:
				enclose_reg_id = 78;
				return 0;
			case 95:
				enclose_reg_id = 79;
				return 0;
			case 96:
				enclose_reg_id = 80;
				return 0;
			case 97:
				enclose_reg_id = 81;
				return 0;
			case 98:
				enclose_reg_id = 98;
				return 0;
			case 99:
				enclose_reg_id = 99;
				return 0;
			case 100:
				enclose_reg_id = 100;
				return 0;
			case 101:
				enclose_reg_id = 101;
				return 0;
			case 102:
				enclose_reg_id = 102;
				return 0;
			case 103:
				enclose_reg_id = 103;
				return 0;
			case 104:
				enclose_reg_id = 104;
				return 0;
			case 105:
				enclose_reg_id = 105;
				return 0;
			case 106:
				enclose_reg_id = 74;
				return 0;
			case 107:
				enclose_reg_id = 75;
				return 0;
			case 108:
				enclose_reg_id = 76;
				return 0;
			case 109:
				enclose_reg_id = 77;
				return 0;
			case 110:
				enclose_reg_id = 78;
				return 0;
			case 111:
				enclose_reg_id = 79;
				return 0;
			case 112:
				enclose_reg_id = 80;
				return 0;
			case 113:
				enclose_reg_id = 81;
				return 0;
			case 114:
				enclose_reg_id = 114;
				return 0;
			case 115:
				enclose_reg_id = 115;
				return 0;
			case 116:
				enclose_reg_id = 116;
				return 0;
			case 117:
				enclose_reg_id = 117;
				return 0;
			case 118:
				enclose_reg_id = 118;
				return 0;
			case 119:
				enclose_reg_id = 119;
				return 0;
			case 120:
				enclose_reg_id = 120;
				return 0;
			case 121:
				enclose_reg_id = 121;
				return 0;
			case 122:
				enclose_reg_id = 122;
				return 0;
			case 123:
				enclose_reg_id = 123;
				return 0;
			case 124:
				enclose_reg_id = 124;
				return 0;
			case 125:
				enclose_reg_id = 125;
				return 0;
			case 126:
				enclose_reg_id = 126;
				return 0;
			case 127:
				enclose_reg_id = 127;
				return 0;
			case 128:
				enclose_reg_id = 128;
				return 0;
			case 129:
				enclose_reg_id = 129;
				return 0;
			case 130:
				enclose_reg_id = 130;
				return 0;
			case 131:
				enclose_reg_id = 131;
				return 0;
			case 132:
				enclose_reg_id = 132;
				return 0;
			case 133:
				enclose_reg_id = 133;
				return 0;
			case 134:
				enclose_reg_id = 134;
				return 0;
			case 135:
				enclose_reg_id = 135;
				return 0;
			case 136:
				enclose_reg_id = 136;
				return 0;
			case 137:
				enclose_reg_id = 137;
				return 0;
			case 138:
				enclose_reg_id = 138;
				return 0;
			case 139:
				enclose_reg_id = 139;
				return 0;
			case 140:
				enclose_reg_id = 140;
				return 0;
			case 141:
				enclose_reg_id = 141;
				return 0;
			case 142:
				enclose_reg_id = 142;
				return 0;
			case 143:
				enclose_reg_id = 143;
				return 0;
			case 144:
				enclose_reg_id = 144;
				return 0;
			case 145:
				enclose_reg_id = 145;
				return 0;
			case 210:
				enclose_reg_id = 98;
				return 0;
			case 211:
				enclose_reg_id = 99;
				return 0;
			case 212:
				enclose_reg_id = 100;
				return 0;
			case 213:
				enclose_reg_id = 101;
				return 0;
			case 214:
				enclose_reg_id = 102;
				return 0;
			case 215:
				enclose_reg_id = 103;
				return 0;
			case 216:
				enclose_reg_id = 104;
				return 0;
			case 217:
				enclose_reg_id = 105;
				return 0;
			case 218:
				enclose_reg_id = 98;
				return 0;
			case 219:
				enclose_reg_id = 99;
				return 0;
			case 220:
				enclose_reg_id = 100;
				return 0;
			case 221:
				enclose_reg_id = 101;
				return 0;
			case 222:
				enclose_reg_id = 102;
				return 0;
			case 223:
				enclose_reg_id = 103;
				return 0;
			case 224:
				enclose_reg_id = 104;
				return 0;
			case 225:
				enclose_reg_id = 105;
				return 0;
			case 226:
				enclose_reg_id = 98;
				return 0;
			case 227:
				enclose_reg_id = 99;
				return 0;
			case 228:
				enclose_reg_id = 100;
				return 0;
			case 229:
				enclose_reg_id = 101;
				return 0;
			case 230:
				enclose_reg_id = 102;
				return 0;
			case 231:
				enclose_reg_id = 103;
				return 0;
			case 232:
				enclose_reg_id = 104;
				return 0;
			case 233:
				enclose_reg_id = 105;
				return 0;
			case 500:
				enclose_reg_id = 74;
				return 0;
			case 501:
				enclose_reg_id = 501;
				return 0;
			default:
				return -1;
		}
	}
	return -1;
}

int getRegId_peekapoo(const std::string arch, const std::string reg_name, unsigned int &reg_id) {
	if (arch == "AMD64") {
		if (__builtin_expect(reg_name=="RDI", 0)) {
			reg_id = 0;
			return 0;
		}
		if (__builtin_expect(reg_name=="RSI", 0)) {
			reg_id = 1;
			return 0;
		}
		if (__builtin_expect(reg_name=="RSP", 0)) {
			reg_id = 2;
			return 0;
		}
		if (__builtin_expect(reg_name=="RBP", 0)) {
			reg_id = 3;
			return 0;
		}
		if (__builtin_expect(reg_name=="RBX", 0)) {
			reg_id = 4;
			return 0;
		}
		if (__builtin_expect(reg_name=="RDX", 0)) {
			reg_id = 5;
			return 0;
		}
		if (__builtin_expect(reg_name=="RCX", 0)) {
			reg_id = 6;
			return 0;
		}
		if (__builtin_expect(reg_name=="RAX", 0)) {
			reg_id = 7;
			return 0;
		}
		if (__builtin_expect(reg_name=="R8", 0)) {
			reg_id = 8;
			return 0;
		}
		if (__builtin_expect(reg_name=="R9", 0)) {
			reg_id = 9;
			return 0;
		}
		if (__builtin_expect(reg_name=="R10", 0)) {
			reg_id = 10;
			return 0;
		}
		if (__builtin_expect(reg_name=="R11", 0)) {
			reg_id = 11;
			return 0;
		}
		if (__builtin_expect(reg_name=="R12", 0)) {
			reg_id = 12;
			return 0;
		}
		if (__builtin_expect(reg_name=="R13", 0)) {
			reg_id = 13;
			return 0;
		}
		if (__builtin_expect(reg_name=="R14", 0)) {
			reg_id = 14;
			return 0;
		}
		if (__builtin_expect(reg_name=="R15", 0)) {
			reg_id = 15;
			return 0;
		}
		if (__builtin_expect(reg_name=="RFLAGS", 0)) {
			reg_id = 16;
			return 0;
		}
		if (__builtin_expect(reg_name=="RIP", 0)) {
			reg_id = 17;
			return 0;
		}
	}
	return -1;
}