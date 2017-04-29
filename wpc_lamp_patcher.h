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
#ifndef __WPCLP_H
#define __WPCLP_H

#define ONE_KB	(1024)
#define ROM_SZ_TO_MBIT_DIVISOR	(1024 * 128) /* 128KBytes = 1-MBit */

// The system ROM is the last 32K of the image file.
// During execution it is mapped to 0x8000 - 0xffff
// And offsets dereferenced inside the system rom need to be normalized
// by clearing the MSB (bit 15)
#define WPC_SYS_ROM_SZ	(1024 * 32) /* 32K System rom bank */
#define WPC_SYS_ROM_CODE_MASK	(0x7fff)

#define DBG(fmt, args...) if (debug) { printf(fmt, ## args); }

#if defined(__i386) || defined(__i386__) || defined(__x86_64) || defined(__x86_64__)
#define HTONS(n) (((((unsigned short)(n) & 0xFF)) << 8) | (((unsigned short)(n) & 0xFF00) >> 8))
#define NTOHS(n) (((((unsigned short)(n) & 0xFF)) << 8) | (((unsigned short)(n) & 0xFF00) >> 8))

#define HTONL(n) (((((unsigned long)(n) & 0xFF)) << 24) | \
                  ((((unsigned long)(n) & 0xFF00)) << 8) | \
                  ((((unsigned long)(n) & 0xFF0000)) >> 8) | \
                  ((((unsigned long)(n) & 0xFF000000)) >> 24))

#define NTOHL(n) (((((unsigned long)(n) & 0xFF)) << 24) | \
                  ((((unsigned long)(n) & 0xFF00)) << 8) | \
                  ((((unsigned long)(n) & 0xFF0000)) >> 8) | \
                  ((((unsigned long)(n) & 0xFF000000)) >> 24))
#else
#define HTONS(n) (n)
#define NTOHS(n) (n)
#define HTONL(n) (n)
#define NTOHL(n) (n)
#endif

#define htons(n) HTONS(n)
#define ntohs(n) NTOHS(n)
#define htonl(n) HTONL(n)
#define ntohl(n) NTOHL(n)

#define GAME_VER_FORMAT_0	(0)
#define GAME_VER_FORMAT_1	(1)

#define AUTO_VER_FORMAT_0   'Z'
#define AUTO_VER_FORMAT_1   9

#define INVALID_OPT_RC		(-2)

typedef enum 
{
    WPC_VAL_CSUM_ERR_OK = 1,
    WPC_VAL_FULL_CHECK  = 2 
} wpc_rom_validate_level_t;

typedef enum
{
    WPC_LDV_INVALID = 0,
    WPC_LDV_3,  // Updated driver w/o an issue. 1995 on
    WPC_LDV_2,  // Most ROMs,  1992 - 1994
    WPC_LDV_Xa, // Transition between early roms & V2 driver ??? (OSVer 2.46)
    WPC_LDV_1A, // Early ROMs, 1990 - 1991
    WPC_LDV_1B, // Early ROMs, 1990 - 1991
    WPC_LDV_1C, // Early ROMs, 1990 - 1991

} wpc_lm_drvr_ver_t;

typedef struct wpc_checksum_info_t
{
    int      cscw_off;  //Checksum Correction Word offset
    int      csum_off;  //Checksum Word offset
    uint16_t cscw;      //Checksum Correction Word
    uint16_t csum;      //Checksum Word
    int      csum_valid;
    uint16_t csum_calc; //Calculated Checksum Word
} wpc_checksum_info_t;

typedef struct wpc_version_info_t
{
    int  version_format; // 0: earlier format 1: later format
                         // Earlier has versions like L-4 later
                         // format is x.y
    int  os_major;
    int  os_minor;
    int  game_major;
    int  game_minor;
    char game_ver_char;
    int  game_ver_Lx_format; // Bool. Some games display rev as Lx-n
                             // eg: LH-5, LX-7
} wpc_version_info_t;


typedef struct wpc_rom_t
{
    unsigned char       *image;           //ROM Image
    unsigned int         size;            //Rom Image size
    unsigned int         sys_rom_off;     //System Rom start addr
    wpc_checksum_info_t  cs;              //Checksum info
    wpc_version_info_t   ver;             //ROM Versioning
    int                  irq_handler_off; //IRQ Handler offset
    wpc_lm_drvr_ver_t    driver_ver;      //Version of the driver detected
    int                  hard_error;      //Error occured during processing    
} wpc_rom_t;

extern int debug;
extern void wpc_print_rom_info(wpc_rom_t *rom, char* filename);
extern int  wpc_validate_rom(wpc_rom_t * rom, wpc_rom_validate_level_t validate_level);
extern int  wpc_update_rom_version_checksum(wpc_rom_t *rom);
extern int  wpc_patch_lamp_matrix(wpc_rom_t *rom, int extra_delay, int *already_updated);
extern int  search_for_sig(unsigned char *bp, int size, uint8_t *sig, int sig_sz);

#endif
