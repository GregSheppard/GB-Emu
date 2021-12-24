#include "SharpLR35902.h"

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
~~~~~~~~~~~~CPU Functions~~~~~~~~~~~~~~~~~~~~~
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


SharpLR35902::SharpLR35902(AddressBus& _bus) : bus(_bus) {
	fillLut<0xFF>();
}

uint8_t SharpLR35902::getNext() {
	return bus.read(r.pc++);
}

uint16_t SharpLR35902::getNext16() {
	uint8_t lower = getNext();
	uint8_t upper = getNext();
	return lower + (upper << 8);
}

void SharpLR35902::decodeCB() {
	(this->*lutCB[bus.read(r.pc++)])();
}

void SharpLR35902::decodeOps(uint8_t op) {
	(this->*lut[op])();
}

void SharpLR35902::run() {
	uint8_t nextOp = bus.read(r.pc++);
	handleInterrupts();
	decodeOps(nextOp);
	if (EI_IME) {
		IME = true;
		EI_IME = false;
	}
}

void SharpLR35902::handleInterrupts() {
	if (IME) {
		uint8_t IE = bus.getInterruptEnable();
		uint8_t IF = bus.getInterruptFlag();

		if ((IE & 0b00000001) && (IF & 0b00000001)) { //VBlank
			r.pc--; bus.addCycles(4); //decrement pc to undo opcode fetch
			IME = false;
			IF &= 0b11111110;
			CALL_ADDR(0x40);
		}
		else if (((IE & 0b00000010) >> 1) && ((IF & 0b00000010) >> 1)) { //LCD STAT
			IME = false;
			IF &= 0b11111101;
			CALL_ADDR(0x48);
		}
		else if (((IE & 0b00000100) >> 2) && ((IF & 0b00000100) >> 2)) { //Timer
			IME = false;
			IF &= 0b11111011;
			CALL_ADDR(0x50);
		}
		else if (((IE & 0b00001000) >> 3) && ((IF & 0b00001000) >> 3)) { //Serial
			IME = false;
			IF &= 0b11110111;
			CALL_ADDR(0x58);
		}
		else if (((IE & 0b00010000) >> 4) && ((IF & 0b00010000) >> 4)) { //Joypad
			IME = false;
			IF &= 0b11101111;
			CALL_ADDR(0x60);
		}
	}
}

void SharpLR35902::setPC(uint16_t addr) {
	r.pc = addr;
}

void SharpLR35902::setup() {
	r.af = 0x01B0;
	r.bc = 0x0013;
	r.de = 0x00D8;
	r.hl = 0x014D;

	r.pc = 0x100;
	r.sp = 0xFFFE;

	IME = 0;
	EI_IME = 0;

	bus.write(0xFF10, 0x80); //NR10
	bus.write(0xFF11, 0xBF); //NR11
	bus.write(0xFF12, 0xF3); //NR12
	bus.write(0xFF14, 0xBF); //NR14
	bus.write(0xFF16, 0x3F); //NR21
	bus.write(0xFF19, 0xBF); //NR24
	bus.write(0xFF1A, 0x7F); //NR30
	bus.write(0xFF1B, 0xFF); //NR31
	bus.write(0xFF1C, 0x9F); //NR32
	bus.write(0xFF1E, 0xBF); //NR34
	bus.write(0xFF20, 0xFF); //NR41
	bus.write(0xFF23, 0xBF); //NR44
	bus.write(0xFF24, 0x77); //NR50
	bus.write(0xFF25, 0xF3); //NR51
	bus.write(0xFF26, 0xF1); //NR52
	bus.write(0xFF40, 0x91); //LCDC
	bus.write(0xFF47, 0xFC); //BGP
	bus.write(0xFF48, 0xFF); //OBP0
	bus.write(0xFF49, 0xFF); //OBP1

	bus.write(0xFF44, 0x90); //PEACHES LOGS?
	bus.write(0xDD03, 0xDF);
	bus.write(0xDD01, 0xDF);
}

void SharpLR35902::log(std::ofstream& file) {
	file << "A: " << std::uppercase << std::setfill('0') << std::setw(2) << std::right << std::hex << +r.a;
	file << " F: " << std::uppercase << std::setfill('0') << std::setw(2) << std::right << std::hex << +r.f;
	file << " B: " << std::uppercase << std::setfill('0') << std::setw(2) << std::right << std::hex << +r.b;
	file << " C: " << std::uppercase << std::setfill('0') << std::setw(2) << std::right << std::hex << +r.c;
	file << " D: " << std::uppercase << std::setfill('0') << std::setw(2) << std::right << std::hex << +r.d;
	file << " E: " << std::uppercase << std::setfill('0') << std::setw(2) << std::right << std::hex << +r.e;
	file << " H: " << std::uppercase << std::setfill('0') << std::setw(2) << std::right << std::hex << +r.h;
	file << " L: " << std::uppercase << std::setfill('0') << std::setw(2) << std::right << std::hex << +r.l;
	file << " SP: " << std::uppercase << std::setfill('0') << std::setw(2) << std::right << std::hex << +r.sp;
	file << " PC: 00:" << std::uppercase << std::setfill('0') << std::setw(4) << std::right << std::hex << +r.pc;
	file << " (" << std::uppercase << std::setfill('0') << std::setw(2) << std::right << std::hex << +bus.read(r.pc);
	file << " " << std::uppercase << std::setfill('0') << std::setw(2) << std::right << std::hex << +bus.read(r.pc + 1);
	file << " " << std::uppercase << std::setfill('0') << std::setw(2) << std::right << std::hex << +bus.read(r.pc + 2);
	file << " " << std::uppercase << std::setfill('0') << std::setw(2) << std::right << std::hex << +bus.read(r.pc + 3);
	file << ")" << std::endl;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
~~~~~~~~~~~~~~~~Helper functions for CPU~~~~~~~~~~~~~~~~~~~
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

R16 SharpLR35902::r16FromBits(uint8_t bits, int group) {
	bits = ((bits & 0b00110000) >> 4);
	if (group == 1) {
		switch (bits) {
		case 0x0:
			return R16::bc;
		case 0x1:
			return R16::de;
		case 0x2:
			return R16::hl;
		case 0x3:
			return R16::sp;
		}
	}
	else if (group == 2) {
		switch (bits) {
		case 0x0:
			return R16::bc;
		case 0x1:
			return R16::de;
		case 0x2:
			return R16::hlp;
		case 0x3:
			return R16::hlm;
		}
	}
	else if (group == 3) {
		switch (bits) {
		case 0x0:
			return R16::bc;
		case 0x1:
			return R16::de;
		case 0x2:
			return R16::hl;
		case 0x3:
			return R16::af;
		}
	}
}

Condition constexpr SharpLR35902::getCond(uint8_t bits) {
	bits = ((bits & 0b00011000) >> 3);

	if (bits == 0x0) {
		return Condition::NZ;
	}
	else if (bits == 0x1) {
		return Condition::Z;
	}
	else if (bits == 0x2) {
		return Condition::NC;
	}
	else if (bits == 0x3) {
		return Condition::C;
	}
}

void SharpLR35902::flag(Condition c, bool set) {
	if (set) { //setting flags
		if (c == Condition::Z) {
			r.f |= 0b10000000;
		}
		else if (c == Condition::N) {
			r.f |= 0b01000000;
		}
		else if (c == Condition::H) {
			r.f |= 0b00100000;
		}
		else if (c == Condition::C) {
			r.f |= 0b00010000;
		}
	}
	else { //unsetting flags
		if (c == Condition::Z) {
			r.f &= 0b01111111;
		}
		else if (c == Condition::N) {
			r.f &= 0b10111111;
		}
		else if (c == Condition::H) {
			r.f &= 0b11011111;
		}
		else if (c == Condition::C) {
			r.f &= 0b11101111;
		}
	}
}

bool constexpr SharpLR35902::getFlag(Condition c) {
	if (c == Condition::Z) {
		return (r.f & 0b10000000) >> 7;
	}
	else if (c == Condition::N) {
		return (r.f & 0b01000000) >> 6;
	}
	else if (c == Condition::H) {
		return (r.f & 0b00100000) >> 5;
	}
	else if (c == Condition::C) {
		return (r.f & 0b00010000) >> 4;
	}
	else if (c == Condition::NZ) {
		return !((r.f & 0b10000000) >> 7);
	}
	else if (c == Condition::NC) {
		return !((r.f & 0b00010000) >> 4);
	}
	else {
		std::cout << "Attempted to get invalid flag!" << std::endl;
	}
}

R8 SharpLR35902::r8FromBits(uint8_t bits) {
	bits = ((bits & 0b00111000) >> 3);
	switch (bits) {
	case 0x0: return R8::b;
	case 0x1: return R8::c;
	case 0x2: return R8::d;
	case 0x3: return R8::e;
	case 0x4: return R8::h;
	case 0x5: return R8::l;
	case 0x6: return R8::MEM;
	case 0x7: return R8::a;
	}
}

R8 SharpLR35902::r8FromSrcBits(uint8_t bits) {
	bits = (bits & 0b00000111);
	switch (bits) {
	case 0x0: return R8::b;
	case 0x1: return R8::c;
	case 0x2: return R8::d;
	case 0x3: return R8::e;
	case 0x4: return R8::h;
	case 0x5: return R8::l;
	case 0x6: return R8::MEM;
	case 0x7: return R8::a;
	}
}

void SharpLR35902::setR(R8 dest, uint8_t value) {
	switch (dest) {
	case R8::a: r.a = value; break;
	case R8::b: r.b = value; break;
	case R8::c: r.c = value; break;
	case R8::d: r.d = value; break;
	case R8::e: r.e = value; break;
	case R8::h: r.h = value; break;
	case R8::l: r.l = value; break;
	case R8::f: r.f = value; break;
	case R8::MEM:
		bus.write(r.hl, value);
		break;
	}
}

uint8_t SharpLR35902::getR(R8 src) {
	switch (src) {
	case R8::a: return r.a;
	case R8::b: return r.b;
	case R8::c: return r.c;
	case R8::d: return r.d;
	case R8::e: return r.e;
	case R8::h: return r.h;
	case R8::l: return r.l;
	case R8::f: return r.f;
	case R8::MEM: return bus.read(r.hl);
	}
}

void SharpLR35902::setR16(R16 dest, uint16_t value) {
	switch (dest) {
	case R16::af: r.af = value; break;
	case R16::bc: r.bc = value; break;
	case R16::de: r.de = value; break;
	case R16::hl: r.hl = value; break;
	case R16::sp: r.sp = value; break;
	}
}

uint16_t SharpLR35902::getR16(R16 src) {
	switch (src) {
	case R16::af: return r.af;
	case R16::bc: return r.bc;
	case R16::de: return r.de;
	case R16::hl: return r.hl;
	case R16::sp: return r.sp;
	case R16::hlp: return r.hl++;
	case R16::hlm: return r.hl--;
	}
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
~~~~~~~~~~~OPCode Functions~~~~~~~~~~~~~
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

void SharpLR35902::STOP() {

}

void SharpLR35902::HALT() {
	halt = true;
}

void SharpLR35902::ld_u16_SP() {
	uint16_t addr = getNext16();
	bus.write(addr, (r.sp & 0xFF));
	bus.write(addr + 1, (r.sp >> 8));
}

void SharpLR35902::LD_FF00_u8_A() {
	bus.write(0xFF00 + getNext(), r.a);
}

void SharpLR35902::ADD_SP_i8() {
	int8_t signedByte = (int8_t)getNext();
	uint16_t oldSP = r.sp;
	int16_t address = (int16_t)oldSP + signedByte;

	flag(Condition::Z, false);
	flag(Condition::N, false);
	flag(Condition::H, ((r.sp ^ signedByte ^ (address & 0xFFFF)) & 0x10) == 0x10);
	flag(Condition::C, ((r.sp ^ signedByte ^ (address & 0xFFFF)) & 0x100) == 0x100);
	bus.addCycles(4); // internal
	r.sp = (uint16_t)address;
}

void SharpLR35902::LD_A_FF00_u8() {
	r.a = bus.read(0xFF00 + getNext());
}

void SharpLR35902::LD_HL_SP_i8() {
	int8_t signedByte = (int8_t)getNext();
	int16_t address = (int16_t)r.sp + signedByte;

	flag(Condition::Z, false);
	flag(Condition::N, false);
	flag(Condition::H, ((r.sp ^ signedByte ^ (address & 0xFFFF)) & 0x10) == 0x10);
	flag(Condition::C, ((r.sp ^ signedByte ^ (address & 0xFFFF)) & 0x100) == 0x100);
	bus.addCycles(4); // internal
	r.hl = (uint16_t)address;
}

void SharpLR35902::JP() {
	r.pc = getNext16();
}

void SharpLR35902::LD_FF00_C_A() {
	bus.write(0xFF00 + r.c, r.a);
}

void SharpLR35902::LD_A_FF00_C() {
	r.a = bus.read(0xFF00 + r.c);
}

void SharpLR35902::LD_U16_A() {
	bus.write(getNext16(), r.a);
}

void SharpLR35902::LD_A_U16() {
	r.a = bus.read(getNext16());
}

void SharpLR35902::CALL() {
	uint16_t addr = getNext16();
	bus.addCycles(4); // internal
	bus.write(--r.sp, ((r.pc) & 0xFF00) >> 8);
	bus.write(--r.sp, (r.pc) & 0x00FF);
	r.pc = addr;
}

void SharpLR35902::JR() {
	int8_t signedByte = getNext();
	int16_t currentAddr = r.pc;
	bus.addCycles(4); // internal modify pc
	r.pc = (uint16_t)(currentAddr + signedByte);
}

void SharpLR35902::CALL_ADDR(uint16_t addr) {
	r.sp--; bus.addCycles(4); // decrement sp
	bus.write(r.sp, ((r.pc) & 0xFF00) >> 8);
	r.sp--;
	bus.write(r.sp, (r.pc) & 0x00FF);
	bus.addCycles(4); // write pc
	r.pc = addr;
}

void SharpLR35902::RLCA() {
	uint8_t oldA = r.a;
	r.a = (r.a << 1) + ((r.a & 0b10000000) >> 7);
	flag(Condition::C, ((oldA & 0b10000000) >> 7));

	flag(Condition::Z, false);
	flag(Condition::N, false);
	flag(Condition::H, false);
}

void SharpLR35902::RRCA() {
	uint8_t oldA = r.a;
	r.a = (r.a >> 1) + ((oldA & 0b00000001) << 7);
	flag(Condition::C, (oldA & 0b00000001));

	flag(Condition::Z, false);
	flag(Condition::N, false);
	flag(Condition::H, false);
}

void SharpLR35902::RLA() {
	uint8_t oldA = r.a;
	r.a = (r.a << 1) + getFlag(Condition::C);
	flag(Condition::C, ((oldA & 0b10000000) >> 7));

	flag(Condition::Z, false);
	flag(Condition::H, false);
	flag(Condition::N, false);
}

void SharpLR35902::RRA() {
	uint8_t oldA = r.a;
	r.a = (r.a >> 1) + (getFlag(Condition::C) << 7);
	flag(Condition::C, (oldA & 0b00000001));

	flag(Condition::Z, false);
	flag(Condition::N, false);
	flag(Condition::H, false);
}

void SharpLR35902::DAA() {
	uint8_t correction = 0x0;
	if (getFlag(Condition::H) || (!getFlag(Condition::N) && (r.a & 0x0F) > 0x9)) {
		correction |= 0x6;
	}

	if (getFlag(Condition::C) || (!getFlag(Condition::N) && (r.a > 0x99))) {
		correction |= 0x60;
	}

	if (getFlag(Condition::N)) {
		r.a -= correction;
	}
	else {
		r.a += correction;
	}

	if (((correction << 2) & 0x100) != 0) {
		flag(Condition::C, true);
	}

	flag(Condition::H, false);
	flag(Condition::Z, r.a == 0);
}

void SharpLR35902::CPL() {
	r.a = ~r.a;
	flag(Condition::N, true);
	flag(Condition::H, true);
}

void SharpLR35902::SCF() {
	flag(Condition::N, false);
	flag(Condition::H, false);
	flag(Condition::C, true);
}

void SharpLR35902::CCF() {
	flag(Condition::C, !getFlag(Condition::C));
	flag(Condition::N, false);
	flag(Condition::H, false);
}

void SharpLR35902::ADD(uint8_t value) {
	flag(Condition::H, ((r.a & 0xF) + (value & 0xF)) > 0xF);
	flag(Condition::C, ((r.a + value) & 0x100) != 0);
	r.a += value;
	flag(Condition::Z, r.a == 0);
	flag(Condition::N, false);
}

void SharpLR35902::ADC(uint8_t value) {
	bool oldC = getFlag(Condition::C);
	flag(Condition::H, ((r.a & 0xF) + ((value) & 0xF) + getFlag(Condition::C)) > 0xF);
	flag(Condition::C, ((r.a + value + getFlag(Condition::C)) & 0x100) != 0);
	r.a += value + oldC;
	flag(Condition::Z, r.a == 0);
	flag(Condition::N, false);
}

void SharpLR35902::SUB(uint8_t value) {
	flag(Condition::C, value > r.a);
	flag(Condition::H, ((r.a & 0xF) - (value & 0xF) < 0));
	r.a -= value; //subtract
	flag(Condition::Z, r.a == 0);
	flag(Condition::N, true);
}

void SharpLR35902::SBC(uint8_t value) {
	bool oldCarry = getFlag(Condition::C);
	flag(Condition::C, (r.a - value - getFlag(Condition::C) < 0));
	flag(Condition::H, (((r.a & 0xF) - (value & 0xF) - oldCarry) < 0));
	r.a -= (value + oldCarry); //subtract
	flag(Condition::Z, r.a == 0);
	flag(Condition::N, true);
}

void SharpLR35902::AND(uint8_t value) {
	r.a &= value;

	flag(Condition::Z, r.a == 0);
	flag(Condition::N, false);
	flag(Condition::H, true);
	flag(Condition::C, false);
}

void SharpLR35902::OR(uint8_t value) {
	r.a |= value;

	flag(Condition::Z, r.a == 0x0);
	flag(Condition::N, false);
	flag(Condition::H, false);
	flag(Condition::C, false);
}

void SharpLR35902::XOR(uint8_t value) {
	r.a ^= value;
	flag(Condition::Z, r.a == 0);
	flag(Condition::N, false);
	flag(Condition::H, false);
	flag(Condition::C, false);
}

void SharpLR35902::CP(uint8_t value) {
	uint8_t result = r.a - value;
	flag(Condition::Z, result == 0);
	flag(Condition::N, true);
	flag(Condition::H, ((r.a & 0xF) - (value & 0xF) < 0));
	flag(Condition::C, value > r.a);
}

void SharpLR35902::RET() {
	uint16_t address = bus.read(r.sp) + (bus.read(r.sp + 1) << 8);
	r.sp += 2;
	bus.addCycles(4); //set pc?
	r.pc = address;
}

void SharpLR35902::RETI() {
	IME = true;
	uint16_t address = bus.read(r.sp) + (bus.read(r.sp + 1) << 8);
	r.sp += 2;
	bus.addCycles(4); //set pc?
	r.pc = address;
}

void SharpLR35902::JP_HL() {
	r.pc = r.hl;
}

void SharpLR35902::LD_SP_HL() {
	r.sp = r.hl;
	bus.addCycles(4); //internal
}

void SharpLR35902::JP_U16() {
	r.pc = getNext16();
	bus.addCycles(4); //branch decision?
}

void SharpLR35902::DI() {
	IME = false;
}

void SharpLR35902::EI() {
	EI_IME = true;
}

void SharpLR35902::RLC(R8 reg) {
	uint8_t oldR = getR(reg);
	setR(reg, (getR(reg) << 1) + ((getR(reg) & 0b10000000) >> 7));
	flag(Condition::C, ((oldR & 0b10000000) >> 7));

	flag(Condition::Z, getR(reg) == 0);
	flag(Condition::N, false);
	flag(Condition::H, false);
}

void SharpLR35902::RRC(R8 reg) {
	flag(Condition::C, (getR(reg) & 0b00000001));
	uint8_t oldReg = getR(reg);
	setR(reg, (getR(reg) >> 1) + ((oldReg & 0b00000001) << 7));
	flag(Condition::Z, getR(reg) == 0);
	flag(Condition::N, false);
	flag(Condition::H, false);
}

void SharpLR35902::RL(R8 reg) {
	uint8_t oldR = getR(reg);
	setR(reg, (getR(reg) << 1) + getFlag(Condition::C));
	flag(Condition::C, ((oldR & 0b10000000) >> 7));

	flag(Condition::Z, getR(reg) == 0);
	flag(Condition::H, false);
	flag(Condition::N, false);
}

void SharpLR35902::RR(R8 reg) {
	uint8_t oldR = getR(reg);
	setR(reg, (getR(reg) >> 1) + (getFlag(Condition::C) << 7));
	flag(Condition::C, (oldR & 0b00000001));

	flag(Condition::Z, getR(reg) == 0);
	flag(Condition::N, false);
	flag(Condition::H, false);
}

void SharpLR35902::SLA(R8 reg) {
	uint8_t oldR = getR(reg);
	setR(reg, (getR(reg) << 1));
	flag(Condition::C, ((oldR & 0b10000000) >> 7));

	flag(Condition::Z, getR(reg) == 0);
	flag(Condition::H, false);
	flag(Condition::N, false);
}

void SharpLR35902::SRA(R8 reg) {
	uint8_t oldR = getR(reg);
	setR(reg, (getR(reg) >> 1) + (oldR & 0b10000000));
	flag(Condition::C, (oldR & 0b00000001));

	flag(Condition::Z, getR(reg) == 0);
	flag(Condition::N, false);
	flag(Condition::H, false);
}

void SharpLR35902::SWAP(R8 reg) {
	setR(reg, ((getR(reg) & 0x0F) << 4) + ((getR(reg) & 0xF0) >> 4));
	flag(Condition::Z, getR(reg) == 0);
	flag(Condition::N, false);
	flag(Condition::H, false);
	flag(Condition::C, false);
}

void SharpLR35902::SRL(R8 reg) {
	flag(Condition::C, (getR(reg) & 0b00000001));
	setR(reg, getR(reg) >> 1);
	flag(Condition::Z, getR(reg) == 0);
	flag(Condition::N, false);
	flag(Condition::H, false);
}