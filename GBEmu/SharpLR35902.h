#include <iostream>
#include <iomanip>
#include <cstdint>
#include <map>
#include <fstream>
#include "Register.h"
#include "AddressBus.h"

enum class R8 {
	b, c, d, e, h, l, a, f, MEM
};

enum class R16 {
	bc, de, hl, af, sp, hlp, hlm
};

enum class Condition {
	NZ, Z, NC, C, N, H
};

class SharpLR35902 {
private:
	//CPU
	Register r;
	bool IME, EI_IME;
	AddressBus bus;

	//ROM
	uint8_t* ROM;
	unsigned int ROMSize;

	//funcs
	void loadROMBanktoMemory(int bank);
	void readROM();
	void setup();
	void log(std::ofstream& file);

	//helper
	void init();
	void decodeOps(uint8_t op);

	void setR(R8 dest, uint8_t value);
	uint8_t getR(R8 src);
	void setR16(R16 dest, uint16_t value);
	uint16_t getR16(R16 src);
	R16 r16FromBits(uint8_t bits, int group);
	R8 r8FromBits(uint8_t bits);
	R8 r8FromSrcBits(uint8_t bits);

	Condition constexpr getCond(uint8_t bits);
	bool constexpr getFlag(Condition c);
	void flag(Condition c, bool set);

	uint8_t getNext();
	uint16_t getNext16();


	//ALU functions define within header

	//flag ops
	void RLCA();
	void RRCA();
	void RLA();
	void RRA();
	void DAA();
	void CPL();
	void SCF();
	void CCF();

	//ALU ops
	void ADD(uint8_t value);
	void ADC(uint8_t value);
	void SUB(uint8_t value);
	void SBC(uint8_t value);
	void AND(uint8_t value);
	void XOR(uint8_t value);
	void OR(uint8_t value);
	void CP(uint8_t value);

	void RET();
	void RETI();
	void JP_HL();
	void LD_SP_HL();

	void JP_U16();
	void DI();
	void EI();

	//CB shift/rotates
	void RLC(R8 reg);
	void RRC(R8 reg);
	void RL(R8 reg);
	void RR(R8 reg);
	void SLA(R8 reg);
	void SRA(R8 reg);
	void SWAP(R8 reg);
	void SRL(R8 reg);

	void nop() {}

	void decodeCB() {
		(this->*lutCB[bus.read(r.pc++)])();
	}

	template<R16 dest, uint16_t value>
	void ld_r16_r16() {
		setR16(dest, value);
	}

	void ld_u16_SP() {
		uint16_t addr = getNext16();
		bus.write(addr, (r.sp & 0xFF));
		bus.write(addr + 1, (r.sp >> 8));
	}

	void JP() {
		r.pc = getNext16();
	}

	void JR() {
		int8_t signedByte = getNext();
		int16_t currentAddr = r.pc;
		r.pc = (uint16_t)(currentAddr + signedByte);
	}

	template<uint8_t op>
	void JR_CC() {
		int8_t signedByte = getNext();
		if (getFlag(getCond(op))) {
			r.pc = (uint16_t)((int16_t)r.pc + signedByte);
		}
	}

	template<uint8_t op>
	void ld_r16_u16() {
		setR16(r16FromBits(op, 1), getNext16());
	}

	template<uint8_t op>
	void ADD_HL_R16() {
		flag(Condition::H, ((getR16(r16FromBits(op, 1)) & 0xFFF) + (getR16(R16::hl) & 0xFFF)) > 0xFFF);
		flag(Condition::C, ((getR16(r16FromBits(op, 1)) + getR16(R16::hl)) & 0x10000) != 0);
		setR16(R16::hl, getR16(R16::hl) + getR16(r16FromBits(op, 1)));
		flag(Condition::N, false);
	}

	template<uint8_t op>
	void LD_MEM_A() {
		bus.write(getR16(r16FromBits(op, 2)), getR(R8::a));
	}

	template<uint8_t op>
	void LD_A_MEM() {
		setR(R8::a, bus.read(getR16(r16FromBits(op, 2))));
	}

	template<uint8_t op>
	void INC_R16() {
		setR16(r16FromBits(op, 1), getR16(r16FromBits(op, 1)) + 1);
	}

	template<uint8_t op>
	void DEC_R16() {
		setR16(r16FromBits(op, 1), getR16(r16FromBits(op, 1)) - 1);
	}

	template<uint8_t op>
	void INC_R8() {
		setR(r8FromBits(op), getR(r8FromBits(op))+1);

		flag(Condition::Z, getR(r8FromBits(op)) == 0);
		flag(Condition::N, false);
		flag(Condition::H, (getR(r8FromBits(op)) & 0x0F) == 0x00);
	}

	template<uint8_t op>
	void DEC_R8() {
		flag(Condition::H, ((getR(r8FromBits(op)) & 0xF) - (1 & 0xF) < 0));
		setR(r8FromBits(op), getR(r8FromBits(op)) - 1);
		flag(Condition::Z, getR(r8FromBits(op)) == 0);
		flag(Condition::N, true);
	}

	template<uint8_t op>
	void ld_r8_u8() {
		setR(r8FromBits(op), getNext());
	}

	template<uint8_t op>
	void ld_r8_r8() {
		setR(r8FromBits(op), getR(r8FromSrcBits(op)));
	}

	template<uint8_t op>
	void OPCODE_GROUP1() {
		uint8_t bits = (op & 0b00111000) >> 3;
		switch (bits) {
		case 0x0: RLCA(); break;
		case 0x1: RRCA(); break;
		case 0x2: RLA(); break;
		case 0x3: RRA(); break;
		case 0x4: DAA(); break;
		case 0x5: CPL(); break;
		case 0x6: SCF(); break;
		case 0x7: CCF(); break;
		}
	}

	template<uint8_t op>
	void ALU() {
		uint8_t bits = (op & 0b00111000) >> 3;
		uint8_t value = getR(r8FromSrcBits(op));
		switch (bits) {
		case 0x0: ADD(value); break;
		case 0x1: ADC(value); break;
		case 0x2: SUB(value); break;
		case 0x3: SBC(value); break;
		case 0x4: AND(value); break;
		case 0x5: XOR(value); break;
		case 0x6: OR(value); break;
		case 0x7: CP(value); break;
		}
	}

	template<uint8_t op>
	void RET_CC() {
		if (getFlag(getCond(op))) {
			uint16_t address = bus.read(r.sp) + (bus.read(r.sp + 1) << 8);
			r.sp += 2;
			r.pc = address;
		}
	}

	void LD_FF00_u8_A() {
		bus.write(0xFF00 + getNext(), r.a);
	}

	void ADD_SP_i8() {
		int8_t signedByte = (int8_t)getNext();
		uint16_t oldSP = r.sp;
		int16_t address = (int16_t)oldSP + signedByte;

		flag(Condition::Z, false);
		flag(Condition::N, false);
		flag(Condition::H, ((r.sp ^ signedByte ^ (address & 0xFFFF)) & 0x10) == 0x10);
		flag(Condition::C, ((r.sp ^ signedByte ^ (address & 0xFFFF)) & 0x100) == 0x100);

		r.sp = (uint16_t)address;
	}

	void LD_A_FF00_u8() {
		r.a = bus.read(0xFF00 + getNext());
	}

	void LD_HL_SP_i8() {
		int8_t signedByte = (int8_t)getNext();
		int16_t address = (int16_t)r.sp + signedByte;

		flag(Condition::Z, false);
		flag(Condition::N, false);
		flag(Condition::H, ((r.sp ^ signedByte ^ (address & 0xFFFF)) & 0x10) == 0x10);
		flag(Condition::C, ((r.sp ^ signedByte ^ (address & 0xFFFF)) & 0x100) == 0x100);

		r.hl = (uint16_t)address;
	}

	template<uint8_t op>
	void POP_R16() {
		uint16_t address = bus.read(r.sp) + (bus.read(r.sp + 1) << 8);
		r.sp += 2;

		R16 reg = r16FromBits(op, 3);
		if (reg == R16::af) {
			r.af = address;
			r.f &= 0b11110000;
		}

		else {
			setR16(reg, address);
		}
		
	}

	template<uint8_t op>
	void OPCODE_GROUP2() {
		uint8_t bits = (op & 0b00110000) >> 4;
		switch (bits) {
		case 0x0: RET(); break;
		case 0x1: RETI(); break;
		case 0x2: JP_HL(); break;
		case 0x3: LD_SP_HL(); break;
		}
	}

	template<uint8_t op>
	void JP_CC() {
		uint16_t addr = getNext16();
		if (getFlag(getCond(op))) {
			r.pc = addr;
		}
	}

	void LD_FF00_C_A() {
		bus.write(0xFF00 + r.c, r.a);
	}

	void LD_A_FF00_C() {
		r.a = bus.read(0xFF00 + r.c);
	}

	void LD_U16_A() {
		bus.write(getNext16(), r.a);
	}

	void LD_A_U16() {
		r.a = bus.read(getNext16());
	}

	template<uint8_t op>
	void OPCODE_GROUP3() {
		uint8_t bits = (op & 0b00111000) >> 3;
		switch (bits) {
		case 0x0: JP_U16(); break;
		case 0x1: decodeCB(); break;
		case 0x6: DI(); break;
		case 0x7: EI(); break;
		}
	}

	template<uint8_t op> 
	void CALL_CC() {
		uint16_t addr = getNext16();
		if (getFlag(getCond(op))) {
			bus.write(--r.sp, ((r.pc) & 0xFF00) >> 8);
			bus.write(--r.sp, (r.pc) & 0x00FF);
			r.pc = addr;
		}
	}

	template<uint8_t op>
	void PUSH() {
		uint16_t reg = getR16(r16FromBits(op, 3));
		bus.write(--r.sp, ((reg) & 0xFF00) >> 8); //upper
		bus.write(--r.sp, (reg) & 0x00FF); //lower
	}

	void CALL() {
		uint16_t addr = getNext16();
		bus.write(--r.sp, ((r.pc) & 0xFF00) >> 8);
		bus.write(--r.sp, (r.pc) & 0x00FF);
		r.pc = addr;
	}

	template<uint8_t op>
	void ALU_U8() {
		uint8_t bits = (op & 0b00111000) >> 3;
		uint8_t value = getNext();
		switch (bits) {
		case 0x0: ADD(value); break;
		case 0x1: ADC(value); break;
		case 0x2: SUB(value); break;
		case 0x3: SBC(value); break;
		case 0x4: AND(value); break;
		case 0x5: XOR(value); break;
		case 0x6: OR(value); break;
		case 0x7: CP(value); break;
		}
	}

	template<uint8_t op>
	void RST() {
		uint16_t addr = (op & 0b00111000);
		bus.write(--r.sp, ((r.pc) & 0xFF00) >> 8);
		bus.write(--r.sp, (r.pc) & 0x00FF);
		r.pc = addr;
	}

	template<uint8_t op>
	void OPCODE_GROUP4() {
		uint8_t bit = (op & 0b00111000) >> 3;
		R8 reg = r8FromSrcBits(op);
		switch (bit) {
		case 0x0: RLC(reg); break;
		case 0x1: RRC(reg); break;
		case 0x2: RL(reg); break;
		case 0x3: RR(reg); break;
		case 0x4: SLA(reg); break;
		case 0x5: SRA(reg); break;
		case 0x6: SWAP(reg); break;
		case 0x7: SRL(reg); break;
		}
	}

	template<uint8_t op>
	void BIT() {
		uint8_t bit = (op & 0b00111000) >> 3;
		uint8_t reg = getR(r8FromSrcBits(op));
		switch (bit) {
		case 0x0: flag(Condition::Z, !(reg & 0b00000001)); break;
		case 0x1: flag(Condition::Z, !((reg & 0b00000010) >> 1)); break;
		case 0x2: flag(Condition::Z, !((reg & 0b00000100) >> 2)); break;
		case 0x3: flag(Condition::Z, !((reg & 0b00001000) >> 3)); break;
		case 0x4: flag(Condition::Z, !((reg & 0b00010000) >> 4)); break;
		case 0x5: flag(Condition::Z, !((reg & 0b00100000) >> 5)); break;
		case 0x6: flag(Condition::Z, !((reg & 0b01000000) >> 6)); break;
		case 0x7: flag(Condition::Z, !((reg & 0b10000000) >> 7)); break;
		}
		flag(Condition::N, false);
		flag(Condition::H, true);
	}

	template<uint8_t op>
	void RES() {
		uint8_t bit = (op & 0b00111000) >> 3;
		R8 reg = r8FromSrcBits(op);
		switch (bit) {
		case 0x0: setR(reg, getR(reg) & 0b11111110); break;
		case 0x1: setR(reg, getR(reg) & 0b11111101); break;
		case 0x2: setR(reg, getR(reg) & 0b11111011); break;
		case 0x3: setR(reg, getR(reg) & 0b11110111); break;
		case 0x4: setR(reg, getR(reg) & 0b11101111); break;
		case 0x5: setR(reg, getR(reg) & 0b11011111); break;
		case 0x6: setR(reg, getR(reg) & 0b10111111); break;
		case 0x7: setR(reg, getR(reg) & 0b01111111); break;
		}
	} 

	template<uint8_t op>
	void SET() {
		uint8_t bit = (op & 0b00111000) >> 3;
		R8 reg = r8FromSrcBits(op);
		switch (bit) {
		case 0x0: setR(reg, getR(reg) | 0b00000001); break;
		case 0x1: setR(reg, getR(reg) | 0b00000010); break;
		case 0x2: setR(reg, getR(reg) | 0b00000100); break;
		case 0x3: setR(reg, getR(reg) | 0b00001000); break;
		case 0x4: setR(reg, getR(reg) | 0b00010000); break;
		case 0x5: setR(reg, getR(reg) | 0b00100000); break;
		case 0x6: setR(reg, getR(reg) | 0b01000000); break;
		case 0x7: setR(reg, getR(reg) | 0b10000000); break;
		}
	}
	
	void HALT() {

	}

	void STOP() {

	}

	//compile time function pointer map for decoding opcodes
	typedef void(SharpLR35902::*fp)();
	std::map<uint8_t, fp> lut;
	std::map<uint8_t, fp> lutCB;

	//template for loop used to fill LUT at compile time
	//since this is a compile time function its okay to be inefficient
	template<int op>
	void fillLut() {
		if (op == 0b00001000) { //LD (u16), SP
			lut[op] = &SharpLR35902::ld_u16_SP;
		}
		else if (op == 0b00000000) { //STOP
			lut[op] = &SharpLR35902::STOP;
		}
		else if (op == 0b00011000) { //JR
			lut[op] = &SharpLR35902::JR;
		}
		else if ((op & 0b11100111) == 0b00100000) { //JR Cond
			lut[op] = &SharpLR35902::JR_CC<op>;
		}
		else if ((op & 0b11001111) == 0b00000001) { //LD r16, u16
			lut[op] = &SharpLR35902::ld_r16_u16<op>;
		}
		else if ((op & 0b11001111) == 0b00001001) {  //ADD HL, r16
			lut[op] = &SharpLR35902::ADD_HL_R16<op>;
		}
		else if ((op & 0b11001111) == 0b00000010) { //LD (r16), A
			lut[op] = &SharpLR35902::LD_MEM_A<op>;
		}
		else if ((op & 0b11001111) == 0b00001010) { //LD A, (r16)
			lut[op] = &SharpLR35902::LD_A_MEM<op>;
		}
		else if ((op & 0b11001111) == 0b00000011) { //INC R16
			lut[op] = &SharpLR35902::INC_R16<op>;
		}
		else if ((op & 0b11001111) == 0b00001011) { //DEC R16
			lut[op] = &SharpLR35902::DEC_R16<op>;
		}
		else if ((op & 0b11000111) == 0b00000100) { //INC R8
			lut[op] = &SharpLR35902::INC_R8<op>;
		}
		else if ((op & 0b11000111) == 0b00000101) { //DEC R8
			lut[op] = &SharpLR35902::DEC_R8<op>;
		}
		else if ((op & 0b11000111) == 0b00000110) { //LD r8, u8
			lut[op] = &SharpLR35902::ld_r8_u8<op>;
		}
		else if ((op & 0b11000111) == 0b00000111) { //OPCODE TABLE 1
			lut[op] = &SharpLR35902::OPCODE_GROUP1<op>;
		}
		else if (op == 0b01110110) { //HALT
			lut[op] = &SharpLR35902::HALT;
		}
		else if ((op & 0b11000000) == 0b01000000 && (op & 0b11000000) != 0b01110110) { //LD r8, r8
			lut[op] = &SharpLR35902::ld_r8_r8<op>;
		}
		else if ((op & 0b11000000) == 0b10000000) { //ALU A, r8
			lut[op] = &SharpLR35902::ALU<op>;
		}
		else if ((op & 0b11100111) == 0b11000000) { //RET COND
			lut[op] = &SharpLR35902::RET_CC<op>;
		}
		else if (op == 0b11100000) { //LD (FF00+u8), A
			lut[op] = &SharpLR35902::LD_FF00_u8_A;
		}
		else if (op == 0b11101000) { //ADD SP, i8
			lut[op] = &SharpLR35902::ADD_SP_i8;
		}
		else if (op == 0b11110000) { //LD A, (FF00+u8)
			lut[op] = &SharpLR35902::LD_A_FF00_u8;
		}
		else if (op == 0b11111000) { //LD HL, SP+i8
			lut[op] = &SharpLR35902::LD_HL_SP_i8;
		}
		else if ((op & 0b11001111) == 0b11000001) { //POP r16
			lut[op] = &SharpLR35902::POP_R16<op>;
		}
		else if ((op & 0b11001111) == 0b11001001) { //(RET) (RETI) (JP HL) (LD SP, HL)
			lut[op] = &SharpLR35902::OPCODE_GROUP2<op>;
		}
		else if ((op & 0b11100111) == 0b11000010) { //JP COND
			lut[op] = &SharpLR35902::JP_CC<op>;
		}
		else if (op == 0b11100010) { //LD (FF00+C), A
			lut[op] = &SharpLR35902::LD_FF00_C_A;
		}
		else if (op == 0b11101010) { //LD (u16), A
			lut[op] = &SharpLR35902::LD_U16_A;
		}
		else if (op == 0b11110010) { //LD A, (0xFF00+C)
			lut[op] = &SharpLR35902::LD_A_FF00_C;
		}
		else if (op == 0b11111010) { //LD A, (u16)
			lut[op] = &SharpLR35902::LD_A_U16;
		}
		else if ((op & 0b11000111) == 0b11000011) { //OPCODE TABLE GROUP 3
			lut[op] = &SharpLR35902::OPCODE_GROUP3<op>;
		}
		else if ((op & 0b11100111) == 0b11000100) { //CALL Cond
			lut[op] = &SharpLR35902::CALL_CC<op>;
		}
		else if ((op & 0b11001111) == 0b11000101) { //PUSH r16
			lut[op] = &SharpLR35902::PUSH<op>;
		}
		else if (op == 0b11001101) { //CALL u16
			lut[op] = &SharpLR35902::CALL;
		}
		else if ((op & 0b11000111) == 0b11000110) { //ALU A, u8
			lut[op] = &SharpLR35902::ALU_U8<op>;
		}
		else if ((op & 0b11000111) == 0b11000111) { //RST
			lut[op] = &SharpLR35902::RST<op>;
		}

		//CB PREFIXED
		if ((op & 0b11000000) == 0b00000000) { //Shifts/Rotates
			lutCB[op] = &SharpLR35902::OPCODE_GROUP4<op>;
		}
		else if ((op & 0b11000000) == 0b01000000) { //BIT bit, r8
			lutCB[op] = &SharpLR35902::BIT<op>;
		}
		else if ((op & 0b11000000) == 0b10000000) { //RES bit, r8
			lutCB[op] = &SharpLR35902::RES<op>;
		}
		else if ((op & 0b11000000) == 0b11000000) { //SET bit, r8
			lutCB[op] = &SharpLR35902::SET<op>;
		}

		fillLut<op - 1>();
	}

	template<> //used to terminate the recursion
	void fillLut<0>() {
		lut[0x0] = &SharpLR35902::nop;
		lutCB[0x0] = &SharpLR35902::OPCODE_GROUP4<0>;
	}

public:
	SharpLR35902();
	void run();
};