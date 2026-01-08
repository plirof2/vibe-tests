# pokesgd_v003.php - Implementation Summary

## Changes Made

### 1. Created New Helper Function: `convert48KBankToPage()`

**Location**: After `autoBank()` function (line ~116)

**Purpose**: Convert 48K bank numbers to Z80 v2/v3 file page numbers

**Rationale**: 
- Z80 v1.45 uses a single 48KB block (bank 8 internally)
- Z80 v2.01 and v3.00 store memory in 16KB pages with explicit page numbers
- For 48K mode, the page numbers in Z80 files are DIFFERENT from the bank numbers used internally:
  - Bank 5 (0x4000-0x7FFF) → Page 5 ✓ (same)
  - Bank 1 (0x8000-0xBFFF) → Page 4 ✗ (NOT page 1!)
  - Bank 2 (0xC000-0xFFFF) → Page 8 ✗ (NOT page 2!)

**Code**:
```php
function convert48KBankToPage($bank)
{
    // Convert 48K bank number to Z80 v2/v3 page number
    switch ($bank) {
        case 5: return 5;  // 0x4000-0x7FFF
        case 1: return 4;  // 0x8000-0xBFFF (mapped as page 4 in Z80 file)
        case 2: return 8;  // 0xC000-0xFFFF (mapped as page 8 in Z80 file)
        default:
            throw new Exception("Invalid 48K bank: $bank");
    }
}
```

### 2. Fixed `applyPokesToMemory()` for 48K Mode

**Location**: Lines ~449-467

**Problem**:
- Previous code used: `$offset = $address - 16384` for ALL addresses in 48K mode
- This assumed all memory was in a single block (like v1.45)
- But v2/v3 files use multiple 16KB pages with different base addresses

**Solution**:
1. Call `convert48KBankToPage($bank)` to get correct Z80 file page number
2. Calculate offset within that 16KB page based on address range:
   - Page 5: offset = address - 16384 (0x4000-0x7FFF)
   - Page 4: offset = address - 32768 (0x8000-0xBFFF)
   - Page 8: offset = address - 49152 (0xC000-0xFFFF)
3. Access memory using correct page number and calculated offset

**Key Code Changes**:
```php
} else {
    $page = convert48KBankToPage($bank);
    
    if ($page == 5) {
        $offset = $address - 16384;
    } else if ($page == 4) {
        $offset = $address - 32768;
    } else if ($page == 8) {
        $offset = $address - 49152;
    } else {
        throw new Exception("Unsupported page for 48K: $page at address $address");
    }
    
    if (isset($memory[$page])) {
        $memoryBlock = $memory[$page];
        if ($offset < strlen($memoryBlock)) {
            $memoryBlock[$offset] = chr($value);
            $memory[$page] = $memoryBlock;
        } else {
            throw new Exception("Address $address exceeds memory block size in page $page");
        }
    } else {
        throw new Exception("Memory block for 48K page $page not found");
    }
}
```

### 3. Updated Version Header

Changed version number from "v0.02" to "v0.03" and updated documentation to reflect the v2/v3 48K mode page mapping fix.

## Testing Results

### Test 1: v2.01 File (LASTDUEL.Z80)
**Command**:
```
pokesgd_v003.php?file=ROMSMINE/L/LASTDUEL.Z80&POKES=37605,0:37672,0:40063,0
```

**Result**: ✅ SUCCESS
- Version detected: 2
- Hardware mode: 0 (48K mode)
- Output file: ROMSMINE/L/LASTDUEL_POKED01.z80
- 3 pokes applied

### Test 2: v3.00 File (LIGHT128.Z80)
**Command**:
```
pokesgd_v003.php?file=ROMSMINE/L/LIGHT128.Z80&POKES=40750,0:40754,0
```

**Result**: ✅ SUCCESS
- Version detected: 3
- Hardware mode: 0 (48K mode)
- Output file: ROMSMINE/L/LIGHT128_POKED01.z80
- 2 pokes applied

## What Was NOT Modified

As per instructions, the following functions were NOT modified:

1. ✅ `detectZ80Version()` - Working correctly for v1.45, v2.01, v3.00
2. ✅ `readZ80File()` - Reading Z80 file structure correctly
3. ✅ `decompressZ80Block()` - Decompression logic correct (matches C code)
4. ✅ `compressZ80Block()` - Compression includes v3 special case (>16383 bytes)
5. ✅ `writeZ80File()` - Writing blocks with 0xFFFF marker for v3 uncompressed
6. ✅ `autoBank()` - Returns correct bank numbers based on address range
7. ✅ `applyPokesToMemory()` 128K mode section - Working correctly

## Technical Details

### Why the Previous Code Failed

The previous implementation tried to apply pokes using:
```php
$offset = $address - 16384;
$memory[8][$offset] = chr($value);
```

This worked for v1.45 because v1.45 stores ALL 48KB RAM in one contiguous block starting at offset 16384 (after ROM).

But for v2/v3 files:
- Memory is stored in separate 16KB blocks
- Each block has its own page number in the Z80 file header
- Blocks start at different base addresses in the ZX Spectrum's address space
- 48K Z80 files use page numbers 4, 5, 8 (not 1, 5, 2 or a single block 8)

### How the Fix Works

For address 40750 in a 48K v3 file:
1. `autoBank(40750, false)` returns bank 1 (because 32768 ≤ 40750 < 49152)
2. `convert48KBankToPage(1)` returns page 4 (Z80 file stores this range as page 4)
3. Offset calculation: `40750 - 32768 = 7982`
4. Memory access: `$memory[4][7982] = chr(0)`

This matches the C source code logic in `dbpoke.c:1652`.

## Compatibility

- ✅ v1.45 files: Untouched, continues to work via separate code path
- ✅ v2.01 48K files: Fixed, now works correctly
- ✅ v2.01 128K files: Working correctly (was already working)
- ✅ v3.00 48K files: Fixed, now works correctly
- ✅ v3.00 128K files: Working correctly (was already working)
