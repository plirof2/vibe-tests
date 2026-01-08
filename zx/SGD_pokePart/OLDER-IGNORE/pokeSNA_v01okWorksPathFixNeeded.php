<?php
/**
 * ZX Spectrum SNA POKE Patching Script v0.01
 * 
 * This script applies POKE patches to ZX Spectrum SNA snapshot files.
 * Based on C source code: dbpoke.c, dbfile.h
 * by Martijn van der Heide
 * 
 * Usage: pokeSNA_v01.php?file=path/to/file.SNA&POKES=address,value:address,value
 * 
 * @version 0.01 - SNA-only implementation, ignores Z80 files
 */

//$SNAPSHOT_DIR = __DIR__ . '/';
$SNAPSHOT_DIR = './';

function parsePokes_SNA($pokesString) {
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
            'value' => $value
        ];
    }
    
    return $pokes;
}

function applySNApoke_SNA(&$memory, $address, $value) {
    $offset = $address - 16384;
    
    if ($offset < 0 || $offset >= strlen($memory)) {
        throw new Exception("Address $address exceeds memory block size");
    }
    
    $memory[$offset] = chr($value);
}

function readSNAFile_SNA($filePath) {
    $data = [
        'header' => '',
        'memory' => ''
    ];
    
    $header = file_get_contents($filePath, false, null, 0, 27);
    
    if ($header === false || strlen($header) != 27) {
        throw new Exception("Cannot read SNA header: $filePath");
    }
    
    $data['header'] = $header;
    
    $memory = file_get_contents($filePath, false, null, 27);
    
    if ($memory === false) {
        throw new Exception("Cannot read SNA memory data: $filePath");
    }
    
    if (strlen($memory) != 49152) {
        throw new Exception("Invalid SNA memory size: " . strlen($memory) . " (expected 49152)");
    }
    
    $data['memory'] = $memory;
    
    return $data;
}

function writeSNAFile_SNA($filePath, $snaData) {
    $content = $snaData['header'] . $snaData['memory'];
    return file_put_contents($filePath, $content, LOCK_EX);
}

function validateSNAFile_SNA($filePath) {
    try {
        $snaData = readSNAFile_SNA($filePath);
        
        if (strlen($snaData['header']) != 27 || strlen($snaData['memory']) != 49152) {
            return false;
        }
        
        return true;
    } catch (Exception $e) {
        return false;
    }
}

function generateOutputFilename_SNA($originalPath) {
    $pathInfo = pathinfo($originalPath);
    $dirname = $pathInfo['dirname'];
    $basename = $pathInfo['filename'];
    
    $pattern = $dirname . '/' . $basename . '_POKED*.SNA';
    $files = glob($pattern);
    
    $maxNum = 0;
    foreach ($files as $file) {
        if (preg_match('/_POKED(\d+)\.SNA$/i', basename($file), $matches)) {
            $num = intval($matches[1]);
            if ($num > $maxNum) {
                $maxNum = $num;
            }
        }
    }
    
    $nextNum = $maxNum + 1;
    
    return $dirname . '/' . $basename . '_POKED' . sprintf('%02d', $nextNum) . '.SNA';
}

function processSNAFile_SNA($inputFile, $outputFile, $pokes) {
    $snaData = readSNAFile_SNA($inputFile);
    
    foreach ($pokes as $poke) {
        applySNApoke_SNA($snaData['memory'], $poke['address'], $poke['value']);
    }
    
    $bytesWritten = writeSNAFile_SNA($outputFile, $snaData);
    
    if ($bytesWritten === false) {
        throw new Exception("Failed to write output file: $outputFile");
    }
    
    if (!validateSNAFile_SNA($outputFile)) {
        if (file_exists($outputFile)) {
            unlink($outputFile);
        }
        throw new Exception("Output file validation failed: $outputFile");
    }
}

try {
    if ($_SERVER['REQUEST_METHOD'] !== 'GET') {
        throw new Exception("Only GET requests are supported");
    }
    
    if (empty($_GET['file'])) {
        throw new Exception("Missing 'file' parameter");
    }
    
    if (empty($_GET['POKES'])) {
        throw new Exception("Missing 'POKES' parameter");
    }
    
    $filename = basename($_GET['file']);
    $filepath = $SNAPSHOT_DIR . $filename;
    
    if (!file_exists($filepath)) {
        throw new Exception("File not found: $filename");
    }
    
    $ext = strtolower(pathinfo($filename, PATHINFO_EXTENSION));
    
    if ($ext !== 'sna') {
        throw new Exception("Only .SNA files are supported (ignoring Z80 files)");
    }
    
    if (!validateSNAFile_SNA($filepath)) {
        throw new Exception("Invalid SNA file: $filename");
    }
    
    $pokes = parsePokes_SNA($_GET['POKES']);
    
    if (empty($pokes)) {
        throw new Exception("No valid pokes to apply");
    }
    
    $outputFile = generateOutputFilename_SNA($filepath);
    
    processSNAFile_SNA($filepath, $outputFile, $pokes);
    
    $outputFilename = basename($outputFile);
    
    header('Content-Type: application/octet-stream');
    header('Content-Disposition: attachment; filename="' . $outputFilename . '"');
    header('Content-Length: ' . filesize($outputFile));
    
    readfile($outputFile);
    
} catch (Exception $e) {
    header('Content-Type: text/plain');
    http_response_code(500);
    echo "Error: " . $e->getMessage();
}
?>
