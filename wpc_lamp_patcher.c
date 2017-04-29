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
/******************************************************************************
 * Version 1.0  06/07/2010: Initial Release
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <ctype.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>
#include "wpc_lamp_patcher.h"


#define VERSION	"1.4"

#define DBG(fmt, args...) if (debug) { printf(fmt, ## args); }
#define STRTOHEX(x) strtoul((x), NULL, 16)
#define STRTODEC(x) strtol((x), NULL, 10)

#define MAX_INPUT_FILE_SZ	(1024 * 1024) /* 1MB */
#define STRBUF_SZ	(256)

int debug = 0;
int force = 0;
int skip_patch  = 0;
int extra_delay = 0;
int auto_version = 0;
char strbuf_g[STRBUF_SZ];


/*******************************************************************************
 *
 ******************************************************************************/
void prog_info(void)
{
    printf("==============================================================\n");
    printf("*** WPC / WPC95 Lamp Matrix Driver Rom patcher ***\n");
    printf("   version %s\n", VERSION);
    printf("   (c) John Honeycutt, http://emmytech.com/arcade\n");
    printf("       honeycutt7483@gmail.com\n");
    printf("This program will update the lamp matrix driver code for WPC &\n");
    printf("WPC95 game roms to eliminate 'ghosting' when using LEDs\n");
    printf("==============================================================\n\n");
}

/*******************************************************************************
 *
 ******************************************************************************/
void help_exit(char *name)
{
    prog_info();
    printf("\nsyntax: %s [-f] [-e] [-n] [-a] <input rom filename> <patched rom filename>\n", name);
    printf("   '-f' Optional 'force' parameter to continue even if original rom checksum\n");
    printf("        is wrong.\n");
    printf("   '-n' Optional 'no-patch' parameter to skip the patching phase.\n");    
    printf("        Can be used to just update rom version and checksum without patching\n");
    printf("        the driver.\n");
    printf("   '-e' Optional 'extra delay' parameter.\n");
    printf("        Without this option a 30uS delay will occur between clearing the\n");
    printf("        matrix row/column and writing for the next period.\n");
    printf("        (30uS is the delay inserted by the newer williams driver code.)\n");
    printf("        Specifying this option causes an extra 26 to 33uS delay to be inserted \n");
    printf("        instead of 30uS. The extra delay might help with problem boards.\n");
    printf("   '-a' Optional 'automatically set version number' parameter.\n");    
    printf("        Can be used to automatically set the version number without prompting.\n");
    printf("        For ALPHA-NUMERIC version numbers, the ALPHA will be \'%c\'.\n", AUTO_VER_FORMAT_0);
    printf("        For MAJOR.MINOR version numbers, the MAJOR will be \'%x\'.\n", AUTO_VER_FORMAT_1);
    exit(0);
}


/*******************************************************************************
 *
 ******************************************************************************/
char *get_text(char* prompt, char *text, size_t size)
{
    size_t i = 0;

    fputs(prompt, stdout);
    fflush(stdout);
    for ( ;; ) {
        int ch = fgetc(stdin);
        if ( ch == '\n' || ch == EOF ) {
            break;
        }
        if ( i < size - 1 ) {
            text[i++] = ch;
        }
    }
    text[i] = '\0';
    return text;
}


/*******************************************************************************
 *
 ******************************************************************************/
int load_file(FILE *fp, unsigned char *bufp, int max_sz)
{
    unsigned char *bp = bufp;
    int cnt;

    while ( (cnt = fread(bp, 1, ONE_KB, fp)) > 0 ) {
        bp+=cnt;
    }
    if ( ferror(fp) ) {
        printf("%s: Error while reading input file: cnt=%d (%s)\n", __FUNCTION__, cnt, strerror(errno));
        return -1;
    }
    if ( feof(fp) ) {
        DBG("%s: EOF\n", __FUNCTION__);
    }
    else {
        DBG("%s: EOF not set\n", __FUNCTION__);
    }

    cnt = bp-bufp;
    DBG("%s: Read %d (0x%x) bytes\n", __FUNCTION__, cnt, cnt);
    return cnt;
}


/*******************************************************************************
 *
 ******************************************************************************/
int write_file(FILE *fp, unsigned char *bufp, int size)
{
    int            cnt = 0;
    unsigned char *bp = bufp;

    int wrt_size, wrt_cnt;

    while ( cnt < size ) {
        if ( (size - cnt) < ONE_KB ) {
            wrt_size = size - cnt;
        }
        else {
            wrt_size = ONE_KB;
        }
        wrt_cnt = fwrite(bp, 1, wrt_size, fp);
        if ( wrt_cnt == -1 ) {
            printf("\nwrite error: (%s)\n", strerror(errno));
            return -1;
        }
        bp  += wrt_cnt;
        cnt += wrt_cnt;
    }
    return cnt;
}


/*******************************************************************************
 *
 ******************************************************************************/
int get_new_rom_version(wpc_rom_t *rom, uint8_t *major, uint8_t *minor)
{
    char  *userline;
    char  *ch;
    unsigned long val;

    printf("\nThe original ROM Version is: REV. ");
    if ( rom->ver.version_format == GAME_VER_FORMAT_1) {
        printf("%x.%x\n", rom->ver.game_major, rom->ver.game_minor);
    }
    else {
        if ( rom->ver.game_ver_Lx_format == 1 ) {
            printf("%c(x)-%x\n", rom->ver.game_ver_char, rom->ver.game_major);
        }
        else {
            printf("%c-%x\n", rom->ver.game_ver_char, rom->ver.game_major);
        }
    }

    if (auto_version == 1) {
        printf("Automatically setting ROM Version to: REV. ");
        if (rom->ver.version_format == GAME_VER_FORMAT_0) {
            *major = AUTO_VER_FORMAT_0;
            *minor = rom->ver.game_major;
            if (rom->ver.game_ver_Lx_format == 1) {
                printf("%c(x)-%x\n", *major, *minor);
            } else {
                printf("%c-%x\n", *major, *minor);
            }
        } else if (rom->ver.version_format == GAME_VER_FORMAT_1) {
            *major = AUTO_VER_FORMAT_1;
            *minor = rom->ver.game_minor;
            printf("%x.%x\n", *major, *minor);
        } else {
            assert(0);
        }

        return 0;
    }

    printf("A new version should be assigned to distinguish the new\n");
    printf("rom from the original rom.\n");
    printf("The format entered should be: ");
    if ( rom->ver.version_format == GAME_VER_FORMAT_1) {
        printf("<number1>.<number2>\n");
        printf("<number1> can be 1 to 99  <number 2> can be 0 to 99\n");
    }
    else {
        if ( rom->ver.game_ver_char == 'P' ) {
            // Prototype rom
            //
            printf("P-<number>\n");
            printf("<number> can be 1 to 2f\n");
            printf("  example: VERSION>P-12\n");
        }
        else {
            printf("<letter>-<number>\n");
            printf("  <letter> can be A to Z except P\n");
            printf("  <number> can be 1 to 99\n");
            printf("    example: VERSION>M-25\n");
        }
    }

    userline = get_text("\nEnter VERSION>", strbuf_g, STRBUF_SZ);
    DBG("Got version: '%s'\n", userline);

    if ( rom->ver.version_format == GAME_VER_FORMAT_1) {
        if ( (ch = strchr(userline, '.')) == NULL ) {
            return -1;
        }
        if ( (ch - userline) < 1 ) {
            return -1;
        }
        if ( strlen(userline) < (ch - userline + 2)) {
            return -1;
        }
        *ch = '\0';
        val = strtol(userline, NULL, 16);
        if ( (val < 1) || (val > 0x99) ) {
            return -1;
        }
        *major = val;
        val = strtol(ch+1, NULL, 16);
        if ( (val < 0) || (val > 0x99) ) {
            return -1;
        }
        *minor = val;
        DBG(" new ver: maj=%x min=%x\n", *major, *minor);
    }
    else {
        if ( (ch = strchr(userline, '-')) == NULL ) {
            return -1;
        }
        if ( (ch - userline) < 1 ) {
            return -1;
        }
        if ( strlen(userline) < (ch - userline + 2)) {
            return -1;
        }
        if ( rom->ver.game_ver_char == 'P' ) {
            // Prototype rom
            //
            *major = 'P';
            val = strtol(ch+1, NULL, 16);
            if ( (val < 1) || (val > 0x2f) ) {
                return -1;
            }
            *minor = val;            
        }
        else {
            if ( !(isalpha(userline[0])) || (toupper(userline[0]) == 'P') ) {
                return -1;
            }
            *major = toupper(userline[0]);
            val = strtol(ch+1, NULL, 16);
            if ( (val < 1) || (val > 0x99) ) {
                return -1;
            }
            *minor = val;
        }
        DBG(" new ver: letter=%c num=%x(%d)\n", *major, *minor, *minor);
    }
    return 0;
}



/*******************************************************************************
 * Function:    main
 * 
 * Description:
 *
 ******************************************************************************/
int main (int argc, char **argv)
{
    char *infile  = NULL;
    char *outfile = NULL;
    int   opt_error = 0;
    unsigned char *fbuf;
    FILE          *fdi, *fdo;
    int            fsize, l, m, n, rc;
    uint8_t        vid1, vid2;
    char           ch;
    wpc_rom_t      rom;
    int            update_not_needed;

    // Parse the command line
    //
    for( n = 1; n < argc; n++ ) {
        switch( (int)argv[n][0] ) {        // Check for option character.
        case '-':
            l = strlen( argv[n] );
            for( m = 1; m < l; ++m ) {     //  Scan through options.
                ch = (int)argv[n][m];
                switch( ch )
                {
                case 'd':
                    debug = 1;
                    break;
                case 'f':
                    force = 1;
                    break;
                case 'e':
                    extra_delay = 1;
                    break;
                case 'n':
                    skip_patch = 1;
                    break;
                case 'a':
                    auto_version = 1;
                    break;
                default:  
                    printf( "Error: unknown option '-%c'\n", ch );
                    opt_error = 1;
                    break;
                }
            }
            break;
        default:
            // Text
            if ( infile == NULL ) {
                infile = argv[n];
            }
            else if (outfile == NULL ) {
                outfile = argv[n];
            }
            else {
                // Already got all text options
                printf( "Error: unexpected string option '%s'\n", argv[n]);
                opt_error = 1;
            }
            break;
        }
    }

    if ( opt_error || (outfile == NULL) ) {
        help_exit(argv[0]);
    }
    prog_info();
    
    if ( strcmp(infile, outfile) == 0 ) {
        printf("Error: Input and Output files must have different names\n");
        exit(0);
    }
    
    DBG("Input file:  '%s'\n", infile);
    fdi = fopen(infile, "rb");
    if (fdi == NULL) {
        printf("Error: Failed to open input file: %s (%s)\n", infile, strerror(errno));
        exit(1);
    }
    
    fbuf = malloc(MAX_INPUT_FILE_SZ);
    if ( fbuf == NULL ) {
        printf("Error allocating memory: (%s)\n", strerror(errno));
        exit(1);
    }

    // Read the input file
    //
    fsize = load_file(fdi, fbuf, MAX_INPUT_FILE_SZ);
    fclose(fdi);

    // Setup the rom struct
    //
    memset(&rom, 0, sizeof(rom));
    rom.image = fbuf;
    rom.size  = fsize;
    DBG("Input ROM Size: %d M-bits (%dK-Bytes)\n\n", rom.size / ROM_SZ_TO_MBIT_DIVISOR, rom.size / ONE_KB);

    // Validate input file
    //
    if ( force ) {
        rc = wpc_validate_rom(&rom, WPC_VAL_CSUM_ERR_OK);
    }
    else {
        rc = wpc_validate_rom(&rom, WPC_VAL_FULL_CHECK);
    }

    if ( rc != 0 ) {
        printf("Error: ROM Validation Failed.\n");
        wpc_print_rom_info(&rom, infile);
        goto err_exit;
    }
    wpc_print_rom_info(&rom, infile);
    
    if ( rom.cs.csum_valid == 0 ) {
        if ( force == 1 ) {
            printf("***WARNING***: ROM Checksum Validation Failed.\n");
            printf("               'force' option specified. continuing...\n");
        }
        else {
            goto err_exit;
        }
    }
    
    // Patch the file
    //
    if ( skip_patch == 1 ) {
        printf("\n***WARNING***: -n option specified.\n");
        printf("               Skipping driver patch.\n");
    }
    else {
        printf("\nPatching Lamp Matrix Driver...\n");
        rc = wpc_patch_lamp_matrix(&rom, extra_delay, &update_not_needed);
        if  ( rc != 0 ) {
            printf("ERROR: Lamp matrix driver patch failed.\n");
            if ( rc == INVALID_OPT_RC ) {
                goto err_exit;
            }
            else {
                goto err_exit_with_msg;
            }
        }
        if ( update_not_needed ) {
            printf("**** Driver Patch NOT Required ****\n");
            printf("This ROM image already contains the updated lamp matrix driver code.\n");
            goto err_exit;
        }
        printf("Done\n");
    }

    // Get new rom version from the user
    //
    if ( (get_new_rom_version(&rom, &vid1, &vid2)) != 0 ) {
        printf("Invalid ROM Version entered.\n");
        exit(1);
    }
    printf("\n");

    // Update the version number and checksum
    //
    printf("Updating ROM versioning and checksum...\n");
    if ( rom.ver.version_format == GAME_VER_FORMAT_1) {
        rom.ver.game_major = vid1;
        rom.ver.game_minor = vid2;
    }
    else {
        rom.ver.game_ver_char = vid1;
        rom.ver.game_major    = vid2;
    }
    rc = wpc_update_rom_version_checksum(&rom);
    if ( rc != 0 ) {
        printf("Error: ROM version/checksum update failed.\n");
        goto err_exit_with_msg;
    }

    if ( debug ) {
        printf("Debug Updated ROM Information:\n");
        wpc_print_rom_info(&rom, NULL);
    }
    
    // Write the output file
    //
    fdo = fopen(outfile, "rb");
    if (fdo != NULL) {
        printf("Error: output file aleady exists: %s\n", outfile);
        fclose(fdo);
        goto err_exit;
    }
    fdo = fopen(outfile, "wb");
    if (fdo == NULL) {
        printf("Error opening output file for writing: %s\n", outfile);
        goto err_exit;
    }
    printf("Writing output file: %s\n", outfile);
    rc = write_file(fdo, rom.image, rom.size);
    fclose(fdo);
    if ( rc != rom.size ) {
        printf("Error Writing Output file. returncode=%d\n", rc);
        goto err_exit;
    }
    printf("Updated ROM file write complete.\n");

    // Validate the updated ROM
    //
    printf("Validating Updated ROM file...\n");
    fdi = fopen(outfile, "rb");
    if (fdi == NULL) {
        printf("Error: Failed to open updated file: %s (%s)\n", outfile, strerror(errno));
        exit(1);
    }

    fsize = load_file(fdi, fbuf, MAX_INPUT_FILE_SZ);
    fclose(fdi);

    memset(&rom, 0, sizeof(rom));
    rom.image = fbuf;
    rom.size  = fsize;
    DBG("Updated ROM Size: %d M-bits (%dK-Bytes)\n\n", rom.size / ROM_SZ_TO_MBIT_DIVISOR, rom.size / ONE_KB);

    rc = wpc_validate_rom(&rom, WPC_VAL_FULL_CHECK);
    if ( rc != 0 ) {
        printf("Error: Updated ROM Validation Failed.\n\n");
        wpc_print_rom_info(&rom, infile);
        goto err_exit;
    }
    printf("Updated ROM Validation Passed!\n\n");
    wpc_print_rom_info(&rom, infile);

    printf("Done.\n");
    free(fbuf);
    exit(0);

err_exit_with_msg:
    printf("***********************\n");
    printf("The program exited because it had trouble analyzing the rom\n");
    printf("file. If you belive your rom is valid and has the ghosting\n");
    printf("issue then you can send an email to honeycutt7483@gmail.com\n");
    printf("Please include the version of this program you are running\n");
    printf("and the game rom name and version you are trying to patch.\n");
    printf("Please attach the rom you are trying to patch to the email.\n");
    printf("***********************\n");

err_exit:
    free(fbuf);
    exit(1);
}

