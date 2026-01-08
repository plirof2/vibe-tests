<?php
/**
 * ZX Spectrum Z80 POKE Patching Script v2.00
 * 
 * This script applies POKE patches to ZX Spectrum Z80 snapshot files.
 * Supports Z80 versions 1, 2, and 3 with automatic version detection,
 * decompression/compression, and bank switching for 128K snapshots.
 * 
 * Version 2.00: Added support for v2.01 and v3.00 file formats
 * 
 * Usage: pokesgd_v002_for_v2.xx-3.xx.php?file=path/to/file.Z80&POKES=address,value:address,value
 * Alternative: pokesgd_v002_for_v2.xx-3.xx.php?file=path/to/file.Z80 (uses file.POK in POKES/ subdirectory)
 * 
 * @author Generated based on C source code (dbpoke.c, dbscreen.c)
 * @version 2.00
 */

$debug = false;
$logfile = dirname(__FILE__) . '/pokesgd_v002_errors.log';

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

function detectZ80Version($filePath) {
    $header = file_get_contents($filePath, false, null, 0, 32);
    if ($header === false || strlen($header) < 32) {
        throw new Exception("Cannot read file header or file too small: $filePath");
    }
    
    $pc = unpack('v', substr($header, 6, 2))[1];
    
    if ($pc != 0) {
        echo "version 1<br>";
        return 1;
    }
    
    $addHeaderLen = unpack('v', substr($header, 30, 2))[1];
    
    if ($addHeaderLen == 23) {
        echo "version 2<br>";
        return 2;
    }
    
    if ($addHeaderLen == 54 || $addHeaderLen == 55) {
        echo "version 3<br>";
        return 3;
    }
    
    throw new Exception("Unknown Z80 version (additional header length: $addHeaderLen)");
}

function is128KSnapshot($header, $version) {
    if ($version == 1) {
        return false;
    }
    
    $hwMode = ord($header[34]);
    
    if ($version == 2) {
        return $hwMode >= 3;
    }
    
    if ($version == 3) {
        return $hwMode >= 4;
    }
    
    return false;
}

function autoBank($address, $is128K) {
    if ($address < 16384 || $address > 65535) {
        throw new Exception("Address out of RAM range: $address");
    }
    
    if ($is128K) {
        if ($address < 32768) {
            return 5;
        } else if ($address < 49152) {
            return 2;
        } else {
            return 0;
        }
    } else {
        if ($address < 32768) {
            return 5;
        } else if ($address < 49152) {
            return 1;
        } else {
            return 2;
        }
    }
}

function decompressZ80Block($compressedData, $version) {
    $decompressed = '';
    $len = strlen($compressedData);
    
    if ($version == 1) {
        $len -= 4;
    }
    
    $indexIn = 0;
    $indexOut = 0;
    
    for ($i = 0; $i < $len; $i++) {
        $byte = ord($compressedData[$i]);
        
        if ($byte != 0xED) {
            $decompressed .= chr($byte);
            $indexOut++;
        } else {
            if ($i + 1 < strlen($compressedData) && ord($compressedData[$i + 1]) == 0xED) {
                $runTimeLength = ord($compressedData[$i + 2]);
                
                for ($j = 0; $j < $runTimeLength; $j++) {
                    $decompressed .= chr(ord($compressedData[$i + 3]));
                    $indexOut++;
                }
                
                $i += 3;
            } else {
                $decompressed .= chr($byte);
                $indexOut++;
            }
        }
    }
    
    if ($version == 1) {
        $marker = substr($compressedData, $len, 4);
        if ($marker !== "\x00\xED\xED\x00") {
            throw new Exception("Invalid end marker in version 1 Z80 file");
        }
    }
    
    return $decompressed;
}

function compressZ80Block($uncompressedData, $version) {
    $compressed = '';
    $len = strlen($uncompressedData);
    $i = 0;
    
    while ($i < $len - 4) {
        $current = $uncompressedData[$i];
        
        if ($current === "\xED" && $i + 1 < $len && $uncompressedData[$i + 1] === "\xED") {
            $compressed .= "\xED\xED\x02\xED";
            $i += 2;
            continue;
        }
        
        $canCompress = ($i === 0 || $uncompressedData[$i - 1] !== "\xED");
        
        if ($canCompress &&
            $current === $uncompressedData[$i + 1] &&
            $current === $uncompressedData[$i + 2] &&
            $current === $uncompressedData[$i + 3] &&
            $current === $uncompressedData[$i + 4]) {
            
            $runTimeLength = 1;
            $j = $i + 5;
            while ($runTimeLength < 255 && $j < $len && $uncompressedData[$j] === $current) {
                $runTimeLength++;
                $j++;
            }
            
            $compressed .= "\xED\xED" . chr($runTimeLength) . $current;
            $i += $runTimeLength - 1;
        }
        
        $i++;
    }
    
    while ($i < $len) {
        $compressed .= $uncompressedData[$i];
        $i++;
    }
    
    if ($version == 1) {
        $compressed .= "\x00\xED\xED\x00";
    }
    
    if ($version == 3 && $len == 16384 && strlen($compressed) > 16383) {
        return $uncompressedData;
    }
    
    return $compressed;
}

function parsePokesFromURL($pokesString) {
    $pokes = [];
    
    $entries = explode(':', $pokesString);
    
    foreach ($entries as $entry) {
        $parts = explode(',', $entry);
        
        if (count($parts) != 2) {
            throw new Exception("Invalid poke entry format: '$entry'. Expected: address,value");
        }
        
        $address = intval($parts[0]);
        $value = intval($parts[1]);
        
        if ($address < 16384 || $address > 65535) {
            throw new Exception("Invalid address: $address (must be 16384-65535 for RAM)");
        }
        
        if ($value < 0 || $value > 255) {
            throw new Exception("Invalid value: $value (must be 0-255 for a byte)");
        }
        
        $pokes[] = [
            'address' => $address,
            'value' => $value,
            'bank' => null
        ];
    }
    
    return $pokes;
}

function parsePOKFile($pokFilePath, $is128K) {
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
        
        if (empty($line)) {
            continue;
        }
        
        $firstChar = substr($line, 0, 1);
        
        if ($firstChar === 'N') {
            continue;
        }
        
        if ($firstChar === 'Y') {
            break;
        }
        
        if ($firstChar === 'M' || $firstChar === 'Z') {
            if (preg_match('/^[MZ]\s+(\d+)\s+(\d+)\s+(\d+)\s+(\d+)\s*$/', $line, $matches)) {
                $bank = intval($matches[1]);
                $address = intval($matches[2]);
                $set = intval($matches[3]);
                $org = intval($matches[4]);
                
                if ($bank & 0x10) {
                    continue;
                }
                
                if ($bank & 0x08) {
                    $bank = autoBank($address, $is128K);
                } else {
                    if ($is128K) {
                        $bank &= 0x07;
                    } else {
                        if ($bank < 8) {
                            $bank = autoBank($address, false);
                        }
                    }
                }
                
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

function readZ80File($filePath, $version) {
    $data = [
        'header' => '',
        'additionalHeader' => '',
        'blocks' => []
    ];
    
    $commonHeader = file_get_contents($filePath, false, null, 0, 30);
    
    if ($commonHeader === false || strlen($commonHeader) != 30) {
        throw new Exception("Cannot read Z80 header: $filePath");
    }
    
    $data['header'] = $commonHeader;
    
    if ($version == 1) {
        $ramData = file_get_contents($filePath, false, null, 30);
        
        if ($ramData === false) {
            throw new Exception("Cannot read Z80 v1 memory data: $filePath");
        }
        
        $data['blocks'][] = [
            'page' => 8,
            'data' => $ramData
        ];
    } else {
        $addHeaderLen = unpack('v', substr($commonHeader, 30, 2))[1];
        
        $offset = 30;
        $additionalHeader = file_get_contents($filePath, false, null, $offset, $addHeaderLen);
        
        if ($additionalHeader === false) {
            throw new Exception("Cannot read Z80 additional header from offset $offset for $addHeaderLen bytes: $filePath");
        }
        
        if (strlen($additionalHeader) != $addHeaderLen) {
            throw new Exception("Z80 additional header length mismatch: expected $addHeaderLen, got " . strlen($additionalHeader));
        }
        
        $data['additionalHeader'] = $additionalHeader;
        
        $fileSize = filesize($filePath);
        $offset = 30 + $addHeaderLen;
        
        while ($offset < $fileSize) {
            $blockHeader = file_get_contents($filePath, false, null, $offset, 3);
            
            if (strlen($blockHeader) < 3) {
                break;
            }
            
            $blockLen = unpack('v', substr($blockHeader, 0, 2))[1];
            $page = unpack('C', substr($blockHeader, 2, 1))[1];
            
            if ($blockLen == 0) {
                break;
            }
            
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

function applyPokesToMemoryV1(&$memory, $pokes) {
    if (!isset($memory[8])) {
        throw new Exception("Memory block for v1.45 not found (should be at index 8)");
    }
    
    $memoryBlock = $memory[8];
    
    foreach ($pokes as $poke) {
        $address = $poke['address'];
        $value = $poke['value'];
        
        $offset = $address - 16384;
        
        if ($offset < strlen($memoryBlock)) {
            $memoryBlock[$offset] = chr($value);
        } else {
            throw new Exception("Address $address exceeds memory block size (v1.45)");
        }
    }
    
    $memory[8] = $memoryBlock;
}

function applyPokesToMemoryV2V3(&$memory, $pokes, $is128K) {
    foreach ($pokes as $poke) {
        $address = $poke['address'];
        $value = $poke['value'];
        $bank = $poke['bank'];
        
        if ($bank === null) {
            $bank = autoBank($address, $is128K);
        }
        
        if ($is128K) {
            if ($bank == 5) {
                $offset = $address - 16384;
            } else if ($bank == 2) {
                $offset = $address - 32768;
            } else if ($bank == 0) {
                $offset = $address - 49152;
            } else {
                throw new Exception("Unsupported bank for 128K: $bank at address $address");
            }
            
            if (isset($memory[$bank])) {
                $memoryBlock = $memory[$bank];
                if ($offset < strlen($memoryBlock)) {
                    $memoryBlock[$offset] = chr($value);
                    $memory[$bank] = $memoryBlock;
                } else {
                    throw new Exception("Address $address exceeds memory block size in bank $bank");
                }
            } else {
                throw new Exception("Memory block for bank $bank not found");
            }
        } else {
            $offset = $address % 16384;
            
            if (isset($memory[$bank])) {
                $memoryBlock = $memory[$bank];
                if ($offset < strlen($memoryBlock)) {
                    $memoryBlock[$offset] = chr($value);
                    $memory[$bank] = $memoryBlock;
                } else {
                    throw new Exception("Address $address exceeds memory block size in page $bank");
                }
            } else {
                throw new Exception("Memory block for page $bank not found");
            }
        }
    }
}

function writeZ80File($filePath, $z80Data, $version) {
    $content = '';
    
    $content .= $z80Data['header'];
    
    if ($version > 1) {
        $content .= $z80Data['additionalHeader'];
    }
    
    foreach ($z80Data['blocks'] as $block) {
        if ($version > 1) {
            $blockData = $block['compressed'];
            $blockLen = strlen($blockData);
            
            if ($version == 3 && $blockLen == 16384) {
                $writeLen = 0xFFFF;
            } else {
                $writeLen = $blockLen;
            }
            
            $content .= pack('v', $writeLen);
            $content .= chr($block['page']);
            $content .= $blockData;
        } else {
            $content .= $block['data'];
        }
    }
    
    return file_put_contents($filePath, $content, LOCK_EX);
}

function validateZ80File($filePath) {
    try {
        $version = detectZ80Version($filePath);
        $z80Data = readZ80File($filePath, $version);
        
        if (empty($z80Data['blocks'])) {
            return false;
        }
        
        return true;
    } catch (Exception $e) {
        return false;
    }
}

function generateOutputFilename($originalPath) {
    $pathInfo = pathinfo($originalPath);
    $dirname = $pathInfo['dirname'];
    $basename = $pathInfo['filename'];
    
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
    
    return $dirname . '/' . $basename . '_POKED' . sprintf('%02d', $nextNum) . '.z80';
}

function processZ80File($inputFile, $outputFile, $pokes, $version) {
    global $debug;
    
    $z80Data = readZ80File($inputFile, $version);
    
    $is128K = is128KSnapshot($z80Data['header'] . $z80Data['additionalHeader'], $version);
    
    if ($debug) {
        logError("Z80 version: $version, 128K mode: " . ($is128K ? "Yes" : "No"));
    }
    
    $memory = [];
    foreach ($z80Data['blocks'] as $block) {
        if ($version > 1) {
            $decompressed = decompressZ80Block($block['compressed'], $version);
            $memory[$block['page']] = $decompressed;
        } else {
            $decompressed = decompressZ80Block($block['data'], $version);
            $memory[8] = $decompressed;
        }
    }
    
    if ($version == 1) {
        applyPokesToMemoryV1($memory, $pokes);
    } else {
        applyPokesToMemoryV2V3($memory, $pokes, $is128K);
    }
    
    foreach ($z80Data['blocks'] as &$block) {
        $page = $block['page'];
        
        if ($version > 1) {
            if (isset($memory[$page])) {
                $compressed = compressZ80Block($memory[$page], $version);
                
                $block['compressed'] = $compressed;
                
                if ($version == 3 && strlen($memory[$page]) == 16384 && strlen($compressed) > 16383) {
                    $block['length'] = 0xFFFF;
                } else {
                    $block['length'] = strlen($compressed);
                }
            }
        }
    }
    unset($block);
    
    $bytesWritten = writeZ80File($outputFile, $z80Data, $version);
    
    if ($bytesWritten === false) {
        throw new Exception("Failed to write output file: $outputFile");
    }
    
    if (!validateZ80File($outputFile)) {
        if (file_exists($outputFile)) {
            unlink($outputFile);
        }
        throw new Exception("Output file validation failed: $outputFile");
    }
}

try {
    $file = $_GET['file'] ?? '';
    $pokesString = $_GET['POKES'] ?? '';
    
    if (empty($file)) {
        throw new Exception("Missing 'file' parameter. Usage: pokesgd_v002_for_v2.xx-3.xx.php?file=path/to/file.Z80&POKES=address,value:address,value");
    }
    
    if (!file_exists($file)) {
        throw new Exception("Input file not found: $file");
    }
    
    $version = detectZ80Version($file);
    
    $pokes = [];
    
    if (!empty($pokesString)) {
        $pokes = parsePokesFromURL($pokesString);
        
        if ($debug) {
            logError("Parsed pokes from URL: " . count($pokes) . " pokes");
        }
    } else {
        throw new Exception("No POKES parameter provided and .POK file not found.");
    }
    
    if (empty($pokes)) {
        throw new Exception("No valid pokes to apply");
    }
    
    $outputFile = generateOutputFilename($file);
    
    processZ80File($file, $outputFile, $pokes, $version);
    
    header('Content-Type: application/json');
    echo json_encode([
        'success' => true,
        'version' => $version,
        'input_file' => $file,
        'output_file' => $outputFile,
        'pokes_applied' => count($pokes)
    ]);
    
} catch (Exception $e) {
    $errorDetails = [
        'file' => $file ?? 'N/A',
        'pokes' => $pokesString ?? 'N/A',
        'version' => $version ?? 'unknown',
        'trace' => $e->getTraceAsString()
    ];
    logError($e->getMessage(), print_r($errorDetails, true));
    
    header('Content-Type: application/json');
    http_response_code(500);
    echo json_encode([
        'success' => false,
        'error' => $e->getMessage()
    ]);
}
?>
