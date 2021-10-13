#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#define FILEREAD_MAXSIZE 1024*1024

size_t File_Read(char* FileName, char** buffer);
uint8_t File_Close();
void File_AppendCrc(uint32_t Value, uint8_t Size, bool BigEndian);
