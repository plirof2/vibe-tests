<?php
/**
 * ZX Spectrum Z80 POKE Patching Script
 * 
 * This script applies POKE patches to ZX Spectrum Z80 snapshot files.
 * Supports Z80 versions 1, 2, and 3 with automatic version detection,
 * decompression/compression, and bank switching for 128K snapshots.
 * 
 * Usage: pokesgd.php?file=path/to/file.Z80&POKES=address,value:address,value
 * Alternative: pokesgd.php?file=path/to/file.Z80 (uses file.POK in POKES/ subdirectory)
 * 
 * @author Generated based on C source code (dbpoke.c, dbscreen.c)
 * @version 1.0
 */

/**
 * Configuration
 * Enable/disable debug logging to file
 */
$debug = false;

/**
 * Path to error log file (relative to this script)
 */
$logfile = dirname(__FILE__) . '/pokesgd_errors.log';


/**
 * Log an error message to the log file
 * 
 * @param string $message The main error message
 * @param string $details Additional details (optional)
 * @return void
 */
function logError($message, $details = '')
{
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


/**
 * Detect the version of a Z80 snapshot file
 * 
 * Version detection based on the algorithm in z80_version_check.py
 * and dbpoke.c
 * 
 * @param string $filePath Path to the Z80 file
 * @return int Z80 version (1, 2, or 3)
 * @throws Exception If file cannot be read or version is unknown
 */
function detectZ80Version($filePath)
{
    // Read first 32 bytes (enough for PC and additional header length)
    $header = file_get_contents($filePath, false, null, 0, 32);
    
    if ($header === false || strlen($header) < 32) {
        throw new Exception("Cannot read file header or file too small: $filePath");
    }
    
    // Read PC (Program Counter) at bytes 6-7 (little-endian)
    $pc = unpack('v', substr($header, 6, 2))[1];
    
    // If PC is non-zero, it's version 1 (old format)
    if ($pc != 0) { echo "version 1<br>";
        return 1;
    }
    
    // For version 2 and 3, PC is 0 and additional header length indicates version
    // Read additional header length at bytes 30-31 (little-endian)
    $addHeaderLen = unpack('v', substr($header, 30, 2))[1];
    
    // Version 2.01 has additional header length of 23 bytes
    if ($addHeaderLen == 23) { echo "version 2<br>";
        return 2;
    }
    
    // Version 3.00 has additional header length of 54 or 55 bytes
    if ($addHeaderLen == 54 || $addHeaderLen == 55) { echo "version 3<br>";
        return 3;
    }
    
    // Unknown version
    throw new Exception("Unknown Z80 version (additional header length: $addHeaderLen)");
}


/**
 * Determine if a Z80 snapshot is a 128K file
 * 
 * Based on the AutoBank function in dbpoke.c (lines 1146-1188)
 * 
 * @param string $header Combined header (30 bytes + additional header)
 * @param int $version Z80 version
 * @return bool True if 128K snapshot, false if 48K
 */
function is128KSnapshot($header, $version)
{
    // Version 1 is always 48K
    if ($version == 1) {
        return false;
    }
    
    // Read hardware mode from additional header (byte 34 = offset 4 in additional header)
    $hwMode = ord($header[34]);
    
    // Version 2: HWMode >= 3 indicates 128K
    if ($version == 2) {
        return $hwMode >= 3;
    }
    
    // Version 3: HWMode >= 4 indicates 128K
    if ($version == 3) {
        return $hwMode >= 4;
    }
    
    return false;
}


/**
 * Automatically determine the correct bank for a given address
 * 
 * Based on the AutoBank function in dbpoke.c (lines 1154-1188)
 * 
 * Memory mapping for ZX Spectrum:
 * - 0x0000-0x3FFF: ROM (16KB)
 * - 0x4000-0x7FFF: RAM page 5 (16KB) - Display file starts here
 * - 0x8000-0xBFFF: RAM page 2 or page 1 (16KB) depending on 48K/128K
 * - 0xC000-0xFFFF: RAM page 0 or page 2 (16KB) depending on 48K/128K
 * 
 * @param int $address Memory address (16384-65535 for RAM)
 * @param bool $is128K True for 128K snapshot, false for 48K
 * @return int Bank number (0-7 for 128K, page mapping for 48K)
 */
function autoBank($address, $is128K)
{
    // Ensure address is in RAM range
    if ($address < 16384 || $address > 65535) {
        throw new Exception("Address out of RAM range: $address");
    }
    
    if ($is128K) {
        // 128K mode bank mapping
        // 0x4000-0x7FFF (16384-32767) -> Bank 5
        if ($address < 32768) {
            return 5;
        }
        // 0x8000-0xBFFF (32768-49151) -> Bank 2
        else if ($address < 49152) {
            return 2;
        }
        // 0xC000-0xFFFF (49152-65535) -> Bank 0
        else {
            return 0;
        }
    } else {
        // 48K mode uses different page mapping (banks 4, 5, 8)
        // According to Z80 format spec:
        // Page 4: 0x8000-0xBFFF
        // Page 5: 0xC000-0xFFFF
        // Page 8: 0x4000-0x7FFF
        
        // 0x4000-0x7FFF (16384-32767) -> Page 8 (bank 8)
        if ($address < 32768) {
            return 8;
        }
        // 0x8000-0xBFFF (32768-49151) -> Page 1 (bank 1 for internal)
        else if ($address < 49152) {
            return 1;
        }
        // 0xC000-0xFFFF (49152-65535) -> Page 2 (bank 2 for internal)
        else {
            return 2;
        }
    }
}


/**
 * Decompress a Z80 memory block using the Z80 compression algorithm
 * 
 * Based on DecrunchZ80Block function in dbscreen.c (lines 1030-1075)
 * 
 * Compression format:
 * - Normal bytes are copied as-is
 * - Sequences are encoded as: ED ED length byte
 * - length = number of repetitions (byte value follows)
 * - Special case: ED ED 02 ED encodes two ED bytes
 * - Version 1 ends with marker: 00 ED ED 00
 * - Version 2/3 has no end marker
 * 
 * @param string $compressedData Compressed block data
 * @param int $version Z80 version (affects end marker handling)
 * @return string Decompressed 16KB block
 * @throws Exception If decompression fails
 */
function decompressZ80Block($compressedData, $version)
{
    $decompressed = '';
    $len = strlen($compressedData);
    
    // Version 1 has a 4-byte end marker (00 ED ED 00) that should not be decompressed
    if ($version == 1) {
        $len -= 4;
    }
    
    // Process each byte
    for ($i = 0; $i < $len; $i++) {
        $byte = ord($compressedData[$i]);
        
        // Check for ED byte (potential crunch marker)
        if ($byte != 0xED) {
            // Not a crunch marker, just copy
            $decompressed .= chr($byte);
        } else {
            // Check if next byte is also 0xED (crunch confirm)
            if ($i + 1 < strlen($compressedData) && ord($compressedData[$i + 1]) == 0xED) {
                // Found crunch marker: ED ED xx yy
                // xx = run length, yy = byte to repeat
                
                if ($i + 3 >= strlen($compressedData)) {
                    throw new Exception("Invalid crunched data at position $i: incomplete sequence");
                }
                
                $runLength = ord($compressedData[$i + 2]);
                $repeatedByte = ord($compressedData[$i + 3]);
                
                // Repeat the byte runLength times
                for ($j = 0; $j < $runLength; $j++) {
                    $decompressed .= chr($repeatedByte);
                }
                
                // Skip over the crunch marker (3 additional bytes after the first ED)
                $i += 3;
            } else {
                // Just a single ED byte, copy it
                $decompressed .= chr($byte);
            }
        }
    }
    
    // Version 1: verify and skip end marker
    if ($version == 1) {
        $marker = substr($compressedData, $len, 4);
        if ($marker !== "\x00\xED\xED\x00") {
            throw new Exception("Invalid end marker in version 1 Z80 file. Expected: 00 ED ED 00, Got: " . bin2hex($marker));
        }
    }
    
    return $decompressed;
}


/**
 * Compress a Z80 memory block using the Z80 compression algorithm
 * 
 * Based on CrunchZ80Block function in dbscreen.c (lines 1077-1138)
 * 
 * Compression rules:
 * - Sequences of 5+ equal bytes are compressed to: ED ED length byte
 * - Special case: ED ED is always compressed to: ED ED 02 ED
 * - Bytes immediately following a single ED are never compressed (unless it's ED ED)
 * - Version 1 adds end marker: 00 ED ED 00
 * - Version 3: if 16KB block compresses to > 16383 bytes, return uncompressed
 * 
 * @param string $uncompressedData Uncompressed block data
 * @param int $version Z80 version
 * @return string Compressed block data (or uncompressed if compression doesn't help)
 */
function compressZ80Block($uncompressedData, $version)
{
    $compressed = '';
    $len = strlen($uncompressedData);
    $i = 0;
    
    // Process bytes (except the last 4, to allow sequence detection)
    while ($i < $len - 4) {
        $current = $uncompressedData[$i];
        
        // Special case: ED ED sequence (always encoded)
        if ($current === "\xED" && $i + 1 < $len && $uncompressedData[$i + 1] === "\xED") {
            $compressed .= "\xED\xED\x02\xED";  // Encode 2 ED bytes
            $i += 2;
            continue;
        }
        
        // Check for 5 or more repeated bytes
        // Exception: if previous byte was ED, don't start compression here
        $canCompress = ($i === 0 || $uncompressedData[$i - 1] !== "\xED");
        
        if ($canCompress &&
            $current === $uncompressedData[$i + 1] &&
            $current === $uncompressedData[$i + 2] &&
            $current === $uncompressedData[$i + 3] &&
            $current === $uncompressedData[$i + 4]) {
            
            // Count how many times the byte repeats
            $runLength = 1;
            $j = $i + 5;
            
            while ($runLength < 255 && $j < $len && $uncompressedData[$j] === $current) {
                $runLength++;
                $j++;
            }
            
            // Write crunch marker: ED ED runLength byte
            $compressed .= "\xED\xED" . chr($runLength) . $current;
            
            // Skip over the repeated sequence
            $i += $runLength - 1;
        } else {
            // No compression, just copy the byte
            $compressed .= $current;
        }
        $i++;
    }
    
    // Copy remaining bytes
    while ($i < $len) {
        $compressed .= $uncompressedData[$i];
        $i++;
    }
    
    // Version 1: add end marker
    if ($version == 1) {
        $compressed .= "\x00\xED\xED\x00";
    }
    
    // Version 3 special case: if 16KB block and compressed > 16383, return uncompressed
    if ($version == 3 && $len == 16384 && strlen($compressed) > 16383) {
        return $uncompressedData;  // Compression doesn't help, return original
    }
    
    return $compressed;
}


/**
 * Parse POKEs from URL parameter string
 * 
 * Format: address,value:address,value
 * Example: 34509,0:34984,58 applies two pokes
 * 
 * @param string $pokesString Comma/colon separated pokes
 * @return array Array of pokes with 'address', 'value', and 'bank' keys
 * @throws Exception If format is invalid or values are out of range
 */
function parsePokesFromURL($pokesString)
{
    $pokes = [];
    
    // Split by colon to get individual poke entries
    $entries = explode(':', $pokesString);
    
    foreach ($entries as $entry) {
        // Split by comma to get address and value
        $parts = explode(',', $entry);
        
        if (count($parts) != 2) {
            throw new Exception("Invalid poke entry format: '$entry'. Expected: address,value");
        }
        
        $address = intval($parts[0]);
        $value = intval($parts[1]);
        
        // Validate address (must be in RAM range 16384-65535)
        if ($address < 16384 || $address > 65535) {
            throw new Exception("Invalid address: $address (must be 16384-65535 for RAM)");
        }
        
        // Validate value (must be 0-255 for a byte)
        if ($value < 0 || $value > 255) {
            throw new Exception("Invalid value: $value (must be 0-255 for a byte)");
        }
        
        $pokes[] = [
            'address' => $address,
            'value' => $value,
            'bank' => null  // Will be determined later based on snapshot type
        ];
    }
    
    return $pokes;
}


/**
 * Parse POKEs from a .POK file
 * 
 * Based on ReadGamePokes function in dbpoke.c (lines 270-464)
 * 
 * .POK file format:
 * - Lines starting with 'N': Poke name (ignored)
 * - Lines starting with 'M': Multi-mode poke (M bank address set org)
 * - Lines starting with 'Z': Single-mode poke (Z bank address set org)
 * - Line starting with 'Y': End of file
 * 
 * Bank field flags (bitmask):
 * - Bit 0x08: Ignore bank (use auto bank)
 * - Bit 0x10: Ask user for value (skip if set)
 * - Bits 0x00-0x07: Bank number (for 128K)
 * 
 * @param string $pokFilePath Path to the .POK file
 * @param bool $is128K True if snapshot is 128K
 * @return array Array of pokes with 'address', 'value', and 'bank' keys
 * @throws Exception If file cannot be read or format is invalid
 */
function parsePOKFile($pokFilePath, $is128K)
{
    $pokes = [];
    
    if (!file_exists($pokFilePath)) {
        throw new Exception("POK file not found: $pokFilePath");
    }
    
    $lines = file($pokFilePath, FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);
    
    if ($lines === false) {
        throw new Exception("Cannot read POK file: $pokFilePath");
    }
    
    foreach ($lines as $line) {
        $line = trim($line);
        
        // Skip empty lines
        if (empty($line)) {
            continue;
        }
        
        $firstChar = substr($line, 0, 1);
        
        // Skip name/comment lines
        if ($firstChar === 'N') {
            continue;
        }
        
        // Check for end of file marker
        if ($firstChar === 'Y') {
            break;
        }
        
        // Parse poke entry lines (M or Z)
        if ($firstChar === 'M' || $firstChar === 'Z') {
            // Format: M/Z bank(3) address(5) set(3) org(3)
            // Example: "M  8 34509   0  53"
            if (preg_match('/^[MZ]\s+(\d+)\s+(\d+)\s+(\d+)\s+(\d+)\s*$/', $line, $matches)) {
                $bank = intval($matches[1]);
                $address = intval($matches[2]);
                $set = intval($matches[3]);
                $org = intval($matches[4]);
                
                // Check bit flags in bank field
                
                // Bit 0x10: Ask user for value - skip this poke
                if ($bank & 0x10) {
                    continue;
                }
                
                // Bit 0x08: Ignore bank flag - use auto bank
                if ($bank & 0x08) {
                    $bank = autoBank($address, $is128K);
                } else {
                    if ($is128K) {
                        // 128K mode: keep only page number (lower 3 bits)
                        $bank &= 0x07;
                    } else {
                        // 48K mode: if bank < 8, convert to auto bank
                        if ($bank < 8) {
                            $bank = autoBank($address, false);
                        }
                    }
                }
                
                // Validate address is in RAM
                if ($address >= 16384 && $address <= 65535) {
                    $pokes[] = [
                        'address' => $address,
                        'value' => $set,
                        'bank' => $bank
                    ];
                }
            }
        }
    }
    
    return $pokes;
}


/**
 * Read a Z80 file and parse its structure
 * 
 * Reads the header(s) and memory blocks, handling different versions
 * 
 * @param string $filePath Path to the Z80 file
 * @param int $version Z80 version
 * @return array Parsed Z80 data structure
 * @throws Exception If file cannot be read or is invalid
 */
function readZ80File($filePath, $version)
{
    $data = [
        'header' => '',           // First 30 bytes (common header)
        'additionalHeader' => '', // Additional header for v2/v3
        'blocks' => []           // Memory blocks
    ];
    
    // Read first 30 bytes (common header for all versions)
    $commonHeader = file_get_contents($filePath, false, null, 0, 30);
    
    if ($commonHeader === false || strlen($commonHeader) != 30) {
        throw new Exception("Cannot read Z80 header: $filePath");
    }
    
    $data['header'] = $commonHeader;
    
    if ($version == 1) {
        // Version 1: All RAM data follows header (48KB = 49152 bytes)
        // May be compressed (check bit 5 of byte 12)
        $ramData = file_get_contents($filePath, false, null, 30);
        
        if ($ramData === false) {
            throw new Exception("Cannot read Z80 v1 memory data: $filePath");
        }
        
        $data['blocks'][] = [
            'page' => 8,  // All RAM in one block
            'data' => $ramData
        ];
    } else {
        // Version 2 or 3: Read additional header
        $addHeaderLen = unpack('v', substr($commonHeader, 30, 2))[1];
        
        $additionalHeader = file_get_contents($filePath, false, null, 30, $addHeaderLen);
        
        if ($additionalHeader === false || strlen($additionalHeader) != $addHeaderLen) {
            throw new Exception("Cannot read Z80 additional header: $filePath");
        }
        
        $data['additionalHeader'] = $additionalHeader;
        
        // Read memory blocks
        $offset = 30 + $addHeaderLen;
        $fileSize = filesize($filePath);
        
        // Read blocks until end of file
        while ($offset < $fileSize) {
            // Read block header: length (2 bytes, little-endian) + page (1 byte)
            $blockHeader = file_get_contents($filePath, false, null, $offset, 3);
            
            if (strlen($blockHeader) < 3) {
                break;  // End of file
            }
            
            $blockLen = unpack('v', substr($blockHeader, 0, 2))[1];
            $page = unpack('C', substr($blockHeader, 2, 1))[1];
            
            // Length of 0 indicates end marker (SLT extension or similar)
            if ($blockLen == 0) {
                break;
            }
            
            // Read compressed data for this block
            $offset += 3;
            $compressedData = file_get_contents($filePath, false, null, $offset, $blockLen);
            
            if ($compressedData === false || strlen($compressedData) != $blockLen) {
                throw new Exception("Cannot read Z80 memory block: $filePath");
            }
            
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


/**
 * Apply POKEs to memory blocks
 * 
 * Modifies bytes at specified addresses in the appropriate memory blocks
 * Handles both 48K and 128K memory mapping
 * 
 * @param array &$memory Reference to memory array (keyed by bank/page)
 * @param array $pokes Array of pokes to apply
 * @param bool $is128K True if snapshot is 128K
 * @throws Exception If address is out of range or bank not found
 */
function applyPokesToMemory(&$memory, $pokes, $is128K)
{
    foreach ($pokes as $poke) {
        $address = $poke['address'];
        $value = $poke['value'];
        $bank = $poke['bank'];
        
        // Determine bank if not set (for URL pokes)
        if ($bank === null) {
            $bank = autoBank($address, $is128K);
        }
        
        if ($is128K) {
            // 128K mode: map address to bank-specific memory block
            if ($bank == 5) {
                // 0x4000-0x7FFF (16384-32767)
                $offset = $address - 16384;
            } else if ($bank == 2) {
                // 0x8000-0xBFFF (32768-49151)
                $offset = $address - 32768;
            } else if ($bank == 0) {
                // 0xC000-0xFFFF (49152-65535)
                $offset = $address - 49152;
            } else {
                throw new Exception("Unsupported bank for 128K: $bank at address $address");
            }
            
            // Apply poke to the appropriate bank's memory
            if (isset($memory[$bank])) {
                $memoryBlock = $memory[$bank];
                if ($offset < strlen($memoryBlock)) {
                    // Modify the byte at the specified offset
                    $memoryBlock[$offset] = chr($value);
                    $memory[$bank] = $memoryBlock;
                } else {
                    throw new Exception("Address $address exceeds memory block size in bank $bank");
                }
            } else {
                throw new Exception("Memory block for bank $bank not found");
            }
        } else {
            // 48K mode: single 48KB block covering all RAM (16384-65535)
            // Address offset from start of RAM
            $offset = $address - 16384;
            
            if (isset($memory[8])) {
                $memoryBlock = $memory[8];
                if ($offset < strlen($memoryBlock)) {
                    // Modify the byte at the specified offset
                    $memoryBlock[$offset] = chr($value);
                    $memory[8] = $memoryBlock;
                } else {
                    throw new Exception("Address $address exceeds memory block size");
                }
            } else {
                throw new Exception("48K memory block not found");
            }
        }
    }
}


/**
 * Write a Z80 file from parsed data structure
 * 
 * @param string $filePath Output file path
 * @param array $z80Data Parsed Z80 data structure
 * @param int $version Z80 version
 * @return int Number of bytes written, or false on failure
 */
function writeZ80File($filePath, $z80Data, $version)
{
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
            $content .= pack('v', $blockLen);  // Little-endian length
            $content .= chr($block['page']);    // Page number
            $content .= $blockData;             // Compressed data
        } else {
            // Version 1: just write the block data (no block header)
            $content .= $block['data'];
        }
    }
    
    return file_put_contents($filePath, $content, LOCK_EX);
}


/**
 * Validate that a file is a valid Z80 snapshot
 * 
 * @param string $filePath Path to the file to validate
 * @return bool True if valid, false otherwise
 */
function validateZ80File($filePath)
{
    try {
        $version = detectZ80Version($filePath);
        $z80Data = readZ80File($filePath, $version);
        
        // Check that we have at least one memory block
        if (empty($z80Data['blocks'])) {
            return false;
        }
        
        return true;
    } catch (Exception $e) {
        return false;
    }
}


/**
 * Generate a unique output filename with incrementing number
 * 
 * Format: originalfilename_POKED##.z80
 * where ## is the next available number
 * 
 * @param string $originalPath Original file path
 * @return string Unique output file path
 */
function generateOutputFilename($originalPath)
{
    $pathInfo = pathinfo($originalPath);
    $dirname = $pathInfo['dirname'];
    $basename = $pathInfo['filename'];
    
    // Look for existing _POKED files to determine next number
    $pattern = $dirname . '/' . $basename . '_POKED*.z80';
    $files = glob($pattern);
    
    $maxNum = 0;
    foreach ($files as $file) {
        // Extract number from filename
        if (preg_match('/_POKED(\d+)\.z80$/i', basename($file), $matches)) {
            $num = intval($matches[1]);
            if ($num > $maxNum) {
                $maxNum = $num;
            }
        }
    }
    
    // Next number is max + 1
    $nextNum = $maxNum + 1;
    
    return $dirname . '/' . $basename . '_POKED' . sprintf('%02d', $nextNum) . '.z80';
}


/**
 * Main processing function to apply pokes to a Z80 file
 * 
 * @param string $inputFile Input Z80 file path
 * @param string $outputFile Output Z80 file path
 * @param array $pokes Array of pokes to apply
 * @param int $version Z80 version
 * @throws Exception If any step fails
 */
function processZ80File($inputFile, $outputFile, $pokes, $version)
{
    global $debug;
    
    // Read Z80 file
    $z80Data = readZ80File($inputFile, $version);
    
    // Determine if 128K snapshot
    $is128K = is128KSnapshot($z80Data['header'] . $z80Data['additionalHeader'], $version);
    
    if ($debug) {
        logError("Z80 version: $version, 128K mode: " . ($is128K ? "Yes" : "No"));
    }
    
    // Decompress memory blocks
    $memory = [];
    foreach ($z80Data['blocks'] as $block) {
        if ($version > 1) {
            // Version 2/3: blocks are keyed by page
            $decompressed = decompressZ80Block($block['compressed'], $version);
            $memory[$block['page']] = $decompressed;
        } else {
            // Version 1: single block for all RAM
            $decompressed = decompressZ80Block($block['data'], $version);
            $memory[8] = $decompressed;
        }
    }
    
    // Apply pokes to memory
    applyPokesToMemory($memory, $pokes, $is128K);
    
    // Recompress modified blocks
    foreach ($z80Data['blocks'] as &$block) {
        $page = $block['page'];
        
        // Determine which key to use for storing data
        $dataKey = ($version > 1) ? 'compressed' : 'data';
        
        if (isset($memory[$page])) {
            $compressed = compressZ80Block($memory[$page], $version);
            
            $block[$dataKey] = $compressed;
            
            if ($version > 1) {
                $block['length'] = strlen($compressed);
            }
        }
    }
    unset($block);  // Break reference
    
    // Write output file
    $bytesWritten = writeZ80File($outputFile, $z80Data, $version);
    
    if ($bytesWritten === false) {
        throw new Exception("Failed to write output file: $outputFile");
    }
    
    // Validate the output file
    if (!validateZ80File($outputFile)) {
        // Delete invalid file
        if (file_exists($outputFile)) {
            unlink($outputFile);
        }
        throw new Exception("Output file validation failed: $outputFile");
    }
}


// ============================================================================
// MAIN SCRIPT EXECUTION
// ============================================================================

try {
    // Get parameters from URL
    $file = $_GET['file'] ?? '';
    $pokesString = $_GET['POKES'] ?? '';
    
    // Validate file parameter
    if (empty($file)) {
        throw new Exception("Missing 'file' parameter. Usage: pokesgd.php?file=path/to/file.Z80&POKES=address,value:address,value");
    }
    
    // Validate input file exists
    if (!file_exists($file)) {
        throw new Exception("Input file not found: $file");
    }
    
    // Detect Z80 version
    $version = detectZ80Version($file);
    
    // Determine pokes source
    $pokes = [];
    
    if (!empty($pokesString)) {
        // Parse pokes from URL parameter
        $pokes = parsePokesFromURL($pokesString);
        
        if ($debug) {
            logError("Parsed pokes from URL: " . count($pokes) . " pokes");
        }
    } else {
        // Try to find .POK file in POKES subdirectory
        $pathInfo = pathinfo($file);
        $fileDir = $pathInfo['dirname'];
        $fileName = $pathInfo['filename'];
        $extension = strtolower($pathInfo['extension']);
        
        // Build path to POKES subdirectory
        $pokesDir = $fileDir . '/POKES';
        
        // Possible .POK file paths (case-insensitive)
        $pokFilePaths = [
            $pokesDir . '/' . $fileName . '.POK',
            $pokesDir . '/' . $fileName . '.pok',
            $pokesDir . '/' . basename($file, '.' . $extension) . '.POK'
        ];
        
        $pokFilePath = null;
        foreach ($pokFilePaths as $path) {
            if (file_exists($path)) {
                $pokFilePath = $path;
                break;
            }
        }
        
        if ($pokFilePath) {
            // Determine if 128K for auto bank resolution
            $header = file_get_contents($file, false, null, 0, 32);
            $is128K = is128KSnapshot($header, $version);
            
            // Parse .POK file
            $pokes = parsePOKFile($pokFilePath, $is128K);
            
            if ($debug) {
                logError("Parsed pokes from file: $pokFilePath - " . count($pokes) . " pokes");
            }
        } else {
            throw new Exception("No POKES parameter provided and .POK file not found. Checked: " . implode(', ', $pokFilePaths));
        }
    }
    
    // Validate we have pokes to apply
    if (empty($pokes)) {
        throw new Exception("No valid pokes to apply");
    }
    
    // Generate unique output filename
    $outputFile = generateOutputFilename($file);
    
    // Process the Z80 file
    processZ80File($file, $outputFile, $pokes, $version);
    
    // Return success response
    header('Content-Type: application/json');
    echo json_encode([
        'success' => true,
        'version' => $version,
        'input_file' => $file,
        'output_file' => $outputFile,
        'pokes_applied' => count($pokes)
    ]);
    
} catch (Exception $e) {
    // Log error
    $errorDetails = [
        'file' => $file ?? 'N/A',
        'pokes' => $pokesString ?? 'N/A',
        'version' => $version ?? 'unknown',
        'trace' => $e->getTraceAsString()
    ];
    logError($e->getMessage(), print_r($errorDetails, true));
    
    // Return error response
    header('Content-Type: application/json');
    http_response_code(500);
    echo json_encode([
        'success' => false,
        'error' => $e->getMessage()
    ]);
}
?>
