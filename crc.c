#include "crc.h"

static uint32_t _Resolution;
static uint32_t _CheckBit;
static uint32_t _Mask;
static uint32_t _Poly;
static uint8_t _DataShift;
static bool _LeftShift;

void CRC_Config(uint8_t Resolution, bool LeftShift, uint32_t Poly) {
	if (Resolution > 32)
		return;

	_Resolution = Resolution;
	if (LeftShift) {
		_CheckBit = (1L << (Resolution-1));
		_DataShift = Resolution-8;
	} else {
		_CheckBit = 1;
		_DataShift = 0;
	}
	_Mask = (1L << Resolution) - 1;

	_Poly = Poly;
	_LeftShift = LeftShift;
}

void CRC_Round(uint32_t* Buffer, uint8_t Data) {
	if (_Resolution == 0)
		return;

	uint32_t localBuffer = *Buffer;
	localBuffer ^= (Data << _DataShift);
	bool inv = false;
	for (int subround = 0; subround < 8; subround++) {
		inv = !!(localBuffer & _CheckBit);
		if (_LeftShift)
			localBuffer <<= 1;
		else
			localBuffer >>= 1;
		if (inv) {
			localBuffer ^= _Poly;
		}
		localBuffer &= _Mask;
	}
	*Buffer = localBuffer;
}
