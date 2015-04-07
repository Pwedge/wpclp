  -------------------------------------------------------
*** WPC / WPC95 Lamp Matrix Driver Rom patcher ***
   (c) John Honeycutt, http://emmytech.com/arcade
This program will update the lamp matrix driver code for WPC &
WPC95 game roms to eliminate 'ghosting' when using LEDs
-------------------------------------------------------

Credits:
 Thanks to Matthias "Maddes" Buecher, http://www.maddes.net/
 I used his WPC_Tool Java source to understand how WPC
 version codes are done and how the ROM checksum is handled.

--------------------------------------------------------
**********************************
*** Important READ LICENSE.TXT ***
**********************************
If this program is going to be of use to you, you will 
be burning and installing a new rom in a pinball machine.
There is always a chance of unexpected things happening.

It is advisable to test any rom updated by this program using pinmame.

Use at your own risk.

*** Important READ LICENSE.TXT ***

--------------------------------------------------------

This program has been verified to compile under linux gcc 4.2.4
and win32 (mingw 2.0.0-3) gcc 3.2.3
http://www.mingw.org/
--------------------------------------------------------

syntax: wpclp [-f] [-e] [-n] <input rom filename> <patched rom filename>
   '-f' Optional 'force' parameter to continue even if original rom checksum
        is wrong.
   '-n' Optional 'no-patch' parameter to skip the patching phase.
        Can be used to just update rom version and checksum without patching
        the driver.
   '-e' Optional 'extra-delay' parameter.
        Without this option a 30uS delay will occur between clearing the
        matrix row/column and writing for the next period.
        (30uS is the delay inserted by the newer williams driver code.)
        Specifying this option causes an extra 26 to 33uS delay to be inserted
        instead of 26uS. The extra delay might help with problem boards.


-f: force option
   This option is useful of the ROM has been manually modified and the
   checksum is not valid.
   An example is manually modifying the rom with a hex editor to
   change strings, etc...

-n: no-patch option
   The '-n' option can be used with the '-f' to update the checksum
   and version number for a manually modified rom without attempting
   to install the lamp driver patch.

-e: extra-delay
   Adds an additional 30uS of delay between clearing the row/column
   and writing the new row/column values.
   It may help with problem boards that still exhibit ghosting even
   with the updated driver.



----------------------------------------------------------
Example output:

honeycut> wpclp creat_l4.rom creat_m4.rom
==============================================================
*** WPC / WPC95 Lamp Matrix Driver Rom patcher ***
   version 1.0
   (c) John Honeycutt, http://emmytech.com/arcade
       honeycutt7483@gmail.com
This program will update the lamp matrix driver code for WPC &
WPC95 game roms to eliminate 'ghosting' when using LEDs
==============================================================

Rom Info
--------
   File: creat_l4.rom
   Size: 4 M-bits (512K-Bytes)
   Checksum Valid: f804
   System Version: 2.52
   Game Version (Old format): L-4
   System ROM Offset: 00078000
---------------------------

Patching Lamp Matrix Driver...
FOUND Lamp Driver Signature at address 0x0007dc39
Done

The original ROM Version is: REV. L-4
A new version should be assigned to distinguish the new
rom from the original rom.
The format entered should be: <letter>-<number>
  <letter> can be A to Z except P
  <number> can be 1 to 99
    example: VERSION>M-25

Enter VERSION>m-4

Updating ROM versioning and checksum...
Writing output file: creat_m4.rom
Updated ROM file write complete.
Validating Updated ROM file...
Updated ROM Validation Passed!

Rom Info
--------
   File: creat_l4.rom
   Size: 4 M-bits (512K-Bytes)
   Checksum Valid: fb04
   System Version: 2.52
   Game Version (Old format): M-4
   System ROM Offset: 00078000
---------------------------
Done.

----------------------------------------------------------------

Entering the new ROM version

After starting the program it will prompt the user to enter a new ROM version.
What is being prompted for here is the ROM revision that will be displayed 
when the game is powered on and when entering the service menu.

Example prompt for an L-4 ROM:
The original ROM Version is: REV. L-4
A new version should be assigned to distinguish the new
rom from the original rom.
The format entered should be: <letter>-<number>
  <letter> can be A to Z except P
  <number> can be 1 to 99
    example: VERSION>M-25

Enter VERSION>

Williams used serveral different formats for their ROM versioning.
    * P-n     Prototype eg: P-2
    * L-n     Production eg: L-4
    * L(x)-n Another form of production or ?home use? eg: LH-1, LX-2
    * major# . minor#   Production eg: 1.06, 9.04

The user can update these versions as follows:
    * P-n:             The number portion can be updated. The letter P can not. eg: update P-2 to P-3

    * L-n or L(x)-n:   The letter L can be changed to any letter other than P. 
                       The number can be updated. The second letter '(x)' portion can 
                       not be changed.

    * major# . minor#: The major number 'x' and the minor number 'y' can be changed.

----------------------------------
Revisions:

6/15/2010
1.0: Initial release

6/21/2010
1.1 -Fix little endian x86 macro check for linux 64-bit OS's.
     Code wasn't properly defining hton(x) / ntoh(x) macros for 64-bit os.

    -Fix command line -e option parsing.

    -Add support for early driver code.
     This appears to apply to ROMs whose last update was in the 1st half of 1992 
     or earlier. Code refers to these driver variants as 1A, 1B, & 1C.
     Games with this early driver code include HD L-3, Hurricane L-2, PZ F-4, BoP L-6.

1.2 -Add support for 4th driver version seen in FT L-5

1.3 -Make code that looks for free space to patch with at end of rom smarter.
    -Fix some issues with Prototype rom versioning

1.4 -Fix game versioning problem intrduced in version 1.3
