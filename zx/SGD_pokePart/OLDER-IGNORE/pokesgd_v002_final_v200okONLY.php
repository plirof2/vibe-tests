<?php
/**
 * ZX Spectrum Z80 POKE Patching Script v0.02
 * 
 * This script applies POKE patches to ZX Spectrum Z80 snapshot files.
 * Supports Z80 versions 1, 2, and 3 with automatic version detection,
 * decompression/compression, and bank switching for 128K snapshots.
 * 
 * Based on C source code: dbpoke.c, dbscreen.c
 * by Martijn van der Heide
 * 
 * Usage: pokesgd_v002.php?file=path/to/file.Z80&POKES=address,value:address,value
 * Alternative: pokesgd_v002.php?file=path/to/file.Z80 (uses file.POK in POKES/ subdirectory)

v1 FIRELORD PROBLEM
 v2.xx Seems OK : ?file=ROMSMINE/L/LITTLEPU.Z80&POKES=62342,201
 v2.xx Seems OK : ?file=ROMSMINE/L/LASTDUEL.Z80&POKES=37605,0:37672,0:40063,0
 v3.xx ok : ?file=ROMSMINE/L/LITTLEPU.Z80&POKES=62342,201
 v3.00 PROB LASTVAMP ?file=ROMSMINE/L/LASTVAMP.Z80&POKES=51813,0:51814,0 ???

 * 
 * @version 0.02 - Added full v2.xx and v3.xx support
 */

$debug = false;
$logfile = dirname(__FILE__) . '/pokesgd_errors.log';

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

function detectZ80Version($filePath)
{
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

function is128KSnapshot($header, $version)
{
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

function autoBank($address, $is128K)
{
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

function decompressZ80Block($compressedData, $version)
{
    $decompressed = '';
    $len = strlen($compressedData);
    
    if ($version == 1) {
        $len -= 4;
    }
    
    for ($i = 0; $i < $len; $i++) {
        $byte = ord($compressedData[$i]);
        
        if ($byte != 0xED) {
            $decompressed .= chr($byte);
        } else {
            if ($i + 1 < strlen($compressedData) && ord($compressedData[$i + 1]) == 0xED) {
                if ($i + 3 >= strlen($compressedData)) {
                    throw new Exception("Invalid crunched data at position $i: incomplete sequence");
                }
                
                $runLength = ord($compressedData[$i + 2]);
                $repeatedByte = ord($compressedData[$i + 3]);
                
                for ($j = 0; $j < $runLength; $j++) {
                    $decompressed .= chr($repeatedByte);
                }
                
                $i += 3;
            } else {
                $decompressed .= chr($byte);
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

function compressZ80Block($uncompressedData, $version)
{
    $compressed = '';
    $len = strlen($uncompressedData);
    $i = 0;
    
    while ($i < $len - 4) {
        $current = $uncompressedData[$i];
        $prevByte = ($i > 0) ? $uncompressedData[$i - 1] : "\x00";
        
        if ($current === "\xED" && $i + 1 < $len && $uncompressedData[$i + 1] === "\xED") {
            $compressed .= "\xED\xED\x02\xED";
            $i += 2;
            continue;
        }
        
        $canCompress = ($prevByte !== "\xED");
        
        if ($canCompress &&
            $current === $uncompressedData[$i + 1] &&
            $current === $uncompressedData[$i + 2] &&
            $current === $uncompressedData[$i + 3] &&
            $current === $uncompressedData[$i + 4]) {
            
            $runLength = 5;
            $j = $i + 5;
            
            while ($runLength < 255 && $j < $len && $uncompressedData[$j] === $current) {
                $runLength++;
                $j++;
            }
            
            $compressed .= "\xED\xED" . chr($runLength) . $current;
            
            $i += $runLength - 1;
        } else {
            $compressed .= $current;
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

function parsePokesFromURL($pokesString)
{
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

function readZ80File($filePath, $version)
{
    $data = [
        'header' => '',
        'additionalHeader' => '',
        'blocks' => []
    ];
    
    $headerData = file_get_contents($filePath, false, null, 0, 32);
    
    if ($headerData === false || strlen($headerData) != 32) {
        throw new Exception("Cannot read Z80 header: $filePath");
    }
    
    $data['header'] = $headerData;
    
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
        $addHeaderLen = unpack('v', substr($data['header'], 30, 2))[1];
        
        $additionalHeader = file_get_contents($filePath, false, null, 32, $addHeaderLen);
        
        if ($additionalHeader === false || strlen($additionalHeader) != $addHeaderLen) {
            throw new Exception("Cannot read Z80 additional header: $filePath");
        }
        
        $data['additionalHeader'] = $additionalHeader;
        
        $offset = 32 + $addHeaderLen;
        $fileSize = filesize($filePath);
        
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

function applyPokesToMemory(&$memory, $pokes, $is128K, $bankToPageMap = [])
{
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
            
            $page = $bank + 3;
            if (isset($memory[$page])) {
                $memoryBlock = $memory[$page];
                if ($offset < strlen($memoryBlock)) {
                    $memoryBlock[$offset] = chr($value);
                    $memory[$page] = $memoryBlock;
                } else {
                    throw new Exception("Address $address exceeds memory block size in bank $bank (page $page)");
                }
            } else {
                throw new Exception("Memory block for bank $bank (page $page) not found");
            }
        } else {
            $offset = $address - 16384;
            
            if (isset($memory[8])) {
                $memoryBlock = $memory[8];
                if ($offset < strlen($memoryBlock)) {
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

function writeZ80File($filePath, $z80Data, $version)
{
    $content = '';
    
    $content .= $z80Data['header'];
    
    if ($version > 1) {
        $content .= $z80Data['additionalHeader'];
    }
    
    foreach ($z80Data['blocks'] as $block) {
        if ($version > 1) {
            $blockData = $block['compressed'];
            $blockLen = strlen($blockData);
            $content .= pack('v', $blockLen);
            $content .= chr($block['page']);
            $content .= $blockData;
        } else {
            $content .= $block['data'];
        }
    }
    
    return file_put_contents($filePath, $content, LOCK_EX);
}

function validateZ80File($filePath)
{
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

function generateOutputFilename($originalPath)
{
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

function processZ80File($inputFile, $outputFile, $pokes, $version)
{
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
    
    $bankToPageMap = [];
    foreach ($z80Data['blocks'] as $block) {
        if ($version > 1) {
            $page = $block['page'];
            $bank = $page - 3;
            if ($bank >= 0) {
                $bankToPageMap[$bank] = $page;
            }
        }
    }
    
    applyPokesToMemory($memory, $pokes, $is128K, $bankToPageMap);
    
    foreach ($z80Data['blocks'] as &$block) {
        $page = $block['page'];
        $bank = $page - 3;
        
        $dataKey = ($version > 1) ? 'compressed' : 'data';
        
        if (isset($memory[$page])) {
            $compressed = compressZ80Block($memory[$page], $version);
            
            $block[$dataKey] = $compressed;
            
            if ($version > 1) {
                $block['length'] = strlen($compressed);
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
        throw new Exception("Missing 'file' parameter. Usage: pokesgd.php?file=path/to/file.Z80&POKES=address,value:address,value");
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
        $pathInfo = pathinfo($file);
        $fileDir = $pathInfo['dirname'];
        $fileName = $pathInfo['filename'];
        $extension = strtolower($pathInfo['extension']);
        
        $pokesDir = $fileDir . '/POKES';
        
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
            $header = file_get_contents($file, false, null, 0, 32);
            $is128K = is128KSnapshot($header, $version);
            
            $pokes = parsePOKFile($pokFilePath, $is128K);
            
            if ($debug) {
                logError("Parsed pokes from file: $pokFilePath - " . count($pokes) . " pokes");
            }
        } else {
            throw new Exception("No POKES parameter provided and .POK file not found. Checked: " . implode(', ', $pokFilePaths));
        }
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
