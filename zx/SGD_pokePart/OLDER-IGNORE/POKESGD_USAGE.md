# pokesgd.php - ZX Spectrum Z80 POKE Patching Script

## Overview
This PHP script applies POKE patches to ZX Spectrum Z80 snapshot files. It supports all three major Z80 formats (v1, v2, v3) with automatic version detection, decompression/compression, and bank switching for 128K snapshots.

## Features

### Z80 Format Support
- **Version 1 (v1.45)**: 48K snapshots with single compressed RAM block
- **Version 2 (v2.01)**: 48K/128K snapshots with 16KB memory blocks, 23-byte additional header
- **Version 3 (v3.00)**: 48K/128K snapshots with 16KB memory blocks, 54-55 byte additional header

### Compression Algorithm
Implements the standard Z80 compression scheme:
- Replaces sequences of 5+ identical bytes with `ED ED xx yy` (where xx=length, yy=byte)
- Special case: `ED ED` sequences always encoded as `ED ED 02 ED`
- Version 1 adds end marker: `00 ED ED 00`
- Version 3 skips compression if result > 16383 bytes

### Memory Banking
Handles automatic bank resolution for:
- **48K mode**: Banks 4, 5, 8 (pages mapping 0x4000-0xFFFF)
- **128K mode**: Banks 0, 2, 5 (pages mapping 0x4000-0xFFFF)

## Usage

### Method 1: URL Parameters
Apply specific pokes via query string:

```
http://localhost/pokesgd.php?file=ROMSMINE/F/FIRELORD.Z80&POKES=34509,0:34984,58
```

Format: `address,value:address,value:...`
- `address`: Memory address (16384-65535 for RAM)
- `value`: Byte value (0-255)

Multiple pokes separated by colon.

### Method 2: .POK File
Use a .POK file from the POKES subdirectory:

```
http://localhost/pokesgd.php?file=ROMSMINE/F/FIRELORD.Z80
```

Script looks for `{path}/POKES/{filename}.POK`

### .POK File Format
```
NInfinite lives -ok           # Poke name (ignored)
Z  8 34509   0  53       # Poke entry
M  8 42900 201  33       # Multi-mode poke
Y                           # End of file
```

Entry format: `{M|Z} bank address set org`
- `M`/`Z`: Poke mode (ignored)
- `bank`: Memory bank (8 for 48K, or page number for 128K)
- `address`: Memory address (16384-65535)
- `set`: Value to write
- `org`: Original value (reference only)

Bank flags:
- Bit 0x08: Ignore bank (use auto bank)
- Bit 0x10: Ask user for value (skipped by script)

## Output

### File Naming
Patched files are saved with incrementing number:
```
{originalname}_POKED{NN}.z80
```

Example: `FIRELORD.Z80` → `FIRELORD_POKED01.z80`, `FIRELORD_POKED02.z80`, ...

### JSON Response
```json
{
  "success": true,
  "version": 1,
  "input_file": "ROMSMINE/F/FIRELORD.Z80",
  "output_file": "ROMSMINE/F/FIRELORD_POKED01.z80",
  "pokes_applied": 2
}
```

Error response:
```json
{
  "success": false,
  "error": "Error message here"
}
```

## Configuration

### Debug Mode
Enable error logging at top of script:

```php
$debug = true;  // Enable/disable error logging
$logfile = dirname(__FILE__) . '/pokesgd_errors.log';
```

When enabled, errors are logged with timestamp and details.

## Error Handling

The script handles these error conditions:

1. **Missing file parameter**: Returns error with usage instructions
2. **File not found**: Returns error with file path
3. **Invalid Z80 version**: Returns error with header length
4. **Invalid address** (< 16384 or > 65535): Returns error
5. **Invalid value** (< 0 or > 255): Returns error
6. **Invalid poke format**: Returns error with expected format
7. **No POKES file found**: Returns error with checked paths
8. **File validation failure**: Deletes output file and returns error

## Memory Addressing

ZX Spectrum memory layout:
- `0x0000-0x3FFF` (0-16383): ROM (read-only, no pokes)
- `0x4000-0x7FFF` (16384-32767): RAM - Bank 5 (128K) / Bank 8 (48K)
- `0x8000-0xBFFF` (32768-49151): RAM - Bank 2 (128K) / Bank 1 (48K)
- `0xC000-0xFFFF` (49152-65535): RAM - Bank 0 (128K) / Bank 2 (48K)

## Examples

### Example 1: Single poke via URL
```
http://localhost/pokesgd.php?file=ROMSMINE/F/FIRELORD.Z80&POKES=34509,0
```
Applies: Address 34509 = value 0 (infinite lives)

### Example 2: Multiple pokes via URL
```
http://localhost/pokesgd.php?file=ROMSMINE/F/FIRELORD.Z80&POKES=34509,0:34984,58:42900,201
```
Applies three pokes at different addresses.

### Example 3: Use .POK file
```
http://localhost/pokesgd.php?file=ROMSMINE/F/FIRELORD.Z80
```
Reads `ROMSMINE/F/POKES/FIRELORD.POK` and applies all valid pokes.

## Testing

The script has been tested with:
- ✓ Z80 version detection (v1, v2, v3)
- ✓ Decompression and compression algorithms
- ✓ URL parameter parsing
- ✓ .POK file parsing (M and Z modes)
- ✓ Bank switching (48K and 128K)
- ✓ Memory addressing and POKE application
- ✓ File validation
- ✓ Error handling
- ✓ Output file generation with incrementing numbers

## Requirements

- PHP 5.6 or higher
- File system write permissions in the target directory

## Notes

- The script only modifies RAM addresses (16384-65535). ROM addresses (0-16383) are rejected.
- Original files are never modified.
- Output files are validated before returning success.
- If validation fails, the output file is deleted.
- Bank switching follows the C source implementation (dbpoke.c).

## References

- Z80 format specification: See Z80_format_specification.txt
- C source implementation: dbpoke.c, dbscreen.c
- Version detection: z80_version_check.py
