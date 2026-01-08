# PokeSGD - Apply Spectrum Game Pokes (PHP Implementation)

A PHP implementation of the SGD (Spectrum Game Database) poke system that applies game cheats to ZX Spectrum Z80 and SNA snapshot files.

## Features

- **Z80 Format Support**: v1.45, v2.01, v3.00 with full RLE compression/decompression
- **SNA Format Support**: 48K snapshots only
- **Bank Auto-Detection**: Automatic memory bank detection for 48K/128K snapshots
- **Multiple Poke Sources**: POKES files or command-line pokes
- **Selective Application**: Apply specific poke groups using pokelistselect
- **Error Logging**: Optional error logging to file
- **Web & CLI**: Accessible via HTTP request or command line

## Installation

Place `pokesgd.php` in any directory accessible by PHP. No external dependencies required.

## Usage

### Web Usage

#### Apply all pokes from POKES file:
```
http://yourserver.com/pokesgd.php?file=ROMSMINE/F/FIRELORD.Z80
```

#### Apply specific poke groups (1-based indices):
```
http://yourserver.com/pokesgd.php?file=ROMSMINE/F/FIRELORD.Z80&pokelistselect=1,3
```

#### Apply command-line pokes:
```
http://yourserver.com/pokesgd.php?file=ROMSMINE/F/FIRELORD.Z80&POKES=34509,0:34984,58:44444,222
```

#### With error logging:
```
http://yourserver.com/pokesgd.php?file=ROMSMINE/F/FIRELORD.Z80&logerrors=/var/log/pokesgd.log
```

### Command Line Usage

```
php pokesgd.php ROMSMINE/F/FIRELORD.Z80
php pokesgd.php ROMSMINE/F/FIRELORD.Z80 pokelistselect=1,3
php pokesgd.php ROMSMINE/F/FIRELORD.Z80 POKES=34509,0:34984,58
php pokesgd.php ROMSMINE/F/FIRELORD.Z80 POKES=34509,0 logerrors=/var/log/pokesgd.log
```

### Parameters

| Parameter | Description | Example |
|-----------|-------------|---------|
| `file` | Path to Z80 or SNA file (required) | `ROMSMINE/F/FIRELORD.Z80` |
| `POKES` | Command-line pokes: address,value pairs separated by `:` | `34509,0:34984,58` |
| `pokelistselect` | Comma-separated poke group numbers (1-based) | `1,3` |
| `logerrors` | Path to error log file (optional) | `/var/log/pokesgd.log` |

## POKE File Format

POKE files are simple text files located alongside the game file or in a `POKES/` subdirectory.

### File Format

```
N<Poke Description>            # New poke group
M<bank> <address> <value> <org>  # More poke entries
Z<bank> <address> <value> <org>  # Last poke entry in group
Y                             # End marker
```

### Field Descriptions

- **Bank**: Memory bank (0-7 for 128K, 8 for 48K, or `-` for auto-detect)
- **Address**: Memory address (16384-65535)
- **Value**: Value to set (0-255, 256/ASK to prompt user → replaced with 244)
- **Original**: Original value at that address

### Example POKE File

```
NInfinite lives  -ok           
Z  8 34509   0  53
NInfinite life energy -ok      
Z  8 34984  58 204
Y
```

### Example with Multiple Groups

```
NInfinite Lives
  8 24576 255 0
  8 25000 255 0
Z  8 26000 255 0
NInfinite Energy
  8 32000 255 0
Z  8 32005 255 0
NLevel Select
  5 30000 1 0
Y
```

Apply only groups 1 and 3:
```
pokesgd.php?file=ROMSMINE/F/FIRELORD.Z80&pokelistselect=1,3
```

## Command-Line Pokes Format

Format: `address1,value1:address2,value2:address3,value3`

- **address**: Memory address (16384-65535)
- **value**: Value to set (0-255)
- **bank**: Automatically detected based on snapshot type

Example:
```
POKES=34509,0:34984,58:44444,222
```

This applies:
- Address 23423 → value 32
- Address 43253 → value 123
- Address 44444 → value 222

## Output

The script creates a patched file in one of these locations:

1. **Primary**: Same directory as input file
   - `ROMSMINE/F/FIRELORD.Z80` → `ROMSMINE/F/FIRELORD_POKED.z80`

2. **Fallback**: Script directory (if no write permission)
   - `./FIRELORD_POKED.z80`

### Web Response (JSON)

Success:
```json
{
  "success": true,
  "output": "ROMSMINE/F/FIRELORD_POKED.z80",
  "pokes_applied": 12
}
```

Failure:
```json
{
  "success": false,
  "error": "File not found: ROMSMINE/F/FIRELORD.Z80"
}
```

### CLI Output

```
SUCCESS: Pokes applied successfully
Output: ROMSMINE/F/FIRELORD_POKED.z80
Pokes applied: 12
Execution time: 45.23ms
```

## Memory Bank Auto-Detection

The script automatically detects the correct memory bank based on:

- **48K snapshots**: Bank 8 (no paging)
- **128K snapshots** (based on Z80 header HWMode field):
  - Address < 32768 → Bank 5
  - Address 32768-49151 → Bank 2
  - Address >= 49152 → Bank 0

## Special Values

- **Value 256 or "ASK"**: Replaced with 244 (user prompt simulation)
- **Bank 8 or "-"**: Indicates 48K snapshot (no paging)
- **Address validation**: Must be in range 16384-65535

## Z80 Compression

The script implements the Z80 run-length encoding (RLE) algorithm:

### Decompression
- Detect `0xED 0xED XX YY` sequences
- XX = length (0-255, where 0 = 256)
- YY = byte to repeat
- All other bytes are literal

### Compression
- Detect repeated sequences of 2+ identical bytes
- Replace with `0xED 0xED XX YY` format
- Keep single bytes and 2-byte runs as literals

## Examples

### Example 1: Apply all pokes from POKE file
```
php pokesgd.php ROMSMINE/F/FIRELORD.Z80
```

### Example 2: Apply specific poke groups
```
php pokesgd.php ROMSMINE/F/FIRELORD.Z80 pokelistselect=1,2
```

### Example 3: Apply command-line pokes to SNA file
```
php pokesgd.php ROMSMINE/B/BUBBLEBO.SNA POKES=43873,195:43929,16
```

### Example 4: With error logging
```
php pokesgd.php ROMSMINE/F/FIRELORD.Z80 logerrors=pokes_errors.log
```

### Example 5: Web request with parameters
```
curl "http://server.com/pokesgd.php?file=ROMSMINE/F/FIRELORD.Z80&POKES=34509,0:34984,58"
```

## Error Handling

The script validates:
- File existence and readability
- File type (Z80 or SNA)
- Address range (16384-65535)
- Value range (0-255)
- Write permissions for output

Errors are logged if `logerrors` parameter is provided.

## Compatibility

- **PHP Version**: 7.0 or higher
- **Z80 Versions**: 1.45, 2.01, 3.00
- **SNA Versions**: 48K only (27KB header + 49KB memory)
- **Platforms**: Any OS with PHP support

## Technical Details

### Z80 v1.45 Format
- 30-byte header
- Single compressed/uncompressed 48KB or 49KB memory block
- Compression flag in Stat1 byte (bit 5)

### Z80 v2.01/v3.00 Format
- 30-byte v145 header
- Extended v201/v300 header with HWMode field
- Banked memory blocks (16KB per bank)
- Each block: 3-byte header (length + bank) + compressed data

### SNA Format
- 27-byte header with register state
- 49KB memory data (no compression)
- PC register optionally at end

## License

This implementation is based on the original SGD C code by Martijn van der Heide and is released under the GNU General Public License version 2.
