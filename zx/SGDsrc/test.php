<?php
/**********************************************************************************************************************************/
/* Project      : SGD                                                                                                             */
/* Module       : test.php                                                                                                         */
/* Description  : Z80 version scanner for ROMSMINE folder                                                                          */
/* Version type : Test script                                                                                                      */
/* Last changed : 2025                                                                                                            */
/* Update count : 1                                                                                                               */
/*                                                                                                                                */
/*                Based on C implementation (dbfile.c GetZ80Version) by Martijn van der Heide                                           */
/*                                                                                                                                */
/*                This script is free software; you can redistribute it and/or                                                     */
/*                modify it under the terms of the GNU General Public License                                                     */
/*                as published by the Free Software Foundation; either version 2                                                  */
/*                of the License, or (at your option) any later version.                                                          */
/*                                                                                                                                */
/**********************************************************************************************************************************/

$logFile = __DIR__ . '/log_test_z80_versions.log';
$romsMineDir = __DIR__ . '/ROMSMINE';

function getZ80Version($filePath) {
    $fileLength = filesize($filePath);
    
    if ($fileLength === false || $fileLength < 30) {
        return ['error' => 'File too small (< 30 bytes)'];
    }
    
    $handle = @fopen($filePath, 'rb');
    if (!$handle) {
        return ['error' => 'Cannot open file'];
    }
    
    $header = fread($handle, 32);
    fclose($handle);
    
    if (strlen($header) < 30) {
        return ['error' => 'Cannot read header'];
    }
    
    $pc = ord($header[6]) | (ord($header[7]) << 8);
    $stat1 = ord($header[12]);
    
    if ($pc != 0) {
        return [
            'version' => 145,
            'compressed' => ($stat1 & 0x20) != 0,
            'pc' => $pc,
            'status' => 'OK'
        ];
    }
    
    if ($fileLength < 32 || strlen($header) < 32) {
        return ['error' => 'Invalid v2.01/v3.00 file (too small)', 'status' => 'ERROR'];
    }
    
    $additionalLength = ord($header[30]) | (ord($header[31]) << 8);
    
    if ($additionalLength == 54 || $additionalLength == 55) {
        return [
            'version' => 300,
            'compressed' => true,
            'pc' => 0,
            'status' => 'OK'
        ];
    }
    
    if ($additionalLength == 23) {
        return [
            'version' => 201,
            'compressed' => true,
            'pc' => 0,
            'status' => 'OK'
        ];
    }
    
    return ['error' => "Unknown version (AddLen={$additionalLength})", 'status' => 'ERROR'];
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
                $versionInfo = getZ80Version($file->getPathname());
                
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
    $log .= sprintf("%-40s | %-8s | %-8s | %-15s\n", "File", "Version", "Size", "Status");
    $log .= str_repeat("-", 120) . "\n";
    
    $versionCounts = [145 => 0, 201 => 0, 300 => 0];
    $errorCount = 0;
    $totalFiles = 0;
    
    foreach ($results as $result) {
        $path = $result['path'];
        $size = $result['size'];
        $info = $result['info'];
        
        if (isset($info['error'])) {
            $versionStr = 'ERROR';
            $status = $info['error'];
            $errorCount++;
        } else {
            $version = $info['version'];
            $versionStr = ($version / 100) . '';
            $status = $info['status'];
            if (isset($versionCounts[$version])) {
                $versionCounts[$version]++;
            }
        }
        
        $log .= sprintf("%-40s | %-8s | %-8d | %-15s\n", $path, $versionStr, $size, $status);
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
    
    if ($errorCount > 0) {
        $log .= "\nErrors encountered: $errorCount\n";
    }
    
    $log .= "\n";
    
    $success = file_put_contents($logFile, $log);
    
    return $success !== false;
}

$startTime = microtime(true);

echo "Z80 Version Scanner\n";
echo "==================\n\n";
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

foreach ($results as $result) {
    if (isset($result['info']['error'])) {
        $errorCount++;
    } elseif (isset($result['info']['version'])) {
        $version = $result['info']['version'];
        if (isset($versionCounts[$version])) {
            $versionCounts[$version]++;
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

if ($errorCount > 0) {
    echo "Errors encountered: $errorCount\n";
}

echo "\nLog written to: $logFile\n";
echo "Execution time: {$executionTime}ms\n\n";
