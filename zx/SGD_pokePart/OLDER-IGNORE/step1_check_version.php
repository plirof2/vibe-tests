<?php
/*
 * check_version.php
 * 
 * Detects Z80 file version and writes to output.log
 * 
 * Usage: check_version.php?file=ROMSMINE/F/FIRELORD.Z80&POKES=34509,0:34984,58
 * 
 * POKES parameter is currently ignored
 *
 * check_version.php?file=ROMSMINE/F/FIRELORD.Z80&POKES=34509,0:34984,58
 * check_version.php?file=ROMSMINE/L/LASTDUEL.Z80&POKES=37605,0:37672,0:40063,0
 *

 */

// Declare global variables for CLI compatibility
global $argc, $argv;

$logFile = 'output.log';

// Get file parameter - support both CLI and web modes
$filePath = '';

if (php_sapi_name() === 'cli') {
    // CLI mode: parse command line arguments
    for ($i = 1; $i < $argc; $i++) {
        $arg = $argv[$i];
        if (strpos($arg, 'file=') === 0) {
            $filePath = substr($arg, 5);
            break;
        }
    }
    if (empty($filePath)) {
        die("Usage: php check_version.php file=ROMSMINE/F/FIRELORD.Z80\n");
    }
} else {
    // Web mode: use GET parameter
    if (!isset($_GET['file'])) {
        die("Error: No file parameter provided\n");
    }
    $filePath = $_GET['file'];
}

// Basic path security - prevent directory traversal
$filePath = str_replace('..', '', $filePath);

// Check if file exists and is readable
if (!file_exists($filePath)) {
    logMessage("Error: File not found: $filePath");
    die("Error: File not found: $filePath\n");
}

if (!is_readable($filePath)) {
    logMessage("Error: File not readable: $filePath");
    die("Error: File not readable: $filePath\n");
}

// Try to detect Z80 version
$version = detectZ80Version($filePath);

// Write to log file
if ($version !== false) {
    $fileSize = filesize($filePath);
    $fileSizeKB = round($fileSize / 1024, 2);
    logMessage("File: $filePath | Size: {$fileSizeKB}KB | Z80 Version: $version");
    echo "File: $filePath\n";
    echo "Size: {$fileSizeKB}KB\n";
    echo "Z80 Version: $version\n";
} else {
    logMessage("Error: Unable to detect Z80 version for file: $filePath");
    die("Error: Unable to detect Z80 version\n");
}

/**
 * Detect Z80 file version
 *
 * Z80 v1.45:
 *   - Header bytes 6-7 (PC) != 0
 *   - No additional header
 *   - Single 48KB memory block (compressed or uncompressed)
 *
 * Z80 v2.01:
 *   - Header bytes 6-7 (PC) == 0 (indicates extended format)
 *   - Additional header length == 23 bytes
 *   - Supports 128K banking
 *
 * Z80 v3.00:
 *   - Header bytes 6-7 (PC) == 0 (indicates extended format)
 *   - Additional header length == 54 bytes (per Z80 spec)
 *   - Extended v2.01 structure
 *
 * @param string $filePath Path to Z80 file
 * @return string|false Version string or false on error
 */
function detectZ80Version($filePath) {
    $handle = fopen($filePath, 'rb');

    if ($handle === false) {
        return false;
    }

    try {
        // Read first 32 bytes (Z80 header + additional header length)
        $header = fread($handle, 32);

        if (strlen($header) < 32) {
            fclose($handle);
            return false;
        }

        // Parse little-endian 16-bit values
        // PC is at bytes 6-7
        $pc = unpack('v', substr($header, 6, 2))[1];

        // Version 2.01/3.00 detection: PC == 0 indicates extended format
        if ($pc == 0) {
            // Read additional header length at bytes 30-31
            $additionalLength = unpack('v', substr($header, 30, 2))[1];

            fclose($handle);

            // Determine version based on additional header length
            if ($additionalLength == 23) {
                return '2.01';
            } else if ($additionalLength == 54) {
                return '3.00';
            } else {
                // Unknown version
                return "Unknown (additional header length: $additionalLength)";
            }
        }

        // Version 1.45 detection: PC != 0 indicates old format
        fclose($handle);
        return '1.45';

    } catch (Exception $e) {
        if (is_resource($handle)) {
            fclose($handle);
        }
        return false;
    }
}

/**
 * Write message to log file
 * 
 * @param string $message Message to log
 */
function logMessage($message) {
    global $logFile;
    
    $timestamp = date('Y-m-d H:i:s');
    $logEntry = "[$timestamp] $message\n";
    
    // Append to log file
    file_put_contents($logFile, $logEntry, FILE_APPEND);
    
    // Also echo to stdout
    echo $logEntry;
}
?>
