/******************************************************************************
 *  ** WPC / WPC95 Lamp Matrix Driver Rom patcher **
 *  Copyright (C) 2010, John Honeycutt
 *  http://www.emmytech.com/arcade
 *
 *    This program is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE 
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF 
 * THE POSSIBILITY OF SUCH DAMAGE.
 *
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <ctype.h>
#include <string.h>
#include <stdint.h>
#include "wpc_lamp_patcher.h"

// rom_ver_seq: "REV. ^-%XA" where '^' can be any character.
static uint8_t rom_ver_seq[] = {0x52, 0x45, 0x56, 0x2e, 0x20, 0x00, 0x2d, 0x25, 0x58, 0x41};

// rom_ver_seq2: "REV. ^%MY-%XA" where '^' can be any character.
static uint8_t rom_ver_seq2[] = {0x52, 0x45, 0x56, 0x2e, 0x20, 0x00, 0x25, 0x4d, 0x59, 0x2d, 0x25, 0x58, 0x41};

/*******************************************************************************
*
******************************************************************************/
uint32_t calc_checksum(uint8_t *buf, int start, int sz) {
	uint32_t csum = 0;
	int end = start + sz;
	int x;

	for (x = start; x < end; x++) {
		csum += buf[x];
	}
	return csum;
}

/*******************************************************************************
*
* Returns: Offset of signature start or -1 for not found
*
******************************************************************************/
int search_for_sig(unsigned char *bp, int size, uint8_t *sig, int sig_sz) {
	int match = -1;
	int x, y;

	DBG("%s: search-sz=%d sig-sz=%d | %02x %02x %02x\n",
	__FUNCTION__, size, sig_sz, bp[0], bp[1], bp[2]);

	for (x=0; x < (size - sig_sz); x++) {
		for(y=0; y < sig_sz; y++) {
			//if (y > 0) printf("  tst: x=%d y=%d [%02x %02x %02x]\n", x, y, bp[x+0], bp[x+1], bp[x+2]);
			if (sig[y] == 0x00) {
				continue; // Variable data. skip check
			}
			
			if (sig[y] != bp[x+y]) {
				break;  // No match.
			}
		} //y
		if (y == sig_sz) {
			DBG("%s: Got match @ offset: 0x%04x (%d)\n", __FUNCTION__, x, x);
			match = x;
			break;
		}
	} //x

	return match;
}

/*******************************************************************************
*
* Search the ROM for the revision byte sequence
*
******************************************************************************/
static int wpc_find_rom_ver_letter(wpc_rom_t *rom, char letter) {
	int pos;
	
	DBG("%s: Search for sig1...\n", __FUNCTION__);
	pos = search_for_sig(&(rom->image[0]),
	rom->size,
	rom_ver_seq,
	sizeof(rom_ver_seq));
	if (pos != -1) {
		pos+= 5;
		DBG("%s: Got sig1 match @ offset: 0x%04x (%d): letter='%c'\n", __FUNCTION__, pos, pos, rom->image[pos]);
		return pos;
	}

	DBG("%s: Search for sig2...\n", __FUNCTION__);
	pos = search_for_sig(&(rom->image[0]),
	rom->size,
	rom_ver_seq2,
	sizeof(rom_ver_seq2));
	if (pos != -1) {
		pos+= 5;
		rom->ver.game_ver_Lx_format = 1;
		DBG("%s: Got sig2 match @ offset: 0x%04x (%d): letter='%c'\n", __FUNCTION__, pos, pos, rom->image[pos]);
		return pos;
	}

	printf("Error: Unable to update game version Letter\n");
	return -1;
}

/*******************************************************************************
*
******************************************************************************/
void wpc_print_rom_info(wpc_rom_t *rom, char* filename) {
	printf("ROM Info\n--------\n");
	if (filename != NULL) {
		printf("\tFile: %s\n", filename);
	}
	printf("\tSize: %d M-bits (%dK-Bytes)\n", rom->size / ROM_SZ_TO_MBIT_DIVISOR, rom->size / ONE_KB);
	
	if (rom->cs.csum_valid) {
		printf("\tChecksum Valid: %04x\n", rom->cs.csum);
	} else {
		printf("\tChecksum INVALID: Expected %04x Calculated %04x\n", rom->cs.csum, rom->cs.csum_calc);
	}
	if (rom->hard_error) {
		goto exit_out;
	}
	
	printf("\tSystem Version: %d.%d\n", rom->ver.os_major, rom->ver.os_minor);
	if (rom->ver.version_format == GAME_VER_FORMAT_1) {
		printf("\tGame Version (Newer format): %x.%x\n", rom->ver.game_major, rom->ver.game_minor);
	} else {
		if (rom->ver.game_ver_Lx_format == 1) {
			printf("\tGame Version (Old format): %c(x)-%x\n", rom->ver.game_ver_char, rom->ver.game_major);
		} else {
			printf("\tGame Version (Old format): %c-%x\n", rom->ver.game_ver_char, rom->ver.game_major);
		}
	}
	printf("\tSystem ROM Offset: %08x\n", rom->sys_rom_off);

	exit_out:
	printf("---------------------------\n");
}

/*******************************************************************************
*
******************************************************************************/
int wpc_validate_rom(wpc_rom_t *rom, wpc_rom_validate_level_t validate_level) {
	int mbits;
	uint32_t csum;
	unsigned int osv_addr;
	int offset;
	unsigned char cs;

	// Validate file size
	mbits = rom->size / ROM_SZ_TO_MBIT_DIVISOR;
	if ((rom->size % ROM_SZ_TO_MBIT_DIVISOR) ||
			((mbits != 1) && (mbits != 2) && (mbits != 4) && (mbits != 8))) {
		printf("Invalid Input file size: %d\n", rom->size);
		printf("Expecting 128KB, 256KB, 512KB, or 1024KB\n");
		return -1;
	}

	// Setup & Verify the checksum
	rom->cs.cscw_off  = rom->size - 20;
	rom->cs.csum_off  = rom->size - 18;
	rom->cs.cscw      = ntohs(*(uint16_t *)&(rom->image[rom->cs.cscw_off]));
	rom->cs.csum      = ntohs(*(uint16_t *)&(rom->image[rom->cs.csum_off]));
	DBG("CSCW: %04x  CSum: %04x\n", rom->cs.cscw, rom->cs.csum);

	csum = calc_checksum(rom->image, 0, rom->size);
	csum &= 0xffff;
	rom->cs.csum_calc = csum;
	DBG("Calculated checksum: %08x\n", csum);

	if (rom->cs.csum != csum) {
		DBG("Checksum Mismatch: ROM=%04x calculated=%04x\n", rom->cs.csum, csum);
		if (validate_level == WPC_VAL_FULL_CHECK) {
			rom->hard_error = 1;
			return -1;
		}
	} else {
		rom->cs.csum_valid = 1;
		DBG("Checksum Verified.\n");
	}

	// Find the OS Version and the Game Version
	rom->sys_rom_off = rom->size - WPC_SYS_ROM_SZ;
	osv_addr  = ntohs(*(uint16_t*)(&rom->image[rom->size - 2])) - 2;
	osv_addr &= WPC_SYS_ROM_CODE_MASK;
	DBG("SysRomOffset: %08x OSVerOffset: %08x OSVerAddr %08x\n",
	rom->sys_rom_off, osv_addr, rom->sys_rom_off + osv_addr);
	if ((rom->sys_rom_off + osv_addr + 1) >= rom->size) {
		printf("Error: Invalid OSVersion offset: %08x\n", rom->sys_rom_off + osv_addr);
		rom->hard_error = 1;
		return -1;
	}
	rom->ver.os_major = rom->image[rom->sys_rom_off + osv_addr];
	rom->ver.os_minor = rom->image[rom->sys_rom_off + osv_addr + 1];
	DBG("System Version: %d.%d\n", rom->ver.os_major, rom->ver.os_minor);
	
	rom->ver.version_format = GAME_VER_FORMAT_0;
	if ((rom->ver.os_major >=3) && (rom->ver.os_minor >=38)) {
		rom->ver.version_format = GAME_VER_FORMAT_1;
		offset = rom->size - 66;
		rom->ver.game_major = rom->image[offset];
		rom->ver.game_minor = rom->image[offset+1];

		// Special case of major eq 0x0d otherwise validate game version against lower byte of checksum.
		if ((rom->ver.game_major == 0x0d) && (rom->ver.game_minor == 0x00)) {
			rom->ver.version_format = GAME_VER_FORMAT_0;
		} else {
			cs = ((rom->ver.game_major & 0x0f) << 4) | ((rom->ver.game_minor & 0xf0) >> 4);
			if (cs != (rom->cs.csum & 0x00ff)) {
				printf("Error: Low byte of checksum does not match game version.\n");
				rom->cs.csum_valid = 0;
				rom->hard_error = 1;
				return -1;
			}
		}
	}

	if (rom->ver.version_format == GAME_VER_FORMAT_0) {
		// Version is LSB of checksum word 0xD0 and above indicate prototype
		rom->ver.game_major = rom->cs.csum & 0x00ff;
		rom->ver.game_minor = 0;

		if (rom->ver.game_major >=0xd0) {
			rom->ver.game_ver_char = 'P';
			rom->ver.game_major -= 0xd0;
		} else {
			// We are not a prototype rom. For unmodified WPC roms the letter should always be 'L'. However if this is the post-check we may have updated it to something else.
			if ((offset = wpc_find_rom_ver_letter(rom, rom->ver.game_ver_char)) == -1) {
				printf("Error: Game version string not found.\n");
				return -1;
			}
			rom->ver.game_ver_char = rom->image[offset];
		}
	}

	if (rom->ver.version_format == GAME_VER_FORMAT_1) {
		DBG("Newer Game Ver format: %x.%x\n", rom->ver.game_major, rom->ver.game_minor);
	} else {
		DBG("Old Game Ver format: %c-%x\n", rom->ver.game_ver_char, rom->ver.game_major);
	}

	return 0;
}

/*******************************************************************************
*
******************************************************************************/
int wpc_update_rom_version_checksum(wpc_rom_t *rom) {
	uint32_t calc_csum;
	uint8_t csum_lbyte;
	int offset;

	if (rom->ver.version_format == GAME_VER_FORMAT_1) {
		csum_lbyte = ((rom->ver.game_major & 0x0f) << 4) | ((rom->ver.game_minor & 0xf0) >> 4);

		// Update the game version in the rom image.
		offset = rom->size - 66;
		rom->image[offset]   = rom->ver.game_major;
		rom->image[offset+1] = rom->ver.game_minor;
	} else {
		if (rom->ver.game_ver_char == 'P') {

			// (Game version + 0xd0) is the checksum lower byte.
			csum_lbyte = rom->ver.game_major + 0xd0;
			DBG("%s: Prototype: New ver=%02x new csum lsb=%02x\n", __FUNCTION__, rom->ver.game_major, csum_lbyte);
		} else {
			// Update the version letter
			DBG("Updating ROM version letter to '%c'\n", rom->ver.game_ver_char);
			if ((offset = wpc_find_rom_ver_letter(rom, rom->ver.game_ver_char)) == -1) {
				return -1;
			}
			rom->image[offset] = rom->ver.game_ver_char;

			// Game version is the checksum lower byte.
			csum_lbyte = rom->ver.game_major;
			DBG("%s: new csum lsb=%02x\n", __FUNCTION__, rom->ver.game_major);
		}
	}

	// Init Correction Word & Checksum word for calculating checksum.
	*(uint16_t *)&(rom->image[rom->cs.cscw_off]) = htons(0x00FF);
	*(uint16_t *)&(rom->image[rom->cs.csum_off]) = htons(0x00FF);
	
	calc_csum = calc_checksum(rom->image, 0, rom->size);
	calc_csum &= 0xffff;
	DBG("Calculated CSum: %04x\n", calc_csum);
	
	// Keep the high calculated byte and replace the low byte byte with the game version info.
	rom->cs.csum = (calc_csum & 0xff00) | csum_lbyte;
	rom->cs.csum_calc = rom->cs.csum;

	// Adjust the correction word.
	// cw-high byte = 0xff - calculated csum low byte
	// cw-low byte  = 0xff - calculated csum high byte
	rom->cs.cscw = ((0x00ff - (calc_csum & 0x00ff)) << 8) | (0xff - (calc_csum >> 8));
	DBG("CSCW: %04x  CSum: %04x\n", rom->cs.cscw, rom->cs.csum);

	// Update the image.
	*(uint16_t *)&(rom->image[rom->cs.cscw_off]) = htons(rom->cs.cscw);
	*(uint16_t *)&(rom->image[rom->cs.csum_off]) = htons(rom->cs.csum);
	
	return 0;
}
