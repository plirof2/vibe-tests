# POKES Implementation Summary

## Document Purpose

This document summarizes the implementation decisions and key algorithms used in `pokesgd.php` for patching ZX Spectrum Z80 snapshot files. It serves as a quick reference for understanding how C source code was migrated to PHP.

## Implementation Decisions and Answers to Key Questions

### Q1: How does Z80 processor handle memory mapping?

**Answer**: ZX Spectrum has a linear 64KB address space (0-65535), but Z80 file formats use different internal representations:

**Physical Memory Map**:
```
0x0000-0x3FFF (0-16383)    : ROM - READ ONLY (cannot be poked)
0x4000-0x7FFF (16384-32767) : RAM Bank 5 (128K) / Page 5/8 (48K v2/v3)
0x8000-0xBFFF (32768-49151) : RAM Bank 2 (128K) / Page 2/4 (48K v2/v3)
0xC000-0xFFFF (49152-65535) : RAM Bank 0 (128K) / Page 0/2 (48K v2/v3)
```

**Version 1.45**:
- Single 48KB RAM block (49152 bytes)
- Direct linear addressing: offset = address - 16384
- No page numbers (uses "bank 8" internally)

**Version 2.01/3.00**:
- Multiple 16KB pages with explicit page numbers
- Each page has its own compressed block
- Page numbers map differently for 48K vs 128K mode

**Incompatibility Prevention**:
- Always detect version FIRST before processing
- Use correct offset calculation based on version
- Map POKE addresses to correct page using `autoBank()` function
- Validate address is in RAM range (16384-65535)

### Q2: Version 3 uncompressed handling - how to store when compression > 16383 bytes?

**Answer**: For v3 files, if a 16KB block compresses to > 16383 bytes, store the block **completely uncompressed** (no compression markers).

**Implementation** (migrated from dbscreen.c:1132-1136):
```php
function compressZ80Block($uncompressedData, $version) {
    // ... standard compression logic ...
    
    // Version 3 special case
    if ($version == 3 && $len == 16384) {
        $compressed = /* standard compression result */;
        
        if (strlen($compressed) > 16383) {
            // Compression doesn't help - return uncompressed
            return $uncompressedData;
        }
    }
    
    return $compressed;
}
```

**Rationale**: 
- A 16KB uncompressed block = 16384 bytes
- If compressed > 16383 bytes, it's actually larger than original
- Storing uncompressed is more space-efficient
- The block header length field can indicate 0xFFFF (uncompressed marker) but this isn't explicitly used in C code

**Note**: This is a special case that may need refinement later. For now, store uncompressed with comment.

### Q3: Page vs Bank terminology - which to use in PHP?

**Answer**: Use **"page"** terminology when referring to Z80 file storage structure and **"bank"** when referring to internal snapshot type (48K vs 128K).

**Terminology Guide**:

| Term              | Usage Context                          | Example                          |
|------------------|--------------------------------------|-----------------------------------|
| **Page**          | Z80 file block number (0-10)        | "Read page 5 from Z80 file"       |
| **Bank**          | Memory bank type (48K or 128K)     | "This is a 128K bank snapshot"   |
| **Page 0/2/5**   | RAM pages for 128K mode                | "Address maps to page 2 in 128K"   |
| **Page 4/5/8**   | RAM pages for 48K mode (v2/v3)       | "Address maps to page 5 in 48K"   |
| **Bank 8**        | Internal bank number (v1.45 only)      | "Version 1 uses bank 8"              |

**PHP Variable Naming**:
```php
$memoryPage = $page;  // Page number from Z80 file header
$is128K = $is128K;  // Bank type (48K or 128K)
$bank = autoBank($address, $is128K);  // Calculated page number

// Good:
if ($page == 5) { /* handle page 5 */ }

// Bad (confusing):
if ($bank == 5) { /* unclear - file bank or internal bank? */ }
```

### Q4: Output filename - keep existing _POKED{NN}.z80 naming?

**Answer**: Yes, maintain existing naming convention.

**Naming Convention**:
```
Input:  ROMSMINE/F/FIRELORD.Z80
Output: ROMSMINE/F/FIRELORD_POKED01.z80
         ROMSMINE/F/FIRELORD_POKED02.z80
         ROMSMINE/F/FIRELORD_POKED03.z80
```

**Implementation**:
```php
function generateOutputFilename($originalPath) {
    $pathInfo = pathinfo($originalPath);
    $dirname = $pathInfo['dirname'];
    $basename = $pathInfo['filename'];
    
    // Find existing _POKED files to get next number
    $pattern = $dirname . '/' . $basename . '_POKED*.z80';
    $files = glob($pattern);
    
    $maxNum = 0;
    foreach ($files as $file) {
        if (preg_match('/_POKED(\d+)\.z80$/i', basename($file), $matches)) {
            $num = intval($matches[1]);
            if ($num > $maxNum) {
                $maxNum = $num;
            }
        }
    }
    
    $nextNum = $maxNum + 1;
    return sprintf('%s/%s_POKED%02d.z80', $dirname, $basename, $nextNum);
}
```

**Benefits**:
- Maintains version history
- Users can revert to previous versions
- Prevents accidental overwrites
- Matches expected behavior from existing script

### Q5: Debug mode - useful for troubleshooting?

**Answer**: Yes, keep debug logging with ability to disable.

**Implementation**:
```php
// Configuration at top of file
$debug = false;  // Enable/disable as needed
$logfile = dirname(__FILE__) . '/pokesgd_errors.log';

function logError($message, $details = '') {
    global $debug, $logfile;
    
    if ($debug) {
        $timestamp = date('Y-m-d H:i:s');
        $entry = "[$timestamp] $message";
        if ($details) {
            $entry .= "\nDetails: $details";
        }
        file_put_contents($logfile, $entry . "\n", FILE_APPEND);
    }
}
```

**When to Use**:
- Development: Keep enabled
- Production: Keep disabled (or enable temporarily for troubleshooting)
- Issues: Enable to capture detailed error information

**What Gets Logged**:
- File parsing errors
- Version detection issues
- Decompression failures
- POKE application problems
- File write failures

## Core Algorithms Migration from C

### Algorithm 1: Version Detection (dbpoke.c → PHP)

**C Source** (dbpoke.c:295+):
```c
if (pc != 0) {
    return 145;  // Version 1.45
}

addHeaderLen = *(word*)(header + 30);
if (addHeaderLen == 23) {
    return 201;  // Version 2.01
}
if (addHeaderLen == 54 || addHeaderLen == 55) {
    return 300;  // Version 3.00
}
```

**PHP Implementation**:
```php
function detectZ80Version($filePath) {
    $header = file_get_contents($filePath, false, null, 0, 32);
    
    $pc = unpack('v', substr($header, 6, 2))[1];
    
    if ($pc != 0) {
        return 1;
    }
    
    $addHeaderLen = unpack('v', substr($header, 30, 2))[1];
    
    if ($addHeaderLen == 23) {
        return 2;
    }
    
    if ($addHeaderLen == 54 || $addHeaderLen == 55) {
        return 3;
    }
    
    throw new Exception("Unknown Z80 version");
}
```

**Key Migration Points**:
- Use `unpack('v', ...)` for little-endian 16-bit integers
- Return integer version numbers (1, 2, 3) instead of 145, 201, 300
- Throw exception for unknown versions

### Algorithm 2: Auto Bank/Page Resolution (dbpoke.c:1146-1188)

**C Source**:
```c
byte AutoBank(word Address, char *Bank) {
    if (Snap128K) {
        if (Address < 32768) return 5;
        else if (Address < 49152) return 2;
        else return 0;
    } else {
        if (Address < 32768) return 5;
        else if (Address < 49152) return 1;
        else return 2;
    }
}
```

**PHP Implementation**:
```php
function autoBank($address, $is128K) {
    if ($is128K) {
        // 128K mode: pages 5, 2, 0 for RAM
        if ($address < 32768) return 5;
        else if ($address < 49152) return 2;
        else return 0;
    } else {
        // 48K mode: pages 5, 1, 2 for v2/v3
        if ($address < 32768) return 5;
        else if ($address < 49152) return 1;
        else return 2;
    }
}
```

**Critical Note**: For v1.45, function returns 8 (internal bank). For v2/v3, it returns page numbers (5, 1, 2) that match Z80 file structure.

### Algorithm 3: Decompression (dbscreen.c:1030-1075)

**C Source**:
```c
for (IndexIn = 0; IndexIn < BlLength; IndexIn++) {
    if (*(BufferIn + IndexIn) != 0xED) {
        *(BufferOut + (IndexOut++)) = *(BufferIn + IndexIn);
    } else if (*(BufferIn + IndexIn + 1) != 0xED) {
        *(BufferOut + (IndexOut++)) = *(BufferIn + IndexIn);
    } else {
        RunTimeLength = *(BufferIn + IndexIn + 2);
        while (RunTimeLength-- > 0 && IndexOut < OutLength) {
            *(BufferOut + (IndexOut++)) = *(BufferIn + IndexIn + 3);
        }
        IndexIn += 3;
    }
}
```

**PHP Implementation**:
```php
function decompressZ80Block($compressedData, $version) {
    $decompressed = '';
    $len = strlen($compressedData);
    
    // Version 1: Skip end marker (4 bytes)
    if ($version == 1) {
        $len -= 4;
    }
    
    for ($i = 0; $i < $len; $i++) {
        $byte = ord($compressedData[$i]);
        
        if ($byte != 0xED) {
            $decompressed .= chr($byte);
        } else if ($i + 1 < $len && ord($compressedData[$i + 1]) == 0xED) {
            // Found crunch marker: ED ED xx yy
            $runLength = ord($compressedData[$i + 2]);
            $repeatedByte = ord($compressedData[$i + 3]);
            
            for ($j = 0; $j < $runLength; $j++) {
                $decompressed .= chr($repeatedByte);
            }
            
            $i += 3;  // Skip over crunch marker
        } else {
            $decompressed .= chr($byte);
        }
    }
    
    // Version 1: Verify end marker
    if ($version == 1) {
        $marker = substr($compressedData, $len, 4);
        if ($marker !== "\x00\xED\xED\x00") {
            throw new Exception("Invalid end marker");
        }
    }
    
    return $decompressed;
}
```

### Algorithm 4: Compression (dbscreen.c:1077-1138)

**C Source**:
```c
for (IndexIn = 0; IndexIn < BlLength - 4; IndexIn++) {
    if (*(BufferIn + IndexIn) == 0xED && 
        *(BufferIn + IndexIn + 1) == 0xED) {
        CrunchIt = TRUE;
    } else if (/* 5+ repetitions */) {
        CrunchIt = TRUE;
    }
    
    if (CrunchIt) {
        // Write: ED ED length byte
        *(BufferOut + (IndexOut++)) = 0xED;
        *(BufferOut + (IndexOut++)) = 0xED;
        *(BufferOut + (IndexOut++)) = RunTimeLength;
        *(BufferOut + (IndexOut++)) = RepeatedCode;
        IndexIn += RunTimeLength - 1;
    } else {
        *(BufferOut + (IndexOut++)) = *(BufferIn + IndexIn);
    }
}
```

**PHP Implementation**:
```php
function compressZ80Block($uncompressedData, $version) {
    $compressed = '';
    $len = strlen($uncompressedData);
    $i = 0;
    
    while ($i < $len - 4) {
        $current = $uncompressedData[$i];
        
        // Special case: ED ED sequence
        if ($current === "\xED" && $i + 1 < $len && $uncompressedData[$i + 1] === "\xED") {
            $compressed .= "\xED\xED\x02\xED";  // Encode 2 ED bytes
            $i += 2;
            continue;
        }
        
        // Check for 5+ repeated bytes
        $canCompress = ($i === 0 || $uncompressedData[$i - 1] !== "\xED");
        
        if ($canCompress &&
            $current === $uncompressedData[$i + 1] &&
            $current === $uncompressedData[$i + 2] &&
            $current === $uncompressedData[$i + 3] &&
            $current === $uncompressedData[$i + 4]) {
            
            // Count repetitions
            $runLength = 1;
            $j = $i + 5;
            
            while ($runLength < 255 && $j < $len && $uncompressedData[$j] === $current) {
                $runLength++;
                $j++;
            }
            
            // Write crunch marker
            $compressed .= "\xED\xED" . chr($runLength) . $current;
            $i += $runLength - 1;
        } else {
            $compressed .= $current;
        }
        $i++;
    }
    
    // Copy remaining bytes
    while ($i < $len) {
        $compressed .= $uncompressedData[$i];
        $i++;
    }
    
    // Version 1: Add end marker
    if ($version == 1) {
        $compressed .= "\x00\xED\xED\x00";
    }
    
    // Version 3: Check if compression is beneficial
    if ($version == 3 && $len == 16384 && strlen($compressed) > 16383) {
        // Return uncompressed instead
        return $uncompressedData;
    }
    
    return $compressed;
}
```

### Algorithm 5: Block Reading for v2/v3 (dbpoke.c:1636-1645)

**C Source**:
```c
while (NRead == 3) {
    if (BlockInfo.Number != IBank) {
        // Skip wrong block
        Offset += BlockInfo.Length + sizeof(struct BlockInfo_s);
    } else {
        // Read and decrunch
        if ((NRead = read(Fh, _BufferCrunched, BlockInfo.Length)) != BlockInfo.Length) {
            // Error
        }
        DecrunchZ80Block(_BufferCrunched, NRead, _BufferDecrunched + UsedBankStart, 16384);
        close(Fh);
        return TRUE;
    }
    if ((NRead = read(Fh, &BlockInfo, 3)) == -1) {
        // Error
    }
}
```

**PHP Implementation**:
```php
function readZ80File($filePath, $version) {
    $data = [
        'header' => '',
        'additionalHeader' => '',
        'blocks' => []
    ];
    
    // Read common header (30 bytes)
    $commonHeader = file_get_contents($filePath, false, null, 0, 30);
    $data['header'] = $commonHeader;
    
    if ($version == 1) {
        // Version 1: Single block
        $ramData = file_get_contents($filePath, false, null, 30);
        $data['blocks'][] = [
            'page' => 8,
            'data' => $ramData
        ];
    } else {
        // Version 2/3: Read additional header
        $addHeaderLen = unpack('v', substr($commonHeader, 30, 2))[1];
        $additionalHeader = file_get_contents($filePath, false, null, 30, $addHeaderLen);
        $data['additionalHeader'] = $additionalHeader;
        
        // Read blocks
        $offset = 30 + $addHeaderLen;
        $fileSize = filesize($filePath);
        
        while ($offset < $fileSize) {
            // Read block header: length(2) + page(1)
            $blockHeader = file_get_contents($filePath, false, null, $offset, 3);
            
            if (strlen($blockHeader) < 3) {
                break;
            }
            
            $blockLen = unpack('v', substr($blockHeader, 0, 2))[1];
            $page = unpack('C', substr($blockHeader, 2, 1))[1];
            
            // Length of 0 = end marker
            if ($blockLen == 0) {
                break;
            }
            
            // Read compressed data
            $offset += 3;
            $compressedData = file_get_contents($filePath, false, null, $offset, $blockLen);
            
            $data['blocks'][] = [
                'page' => $page,
                'compressed' => $compressedData,
                'length' => $blockLen
            ];
            
            $offset += $blockLen;
        }
    }
    
    return $data;
}
```

## Critical Implementation Checklist for v002

### Phase 1: Core Infrastructure (REQUIRED)

- [x] Version detection (already works)
- [ ] **Fix `readZ80File()` to parse block headers for v2/v3**
- [ ] **Fix `writeZ80File()` to write block headers for v2/v3**
- [ ] **Update `autoBank()` to return page numbers for 48K mode**
- [ ] **Add v3 compression special case (> 16383 bytes)**

### Phase 2: POKE Application Logic (REQUIRED)

- [ ] **Fix `applyPokesToMemory()` to match page numbers correctly**
- [ ] Update address offset calculation for different pages
- [ ] Test with LASTDUEL.Z80 (v2.01)

### Phase 3: Testing & Validation (RECOMMENDED)

- [ ] Test with v3.00 files
- [ ] Verify backward compatibility with v1.45
- [ ] Test both 48K and 128K snapshots
- [ ] Validate output files with existing tools

## File Format References

### Z80 Version 1.45
- Header: 30 bytes
- Data: Single 49152-byte block (compressed)
- End marker: `00 ED ED 00` (if compressed)

### Z80 Version 2.01
- Header: 30 + 23 bytes (additional header)
- Hardware mode: Byte 34 (3+ = 128K)
- Data: Multiple 16KB blocks
- Block structure: 3 bytes header + compressed data
  - Bytes 0-1: Length (little-endian)
  - Byte 2: Page number
  - Bytes 3+: Compressed data

### Z80 Version 3.00
- Header: 30 + 54-55 bytes (additional header)
- Hardware mode: Byte 34 (4+ = 128K)
- Data: Multiple 16KB blocks (same as v2.01)
- Special case: Uncompressed blocks if compression > 16383 bytes

## Quick Reference

### Memory Address Ranges by Page

| Address Range | Version | Page/Bank | Description        |
|--------------|----------|------------|-------------|
| 0x4000-0x7FFF | All    | 5/8        | Display file  |
| 0x8000-0xBFFF | v2/v3   | 2/1        | Attributes    |
| 0xC000-0xFFFF | All    | 0/2        | Stack/vars  |

### Hardware Mode Detection

```php
function is128KSnapshot($header, $version) {
    // Read hardware mode from byte 34
    $hwMode = ord($header[34]);
    
    if ($version == 1) {
        return false;  // Always 48K
    }
    
    if ($version == 2) {
        return ($hwMode >= 3);  // HWMode >= 3 = 128K
    }
    
    if ($version == 3) {
        return ($hwMode >= 4);  // HWMode >= 4 = 128K
    }
    
    return false;
}
```

### Common Pitfalls

1. **Mixing up page and bank numbers**: Page numbers in Z80 files don't directly equal bank numbers used internally by C code
2. **Incorrect offset calculation**: v2/v3 use page-based offsets, v1.45 uses linear offsets
3. **Forggetting end marker**: v1.45 requires `00 ED ED 00` after compressed data
4. **Missing block header**: v2/v3 blocks must start with 3-byte header (length + page)
5. **Wrong byte order**: All multi-byte values in Z80 files are little-endian
6. **128K vs 48K page mapping**: Different address ranges map to different page numbers

## Test Commands

### Version 2.01 Test
```bash
# LASTDUEL.Z80 is version 2.01
php pokesgd.php?file=ROMSMINE/L/LASTDUEL.Z80&POKES=37605,0:37672,0:40063,0
```

### Version 3.00 Test
```bash
# LASTVAMP.Z80 is version 3.00
php pokesgd.php?file=ROMSMINE/L/LASTVAMP.Z80&POKES=34509,0:34984,58
```

### Version 1.45 Test (Backward Compatibility)
```bash
# FIRELORD.Z80 is version 1.45
php pokesgd.php?file=ROMSMINE/F/FIRELORD.Z80&POKES=34509,0
```

## Migration Status

| C Function           | Lines  | PHP Function         | Status     |
|---------------------|---------|---------------------|------------|
| AutoBank()          | 1146-1188 | autoBank()          | ⚠️ Need update |
| DecrunchZ80Block()  | 1030-1075 | decompressZ80Block() | ✅ Complete |
| CrunchZ80Block()    | 1077-1138 | compressZ80Block()    | ⚠️ Add v3 case |
| InsertGamePokes() - v1 | 1591-1606 | applyPokesToMemory() | ⚠️ Rewrite |
| InsertGamePokes() - v2/v3 | 1620-1675 | applyPokesToMemory() | ⚠️ Rewrite |
| Block header reading  | 1636-1645 | readZ80File()       | ⚠️ Add parsing |
| Block writing       | 1656-1666 | writeZ80File()      | ⚠️ Add writing |

## Next Steps

1. Implement the critical changes marked with ⚠️ above
2. Test with LASTDUEL.Z80 (v2.01)
3. Test with LASTVAMP.Z80 (v3.00)
4. Verify backward compatibility with v1.45 files
5. Create comprehensive test suite
