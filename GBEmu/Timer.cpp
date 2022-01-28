#include "Timer.h"
#include <iostream>

Timer::Timer(AddressBus& _bus) : bus(_bus) {
	DIV = bus.getRegister(0xFF04);
	TIMA = bus.getRegister(0xFF05);
	TMA = bus.getRegister(0xFF06);
	TAC = bus.getRegister(0xFF07);
	DIVCounter = 0;
	TIMACounter = 0;
}

void Timer::tick() {
	DIVCounter += bus.getCycles();
	if (DIVCounter >= 255) {
		DIVCounter -= 255;
		(*DIV)++;
	}
	if (getTIMAEnable()) {
		TIMACounter += bus.getCycles();
		while (TIMACounter >= getTIMAIncrement()) {
			TIMACounter -= getTIMAIncrement();

			if ((*TIMA) == 0xFF) {
				(*TIMA) = (*TMA);
				timerInterruptRequest();
			}
			else {
				(*TIMA)++;
			}
		}
	}
}

bool Timer::getTIMAEnable() {
	return (*TAC & 0b00000100) >> 2;
}

int Timer::getTIMAIncrement() {
	int mode = (*TAC & 0b00000011);
	switch (mode) {
	case 0:
		return 1024;
		break;
	case 1:
		return 16;
		break;
	case 2:
		return 64;
		break;
	case 3:
		return 256;
		break;
	}
}

void Timer::timerInterruptRequest() {
	uint8_t* IF = bus.getRegister(0xFF0F);
	*IF |= 0b00000100;
}