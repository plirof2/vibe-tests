# Build Commands
- `make` - Build the SGD executable (sgd.exe) using Watcom C32 compiler
- Uses wcc386 compiler with optimizations: -oo -mf -fp3 -fpi -3r -oxnt -zp1 -j -ei -wx -we
- Links with wlink using dos4g extender
- No automated test framework exists (legacy DOS application)
- No linting tools configured
- Manual testing required: run sgd.exe and test functionality interactively

# Code Style Guidelines

## File Structure
- Every file starts with 66-character block header comment containing: Project, Module, Description, Version type, Last changed, Update count, OS type, Compiler, Linker, and Copyright/GPL license
- Include guards use _H_INC suffix (e.g., DBMAIN_H_INC)
- Module-specific .c files define __MODULE_MAIN__ to enable EXTERN as empty, .h files define EXTERN as extern
- Example EXTERN pattern:
  ```c
  #ifdef __MODULE_MAIN__
  #define EXTERN
  #else
  #define EXTERN extern
  #endif
  EXTERN struct ComboInfo *SWindow;
  ```

## Imports and Includes
- Local headers included after system headers
- Order: system headers (dos.h, fcntl.h, io.h, etc.) then wbench32/wbench32.h then local module headers
- Always include "wbench32/wbench32.h" for UI framework
- Pragma pack() used before structure definitions
- Example include order:
  ```c
  #include <dos.h>
  #include <fcntl.h>
  #include <io.h>
  #include "wbench32/wbench32.h"
  #include "dbmain.h"
  #include "dbconfig.h"
  #pragma pack()
  ```

## Types and Typedefs
- byte (unsigned char), word (unsigned short), dword (unsigned int), bool (char), comboent (long)
- Pointer syntax: type *name with space before asterisk
- Structures use _s suffix (e.g., DBaseEntry_s, ComboInfo)
- Constants in ALL_CAPS with underscore separation (e.g., DBNAME, MAXDIR)
- Boolean values: TRUE and FALSE (defined as (bool)1 and (bool)0)
- Type definitions come from wbench32.h or defined locally

## Naming Conventions
- Functions: lowercase with PascalCase module prefixes (e.g., HandleAddEntry, TestPlayGhosted, GhostMenuBackup)
- Variables: camelCase for locals, PascalCase for struct members
- Hungarian-style for flags and bitfields (e.g., BF_FWIDTH, WINB_OUT, WINB_SURROUND)
- Static functions prefixed with underscore (e.g., _SetupObjectsWindow, _PrintLine, _LoadHardwareTable)
- Global variables declared with EXTERN macro in headers
- Window/Control variables use descriptive names: ViewWindow, ButtonP, NEntry

## Formatting
- Tab-based indentation (no specific width enforced)
- Opening brace on new line for functions
- Space after comma, before and after operators
- Line length kept under 120 characters
- Multiple variable declarations aligned vertically
- Function implementation pattern:
  ```c
  bool ReadDatabase (void)
  /**********************************************************************************************************************************/
  /* Pre   : None.                                                                                                                  */
  /* Post  : The database has been read from disk. Return value is TRUE on success.                                                 */
  /* Import: None.                                                                                                                  */
  /**********************************************************************************************************************************/
  {
    /* Implementation */
  }
  ```

## Comments
- /* */ style only - NO // comments allowed
- Detailed Pre/Post/Import comment block before every function
  - Pre: Preconditions (what must be true before calling)
  - Post: Postconditions (what is true after execution)
  - Import: Dependencies or side effects
- Inline comments sparse, only for non-obvious logic
- Block comments use /* on separate line, */
- Comment blocks formatted with exact width (66 characters for file headers)

## Error Handling
- Functions return bool TRUE/FALSE for success/failure
- Use ErrorRequester() for user-facing error messages
- Use perror() for system errors (with exit(1) in utility files)
- Check FileNotFound global variable for file operation errors
- Validate database entries after load with ValidateEntries()
- Confirm actions before destructive operations (ConfirmDelete, ConfirmOverwrite)
- Example error pattern:
  ```c
  if (!FileOpen) {
    ErrorRequester(-1, -1, NULL, NULL, NULL, "ERROR: Could not open file");
    return FALSE;
  }
  ```

## Module Organization
- Header files (.h) contain: include guards, EXTERN macro setup, type definitions, function prototypes
- Implementation files (.c) contain: __MODULE_MAIN__ define, includes, static variable definitions, static function prototypes, function implementations
- Each module handles specific functionality (dbmain, dbfile, dbconfig, dbdbase, dbscreen, etc.)
- Uses wbench32 UI framework library for windowing and controls
- Modules: dbmain (main program), dbfile (file I/O), dbconfig (configuration), dbdbase (database operations), dbscreen (screen handling), dbmenu (menu system), dbselect (search/select), dbpoke (game cheats/pokes)

## Database Operations
- Database entries 240 bytes each (DBLEN constant)
- Store database in GAMES.DAT (DBNAME constant)
- Backup files use .BAK extension
- Temporary files use .TMP extension
- XMS (Extended Memory Specification) used for large data structures
- DBaseEntry_s structure contains: Name, Year, Publisher, Author, Memory, Players, Type, DirIndex, FileSize, etc.
- Functions: ReadDatabase(), WriteDatabase(), AddDBEntry(), DelDBEntry(), SortEntry(), ValidateEntries()

## Special Patterns
- ComboInfo structures for list/selection windows (TWindow, SWindow)
- WindowInfo structures for dialog windows (ViewWindow, CWindow, MessageWindow)
- ButtonInfo structures for UI controls (ButtonP, PCancel, POK)
- Ghost/Unghost functions to enable/disable UI elements based on state (GhostMenuBackup, UnghostMenuBackup, TestPlayGhosted)
- Validate functions to check data integrity after operations (ValidateAnEntry, ValidateEntries)
- ComboEntry structures for window entries (NEntry, CurrentSEntry, NewEntry)
- FontInfo structures for text rendering (SpectrumFont, _UsedMenuFont)
- Color constants: DBLACK, DWHITE, etc. for Spectrum palette colors

## File Format Support
- TAP (tape files), Z80 (snapshots), SNA (snapshots), TZX (tape files with extensions)
- MDR (microdrive), SCR (screen dumps), SLT ( Spectrum Loading Tool format)
- VOC (voice files), LTP (tape listing), TRD (TR-DOS disk), DSK (disk images)
- File type detection via GetFileType() function
- File validation functions: GetZ80Version(), GetTZXVersion(), GetSLTExtensionOffset()
- Index insertion functions: InsertTAPIndex(), InsertMDRIndex(), InsertTZXIndex()
