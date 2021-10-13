#include "file_read.h"

static FILE* fd_read = NULL;
static FILE* fd_write = NULL;

size_t File_Read(char* FileName, char** buffer) {
	if (FileName) {
		fd_read = fopen(FileName, "r+");
		fd_write = fd_read;
	} else {
		fd_read = stdin;
		fd_write = stdout;
	}

	if (fd_read < 0)
		return 0;

	*buffer = malloc(FILEREAD_MAXSIZE);
	char* wptr = *buffer;

	while (!feof(fd_read)) {
		*wptr = getc(fd_read);
		if (!feof(fd_read))
			wptr++;
	}

// 	fprintf(stderr, "Size = %d\n", (size_t)(wptr - *buffer));

	return (size_t)(wptr - *buffer);
}

uint8_t File_Close() {
	if (fd_write == fd_read)
		fd_read = NULL;

	if (fd_read && (fd_read != stdin))
		fclose(fd_read);
	if (fd_write && (fd_write != stdout))
		fclose(fd_write);
	fd_read = NULL;
	fd_write = NULL;
}

void File_AppendCrc(uint32_t Value, uint8_t Size, bool BigEndian) {
	uint32_t TempBuffer = Value;
	if (BigEndian) {
		uint8_t ToShift = 8 * (Size-1);
		for (int i = 0; i < Size; i++) {
			fputc((TempBuffer >> ToShift) & 0xFF, fd_write);
			TempBuffer <<= 8;
		}
	} else {
		for (int i = 0; i < Size; i++) {
			fputc(TempBuffer & 0xFF, fd_write);
			TempBuffer >>= 8;
		}
	}

}
