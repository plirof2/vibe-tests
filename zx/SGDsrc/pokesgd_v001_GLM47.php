<?php
/**********************************************************************************************************************************/
/* Project      : SGD                                                                                                             */
/* Module       : pokesgd.php                                                                                                    */
/* Description  : PHP script to apply Spectrum game pokes to Z80/SNA files                                                       */
/* Version type : Standalone script                                                                                               */
/* Last changed : 2025                                                                                                            */
/* Update count : 1                                                                                                               */
/*                                                                                                                                */
/*                Based on C implementation (dbpoke.c) by Martijn van der Heide                                                   */
/*                                                                                                                                */
/*                This script is free software; you can redistribute it and/or                                                     */
/*                modify it under the terms of the GNU General Public License                                                     */
/*                as published by the Free Software Foundation; either version 2                                                  */
/*                of the License, or (at your option) any later version.                                                          */
/*                                                                                                                                */
/**********************************************************************************************************************************/

class PokeSgd {
    private $logFile = null;
    private $debug = false;

    public function __construct($logFile = null) {
        $this->logFile = $logFile;
    }

    private function logError($message) {
        if ($this->logFile) {
            $timestamp = date('[Y-m-d H:i:s]');
            file_put_contents($this->logFile, $timestamp . ' ' . $message . "\n", FILE_APPEND);
        }
    }

    private function readBytes($handle, $length) {
        $data = fread($handle, $length);
        if (strlen($data) != $length) {
            return false;
        }
        return $data;
    }

    private function writeBytes($handle, $data) {
        return fwrite($handle, $data);
    }

    private function getByte($data, $offset) {
        return ord($data[$offset]);
    }

    private function getWord($data, $offset) {
        return ord($data[$offset]) | (ord($data[$offset + 1]) << 8);
    }

    private function setByte(&$data, $offset, $value) {
        $data[$offset] = chr($value & 0xFF);
    }

    private function setWord(&$data, $offset, $value) {
        $data[$offset] = chr($value & 0xFF);
        $data[$offset + 1] = chr(($value >> 8) & 0xFF);
    }

    /**********************************************************************************************************************************/
    /* Z80 Compression/Decompression Module                                                                                           */
    /**********************************************************************************************************************************/

    private function decrunchZ80Block($data, $targetLength) {
        $result = '';
        $pos = 0;
        $dataLen = strlen($data);

        while (strlen($result) < $targetLength && $pos < $dataLen) {
            if ($pos + 1 < $dataLen && ord($data[$pos]) == 0xED && ord($data[$pos + 1]) == 0xED) {
                if ($pos + 2 < $dataLen) {
                    $length = ord($data[$pos + 2]);
                    if ($length == 0) {
                        $length = 256;
                    }
                    if ($pos + 3 < $dataLen) {
                        $byte = $data[$pos + 3];
                        $result .= str_repeat($byte, $length);
                        $pos += 4;
                    } else {
                        break;
                    }
                } else {
                    break;
                }
            } else {
                $result .= $data[$pos];
                $pos++;
            }
        }

        if (strlen($result) < $targetLength) {
            $result .= str_repeat("\0", $targetLength - strlen($result));
        }

        return $result;
    }

    private function crunchZ80Block($data) {
        $result = '';
        $len = strlen($data);
        $i = 0;

        while ($i < $len) {
            $byte = $data[$i];
            $runLen = 1;

            while ($i + $runLen < $len && $data[$i + $runLen] == $byte && $runLen < 256) {
                $runLen++;
            }

            if ($runLen >= 4) {
                $result .= "\xED\xED" . chr($runLen == 256 ? 0 : $runLen) . $byte;
                $i += $runLen;
            } else {
                if ($i + 1 < $len && ord($data[$i]) == 0xED && ord($data[$i + 1]) == 0xED) {
                    $result .= "\xED\xED\x01" . $byte;
                    $i += 2;
                } else {
                    $result .= $data[$i];
                    $i++;
                }
            }
        }

        return $result;
    }

    /**********************************************************************************************************************************/
    /* Z80 Header Parsing Module                                                                                                    */
    /**********************************************************************************************************************************/

    private function parseZ80Header($handle) {
        fseek($handle, 0);
        $header = $this->readBytes($handle, 30);

        if ($header === false || strlen($header) < 30) {
            return false;
        }

        if (strlen($header) == 30) {
            return ['version' => 145, 'data' => $header];
        } else {
            return false;
        }
    }

    private function getZ80v145Info($data) {
        return [
            'stat1' => ord($data[6]),
            'isCompressed' => (ord($data[6]) & 0x20) != 0,
            'hasAdditionalData' => (ord($data[6]) & 0x20) != 0
        ];
    }

    private function getZ80v201Info($data) {
        if (strlen($data) < 31) {
            return ['hwMode' => 0];
        }
        $pc = ord($data[29]) | (ord($data[30]) << 8);
        if (strlen($data) > 31) {
            return ['hwMode' => ord($data[34])];
        }
        return ['hwMode' => 0, 'pc' => $pc];
    }

    /**********************************************************************************************************************************/
    /* SNA Header Parsing Module                                                                                                    */
    /**********************************************************************************************************************************/

    private function parseSNAHeader($handle) {
        fseek($handle, 0);
        $header = $this->readBytes($handle, 27);

        if ($header === false || strlen($header) < 27) {
            return false;
        }

        return [
            'i' => ord($header[0]),
            'hl_prime' => $this->getWord($header, 1),
            'de_prime' => $this->getWord($header, 3),
            'bc_prime' => $this->getWord($header, 5),
            'af_prime' => $this->getWord($header, 7),
            'hl' => $this->getWord($header, 9),
            'de' => $this->getWord($header, 11),
            'bc' => $this->getWord($header, 13),
            'iy' => $this->getWord($header, 15),
            'ix' => $this->getWord($header, 17),
            'iff2' => (ord($header[19]) & 0x04) ? 1 : 0,
            'r' => ord($header[20]),
            'af' => $this->getWord($header, 21),
            'sp' => $this->getWord($header, 23),
            'im' => ord($header[26])
        ];
    }

    /**********************************************************************************************************************************/
    /* Bank Auto-Detection Module                                                                                                    */
    /**********************************************************************************************************************************/

    private function autoBank($address, $snapshotType, $z80Version, $hwMode) {
        if ($snapshotType == 'SNA') {
            return 8;
        }

        if ($z80Version == 145) {
            return 0;
        }

        if ($z80Version == 201 && $hwMode < 3) {
            if ($address < 32768) return 5;
            if ($address < 49152) return 1;
            return 2;
        }

        if ($z80Version == 300 && $hwMode < 4) {
            if ($address < 32768) return 5;
            if ($address < 49152) return 1;
            return 2;
        }

        if ($address < 32768) return 5;
        if ($address < 49152) return 2;
        return 0;
    }

    /**********************************************************************************************************************************/
    /* POKE File Parser Module                                                                                                      */
    /**********************************************************************************************************************************/

    private function parsePokesFile($filePath) {
        if (!file_exists($filePath)) {
            return false;
        }

        $content = file_get_contents($filePath);
        if ($content === false) {
            return false;
        }

        $lines = explode("\n", $content);
        $pokeGroups = [];
        $currentGroup = null;

        foreach ($lines as $line) {
            $line = rtrim($line);
            if (strlen($line) == 0) continue;

            $type = $line[0];

            if ($type == 'N') {
                if ($currentGroup !== null && count($currentGroup['pokes']) > 0) {
                    $pokeGroups[] = $currentGroup;
                }
                $name = trim(substr($line, 1));
                $currentGroup = ['name' => $name, 'pokes' => []];
            } elseif ($type == 'M' || $type == 'Z') {
                if ($currentGroup === null) continue;

                $parts = preg_split('/\s+/', trim(substr($line, 1)));
                if (count($parts) >= 4) {
                    $poke = [
                        'bank' => intval($parts[0]),
                        'address' => intval($parts[1]),
                        'value' => intval($parts[2]),
                        'org' => intval($parts[3])
                    ];
                    $currentGroup['pokes'][] = $poke;
                }
            } elseif ($type == 'Y') {
                if ($currentGroup !== null && count($currentGroup['pokes']) > 0) {
                    $pokeGroups[] = $currentGroup;
                }
                break;
            }
        }

        if ($currentGroup !== null && count($currentGroup['pokes']) > 0) {
            $pokeGroups[] = $currentGroup;
        }

        return $pokeGroups;
    }

    private function parseCommandLinePokes($pokesString, $snapshotType, $z80Version, $hwMode) {
        $entries = explode(':', $pokesString);
        $pokes = [];

        foreach ($entries as $entry) {
            $parts = explode(',', $entry);
            if (count($parts) == 2) {
                $address = intval($parts[0]);
                $value = intval($parts[1]);

                if ($address >= 16384 && $address <= 65535 && $value >= 0 && $value <= 255) {
                    $bank = $this->autoBank($address, $snapshotType, $z80Version, $hwMode);
                    $pokes[] = [
                        'bank' => $bank,
                        'address' => $address,
                        'value' => $value,
                        'org' => 0
                    ];
                }
            }
        }

        return [['name' => 'Command Line Pokes', 'pokes' => $pokes]];
    }

    /**********************************************************************************************************************************/
    /* Poke Application Module                                                                                                      */
    /**********************************************************************************************************************************/

    private function applyPokesToMemory(&$memory, $pokes, $bank = null) {
        $appliedCount = 0;
        foreach ($pokes as $poke) {
            if ($poke['address'] < 16384 || $poke['address'] > 65535) {
                $this->logError("Invalid address: " . $poke['address']);
                continue;
            }

            $value = $poke['value'];
            if ($value == 256 || $value == 'ASK' || strtoupper($value) == 'ASK') {
                $value = 244;
            }

            if ($value < 0 || $value > 255) {
                $this->logError("Invalid value: " . $value . " at address " . $poke['address']);
                continue;
            }

            $offset = $poke['address'] - 16384;
            if ($offset >= 0 && $offset < strlen($memory)) {
                $memory[$offset] = chr($value);
                $appliedCount++;
            } else {
                $this->logError("Address out of memory range: " . $poke['address']);
            }
        }
        return $appliedCount;
    }

    private function applyPokesToSNA($inputFile, $outputFile, $pokeGroups, $selectedGroups) {
        $inputHandle = fopen($inputFile, 'rb');
        if (!$inputHandle) {
            $this->logError("Cannot open input SNA file: $inputFile");
            return false;
        }

        $header = $this->readBytes($inputHandle, 27);
        if ($header === false) {
            fclose($inputHandle);
            $this->logError("Cannot read SNA header from: $inputFile");
            return false;
        }

        $memory = $this->readBytes($inputHandle, 49152);
        fclose($inputHandle);

        if ($memory === false) {
            $this->logError("Cannot read SNA memory from: $inputFile");
            return false;
        }

        $totalApplied = 0;
        foreach ($pokeGroups as $index => $group) {
            if ($selectedGroups === null || in_array($index + 1, $selectedGroups)) {
                $totalApplied += $this->applyPokesToMemory($memory, $group['pokes'], 8);
            }
        }

        $outputHandle = fopen($outputFile, 'wb');
        if (!$outputHandle) {
            $this->logError("Cannot create output SNA file: $outputFile");
            return false;
        }

        fwrite($outputHandle, $header);
        fwrite($outputHandle, $memory);
        fclose($outputHandle);

        return ['success' => true, 'pokes_applied' => $totalApplied];
    }

    private function applyPokesToZ80v145($inputFile, $outputFile, $pokeGroups, $selectedGroups) {
        $inputHandle = fopen($inputFile, 'rb');
        if (!$inputHandle) {
            $this->logError("Cannot open input Z80 file: $inputFile");
            return false;
        }

        $header = $this->readBytes($inputHandle, 30);
        if ($header === false) {
            fclose($inputHandle);
            $this->logError("Cannot read Z80 header from: $inputFile");
            return false;
        }

        $headerInfo = $this->getZ80v145Info($header);
        $header[6] = chr(ord($header[6]) | 0x20);

        $memoryData = fread($inputHandle, filesize($inputFile) - 30);
        fclose($inputHandle);

        if ($headerInfo['isCompressed']) {
            $memory = $this->decrunchZ80Block($memoryData, 49152);
        } else {
            $memory = $memoryData;
            if (strlen($memory) < 49152) {
                $memory .= str_repeat("\0", 49152 - strlen($memory));
            }
        }

        $totalApplied = 0;
        foreach ($pokeGroups as $index => $group) {
            if ($selectedGroups === null || in_array($index + 1, $selectedGroups)) {
                $totalApplied += $this->applyPokesToMemory($memory, $group['pokes'], 0);
            }
        }

        $compressedMemory = $this->crunchZ80Block($memory);

        $outputHandle = fopen($outputFile, 'wb');
        if (!$outputHandle) {
            $this->logError("Cannot create output Z80 file: $outputFile");
            return false;
        }

        fwrite($outputHandle, $header);
        fwrite($outputHandle, $compressedMemory);
        fclose($outputHandle);

        return ['success' => true, 'pokes_applied' => $totalApplied];
    }

    private function applyPokesToZ80v201v300($inputFile, $outputFile, $pokeGroups, $selectedGroups, $version) {
        $inputHandle = fopen($inputFile, 'rb');
        if (!$inputHandle) {
            $this->logError("Cannot open input Z80 file: $inputFile");
            return false;
        }

        $header30 = $this->readBytes($inputHandle, 30);
        if ($header30 === false) {
            fclose($inputHandle);
            $this->logError("Cannot read Z80 header from: $inputFile");
            return false;
        }

        $header201 = $this->readBytes($inputHandle, 2);
        if ($header201 === false) {
            fclose($inputHandle);
            $this->logError("Cannot read Z80 v201 header from: $inputFile");
            return false;
        }

        $pc = ord($header201[0]) | (ord($header201[1]) << 8);

        $header201v2 = $this->readBytes($inputHandle, 2);
        $hwMode = 0;
        $hasExtendedHeader = false;

        if ($header201v2 !== false && strlen($header201v2) == 2 && ord($header201v2[0]) == 0xFF && ord($header201v2[1]) == 0xFF) {
            $hasExtendedHeader = true;
            $extHeader = $this->readBytes($inputHandle, 3);
            if ($extHeader !== false && strlen($extHeader) >= 3) {
                $hwMode = ord($extHeader[0]);
            }
        }

        $header30[6] = chr(ord($header30[6]) | 0x20);

        $outputHandle = fopen($outputFile, 'wb');
        if (!$outputHandle) {
            fclose($inputHandle);
            $this->logError("Cannot create output Z80 file: $outputFile");
            return false;
        }

        fwrite($outputHandle, $header30);
        fwrite($outputHandle, $header201);

        if ($hasExtendedHeader) {
            fwrite($outputHandle, $header201v2);
            fwrite($outputHandle, $extHeader);
        }

        $totalApplied = 0;
        $bankPokes = [];

        foreach ($pokeGroups as $index => $group) {
            if ($selectedGroups === null || in_array($index + 1, $selectedGroups)) {
                foreach ($group['pokes'] as $poke) {
                    $bank = $poke['bank'];
                    if ($bank >= 8) {
                        $bank = $this->autoBank($poke['address'], 'Z80', $version, $hwMode);
                    }
                    if (!isset($bankPokes[$bank])) {
                        $bankPokes[$bank] = [];
                    }
                    $bankPokes[$bank][] = $poke;
                }
            }
        }

        while (true) {
            $blockHeader = $this->readBytes($inputHandle, 3);
            if ($blockHeader === false || strlen($blockHeader) < 3) {
                break;
            }

            $length = ord($blockHeader[0]) | (ord($blockHeader[1]) << 8);
            $bank = ord($blockHeader[2]);

            if ($length == 0) {
                fwrite($outputHandle, $blockHeader);
                break;
            }

            $blockData = $this->readBytes($inputHandle, $length);
            if ($blockData === false) {
                fclose($inputHandle);
                fclose($outputHandle);
                $this->logError("Cannot read Z80 memory block");
                return false;
            }

            $memory = $this->decrunchZ80Block($blockData, 16384);

            if (isset($bankPokes[$bank])) {
                $totalApplied += $this->applyPokesToMemory($memory, $bankPokes[$bank], $bank);
            }

            $compressedBlock = $this->crunchZ80Block($memory);
            $compressedLen = strlen($compressedBlock);

            if ($compressedLen >= 16384) {
                $compressedLen = 0xFFFF;
                $compressedBlock = $memory;
            }

            fwrite($outputHandle, chr($compressedLen & 0xFF) . chr(($compressedLen >> 8) & 0xFF) . chr($bank));
            fwrite($outputHandle, $compressedBlock);
        }

        fclose($inputHandle);
        fclose($outputHandle);

        return ['success' => true, 'pokes_applied' => $totalApplied];
    }

    /**********************************************************************************************************************************/
    /* Main Processing Function                                                                                                      */
    /**********************************************************************************************************************************/

    public function processPokes($filePath, $commandLinePokes = null, $pokelistselect = null) {
        if (!file_exists($filePath)) {
            $this->logError("File not found: $filePath");
            return ['success' => false, 'error' => 'File not found: ' . $filePath];
        }

        $pathInfo = pathinfo($filePath);
        $extension = strtoupper($pathInfo['extension']);
        $directory = $pathInfo['dirname'];
        $filename = $pathInfo['filename'];

        if ($extension != 'Z80' && $extension != 'SNA') {
            $this->logError("Unsupported file type: $extension");
            return ['success' => false, 'error' => 'Unsupported file type: ' . $extension];
        }

        $pokeGroups = null;
        $z80Version = 145;
        $hwMode = 0;
        $snapshotType = $extension;

        if ($commandLinePokes !== null) {
            $pokeGroups = $this->parseCommandLinePokes($commandLinePokes, $snapshotType, $z80Version, $hwMode);
        } else {
            $pokeFile = $directory . '/POKES/' . $filename . '.pok';
            if (!file_exists($pokeFile)) {
                $pokeFile = $directory . '/' . $filename . '.pok';
            }

            if (file_exists($pokeFile)) {
                $pokeGroups = $this->parsePokesFile($pokeFile);
                if ($pokeGroups === false) {
                    $this->logError("Error parsing POKES file: $pokeFile");
                    return ['success' => false, 'error' => 'Error parsing POKES file'];
                }

                if ($extension == 'Z80') {
                    $inputHandle = fopen($filePath, 'rb');
                    if ($inputHandle) {
                        $header = $this->parseZ80Header($inputHandle);
                        if ($header) {
                            $z80Version = $header['version'];
                            if ($z80Version >= 201) {
                                $info = $this->getZ80v201Info($header['data']);
                                $hwMode = $info['hwMode'];
                            }
                        }
                        fclose($inputHandle);
                    }
                }
            } else {
                $this->logError("POKES file not found for: $filePath");
                return ['success' => false, 'error' => 'POKES file not found'];
            }
        }

        if ($pokeGroups === null || count($pokeGroups) == 0) {
            $this->logError("No pokes to apply");
            return ['success' => false, 'error' => 'No pokes to apply'];
        }

        $selectedGroups = null;
        if ($pokelistselect !== null) {
            $parts = explode(',', $pokelistselect);
            $selectedGroups = array_map('intval', $parts);
        }

        $outputFile = $directory . '/' . $filename . '_POKED.' . strtolower($extension);

        if (!is_writable($directory)) {
            $outputFile = dirname(__FILE__) . '/' . $filename . '_POKED.' . strtolower($extension);
        }

        $result = null;

        if ($extension == 'SNA') {
            $result = $this->applyPokesToSNA($filePath, $outputFile, $pokeGroups, $selectedGroups);
        } elseif ($extension == 'Z80') {
            if ($z80Version == 145) {
                $result = $this->applyPokesToZ80v145($filePath, $outputFile, $pokeGroups, $selectedGroups);
            } else {
                $result = $this->applyPokesToZ80v201v300($filePath, $outputFile, $pokeGroups, $selectedGroups, $z80Version);
            }
        }

        if ($result === false || !$result['success']) {
            $this->logError("Error applying pokes");
            return ['success' => false, 'error' => 'Error applying pokes'];
        }

        $result['output'] = $outputFile;
        return $result;
    }
}

/**********************************************************************************************************************************/
/* Main Entry Point                                                                                                                */
/**********************************************************************************************************************************/

$startTime = microtime(true);

$file = isset($_GET['file']) ? $_GET['file'] : (isset($argv[1]) ? $argv[1] : null);
$POKES = isset($_GET['POKES']) ? $_GET['POKES'] : null;
$pokelistselect = isset($_GET['pokelistselect']) ? $_GET['pokelistselect'] : null;
$logerrors = isset($_GET['logerrors']) ? $_GET['logerrors'] : null;

if ($POKES !== null && strpos($POKES, 'POKES=') === 0) {
    $POKES = substr($POKES, 6);
}

if ($POKES === null && php_sapi_name() === 'cli') {
    for ($i = 2; $i < $argc; $i++) {
        if (strpos($argv[$i], 'POKES=') === 0) {
            $POKES = substr($argv[$i], 6);
        } elseif (strpos($argv[$i], 'pokelistselect=') === 0) {
            $pokelistselect = substr($argv[$i], 15);
        } elseif (strpos($argv[$i], 'logerrors=') === 0) {
            $logerrors = substr($argv[$i], 10);
        }
    }
}

if ($POKES !== null && strpos($POKES, 'POKES=') === 0) {
    $POKES = substr($POKES, 6);
}

$pokeSgd = new PokeSgd($logerrors);
$result = $pokeSgd->processPokes($file, $POKES, $pokelistselect);

$endTime = microtime(true);
$executionTime = round(($endTime - $startTime) * 1000, 2);

if (php_sapi_name() === 'cli') {
    if ($result['success']) {
        echo "SUCCESS: Pokes applied successfully\n";
        echo "Output: " . $result['output'] . "\n";
        echo "Pokes applied: " . $result['pokes_applied'] . "\n";
        echo "Execution time: " . $executionTime . "ms\n";
    } else {
        echo "ERROR: " . $result['error'] . "\n";
        exit(1);
    }
} else {
    header('Content-Type: application/json');
    echo json_encode($result);
}
