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

// Old driver version 1A.
// Last seen in OS 1.21
// BoP (L-6 1.21)
// HD  (L-3 1.18)
//
static uint8_t drvr_OLD_V1A_sig[] = 
{
    0x9e, 0x00, 0x30, 0x01, 0x96, 0x00, 0x48, 0x26,
    0x05, 0x8e, 0x00,
    0x00, 0x86, 0x00, 0x9f, 0x00, 0x97, 0x00, 0xe6,
    0x88, 0x10, 0x53, 0xe4, 0x08, 0xd7, 0x00, 0xe6,
    0x88, 0x10, 0xe4, 0x88, 0x18, 0xdb, 0x00, 0xd7,  
    0x00, 0xe6, 0x88, 0x20, 0xe4, 0x88, 0x28, 0xd7, 
    0x00, 0xe6, 0x88, 0x20, 0x53, 0xd4, 0x00, 0xdb,
    0x00, 0xd7, 0x00, 0xe6, 0x88, 0x30, 0xe4, 0x88, 
    0x38, 0xd7, 0x00, 0xe6, 0x88, 0x30, 0x53, 0xd4, 
    0x00, 0xdb, 0x00, 0x7f, 0x3f, 0xe4, 0xb7, 0x3f, 
    0xe5, 0xf7, 0x3f, 0xe4, 0x7e, 0x00, 0x00
};

// Old driver version 1B.
// Last seen in OS 2.12 
// PZ (F-4 2.12)
// PZ (L-3 2.10)
//
static uint8_t drvr_OLD_V1B_sig[] = 
{
    0x9e, 0x00, 0x30, 0x01, 0x96, 0x00, 0x48, 0x26,
    0x1a, 0x96, 0x00, 0x27, 0x11, 0x0a, 0x00, 0x2b,
    0x09, 0x7f, 0x3f, 0xe4, 0x7f, 0x3f, 0xe5, 0x7e,
    0x00, 0x00, 0x96, 0x00, 0x97, 0x00, 0x8e, 0x00,
    0x00, 0x86, 0x00, 0x9f, 0x00, 0x97, 0x00, 0xe6,
    0x88, 0x10, 0x53, 0xe4, 0x08, 0xd7, 0x00, 0xe6,
    0x88, 0x10, 0xe4, 0x88, 0x18, 0xdb, 0x00, 0xd7,  
    0x00, 0xe6, 0x88, 0x20, 0xe4, 0x88, 0x28, 0xd7, 
    0x00, 0xe6, 0x88, 0x20, 0x53, 0xd4, 0x00, 0xdb,
    0x00, 0xd7, 0x00, 0xe6, 0x88, 0x30, 0xe4, 0x88, 
    0x38, 0xd7, 0x00, 0xe6, 0x88, 0x30, 0x53, 0xd4, 
    0x00, 0xdb, 0x00, 0x7f, 0x3f, 0xe4, 0xb7, 0x3f, 
    0xe5, 0xf7, 0x3f, 0xe4, 0x7e, 0x00, 0x00
};

// Old driver version 1C code.  
// Last seen in OS 2.21 (Hurricane L-2)
//
static uint8_t drvr_OLD_V1C_sig[] = 
{
    0x9e, 0x00, 0x30, 0x01, 0x96, 0x00, 0x48, 0x26,
    0x1a, 0x96, 0x00, 0x27, 0x11, 0x0a, 0x00, 0x2b,
    0x09, 0x7f, 0x3f, 0xe4, 0x7f, 0x3f, 0xe5, 0x7e,
    0x00, 0x00, 0x96, 0x00, 0x97, 0x00, 0x8e, 0x00,
    0x00, 0x86, 0x00, 0x9f, 0x00, 0x97, 0x00, 0xe6,
    0x88, 0x10, 0x53, 0xe4, 0x08, 0xd7, 0x00, 0xe6,
    0x88, 0x10, 0xe4, 0x88, 0x18, 0xdb, 0x00, 0x7f,
    0x3f, 0xe4, 0xb7, 0x3f, 0xe5, 0xf7, 0x3f, 0xe4,
    0x96, 0x00, 0x27, 0x08, 0x8E, 0x00, 0x00, 0x9F,
    0x08, 0xB7, 0x3F, 0xF8, 0x7E, 0x00, 0x00 
};
 
// Old driver version 2 code. Was introduced somewhere 
// between OS version 2.22 and 2.43
//
static uint8_t drvr_OLD_V2_sig[] = 
{
    0x26, 0x19, 0x96, 0x00, 0x27, 0x10, 0x0a, 0x00, 
    0x2b, 0x08, 0x7f, 0x3f, 0xe4, 0x7f, 0x3f, 0xe5, 
    0x20, 0x55, 0x96, 0x00, 0x97, 0x00, 0x8e, 0x00, 
    0x00, 0x86, 0x01, 0x9f, 0x00, 0x97, 0x00, 0xe6, 
    0x89, 0x00, 0x10, 0x53, 0xe4, 0x89, 0x00, 0x08,
    0xd7, 0x00, 0xe6, 0x89, 0x00, 0x10, 0xe4, 0x89, 
    0x00, 0x18, 0xdb, 0x00, 0xd7, 0x00, 0xe6, 0x89, 
    0x00, 0x20, 0xe4, 0x89, 0x00, 0x28, 0xd7, 0x00, 
    0xe6, 0x89, 0x00, 0x20, 0x53, 0xd4, 0x00, 0xdb, 
    0x00, 0xd7, 0x00, 0xe6, 0x89, 0x00, 0x30, 0xe4, 
    0x89, 0x00, 0x38, 0xd7, 0x00, 0xe6, 0x89, 0x00, 
    0x30, 0x53, 0xd4, 0x00, 0xdb, 0x00, 0x7f, 0x3f, 
    0xe4, 0xb7, 0x3f, 0xe5, 0xf7 ,0x3f, 0xe4
};
static int drvr_OLD_V2_idx_load_instr_pos[] = { 31, 36, 42, 46, 0 };

// Yet another version of the driver code. 
// This is seen in FT L5 rom OS ver 2.46
//
static uint8_t drvr_OLD_VXa_sig[] = 
{

    0x48, 0x26, 0x20, 0x96, 0x67, 0x27, 0x17, 0x0a,
    0x00, 0x2b, 0x0f, 0x7f, 0x3f, 0xe4, 0x7f, 0x3f,
    0xe5, 0x20, 0x3f, 0x7f, 0x3f, 0xe4, 0x7f, 0x3f,
    0xf8, 0x3b, 0x96, 0x00, 0x97, 0x00, 0x8e, 0x02,
    0x80, 0x86, 0x01, 0x9f, 0x00, 0x97, 0x00, 0xe6,
    0x89, 0x00, 0x10, 0x53, 0xe4, 0x89, 0x00, 0x08,
    0xd7, 0x00, 0xe6, 0x89, 0x00, 0x10, 0xe4, 0x89,
    0x00, 0x18, 0xdb, 0x00, 0x7f, 0x3f, 0xe4, 0xb7,
    0x3f, 0xe5, 0xf7, 0x3f, 0xe4
};
static int drvr_OLD_VXa_idx_load_instr_pos[] = { 39, 44, 50, 54, 0 };


// New driver code. Introduced in 1995 somewhere between
// OS version 3.42 and 3.48.
// This is also the timeframe williams changed there game versioning
// from L-n to  n.m format.
//
static uint8_t drvr_new_sig[] = 
{
    0x26, 0x1a, 0x96, 0x00, 0x27, 0x11, 0x0a, 0x00, 
    0x2b, 0x09, 0x4f, 0xb7, 0x3f, 0xe4, 0xb7, 0x3f,
    0xe5, 0x20, 0x59, 0x96, 0x00, 0x97, 0x00, 0x8e,
    0x00, 0x00, 0x86, 0x01, 0x9f, 0x00, 0x97, 0x00,
    0xe6, 0x89, 0x00, 0x10, 0x53, 0xe4, 0x89, 0x00,
    0x08, 0xd7, 0x00, 0xe6, 0x89, 0x00, 0x10, 0xe4,
    0x89, 0x00, 0x18, 0xdb, 0x00, 0xd7, 0x00, 0xe6,
    0x89, 0x00, 0x20, 0xe4, 0x89, 0x00, 0x28, 0xd7, 
    0x00, 0x5f, 0xf7, 0x3f, 0xe5, 0xf7, 0x3f, 0xe4,
    0xe6, 0x89, 0x00, 0x20, 0x53, 0xd4, 0x00, 0xdb,
    0x00, 0xd7, 0x00, 0xe6, 0x89, 0x00, 0x30, 0xe4,
    0x89, 0x00, 0x38, 0xd7, 0x00, 0xe6, 0x89, 0x00,
    0x30, 0x53, 0xd4, 0x00, 0xdb, 0x00, 0xf7, 0x3f,
    0xe4, 0xb7, 0x3f, 0xe5
};

#define PATCH_V2_END_OFFSET (0x67)
static uint8_t rc_clr_code[]          = { 0x5f, 0xf7, 0x3f, 0xe5, 0xf7, 0x3f, 0xe4 };
static uint8_t wrt_row_wrt_col_code[] = { 0xf7 ,0x3f, 0xe4, 0xb7, 0x3f, 0xe5 };

static int wpc_patch_lamp_matrix_old_V2_drvr(wpc_rom_t *rom, unsigned long patch_start_offset, int extra_delay);
static int wpc_patch_lamp_matrix_old_VXa_drvr(wpc_rom_t *rom, unsigned long patch_start_offset);
static int wpc_patch_new_drvr_increase_delay(wpc_rom_t *rom, unsigned long patch_start_offset);
static int wpc_patch_lamp_matrix_old_V1_drvr(wpc_rom_t *rom, unsigned long patch_start_offset, int extra_delay, char drvr_ver);


/*******************************************************************************
 *
 ******************************************************************************/
int wpc_patch_lamp_matrix(wpc_rom_t *rom, int extra_delay, int *already_updated)
{
    int           rc;
    unsigned long irqh_offset, patch_start_offset;
    int           sig_offset;
    int           search_sz;
    unsigned int  osver;

    *already_updated = 0;
    osver = (rom->ver.os_major * 100) + rom->ver.os_minor;

    // Get address of IRQ Handler.
    //
    irqh_offset =  ntohs(*(uint16_t *)&(rom->image[rom->size - 8]));
    DBG("%s: IRQHandler addr: %08lx\n", __FUNCTION__, irqh_offset);
    irqh_offset &= WPC_SYS_ROM_CODE_MASK;
    irqh_offset += rom->sys_rom_off;
    DBG("%s: IRQHandler offset in image: %08lx(%lu)\n", __FUNCTION__, irqh_offset, irqh_offset);

    if ( (rom->size - irqh_offset - 0xff) < 600 ) {
        search_sz = rom->size - irqh_offset - 0xff;
    }
    else {
        search_sz = 600;
    }

    // See if rom already has the new driver
    //
    if ( osver >= 342 ) {
        DBG("%s: New Drv Search: start %08lx\n", __FUNCTION__, irqh_offset);
        sig_offset = search_for_sig(&(rom->image[irqh_offset]), search_sz, drvr_new_sig, sizeof(drvr_new_sig));
        if ( sig_offset != -1 ) {
            printf("FOUND updated (NEW) Lamp Driver Signature at address: 0x%08lx\n", sig_offset + irqh_offset);
            rom->driver_ver = WPC_LDV_3;
            
            if ( extra_delay == 0 ) {
                *already_updated = 1;
                return 0;
            }
            printf("--------------------------------------------------\n");
            printf("EXTRA-DELAY Option Specified.\n");
            printf("Updating newer driver code to increase lamp matrix\n");
            printf("clear-to-write delay from 26uS to 51uS\n");
            printf("--------------------------------------------------\n");
            
            patch_start_offset = sig_offset + irqh_offset;
            rc = wpc_patch_new_drvr_increase_delay(rom, patch_start_offset);
            return rc;
        }
    }

    // Search for the old driver V2 signature
    //
    if ( (osver >= 222) && (osver < 348) ) {
        DBG("%s: V2 Search: start %08lx\n", __FUNCTION__, irqh_offset);
        sig_offset = search_for_sig(&(rom->image[irqh_offset]), search_sz, drvr_OLD_V2_sig, sizeof(drvr_OLD_V2_sig));
        patch_start_offset = sig_offset + irqh_offset;
        
        if ( sig_offset != -1 ) {   
            printf("FOUND Lamp Driver V2 Signature at address 0x%08lx\n", patch_start_offset);
            rom->driver_ver = WPC_LDV_2;

            if ( extra_delay == 1 ) {
                printf("--------------------------------------------------\n");
                printf("EXTRA-DELAY Option Specified.\n");
                printf("Updating driver code with extended 51uS\n");
                printf("clear-to-write delay\n");
                printf("--------------------------------------------------\n");
            }
            rc = wpc_patch_lamp_matrix_old_V2_drvr(rom, patch_start_offset, extra_delay);
            return rc;
        }
        else {
            if ( osver > 246 ) {
                printf("Old Lamp Driver Signature not found.\n");
                return -1;
            }
        }
    }

    // Check for driver code seen in FT L5 ROM's
    //
    if ( osver >=243 ) {
        DBG("%s: VXa Search: start %08lx\n", __FUNCTION__, irqh_offset);
        sig_offset = search_for_sig(&(rom->image[irqh_offset]), search_sz, drvr_OLD_VXa_sig, sizeof(drvr_OLD_VXa_sig));
        patch_start_offset = sig_offset + irqh_offset;
        if ( sig_offset != -1 ) {   
            printf("FOUND Lamp Driver VXa Signature at address 0x%08lx\n", patch_start_offset);
            rom->driver_ver = WPC_LDV_Xa;
            if ( extra_delay == 1 ) {
                printf("--------------------------------------------------\n");
                printf("*** ERROR ***\n");
                printf("EXTRA-DELAY Option Specified.\n");
                printf("This rom contains driver code for which the extra delay\n");
                printf("option can not be applied.\n");
                printf("--------------------------------------------------\n");
                return INVALID_OPT_RC;
            }
            
            rc = wpc_patch_lamp_matrix_old_VXa_drvr(rom, patch_start_offset);
            return rc;
        }
        
    }
   

    // Search for old driver V1 signature
    //
    if ( osver < 243 ) {
        DBG("%s: V1 Search: start %08x\n", __FUNCTION__, rom->sys_rom_off);
        sig_offset = search_for_sig(&(rom->image[rom->sys_rom_off]), rom->size - rom->sys_rom_off - 0xff, drvr_OLD_V1C_sig, sizeof(drvr_OLD_V1C_sig));
        patch_start_offset = sig_offset + rom->sys_rom_off;
        if ( sig_offset != -1 ) {   
            printf("FOUND Lamp Driver V1C Signature at address 0x%08lx\n", patch_start_offset);
            rom->driver_ver = WPC_LDV_1C;
            rc = wpc_patch_lamp_matrix_old_V1_drvr(rom, patch_start_offset, extra_delay, 'C');
            return rc;
        }
        DBG("Lamp Driver V1C Signature NOT FOUND.\n");

        sig_offset = search_for_sig(&(rom->image[rom->sys_rom_off]), rom->size - rom->sys_rom_off - 0xff, drvr_OLD_V1B_sig, sizeof(drvr_OLD_V1B_sig));
        patch_start_offset = sig_offset + rom->sys_rom_off;
        if ( sig_offset != -1 ) {   
            printf("FOUND Lamp Driver V1B Signature at address 0x%08lx\n", patch_start_offset);
            rom->driver_ver = WPC_LDV_1B;
            rc = wpc_patch_lamp_matrix_old_V1_drvr(rom, patch_start_offset, extra_delay, 'B');
            return rc;
        }
        DBG("Lamp Driver V1B Signature NOT FOUND.\n");

        sig_offset = search_for_sig(&(rom->image[rom->sys_rom_off]), rom->size - rom->sys_rom_off - 0xff, drvr_OLD_V1A_sig, sizeof(drvr_OLD_V1A_sig));
        patch_start_offset = sig_offset + rom->sys_rom_off;
        if ( sig_offset != -1 ) {   
            printf("FOUND Lamp Driver V1A Signature at address 0x%08lx\n", patch_start_offset);
            rom->driver_ver = WPC_LDV_1A;
            rc = wpc_patch_lamp_matrix_old_V1_drvr(rom, patch_start_offset, extra_delay, 'A');
            return rc;
        }
        DBG("Lamp Driver V1A Signature NOT FOUND.\n");
    }
    
    printf("Lamp Driver Signature not found.\n");
    return -1;
}

/*******************************************************************************
 *
 ******************************************************************************/
static int wpc_patch_lamp_matrix_old_V2_drvr(wpc_rom_t *rom, unsigned long patch_start_offset, int extra_delay)
{
    int x, y;

    // The patched driver code uses 4 more bytes than the old code.
    // The lamp tables are accessed in 'indexed' mode.
    // The original code was generated using a complete 16-bit (2-byte) offset.
    // For the roms I have checked these tables are at small offsets from 'X' 
    // which means we can reduce a 4 byte instruction down to 3 bytes.
    // (Saves CPU cycles too :-)
    // ie: "LDAB 16,X" is currently 'E6 89 00 10'
    //      As long as the offset is less than 128 we can change this to:
    //      'E6 88 10'and save a byte.
    // There are 10 of these instructions in the lamp driver code.
    // We only need 4 extra bytes for the patch.
    //
    
    // Verify the offsets are less than 128 for the 4 instructions we want 
    // to compress
    //
    x = 0;
    while ( drvr_OLD_V2_idx_load_instr_pos[x] != 0 ) {
        uint16_t offset;
        offset = *(uint16_t *)&(rom->image[patch_start_offset +  drvr_OLD_V2_idx_load_instr_pos[x] + 2]);        
        if ( ntohs(offset) >= 128 ) {
            printf("ERROR: LDX offset gt 128. Can not compress instructions: (%04x)\n", ntohs(offset));
            return -1;
        }
        x++;
    }

    // Compress the LDAx instructions to free some bytes 
    //
    x = 0;
    while ( drvr_OLD_V2_idx_load_instr_pos[x] != 0 ) {
        rom->image[patch_start_offset + drvr_OLD_V2_idx_load_instr_pos[x] + 1 - x] = 0x88;  // Change the 0x89 -> 0x88

        for ( y = (patch_start_offset + drvr_OLD_V2_idx_load_instr_pos[x] + 2 - x);
              y < (patch_start_offset + 64);
              y++ ) {
            // Move following instructions up.
            //
            rom->image[y] =  rom->image[y+1];
        }
        x++;
    }

    // Flip the instructions at the end of the patch so we write the lamp row before lamp column.
    //
    rom->image[patch_start_offset + PATCH_V2_END_OFFSET - 6] = 0xf7;
    rom->image[patch_start_offset + PATCH_V2_END_OFFSET - 4] = 0xe4;
    rom->image[patch_start_offset + PATCH_V2_END_OFFSET - 3] = 0xb7;
    rom->image[patch_start_offset + PATCH_V2_END_OFFSET - 1] = 0xe5;

    // We are removing the CLR (3 bytes) before the column/row writes so shift the code that 
    // comes after the new row/column clear code down 3 bytes.
    //
    for ( x = (patch_start_offset + PATCH_V2_END_OFFSET - 7); 
          x > (patch_start_offset + PATCH_V2_END_OFFSET - 37); 
          x-- ) {
        rom->image[x] = rom->image[x-3];  
    } 

    // We now have 7 bytes free to do the row/column clear instructions.
    //
    memcpy(&(rom->image[patch_start_offset+60]), rc_clr_code, 7);

    if ( extra_delay ) {

        // For extra delay move the row/column clear sequence up in the code.
        //
        DBG("%s: ExtraDelay: mv %04lx to %04lx size=%d\n", __FUNCTION__, 
            patch_start_offset + 31, patch_start_offset + 31 + 7, 29);
        for ( x = (patch_start_offset + 31 + 28); x >= (patch_start_offset + 31); x-- ) {
            rom->image[x+7] = rom->image[x]; 
        }
        memcpy(&(rom->image[patch_start_offset + 31]), rc_clr_code, 7);
    }

    return 0;
}
/*******************************************************************************
 *
 ******************************************************************************/
static int wpc_patch_lamp_matrix_old_VXa_drvr(wpc_rom_t *rom, unsigned long patch_start_offset)
{
    unsigned long patch_end         = patch_start_offset + sizeof(drvr_OLD_VXa_sig);
    unsigned long hole_start_offset = patch_start_offset + 35;
    int x, y;

    DBG("%s: patch start (%08lx) patch end (%08lx)\n", __FUNCTION__, patch_start_offset, patch_end);

    // The patched driver code uses 4 more bytes than the old code.
    // The lamp tables are accessed in 'indexed' mode.
    // The original code was generated using a complete 16-bit (2-byte) offset.
    // For the roms I have checked these tables are at small offsets from 'X' 
    // which means we can reduce a 4 byte instruction down to 3 bytes.
    // (Saves CPU cycles too :-)
    // ie: "LDAB 16,X" is currently 'E6 89 00 10'
    //      As long as the offset is less than 128 we can change this to:
    //      'E6 88 10'and save a byte.
    // This version of the driver has 4 of these instructions.
    //
    // This patch will create about a 19 uS delay between turning the row/col off 
    // and back on.


    // Verify the offsets are less than 128 for the 4 instructions we want 
    // to compress
    //
    x = 0;
    while ( drvr_OLD_VXa_idx_load_instr_pos[x] != 0 ) {
        uint16_t offset;
        offset = *(uint16_t *)&(rom->image[patch_start_offset + drvr_OLD_VXa_idx_load_instr_pos[x] + 2]);        
        if ( ntohs(offset) >= 128 ) {
            printf("ERROR: LDX offset gt 128. Can not compress instructions: (%04x)\n", ntohs(offset));
            return -1;
        }
        x++;
    }


    // Flip the instructions at the end of the patch so we write the lamp row before lamp column.
    //
    rom->image[patch_end - 6] = 0xf7;
    rom->image[patch_end - 4] = 0xe4;
    rom->image[patch_end - 3] = 0xb7;
    rom->image[patch_end - 1] = 0xe5;

    //Move the code down 3 bytes to overwite the CLR instruction before the lamp row/col writes.
    //
    for ( x = patch_end - 10; x >= hole_start_offset; x-- ) {
        rom->image[x+3] = rom->image[x];
    }
    
    // Compress the the 4 indexed mode instruction to open up 4 more bytes at the top of the patch.
    //
    for ( x = 0; x < 4; x++ ) {
        rom->image[patch_start_offset + drvr_OLD_VXa_idx_load_instr_pos[3-x] + 3 + 2 + x] = 0x88;  // Change the 0x89 -> 0x88
        DBG("%s: idx_chg (%d) addr=%08lx\n", __FUNCTION__, 3-x, patch_start_offset + drvr_OLD_VXa_idx_load_instr_pos[3-x] + 3 + 2 + x);

        // Shift the code down 1 byte
        //
        for ( y = (patch_start_offset + drvr_OLD_VXa_idx_load_instr_pos[3-x] + 3 + x);
              y >= (hole_start_offset + 3 + x);
              y-- ) {

            rom->image[y+1] =  rom->image[y];
        }
    }

    // We now have 7 bytes free to do the row/column clear instructions.
    //
    memcpy(&(rom->image[hole_start_offset]), rc_clr_code, 7);

    return 0;
}


/*******************************************************************************
 *
 ******************************************************************************/
#define MV_START_OFF	(32)
static int wpc_patch_new_drvr_increase_delay(wpc_rom_t *rom, unsigned long patch_start_offset)
{
    int x;

    // For extra delay move the row/column clear sequence up in the code.
    //
    DBG("%s: ExtraDelay: mv %04lx to %04lx size=%d\n", __FUNCTION__, 
        patch_start_offset + MV_START_OFF, patch_start_offset + MV_START_OFF + 7, MV_START_OFF + 1);

    for ( x = (patch_start_offset + MV_START_OFF + 32); x >= (patch_start_offset + MV_START_OFF); x-- ) {
        rom->image[x+7] = rom->image[x]; 
    }

    memcpy(&(rom->image[patch_start_offset + MV_START_OFF]), rc_clr_code, 7);

    return 0;
}

/*******************************************************************************
 *
 * This routine patches the really old code.
 * Signatures V1A, V1B, & V1C.
 * The index mode intructions are already space optimized in these driver versions.
 * ie: we can't reduce the size of 'LDAA 32, x' etc... from 4 bytes to 3 to reclaim memory
 * for the patch.
 * What we do instead is check for free space at the end of the rom. If found, 
 * we jump to that area and implement the patch there.
 * 
 *
 ******************************************************************************/
static int wpc_patch_lamp_matrix_old_V1_drvr(wpc_rom_t *rom, unsigned long patch_start_offset, int extra_delay, char drvr_ver)
{
    int x, chk_len;
    unsigned long patch_addr, jump_to_patch_addr;
    unsigned long move_code_start_offset;
    unsigned long move_code_len;

    if ( extra_delay == 1 ) {
        printf("--------------------------------------------------\n");
        printf("*** ERROR ***\n");
        printf("EXTRA-DELAY Option Specified.\n");
        printf("This rom contains driver code for which the extra delay\n");
        printf("option can not be applied.\n");
        printf("--------------------------------------------------\n");
        return INVALID_OPT_RC;
    }

    // Verify space is available at end of rom just before copyright info.
    //
    if ( (drvr_ver == 'A') || (drvr_ver == 'B') ) {
        chk_len = 40;
    }
    else {
        //VerC
        chk_len = 48;
    }
    jump_to_patch_addr = patch_addr = rom->size - 0x100 - chk_len;

    DBG("Checking space for patch at address: %08lx len=%d\n", patch_addr, chk_len);
    for ( x = patch_addr; x < patch_addr + chk_len; x++ ) {
        if ( rom->image[x] != 0xff ) {
            printf("Error: no space available at end of rom to apply patch\n");
            return -1;
        }
    }
    DBG("Found space for patch at address: %08lx\n", patch_addr);

    // Version 1A & 1B code.
    // Move the last 16 instructions of the driver to the patch area.
    // Place the new Row/Column clear code at the address where the 
    // first of the 16 moved instructions lived. Then jump to the moved code.
    // The last of the 16 moved instructions is already a JMP so we don't need to worry
    // about getting back from the patched area.
    //
    if ( (drvr_ver == 'A') || (drvr_ver == 'B') ) {
        if ( drvr_ver == 'A' ) {
            move_code_start_offset = patch_start_offset + 44;
            move_code_len          = 38;
        }
        else {
            move_code_start_offset = patch_start_offset + 65;
            move_code_len          = 38;
        }
        DBG("%s: mv start %08lx end %08lx\n", __FUNCTION__, move_code_start_offset, move_code_start_offset + move_code_len);

        
        // Move the code omitting the CLR instruction & flip the row/column writes.
        //
        for ( x=move_code_start_offset; x < (move_code_start_offset + move_code_len - 12); x++ ) {
            rom->image[patch_addr++] = rom->image[x];
        }
        for ( x=0; x < 6; x++ ) {
            rom->image[patch_addr++] = wrt_row_wrt_col_code[x];
        }
        for ( x=(move_code_start_offset + move_code_len - 3); x < (move_code_start_offset + move_code_len); x++ ) {
            rom->image[patch_addr++] = rom->image[x];
        }

        // Do the early row/column turn off.
        //
        memcpy(&(rom->image[move_code_start_offset]), rc_clr_code, 7);
        
        // Add the jump to the patched code.
        //
        rom->image[move_code_start_offset+7] = 0x7E;
        *(uint16_t *)&(rom->image[move_code_start_offset+8]) = htons(jump_to_patch_addr); 

        return 0;
    }

    if ( drvr_ver == 'C' ) {
        // Version 1C code. I've only seen this for Hurricane L-2
        // It is a shortened version of the driver so we can't do as long a delay.
        // We also patch a little past the final row/column writes to take advantage 
        // of a JMP instruction in the code to get us back from the patched area.
        //
        move_code_start_offset = patch_start_offset + 35;
        move_code_len          = 44;
        DBG("%s: mv start %08lx end %08lx\n", __FUNCTION__, move_code_start_offset, move_code_start_offset + move_code_len);
        
        // Move the code chunk that comes before the CLR instruction
        //
        for ( x=move_code_start_offset; x < (move_code_start_offset + 20); x++ ) {
            rom->image[patch_addr++] = rom->image[x];
        }
        
        // Add the updated (flip row/column write) code
        //
        for ( x=0; x < 6; x++ ) {
            rom->image[patch_addr++] = wrt_row_wrt_col_code[x];
        }
        
        // Move the remaining code that comes after the row/column write.
        //
        for ( x=(move_code_start_offset + 29); x < (move_code_start_offset + 29 + 15); x++ ) {
            rom->image[patch_addr++] = rom->image[x];
        }

        // Do the early row/column turn off.
        //
        memcpy(&(rom->image[move_code_start_offset]), rc_clr_code, 7);
        
        // Add the jump to the patched code.
        //
        rom->image[move_code_start_offset+7] = 0x7E;
        *(uint16_t *)&(rom->image[move_code_start_offset+8]) = htons(jump_to_patch_addr); 
        return 0;
    }
    
    printf("***Error: Unsupported driver code.\n");
    return -1;
}
