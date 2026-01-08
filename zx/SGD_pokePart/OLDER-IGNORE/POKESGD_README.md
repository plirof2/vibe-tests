# POKESGD_README.md

## Overview

This document provides comprehensive guidance for implementing and maintaining `pokesgd.php`, the ZX Spectrum Z80 POKE patching script.

## Z80 Memory Mapping and Addressing

### ZX Spectrum Memory Layout

The ZX Spectrum memory map is linear from 0 to 65535:

```
Address Range    Size     Description                    Z80 Page Mapping
-------------    -----     -----------                   ----------------
0x0000-0x3FFF    16KB       ROM (read-only)              Page 0, 1, 2 (varies)
0x4000-0x7FFF    16KB       RAM - Display File              Page 5 (128K) / Page 8 (48K v2/v3)
0x8000-0xBFFF    16KB       RAM - Attributes/Misc           Page 2 (128K) / Page 1 (48K v2/v3)  
0xC000-0xFFFF    16KB       RAM - Stack/Variables          Page 0 (128K) / Page 2 (48K v2/v3)
```

**Key Points**:
- ROM addresses (0-16383) are READ-ONLY and **cannot be poked**
- POKEs only apply to RAM addresses (16384-65535)
- The POKE script validates all addresses are in RAM range

### Version-Specific Memory Handling

#### Version 1.45 (Old Format)
- **Single 48KB block** containing all RAM (49152 bytes)
- Stored as compressed or uncompressed data
- Memory is contiguous: just offset by -16384
- Example: Address 25000 → Offset = 25000 - 16384 = 8616

#### Version 2.01 and 3.00 (New Format with Banking)

Memory is split into **16KB pages**, each with its own compressed block:

```
48K Mode Page Mapping:
  Page 4  →  0x8000-0xBFFF  (32768-49151)
  Page 5  →  0x4000-0x7FFF  (16384-32767)
  Page 8  →  0xC000-0xFFFF  (49152-65535)

128K Mode Page Mapping:
  Page 0  →  0xC000-0xFFFF  (49152-65535)
  Page 2  →  0x8000-0xBFFF  (32768-49151)
  Page 5  →  0x4000-0x7FFF  (16384-32767)
  Pages 3,4,6,7,9,10  →  Additional RAM banks (not used for 48K POKEs)
```

**Critical Difference**:
- **v1.45**: Uses "bank" terminology internally (always bank 8)
- **v2/v3**: Uses "page" numbers in file headers
- **POKE compatibility**: POKEs must be mapped to the correct page based on address range

### Address to Page Mapping Algorithm

The C code (dbpoke.c:1146-1188) implements this logic:

```c
byte AutoBank(word Address, char *Bank)
{
    if (Snap128K) {
        // 128K mode: direct page mapping
        if (Address < 32768) return 5;
        else if (Address < 49152) return 2;
        else return 0;
    } else {
        // 48K mode: page mapping
        if (Address < 32768) return 5;   // 0x4000-0x7FFF
        else if (Address < 49152) return 4;   // 0x8000-0xBFFF
        else return 8;                    // 0xC000-0xFFFF
    }
}
```

### Incompatibility Prevention

**To ensure compatibility between v1.45 and v2/v3:**

1. **Always detect version first** before reading file structure
2. **Use correct offset calculation**:
   - v1.45: Simple linear offset (Address - 16384)
   - v2/v3: Page-based offset based on address range
3. **Validate address ranges**:
   - Reject addresses 0-16383 (ROM)
   - Accept addresses 16384-65535 (RAM)
4. **Preserve compression format**:
   - v1.45: Has end marker `00 ED ED 00`
   - v2/v3: No end marker
5. **Handle block headers**:
   - v1.45: No block headers (just data)
   - v2/v3: Each block has 3-byte header (length + page)

## Z80 File Format Specifications

### Version 1.45 (30-byte header only)

```
Offset  Length  Description
-------  -------  -----------
0-1     2        A register
2-3     2        F register
4-5     2        BC register pair
6-7     2        PC (Program Counter) - **Non-zero = v1.45**
8-9     2        SP (Stack Pointer)
10      1        I register
11      1        R register (refresh)
12      1        Status byte (bit 5 = compressed flag)
13-21    9        Other registers
30-     49152    RAM data (compressed or uncompressed)
                   + end marker if compressed: 00 ED ED 00
```

### Version 2.01 (30-byte header + 23-byte additional header)

```
Additional Header (23 bytes):
Offset  Length  Description
-------  -------  -----------
30-31    2        Additional header length (always 23)
32-33    2        PC (always 0 in v2/v3)
34      1        Hardware mode (3+ = 128K for v2.01)
35-55    21       Other hardware registers
```

Memory blocks follow (each 16KB):
```
Offset  Length  Description
-------  -------  -----------
53      3        Block header: length(2) + page(1)
55+     varies   Compressed data for this page
...     ...      More blocks follow
```

### Version 3.00 (30-byte header + 54-55 byte additional header)

```
Additional Header (54-55 bytes):
Offset  Length  Description
-------  -------  -----------
30-31    2        Additional header length (54 or 55)
32-33    2        PC (always 0 in v2/v3)
34      1        Hardware mode (4+ = 128K for v3.00)
35-55    21-55    Other hardware registers + extra fields
```

Memory blocks follow (same structure as v2.01)

## Compression Algorithm

### Z80 Compression Scheme (All Versions)

The compression is a simple RLE (Run-Length Encoding):

**Compression Rules:**
1. **Normal bytes**: Copied as-is
2. **Repeated sequences (5+ bytes)**: Encoded as `ED ED length byte`
   - `ED ED`: Marker
   - `length`: Number of repetitions (1-255)
   - `byte`: The byte to repeat
3. **Special case - ED ED sequence**: Always encoded as `ED ED 02 ED`
   - This handles two ED bytes followed by anything
4. **Version 1 end marker**: `00 ED ED 00` (v2/v3 have no end marker)

**Example:**
```
Original:  A A A A A A A B
Compressed: A A ED ED 06 A B
            (A A)  (repeat 6x A) B
```

### Compression Edge Cases

**Bytes after ED:**
A single `0xED` byte is never compressed if the next byte is NOT `0xED`.

**Version 3 Special Case:**
If a 16KB block compresses to > 16383 bytes, the algorithm returns the **uncompressed** data instead (C code: dbscreen.c:1132-1136). This is because the compression overhead exceeds the benefit.

## Implementation Guidelines

### Critical Functions

#### 1. Version Detection (`detectZ80Version()`)

```php
function detectZ80Version($filePath) {
    $header = file_get_contents($filePath, false, null, 0, 32);
    
    // Read PC (bytes 6-7)
    $pc = unpack('v', substr($header, 6, 2))[1];
    
    if ($pc != 0) {
        return 1;  // Version 1.45
    }
    
    // Read additional header length (bytes 30-31)
    $addHeaderLen = unpack('v', substr($header, 30, 2))[1];
    
    if ($addHeaderLen == 23) {
        return 2;  // Version 2.01
    }
    
    if ($addHeaderLen == 54 || $addHeaderLen == 55) {
        return 3;  // Version 3.00
    }
    
    throw new Exception("Unknown Z80 version");
}
```

#### 2. Reading Z80 Files (`readZ80File()`)

**For v1.45:**
```php
// Read single block (49152 bytes after header)
$ramData = file_get_contents($filePath, false, null, 30);
$data['blocks'][] = [
    'page' => 8,
    'data' => $ramData
];
```

**For v2/v3:**
```php
// Read additional header first
$addHeaderLen = unpack('v', substr($commonHeader, 30, 2))[1];
$additionalHeader = file_get_contents($filePath, false, null, 30, $addHeaderLen);

// Read blocks until end of file
$offset = 30 + $addHeaderLen;
while ($offset < $fileSize) {
    // Read block header: length(2) + page(1)
    $blockHeader = file_get_contents($filePath, false, null, $offset, 3);
    
    $blockLen = unpack('v', substr($blockHeader, 0, 2))[1];
    $page = unpack('C', substr($blockHeader, 2, 1))[1];
    
    if ($blockLen == 0) {
        break;  // End marker
    }
    
    $offset += 3;
    $compressedData = file_get_contents($filePath, false, null, $offset, $blockLen);
    
    $data['blocks'][] = [
        'page' => $page,
        'compressed' => $compressedData,
        'length' => $blockLen
    ];
    
    $offset += $blockLen;
}
```

#### 3. Applying POKEs (`applyPokesToMemory()`)

**Critical: Match POKE bank to Z80 page**

```php
foreach ($pokes as $poke) {
    $address = $poke['address'];
    $value = $poke['value'];
    
    if ($version == 1) {
        // Simple offset
        $offset = $address - 16384;
        $memoryBlock[8][$offset] = chr($value);
    } else {
        // v2/v3: Find correct page
        $page = autoBank($address, $is128K);
        
        // Calculate offset within 16KB page
        if ($address < 32768) {
            $offset = $address - 16384;  // 0x4000-0x7FFF
        } else if ($address < 49152) {
            $offset = $address - 32768;  // 0x8000-0xBFFF
        } else {
            $offset = $address - 49152;  // 0xC000-0xFFFF
        }
        
        $memoryBlock[$page][$offset] = chr($value);
    }
}
```

#### 4. Compressing Blocks (`compressZ80Block()`)

```php
function compressZ80Block($uncompressedData, $version) {
    // Standard compression logic (see algorithm above)
    $compressed = /* compression result */;
    
    // Version 1: Add end marker
    if ($version == 1) {
        $compressed .= "\x00\xED\xED\x00";
    }
    
    // Version 3: Check if compression is beneficial
    if ($version == 3 && strlen($uncompressedData) == 16384) {
        if (strlen($compressed) > 16383) {
            // Return uncompressed instead
            return $uncompressedData;
        }
    }
    
    return $compressed;
}
```

#### 5. Writing Z80 Files (`writeZ80File()`)

```php
function writeZ80File($filePath, $z80Data, $version) {
    $content = '';
    
    // Write common header (30 bytes)
    $content .= $z80Data['header'];
    
    // Write additional header for v2/v3
    if ($version > 1) {
        $content .= $z80Data['additionalHeader'];
    }
    
    // Write memory blocks
    foreach ($z80Data['blocks'] as $block) {
        if ($version > 1) {
            // Write block header (length + page)
            $blockData = $block['compressed'];
            $blockLen = strlen($blockData);
            $content .= pack('v', $blockLen);  // Little-endian
            $content .= chr($block['page']);
            $content .= $blockData;
        } else {
            // Version 1: Just write the data
            $content .= $block['data'];
        }
    }
    
    return file_put_contents($filePath, $content);
}
```

## Testing

### Test Files Required

1. **Version 1.45**: `ROMSMINE/F/FIRELORD.Z80`
   - Test backward compatibility
   - Verify end marker handling

2. **Version 2.01**: `ROMSMINE/L/LASTDUEL.Z80`
   - Test block header reading
   - Verify page mapping (48K or 128K)
   - Example command: `?file=ROMSMINE/L/LASTDUEL.Z80&POKES=37605,0:37672,0:40063,0`

3. **Version 3.00**: `ROMSMINE/L/LASTVAMP.Z80`
   - Test v3 additional header (54-55 bytes)
   - Test compression fallback (if > 16383 bytes)
   - Test 128K page mapping

### Validation Checklist

- [ ] Version detection works for all three versions
- [ ] v1.45 files read correctly (single block, end marker)
- [ ] v2.01 files read correctly (block headers, page numbers)
- [ ] v3.00 files read correctly (extended header, page numbers)
- [ ] POKEs apply to correct memory offset in all versions
- [ ] 48K mode uses correct pages (4, 5, 8)
- [ ] 128K mode uses correct pages (0, 2, 5)
- [ ] v3 compression returns uncompressed when > 16383 bytes
- [ ] Output files are valid Z80 format
- [ ] Backward compatibility maintained

## Common Issues and Solutions

### Issue 1: Wrong memory block poked

**Symptom**: POKE doesn't affect expected memory location

**Cause**: Page number doesn't match address range

**Solution**: Ensure `autoBank()` returns correct page for address range:
- 16384-32767 → Page 5 (v1.45: bank 8)
- 32768-49151 → Page 2 (v1.45: bank 8)
- 49152-65535 → Page 0 (v1.45: bank 8)

### Issue 2: Decompression fails

**Symptom**: Error during decompression

**Cause**: Missing end marker handling for v1.45 or incorrect offset

**Solution**: 
- v1.45: Skip last 4 bytes before decompressing (00 ED ED 00)
- v2/v3: Decompress entire block (no end marker)
- Verify `0xFFFF` block length handling for v3 (uncompressed block marker)

### Issue 3: Compressed data too large

**Symptom**: v3 block compressed to > 16383 bytes

**Solution**: Return uncompressed data instead (as per C code dbscreen.c:1132-1136)

## References

- C Source: `dbpoke.c` (lines 270-1758)
- C Source: `dbscreen.c` (lines 1030-1138)
- Z80 Spec: `Z80_format_specification.txt`
- Python Check: `z80_version_check.py`
- PHP v1.45: `pokesgd_v001_okFirelordv1,45.php`
- PHP Version Check: `z80_version_check2.php`

## Version History

- **v0.01**: Initial implementation for v1.45 only
- **v0.02**: Added v2.01 and v3.00 support (planned)
