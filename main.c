#include <argp.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "crc.h"
#include "file_read.h"

enum outFormat_t {
	outFormat_dec = 0,
	outFormat_hex,
	outFormat_bin,
	outFormat_oct
};

struct arguments {
	char* inputString;
	bool ignNonprintable;
	bool ignPrefix;
	enum outFormat_t outFormat;
	bool stringMode;

	uint8_t CRC_Resolution;
	bool CRC_LeftShift;
	uint32_t CRC_Poly;
	uint32_t CRC_IV;
	uint32_t CRC_XOR;

	char* filename;
	uint8_t WriteToFileN;
	bool BigEndian;
	bool Debug;
};

int main(int argc, char *argv[]);
void CRC_Handle(uint8_t* ptr, size_t size, struct arguments* arguments);
void CRC_ShowResult(uint32_t result, uint8_t Resolution, enum outFormat_t outFormat, bool ignPrefix);



const char *argp_program_version = "programname programversion";
const char *argp_program_bug_address = "<your@email.address>";
static char doc[] = "Your program description.";
static char args_doc[] = "";
static struct argp_option options[] = {
	{ 0, 0, 0, 0, "Input parsing", 1},
	{ "input", 'i', "string", 0, "Use STRING as input value", 1},
	{ "ascii", 'a', 0, 0, "Ignore non-printable characters", 1},
// 	{ "lines", 'l', 0, 0, "Parse line-by-line", 1},

	{ 0, 0, 0, 0, "CRC Options", 2},
	{ "size", 's', "size", 0, "CRC size (bits)", 2},
	{ "poly", 'p', "POLY", 0, "Poly", 2},
	{ "IV", 'v', "value", 0, "Initial value", 2},
	{ "end-xor", 'x', "mask", 0, "XOR result", 2},
	{ "left", 'l', 0, 0, "Shift left", 2},

	{ 0, 0, 0, 0, "Output formats", 3},
	{ "bin", 'b', 0, 0, "Output in BIN", 3},
	{ "oct", 'o', 0, 0, "Output in OCT", 3},
	{ "dec", 'd', 0, 0, "Output in DEC", 3},
	{ "hex", 'h', 0, 0, "Output in HEX", 3},
	{ "no-prefix", '0', 0, 0, "Remove start prefix (0x, 0b...)", 3},

	{ "file", 'f', "FILENAME", 0, "Use file, instead of string/stdin", 1},
	{ "write-le", 'w', "N", 0, "Write CRC to end of file (or to stdout). Little-endian", 4},
	{ "write-be", 'W', "N", 0, "Write CRC to end of file (or to stdout). Big-endian", 4},

	{ "debug", '?', 0, 0, "Debug mode", 5},
	{ 0 }
};

static error_t parse_opt(int key, char *arg, struct argp_state *state) {
	struct arguments *arguments = state->input;
	switch (key) {
		case 'i':
			if (arg) {
				arguments->inputString = calloc(strlen(arg)+1, sizeof(char));
				strcpy(arguments->inputString, arg);
				arguments->stringMode = true;
			}
			break;
		case 'a':
			arguments->ignNonprintable = true;
			break;

		case 's':
			arguments->CRC_Resolution = strtoul(arg, 0, 0);
			break;
		case 'p':
			arguments->CRC_Poly = strtoul(arg, 0, 0);
			break;
		case 'v':
			arguments->CRC_IV = strtoul(arg, 0, 0);
			break;
		case 'x':
			arguments->CRC_XOR = strtoul(arg, 0, 0);
			break;
		case 'l':
			arguments->CRC_LeftShift = true;
			break;

		case 'b':
			arguments->outFormat = outFormat_bin;
			break;
		case 'o':
			arguments->outFormat = outFormat_oct;
			break;
		case 'd':
			arguments->outFormat = outFormat_dec;
			break;
		case 'h':
			arguments->outFormat = outFormat_hex;
			break;
		case '0':
			arguments->ignPrefix = true;
			break;

		case 'w':
			arguments->WriteToFileN = strtoul(arg, 0, 0);
			arguments->BigEndian = false;
			break;
		case 'W':
			arguments->WriteToFileN = strtoul(arg, 0, 0);
			arguments->BigEndian = true;
			break;

		case 'f':
			if (arg) {
				arguments->filename = calloc(strlen(arg)+1, sizeof(char));
				strcpy(arguments->filename, arg);
			}
			break;
		case '?':
			arguments->Debug = true;
			break;
		case ARGP_KEY_ARG: return 0;
		default: return ARGP_ERR_UNKNOWN;
	}
	return 0;
}

static struct argp argp = { options, parse_opt, args_doc, doc, 0, 0, 0 };

int main(int argc, char *argv[])
{
	struct arguments arguments = {0};
	bool errset = false;
	argp_parse(&argp, argc, argv, 0, 0, &arguments);

// 	CRC_Config(16, false, 0xA001); // CRC16_ARC
// 	./crcc -s 16 -p 40961
// 	@tn=20&dp=100&cs=24051#
// 	-l -s 16 -p 4129 -i "tn=20&dp=100&cs="
	if (arguments.CRC_Poly == 0) {
		fprintf(stderr, "CRC Poly not set. Use -p to set");
		errset = true;
	}

	if (arguments.Debug) {
		if (arguments.filename)
			fprintf(stderr, "In file: %s\n", arguments.filename);
		if (arguments.inputString)
			fprintf(stderr, "In str: %s\n", arguments.inputString);
	}

	if (!errset) {
		size_t filesize = 0;
		if (arguments.inputString)
			filesize = strlen(arguments.inputString);
		else
			filesize = File_Read(arguments.filename, &arguments.inputString);
		CRC_Handle(arguments.inputString, filesize, &arguments);
	}

	if (arguments.filename)
		free(arguments.filename);
	if (arguments.inputString)
		free(arguments.inputString);
	File_Close();

	return 0;
}

void CRC_Handle(uint8_t* ptr, size_t size, struct arguments* arguments) {
	CRC_Config(arguments->CRC_Resolution, arguments->CRC_LeftShift, arguments->CRC_Poly);
	uint32_t buffer = arguments->CRC_IV;

	char chr = 'x'; // Non-zero for start
	size_t backcounter = size;
	bool cont = true;

	while (cont = (
		(arguments->stringMode && (chr != '\0')) || \
		(!arguments->stringMode && (backcounter > 0))
	)) {
		chr = *(ptr++);
		if (arguments->stringMode && !chr) cont = 0;
		if (arguments->stringMode && arguments->ignNonprintable \
			&& ((chr < 0x20) || (chr > 0x7F))) cont = 0;

		if (cont) {
			CRC_Round(&buffer, chr);
			if (arguments->Debug)
				fprintf(stderr, "Char [%c] = 0x%02x\n", chr, chr);
		}

		if (backcounter) backcounter--;
	}

	buffer ^= arguments->CRC_XOR;

	if (arguments->WriteToFileN) {
		File_AppendCrc(buffer, arguments->WriteToFileN, arguments->BigEndian);
	} else {
		CRC_ShowResult(buffer, arguments->CRC_Resolution, arguments->outFormat, arguments->ignPrefix);
	}
}

void CRC_ShowResult(uint32_t result, uint8_t Resolution, enum outFormat_t outFormat, bool ignPrefix) {
	switch (outFormat) {
		case outFormat_dec:
			fprintf(stdout, "%ld\n", result);
			break;
		case outFormat_hex:
			if (ignPrefix)
				fprintf(stdout, "%lx\n", result);
			else
				fprintf(stdout, "0x%lx\n", result);
			break;
		case outFormat_bin:
			if (ignPrefix)
				fprintf(stdout, "%lb\n", result);
			else
				fprintf(stdout, "0b%lb\n", result);
			break;
		case outFormat_oct:
			if (ignPrefix)
				fprintf(stdout, "%lo\n", result);
			else
				fprintf(stdout, "0%lo\n", result);
			break;
	}

}
