<?php
/*
 * decompression_fixed.php
 *
 * Checks Z80 file compression, reports version, decompresses and recompresses
 * Full implementation for v1.45, v2.01 and v3.00
 *
 * Fixed version - All bugs from original corrected
 *
 * Usage: php decompression_fixed.php file=ROMSMINE/F/FIRELORD.Z80
 * Output: filename_recompressed.z80
 */

global $argc, $argv;

$logFile = 'output_fixed.log';

// Get file parameter
$filePath = '';
if (php_sapi_name() === 'cli') {
    for ($i = 1; $i < $argc; $i++) {
        $arg = $argv[$i];
        if (strpos($arg, 'file=') === 0) {
            $filePath = substr($arg, 5);
            break;
        }
    }
    if (empty($filePath)) {
        die("Usage: php decompression_fixed.php file=ROMSMINE/F/FIRELORD.Z80\n");
    }
} else {
    if (!isset($_GET['file']) || empty($_GET['file'])) {
        die("Error: No file parameter provided\n");
    }
    $filePath = $_GET['file'];
}

// Basic path security
$filePath = str_replace('..', '', $filePath);

// Check if file exists and is readable
if (!file_exists($filePath)) {
    die("Error: File not found: $filePath\n");
}

if (!is_readable($filePath)) {
    die("Error: File not readable: $filePath\n");
}

// Detect Z80 version
$version = detectZ80Version($filePath);

if ($version === false) {
    die("Error: Unable to detect Z80 version for file: $filePath\n");
}

$fileSize = filesize($filePath);
$fileSizeKB = round($fileSize / 1024, 2);

if ($version == '1.45') {
    processZ80v145($filePath, $fileSize);
} else {
    processZ80v201plus($filePath, $version, $fileSize);
}

function detectZ80Version($filePath) {
    $handle = fopen($filePath, 'rb');
    if ($handle === false) {
        return false;
    }

    $header = fread($handle, 30);

    if (strlen($header) < 30) {
        fclose($handle);
        return false;
    }

    $pc = unpack('v', substr($header, 6, 2))[1];

    if ($pc == 0) {
        $additionalLength = unpack('v', fread($handle, 2))[1];
        fclose($handle);

        if ($additionalLength == 23) {
            return '2.01';
        } elseif ($additionalLength == 55) {
            return '3.00';
        } else {
            return "Unknown (additional header length: $additionalLength)";
        }
    }

    fclose($handle);
    return '1.45';
}

function processZ80v145($filePath, $fileSize) {
    global $logFile;

    $fileSizeKB = round($fileSize / 1024, 2);

    $handle = fopen($filePath, 'rb');
    $header = fread($handle, 30);
    fclose($handle);

    $stat1 = ord($header[12]);
    $isCompressed = ($stat1 & 0x20) != 0;

    if ($isCompressed) {
        logMessage("File: $filePath | Size: {$fileSizeKB}KB | Version: 1.45 | Compressed: YES");
        echo "File: $filePath\n";
        echo "Size: {$fileSizeKB}KB\n";
        echo "Z80 Version: 1.45\n";
        echo "Compressed: YES\n";
        echo "\nDecompressing...\n";

        $handle = fopen($filePath, 'rb');
        fseek($handle, 30);
        $compressedData = fread($handle, $fileSize - 30);
        fclose($handle);

        $compressedLen = strlen($compressedData) - 4;
        $dataToDecompress = substr($compressedData, 0, $compressedLen);

        $decompressedData = decompressZ80v145($dataToDecompress);

        if ($decompressedData === false) {
            logMessage("Error: Decompression failed for: $filePath");
            die("Error: Decompression failed\n");
        }

        echo "Decompressed size: " . strlen($decompressedData) . " bytes\n";

        if (strlen($decompressedData) != 49152) {
            echo "ERROR: Expected 49152 bytes, got " . strlen($decompressedData) . " bytes\n";
            logMessage("Error: Decompression size mismatch for: $filePath");
            die("Error: Decompression size mismatch\n");
        }

        echo "\nRecompressing...\n";

        $recompressedData = compressZ80v145($decompressedData);

        if ($recompressedData === false) {
            logMessage("Error: Recompression failed for: $filePath");
            die("Error: Recompression failed\n");
        }

        echo "Recompressed size: " . strlen($recompressedData) . " bytes\n";

        $outputPath = preg_replace('/\.(z80|Z80)$/', '', $filePath) . '_recompressed.z80';
        $handle = fopen($outputPath, 'wb');
        fwrite($handle, $header);
        fwrite($handle, $recompressedData);
        fclose($handle);

        echo "\nSaved to: $outputPath\n";
        logMessage("Recompressed file saved to: $outputPath");
    } else {
        logMessage("File: $filePath | Size: {$fileSizeKB}KB | Version: 1.45 | Compressed: NO");
        echo "File: $filePath\n";
        echo "Size: {$fileSizeKB}KB\n";
        echo "Z80 Version: 1.45\n";
        echo "Compressed: NO\n";
        echo "\nFile is already uncompressed. No action taken.\n";
    }
}

function processZ80v201plus($filePath, $version, $fileSize) {
    global $logFile;

    $fileSizeKB = round($fileSize / 1024, 2);

    $handle = fopen($filePath, 'rb');
    $header = fread($handle, 30);

    $pc = unpack('v', substr($header, 6, 2))[1];

    if ($pc == 0) {
        $additionalLength = unpack('v', fread($handle, 2))[1];
        $additionalHeader = fread($handle, $additionalLength);

        $blocks = array();
        $totalBlocks = 0;
        $compressedBlocks = 0;

        while (true) {
            $blockLenBytes = fread($handle, 2);
            if (strlen($blockLenBytes) < 2) break;

            $blockLen = unpack('v', $blockLenBytes)[1];

            if ($blockLen == 0) break;

            $totalBlocks++;

            $compressedDataLen = $blockLen - 1;
            $compressedData = fread($handle, $compressedDataLen);
            $pageNum = ord(fread($handle, 1));

            $isBlockCompressed = ($blockLen < 16384);

            if ($isBlockCompressed) {
                $compressedBlocks++;
                $decompressed = decompressZ80Block($compressedData);
                $decompressedSize = strlen($decompressed);
            } else {
                $decompressed = $compressedData;
                $decompressedSize = $compressedDataLen;
            }

            $blocks[] = array(
                'page' => $pageNum,
                'block_len' => $blockLen,
                'compressed_size' => $compressedDataLen,
                'decompressed_size' => $decompressedSize,
                'is_compressed' => $isBlockCompressed,
                'decompressed_data' => $decompressed
            );
        }

        fclose($handle);

        $isCompressed = ($compressedBlocks > 0);
        $compressionStatus = $isCompressed ? "YES" : "NO";

        logMessage("File: $filePath | Size: {$fileSizeKB}KB | Version: $version | Compressed: $compressionStatus");
        echo "File: $filePath\n";
        echo "Size: {$fileSizeKB}KB\n";
        echo "Z80 Version: $version\n";
        echo "Compressed: $compressionStatus\n";
        echo "Total memory blocks: $totalBlocks\n";
        echo "Compressed blocks: $compressedBlocks\n\n";

        echo "Memory blocks:\n";
        foreach ($blocks as $i => $block) {
            $status = $block['is_compressed'] ? "COMPRESSED" : "UNCOMPRESSED";
            $correct = $block['decompressed_size'] == 16384 ? "✓" : "✗";
            echo "  Block " . ($i + 1) . ": Page " . $block['page'] .
                 ", Size: " . $block['block_len'] .
                 " bytes -> " . $block['decompressed_size'] .
                 " bytes ($status) $correct\n";
        }

        if ($isCompressed) {
            echo "\nRecompressing all blocks...\n";

            $outputPath = preg_replace('/\.(z80|Z80)$/', '', $filePath) . '_recompressed.z80';
            $outHandle = fopen($outputPath, 'wb');

            fwrite($outHandle, $header);
            fwrite($outHandle, pack('v', $additionalLength));
            fwrite($outHandle, $additionalHeader);

            foreach ($blocks as $block) {
                if ($block['is_compressed']) {
                    $recompressed = compressZ80Block($block['decompressed_data']);
                    $blockLen = strlen($recompressed) + 1;
                    fwrite($outHandle, pack('v', $blockLen));
                    fwrite($outHandle, $recompressed);
                    fwrite($outHandle, chr($block['page']));
                } else {
                    $blockLen = $block['decompressed_size'];
                    fwrite($outHandle, pack('v', $blockLen));
                    fwrite($outHandle, $block['decompressed_data']);
                    fwrite($outHandle, chr($block['page']));
                }
            }

            fwrite($outHandle, pack('v', 0));

            fclose($outHandle);

            echo "Recompressed all blocks to: $outputPath\n";
            logMessage("Recompressed file saved to: $outputPath");
        } else {
            echo "\nNo compression to process. No action taken.\n";
        }
    } else {
        fclose($handle);
    }
}

function decompressZ80v145($data) {
    $output = '';
    $len = strlen($data);
    $pos = 0;

    while ($pos < $len) {
        $byte = ord($data[$pos]);

        if ($byte == 0xED && $pos + 1 < $len && ord($data[$pos + 1]) == 0xED) {
            if ($pos + 3 < $len) {
                $xx = ord($data[$pos + 2]);
                $yy = ord($data[$pos + 3]);

                if ($xx == 2 && $yy == 0xED) {
                    $output .= chr(0xED);
                    $output .= chr(0xED);
                    $pos += 4;
                    continue;
                }

                if ($xx == 0) {
                    break;
                }

                if ($xx > 0) {
                    $output .= str_repeat(chr($yy), $xx);
                }
                $pos += 4;
            } else {
                break;
            }
        } else {
            $output .= $data[$pos];
            $pos++;
        }
    }

    return $output;
}

function compressZ80v145($data) {
    $output = '';
    $len = strlen($data);
    $pos = 0;

    while ($pos < $len) {
        $currentByte = $data[$pos];

        if ($currentByte == chr(0xED)) {
            if ($pos + 1 < $len && $data[$pos + 1] == chr(0xED)) {
                $count = 2;
                while ($pos + $count < $len && $data[$pos + $count] == chr(0xED)) {
                    $count++;
                }

                while ($count >= 2) {
                    $output .= chr(0xED) . chr(0xED) . chr(0x02) . chr(0xED);
                    $pos += 2;
                    $count -= 2;
                }

                if ($count == 1) {
                    $output .= chr(0xED);
                    $pos++;
                }
            } else {
                $output .= $currentByte;
                $pos++;
            }
        } else {
            $count = 1;
            while ($pos + $count < $len && $data[$pos + $count] == $currentByte) {
                $count++;
            }

            if ($count >= 5 && $count <= 255) {
                $output .= chr(0xED) . chr(0xED) . chr($count) . $currentByte;
                $pos += $count;
            } else {
                $output .= str_repeat($currentByte, $count);
                $pos += $count;
            }
        }
    }

    $output .= chr(0x00) . chr(0xED) . chr(0xED) . chr(0x00);

    return $output;
}

function decompressZ80Block($data) {
    $output = '';
    $len = strlen($data);
    $pos = 0;

    while ($pos < $len) {
        $byte = ord($data[$pos]);

        if ($byte == 0xED && $pos + 1 < $len && ord($data[$pos + 1]) == 0xED) {
            if ($pos + 3 < $len) {
                $xx = ord($data[$pos + 2]);
                $yy = ord($data[$pos + 3]);

                if ($xx == 2 && $yy == 0xED) {
                    $output .= chr(0xED);
                    $output .= chr(0xED);
                    $pos += 4;
                    continue;
                }

                if ($xx == 0) {
                    break;
                }

                if ($xx > 0) {
                    $output .= str_repeat(chr($yy), $xx);
                }
                $pos += 4;
            } else {
                break;
            }
        } else {
            $output .= $data[$pos];
            $pos++;
        }
    }

    return $output;
}

function compressZ80Block($data) {
    $output = '';
    $len = strlen($data);
    $pos = 0;

    while ($pos < $len) {
        $byte = $data[$pos];

        if ($byte == chr(0xED)) {
            if ($pos + 1 < $len && $data[$pos + 1] == chr(0xED)) {
                $count = 2;
                while ($pos + $count < $len && $data[$pos + $count] == chr(0xED)) {
                    $count++;
                }

                while ($count >= 2) {
                    $output .= chr(0xED) . chr(0xED) . chr(0x02) . chr(0xED);
                    $pos += 2;
                    $count -= 2;
                }

                if ($count == 1) {
                    $output .= chr(0xED);
                    $pos++;
                }
            } else {
                $output .= $byte;
                $pos++;
            }
        } else {
            $count = 1;
            while ($pos + $count < $len && $data[$pos + $count] == $byte) {
                $count++;
            }

            if ($count >= 5 && $count <= 255) {
                $output .= chr(0xED) . chr(0xED) . chr($count) . $byte;
                $pos += $count;
            } else {
                $output .= str_repeat($byte, $count);
                $pos += $count;
            }
        }
    }

    return $output;
}

function logMessage($message) {
    global $logFile;

    $timestamp = date('Y-m-d H:i:s');
    $logEntry = "[$timestamp] $message\n";

    file_put_contents($logFile, $logEntry, FILE_APPEND);

    echo $logEntry;
}
?>
