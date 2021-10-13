#include <stdint.h>
#include <stdbool.h>

void CRC_Config(uint8_t Resolution, bool LeftShift, uint32_t Poly);
void CRC_Round(uint32_t* Buffer, uint8_t Data);
