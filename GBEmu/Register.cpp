#include "Register.h"

void Register::AF(uint16_t word) {
	A = word >> 0x8;
	F = word & 0xFF;
}

void Register::BC(uint16_t word) {
	B = word >> 0x8;
	C = word & 0xFF;
}

void Register::DE(uint16_t word) {
	D = word >> 0x8;
	E = word & 0xFF;
}

void Register::HL(uint16_t word) {
	H = word >> 0x8;
	L = word & 0xFF;
}