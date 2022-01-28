#pragma once
#include <cstdint>
#include "AddressBus.h"

class Timer {
private:
	AddressBus& bus;

	uint8_t* DIV;
	uint8_t* TIMA;
	uint8_t* TMA;
	uint8_t* TAC;

	int DIVCounter;
	int TIMACounter;
public:
	void tick();
	Timer(AddressBus& _bus);
	bool getTIMAEnable();
	int getTIMAIncrement();
	void timerInterruptRequest();
};