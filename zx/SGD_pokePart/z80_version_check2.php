<?php
/**********************************************************************************************************************************/
/* Project      : SGD                                                                                                             */
/* Module       : test.php                                                                                                         */
/* Description  : Z80 version and compression scanner for ROMSMINE folder                                                         */
/* Version type : Test script                                                                                                      */
/* Last changed : 2025                                                                                                            */
/* Update count : 2                                                                                                               */
/*                                                                                                                                */
/*                Based on C implementation (dbfile.c GetZ80Version, dbscreen.c DecrunchZ80Block/CrunchZ80Block) by Martijn van der Heide                                           */
/*                                                                                                                                */
/*                This script is free software; you can redistribute it and/or                                                     */
/*                modify it under the terms of the GNU General Public License                                                     */
/*                as published by the Free Software Foundation; either version 2                                                  */
/*                of the License, or (at your option) any later version.                                                          */
/*                                                                                                                                */
/**********************************************************************************************************************************/

$logFile = __DIR__ . '/tmp/log_test_z80_versions.log';
$romsMineDir = __DIR__ . '/ROMSMINE';

function getZ80VersionAndCompression($filePath) {
    $fileLength = filesize($filePath);
    
    if ($fileLength === false || $fileLength < 30) {
        return ['error' => 'File too small (< 30 bytes)'];
    }
    
    $handle = @fopen($filePath, 'rb');
    if (!$handle) {
        return ['error' => 'Cannot open file'];
    }
    
    $header = fread($handle, 32);
    
    if (strlen($header) < 30) {
        fclose($handle);
        return ['error' => 'Cannot read header'];
    }
    
    $pc = ord($header[6]) | (ord($header[7]) << 8);
    $stat1 = ord($header[12]);
    
    if ($pc != 0) {
        fclose($handle);
        $compressed = ($stat1 & 0x20) != 0;
        return [
            'version' => 145,
            'compressed' => $compressed,
            'compression_status' => $compressed ? 'Compressed' : 'Uncompressed',
            'compression_type' => $compressed ? 'RLE (Run-Length Encoding)' : 'Raw',
            'bank_info' => 'Single 49KB block',
            'pc' => $pc,
            'status' => 'OK'
        ];
    }
    
    fclose($handle);
    
    if ($fileLength < 32 || strlen($header) < 32) {
        return ['error' => 'Invalid v2.01/v3.00 file (too small)', 'status' => 'ERROR'];
    }
    
    $additionalLength = ord($header[30]) | (ord($header[31]) << 8);
    
    $version = 201;
    if ($additionalLength == 54 || $additionalLength == 55) {
        $version = 300;
    } elseif ($additionalLength != 23) {
        return ['error' => "Unknown version (AddLen={$additionalLength})", 'status' => 'ERROR'];
    }
    
    $headerOffset = 30 + $additionalLength;
    $handle = @fopen($filePath, 'rb');
    if (!$handle) {
        return ['error' => 'Cannot reopen file', 'status' => 'ERROR'];
    }
    
    fseek($handle, $headerOffset);
    
    $banks = [];
    $hasCompressed = false;
    $hasUncompressed = false;
    
    $dataEnd = $fileLength;
    $pos = $headerOffset;
    $dataAfterHeaders = '';
    
    fseek($handle, $headerOffset);
    if ($dataEnd > $headerOffset) {
        $dataAfterHeaders = fread($handle, min(4096, $dataEnd - $headerOffset));
    }
    fclose($handle);
    
    $hasRLE = false;
    $rleCount = 0;
    for ($i = 0; $i < strlen($dataAfterHeaders) - 1; $i++) {
        if (ord($dataAfterHeaders[$i]) == 0xED && ord($dataAfterHeaders[$i + 1]) == 0xED) {
            $hasRLE = true;
            $rleCount++;
        }
    }
    
    $remainingBytes = $dataEnd - $headerOffset;
    
    if ($hasRLE) {
        $hasCompressed = true;
        if ($remainingBytes < 20000) {
            $overallStatus = 'Compressed (RLE)';
            $bankInfo = $rleCount . ' RLE markers';
        } else {
            $overallStatus = 'Compressed (RLE)';
            $bankInfo = $rleCount . ' RLE markers';
        }
    } elseif ($remainingBytes < 20000) {
        $hasUncompressed = true;
        $overallStatus = 'Uncompressed (small)';
        $bankInfo = 'Minimal data';
    } else {
        $hasUncompressed = true;
        $overallStatus = 'Uncompressed (raw)';
        $bankInfo = 'No RLE markers';
    }
    
    return [
        'version' => $version,
        'compressed' => $hasCompressed,
        'compression_status' => $overallStatus,
        'compression_type' => 'Per-bank RLE',
        'bank_info' => $bankInfo,
        'banks' => $banks,
        'pc' => 0,
        'status' => 'OK'
    ];
}

function scanZ80Files($baseDir) {
    $results = [];
    
    if (!is_dir($baseDir)) {
        return ['error' => 'Directory not found: ' . $baseDir];
    }
    
    $iterator = new RecursiveIteratorIterator(
        new RecursiveDirectoryIterator($baseDir, RecursiveDirectoryIterator::SKIP_DOTS),
        RecursiveIteratorIterator::SELF_FIRST
    );
    
    foreach ($iterator as $file) {
        if ($file->isFile()) {
            $extension = strtolower($file->getExtension());
            
            if ($extension === 'z80') {
                $relativePath = substr($file->getPathname(), strlen($baseDir) + 1);
                $fileSize = $file->getSize();
                $versionInfo = getZ80VersionAndCompression($file->getPathname());
                
                $results[] = [
                    'path' => 'ROMSMINE/' . $relativePath,
                    'size' => $fileSize,
                    'info' => $versionInfo
                ];
            }
        }
    }
    
    return $results;
}

function writeLog($results, $logFile, $baseDir) {
    $timestamp = date('Y-m-d H:i:s');
    
    $log = "Timestamp: $timestamp\n";
    $log .= "Scanning: $baseDir\n";
    $log .= str_repeat("=", 120) . "\n";
    $log .= sprintf("%-40s | %-8s | %-15s | %-15s\n", "File", "Version", "Compression", "Status");
    $log .= str_repeat("-", 120) . "\n";
    
    $versionCounts = [145 => 0, 201 => 0, 300 => 0];
    $compressionCounts = [
        'v145_compressed' => 0,
        'v145_uncompressed' => 0,
        'v201_compressed' => 0,
        'v201_uncompressed' => 0,
        'v300_compressed' => 0,
        'v300_uncompressed' => 0
    ];
    $errorCount = 0;
    $totalFiles = 0;
    
    foreach ($results as $result) {
        $path = $result['path'];
        $size = $result['size'];
        $info = $result['info'];
        
        if (isset($info['error'])) {
            $versionStr = 'ERROR';
            $compressionStr = 'N/A';
            $status = $info['error'];
            $errorCount++;
        } else {
            $version = $info['version'];
            $versionStr = 'v' . ($version / 100);
            $compressionStr = $info['compression_status'];
            $status = $info['status'];
            
            if (isset($versionCounts[$version])) {
                $versionCounts[$version]++;
            }
            
            if ($version == 145) {
                if ($info['compressed']) {
                    $compressionCounts['v145_compressed']++;
                } else {
                    $compressionCounts['v145_uncompressed']++;
                }
            } else            if ($version == 201) {
                if (strpos($info['compression_status'], 'Compressed') !== false) {
                    $compressionCounts['v201_compressed']++;
                } else {
                    $compressionCounts['v201_uncompressed']++;
                }
            } else            if ($version == 300) {
                if (strpos($info['compression_status'], 'Compressed') !== false) {
                    $compressionCounts['v300_compressed']++;
                } else {
                    $compressionCounts['v300_uncompressed']++;
                }
            }
        }
        
        $log .= sprintf("%-40s | %-8s | %-15s | %-15s\n", $path, $versionStr, $compressionStr, $status);
        $totalFiles++;
    }
    
    $log .= str_repeat("-", 120) . "\n";
    
    $log .= "\n";
    $log .= "Summary:\n";
    $log .= "--------\n";
    $log .= "Total Z80 files found: $totalFiles\n";
    
    if ($totalFiles > $errorCount) {
        $log .= "\nVersions detected:\n";
        foreach ($versionCounts as $version => $count) {
            if ($count > 0) {
                $log .= "  - v" . ($version / 100) . ": $count files\n";
            }
        }
    }
    
    $log .= "\nCompression breakdown:\n";
    $log .= "  v1.45:\n";
    $log .= "    - Compressed: " . $compressionCounts['v145_compressed'] . " files\n";
    $log .= "    - Uncompressed: " . $compressionCounts['v145_uncompressed'] . " files\n";
    $log .= "  v2.01:\n";
    $log .= "    - Compressed: " . ($compressionCounts['v201_compressed'] ?? 0) . " files\n";
    $log .= "    - Uncompressed: " . ($compressionCounts['v201_uncompressed'] ?? 0) . " files\n";
    $log .= "  v3.00:\n";
    $log .= "    - Compressed: " . ($compressionCounts['v300_compressed'] ?? 0) . " files\n";
    $log .= "    - Uncompressed: " . ($compressionCounts['v300_uncompressed'] ?? 0) . " files\n";
    
    if ($errorCount > 0) {
        $log .= "\nErrors encountered: $errorCount\n";
    }
    
    $log .= "\n";
    
    $success = file_put_contents($logFile, $log);
    
    return $success !== false;
}

$startTime = microtime(true);

echo "Z80 Version and Compression Scanner\n";
echo "================================\n\n";
echo "Scanning directory: $romsMineDir\n";

if (!is_dir($romsMineDir)) {
    echo "ERROR: ROMSMINE directory not found!\n";
    echo "Expected: $romsMineDir\n\n";
    exit(1);
}

$results = scanZ80Files($romsMineDir);

if (isset($results['error'])) {
    echo "ERROR: " . $results['error'] . "\n";
    exit(1);
}

$success = writeLog($results, $logFile, $romsMineDir);

if (!$success) {
    echo "ERROR: Cannot write log file: $logFile\n";
    exit(1);
}

$endTime = microtime(true);
$executionTime = round(($endTime - $startTime) * 1000, 2);

$totalFiles = count($results);
$errorCount = 0;
$versionCounts = [145 => 0, 201 => 0, 300 => 0];
$compressionCounts = [
    'compressed' => 0,
    'uncompressed' => 0
];

foreach ($results as $result) {
    if (isset($result['info']['error'])) {
        $errorCount++;
    } elseif (isset($result['info']['version'])) {
        $version = $result['info']['version'];
        if (isset($versionCounts[$version])) {
            $versionCounts[$version]++;
        }
        if ($result['info']['compressed']) {
            $compressionCounts['compressed']++;
        } else {
            $compressionCounts['uncompressed']++;
        }
    }
}

echo "\n";
echo "Scan complete!\n";
echo "------------\n";
echo "Total Z80 files found: $totalFiles\n";
echo "Versions detected:\n";

foreach ($versionCounts as $version => $count) {
    if ($count > 0) {
        echo "  - v" . ($version / 100) . ": $count files\n";
    }
}

echo "\nCompression:\n";
echo "  - Compressed: " . $compressionCounts['compressed'] . " files\n";
echo "  - Uncompressed: " . $compressionCounts['uncompressed'] . " files\n";

if ($errorCount > 0) {
    echo "Errors encountered: $errorCount\n";
}

echo "\nLog written to: $logFile\n";
echo "Execution time: {$executionTime}ms\n\n";