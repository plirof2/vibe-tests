<?php
/**
 * ZX Spectrum Z80 POKE Patching Script v1.xx Only
 * 
 * This script applies POKE patches to ZX Spectrum Z80 snapshot files.
 * Supports Z80 version 1 only.
 * 
 * Usage: poke145.php?file=path/to/file.Z80&POKES=address,value:address,value
 * 
 * @author Generated based on C source code (dbpoke.c, dbscreen.c)
 * @version 1.00
 */

$debug = false;
$logfile = dirname(__FILE__) . '/poke145_errors.log';

function logError_v145($message, $details = '') {
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

function autoBank_v145($address) {
    if ($address < 16384 || $address > 65535) {
        throw new Exception("Address out of RAM range: $address");
    }
    
    if ($address < 32768) {
        return 5;
    } else if ($address < 49152) {
        return 1;
    } else {
        return 2;
    }
}

function decompressZ80Block_v145($compressedData) {
    $decompressed = '';
    $len = strlen($compressedData) - 4;
    
    for ($i = 0; $i < $len; $i++) {
        $byte = ord($compressedData[$i]);
        
        if ($byte != 0xED) {
            $decompressed .= chr($byte);
        } else {
            if ($i + 1 < strlen($compressedData) && ord($compressedData[$i + 1]) == 0xED) {
                $runTimeLength = ord($compressedData[$i + 2]);
                
                for ($j = 0; $j < $runTimeLength; $j++) {
                    $decompressed .= chr(ord($compressedData[$i + 3]));
                }
                
                $i += 3;
            } else {
                $decompressed .= chr($byte);
            }
        }
    }
    
    $marker = substr($compressedData, $len, 4);
    if ($marker !== "\x00\xED\xED\x00") {
        throw new Exception("Invalid end marker in version 1 Z80 file");
    }
    
    return $decompressed;
}

function compressZ80Block_v145($uncompressedData) {
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
            $i += $runTimeLength;
            continue;
        }
        
        $compressed .= $current;
        $i++;
    }
    
    while ($i < $len) {
        $current = $uncompressedData[$i];
        
        if ($current === "\xED" && $i + 1 < $len && $uncompressedData[$i + 1] === "\xED") {
            $compressed .= "\xED\xED\x02\xED";
            $i += 2;
        } else {
            $compressed .= $current;
            $i++;
        }
    }
    
    $compressed .= "\x00\xED\xED\x00";
    
    return $compressed;
}

function parsePokesFromURL_v145($pokesString) {
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

function parsePOKFile_v145($pokFilePath) {
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
                
                if ($bank & 0x10) {
                    continue;
                }
                
                if ($bank & 0x08) {
                    $bank = autoBank_v145($address);
                } else {
                    if ($bank < 8) {
                        $bank = autoBank_v145($address);
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

function readZ80File_v145($filePath) {
    $data = [
        'header' => '',
        'blocks' => []
    ];
    
    $header = file_get_contents($filePath, false, null, 0, 30);
    
    if ($header === false || strlen($header) != 30) {
        throw new Exception("Cannot read Z80 header: $filePath");
    }
    
    $data['header'] = $header;
    
    $ramData = file_get_contents($filePath, false, null, 30);
    
    if ($ramData === false) {
        throw new Exception("Cannot read Z80 v1 memory data: $filePath");
    }
    
    $data['blocks'][] = [
        'page' => 8,
        'data' => $ramData
    ];
    
    return $data;
}

function applyPokesToMemory_v145(&$memory, $pokes) {
    if (!isset($memory[8])) {
        throw new Exception("Memory block for v1.xx not found (should be at index 8)");
    }
    
    $memoryBlock = $memory[8];
    
    foreach ($pokes as $poke) {
        $address = $poke['address'];
        $value = $poke['value'];
        
        $offset = $address - 16384;
        
        if ($offset < strlen($memoryBlock)) {
            $memoryBlock[$offset] = chr($value);
        } else {
            throw new Exception("Address $address exceeds memory block size (v1.xx)");
        }
    }
    
    $memory[8] = $memoryBlock;
}

function writeZ80File_v145($filePath, $z80Data) {
    $content = '';
    
    $content .= $z80Data['header'];
    
    foreach ($z80Data['blocks'] as $block) {
        $content .= $block['data'];
    }
    
    return file_put_contents($filePath, $content, LOCK_EX);
}

function validateZ80File_v145($filePath) {
    try {
        $z80Data = readZ80File_v145($filePath);
        
        if (empty($z80Data['blocks'])) {
            return false;
        }
        
        return true;
    } catch (Exception $e) {
        return false;
    }
}

function generateOutputFilename_v145($originalPath) {
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

function processZ80File_v145($inputFile, $outputFile, $pokes) {
    global $debug;
    
    $z80Data = readZ80File_v145($inputFile);
    
    $memory = [];
    $decompressed = decompressZ80Block_v145($z80Data['blocks'][0]['data']);
    $memory[8] = $decompressed;
    
    applyPokesToMemory_v145($memory, $pokes);
    
    $compressed = compressZ80Block_v145($memory[8]);
    $z80Data['blocks'][0]['data'] = $compressed;
    
    $bytesWritten = writeZ80File_v145($outputFile, $z80Data);
    
    if ($bytesWritten === false) {
        throw new Exception("Failed to write output file: $outputFile");
    }
    
    if (!validateZ80File_v145($outputFile)) {
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
        throw new Exception("Missing 'file' parameter. Usage: poke145.php?file=path/to/file.Z80&POKES=address,value:address,value");
    }
    
    if (!file_exists($file)) {
        throw new Exception("Input file not found: $file");
    }
    
    $pokes = [];
    
    if (!empty($pokesString)) {
        $pokes = parsePokesFromURL_v145($pokesString);
        
        if ($debug) {
            logError_v145("Parsed pokes from URL: " . count($pokes) . " pokes");
        }
    }
    
    if (empty($pokes)) {
        throw new Exception("No valid pokes to apply");
    }
    
    $outputFile = generateOutputFilename_v145($file);
    
    processZ80File_v145($file, $outputFile, $pokes);
    
    header('Content-Type: application/json');
    echo json_encode([
        'success' => true,
        'version' => 1,
        'input_file' => $file,
        'output_file' => $outputFile,
        'pokes_applied' => count($pokes)
    ]);
    
} catch (Exception $e) {
    $errorDetails = [
        'file' => $file ?? 'N/A',
        'pokes' => $pokesString ?? 'N/A',
        'trace' => $e->getTraceAsString()
    ];
    logError_v145($e->getMessage(), print_r($errorDetails, true));
    
    header('Content-Type: application/json');
    http_response_code(500);
    echo json_encode([
        'success' => false,
        'error' => $e->getMessage()
    ]);
}
?>
