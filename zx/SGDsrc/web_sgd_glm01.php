<?php
/**********************************************************************************************************************************/
/* Project      : SGD Web                                                                                                          */
/* Module       : Main Application                                                                                                   */
/* Description  : Single-file web application for Spectrum Games Database                                                            */
/* Version      : glm01                                                                                                             */
/* Last changed : 30-12-2025                                                                                                        */
/* PHP Version  : 8.x                                                                                                               */
/*                                                                                                                                  */
/**********************************************************************************************************************************/

session_start();

/*==================================================================================================================================*/
/* CONFIGURATION                                                                                                                     */
/*==================================================================================================================================*/

define('DAT_FILE', 'HAD_ZX.DAT');
define('INI_FILE', 'ROMSMINE/_SGD304/sgd.ini');

define('QAOP_PATH', 'QAOP/qaop.html');
define('ROMSMINE_PATH', 'ROMSMINE/');
define('SGD_FILEPATH_PREFIX', 'E:\OPT\SPECTRUM\ROMSMINE');
define('SGD_FILEPATH_PREFIX2', 'E:/OPT/SPECTRUM/ROMSMINE/');

$EDIT_USERS = [
    'admin' => 'admin123',
    'editor' => 'editor456'
];

$fieldDefinitions = [
    'Name' => ['start' => 1, 'length' => 36],
    'Year' => ['start' => 38, 'length' => 4],
    'Publisher' => ['start' => 43, 'length' => 36],
    'Memory' => ['start' => 80, 'length' => 3],
    'NoOfPlayers' => ['start' => 84, 'length' => 1],
    'Together' => ['start' => 86, 'length' => 1],
    'Joysticks' => ['start' => 88, 'length' => 5],
    'PC-Name' => ['start' => 94, 'length' => 12],
    'Type' => ['start' => 107, 'length' => 7],
    'PathIndex' => ['start' => 115, 'length' => 3],
    'FileSize' => ['start' => 119, 'length' => 7],
    'Orig_screen' => ['start' => 127, 'length' => 1],
    'Origin' => ['start' => 129, 'length' => 1],
    'FloppyId' => ['start' => 131, 'length' => 4],
    'Emul_override' => ['start' => 136, 'length' => 2],
    'AYSound' => ['start' => 139, 'length' => 1],
    'MultiLoad' => ['start' => 141, 'length' => 1],
    'Language' => ['start' => 143, 'length' => 3],
    'Score' => ['start' => 147, 'length' => 3],
    'Author' => ['start' => 151, 'length' => 100],
];

/*==================================================================================================================================*/
/* HELPER FUNCTIONS                                                                                                                  */
/*==================================================================================================================================*/

function getField($line, $field) {
    global $fieldDefinitions;
    $start = $fieldDefinitions[$field]['start'];
    $length = $fieldDefinitions[$field]['length'];
    $line = str_replace("'", " ", $line);
    
    if ($field === 'Memory' || $field === 'Score') {
        $value = trim(substr($line, $start - 1, $length));
        return str_pad($value, 3, ' ', STR_PAD_LEFT);
    }
    if ($field === 'FloppyId') {
        $value = trim(substr($line, $start - 1, $length));
        return str_pad($value, 4, ' ', STR_PAD_LEFT);
    }
    if ($field === 'FileSize') {
        $value = trim(substr($line, $start - 1, $length));
        return str_pad($value, 7, ' ', STR_PAD_LEFT);
    }
    return trim(substr($line, $start - 1, $length));
}

function setField($line, $field, $value) {
    global $fieldDefinitions;
    $start = $fieldDefinitions[$field]['start'] - 1;
    $length = $fieldDefinitions[$field]['length'];
    
    if ($field === 'Memory' || $field === 'Score') {
        $value = str_pad($value, 3, ' ', STR_PAD_LEFT);
    } elseif ($field === 'FloppyId') {
        $value = str_pad($value, 4, ' ', STR_PAD_LEFT);
    } elseif ($field === 'FileSize') {
        $value = str_pad($value, 7, ' ', STR_PAD_LEFT);
    }
    
    $paddedValue = str_pad(substr($value, 0, $length), $length, ' ');
    return substr_replace($line, $paddedValue, $start, $length);
}

function parseDatabaseLine($line) {
    global $fieldDefinitions;
    $game = [];
    
    foreach ($fieldDefinitions as $field => $definition) {
        $game[$field] = getField($line, $field);
    }
    
    return $game;
}

function formatDatabaseLine($data) {
    global $fieldDefinitions;
    $line = str_repeat(' ', 252);
    
    foreach ($fieldDefinitions as $field => $definition) {
        $line = setField($line, $field, $data[$field]);
    }
    
    return $line . "\r\n";
}

function readDatabase() {
    global $fieldDefinitions;
    
    $games = [];
    
    if (!file_exists(DAT_FILE)) {
        return $games;
    }
    
    $file = fopen(DAT_FILE, 'r');
    if ($file) {
        while (($line = fgets($file)) !== false) {
            if (strlen(trim($line)) > 0) {
                $games[] = parseDatabaseLine($line);
            }
        }
        fclose($file);
    }
    
    return $games;
}

function backupDatabase($filepath) {
    $backupPath = $filepath . '_' . date('ymd_Hi') . '.dat';
    return copy($filepath, $backupPath);
}

function writeDatabase($entries, $expectedMtime = null) {
    if (!file_exists(DAT_FILE)) {
        return ['success' => false, 'error' => 'Database file not found'];
    }
    
    if ($expectedMtime !== null && filemtime(DAT_FILE) !== $expectedMtime) {
        return ['success' => false, 'error' => 'Database modified by another user. Please reload the page.'];
    }
    
    if (!backupDatabase(DAT_FILE)) {
        return ['success' => false, 'error' => 'Failed to create backup'];
    }
    
    $file = fopen(DAT_FILE, 'w');
    if ($file) {
        foreach ($entries as $entry) {
            fwrite($file, formatDatabaseLine($entry));
        }
        fclose($file);
        return ['success' => true];
    }
    
    return ['success' => false, 'error' => 'Failed to write database'];
}

function loadGameDirs() {
    $gameDirs = [];
    
    if (!file_exists(INI_FILE)) {
        return $gameDirs;
    }
    
    if ($fileHandle = fopen(INI_FILE, 'r')) {
        while (($line = fgets($fileHandle)) !== false) {
            $line = trim($line);
            if (strpos($line, 'GameDir') === 0) {
                $parts = explode('=', $line);
                if (count($parts) === 2) {
                    $path = trim($parts[1]);
                    $gameDirs[] = $path;
                }
            }
        }
        fclose($fileHandle);
    }
    
    return $gameDirs;
}

function resolveLetterDir($pathIndex, $gameDirs) {
    global $SGD_FILEPATH_PREFIX2;
    
    $letterDir = '';
    $pathIndex = intval($pathIndex);
    
    if ($pathIndex >= 0 && $pathIndex < count($gameDirs)) {
        $fullPath = trim($gameDirs[$pathIndex]);
        $prefix = $SGD_FILEPATH_PREFIX2;
        
        if (strpos($fullPath, $prefix) === 0) {
            $letterDir = substr($fullPath, strlen($prefix));
            $letterDir = trim($letterDir, '/\\');
            $letterDir = str_replace('\\', '/', $letterDir);
        } else {
            $parts = explode(DIRECTORY_SEPARATOR, $fullPath);
            $letterDir = end($parts);
            if (empty($letterDir)) {
                $letterDir = $fullPath;
            }
        }
    }
    
    return $letterDir;
}

function getActualFilename($dir, $filename, $pathIndex, $subfolder = "", $gameDirs) {
    $known_ext = ["sna", "z80", "TAP", "slt"];
    
    if ($pathIndex > 100) {
        return $filename;
    }
    
    $path_parts = pathinfo($filename);
    $filenamebase = $path_parts['filename'];
    
    $normalizedPath = str_replace("\\", "/", trim($gameDirs[$pathIndex]));
    $DOSprefixPath = "E:/OPT/SPECTRUM/ROMSMINE/";
    $lastSlashPos = strrpos($normalizedPath, $DOSprefixPath);
    
    if ($lastSlashPos !== false) {
        $lastFolder = substr($normalizedPath, $lastSlashPos + strlen($DOSprefixPath));
    } else {
        $lastFolder = $normalizedPath;
    }
    
    $scanDir = $dir . $lastFolder . '/' . $subfolder . '/';
    $files = @scandir($scanDir);
    
    if (!$files) {
        return $filenamebase . ".SNA";
    }
    
    foreach ($files as $file) {
        foreach ($known_ext as $extension) {
            $fullFilename = $filenamebase . '.' . $extension;
            
            if (str_replace(' ', '', strtolower($file)) == str_replace(' ', '', strtolower($fullFilename))) {
                return $file;
            }
        }
    }
    
    return $fullFilename;
}

function getActualExtrasFilename($dir, $filename, $pathIndex, $subfolder = "INLAYS", $gameDirs) {
    $known_ext = ["gif", "jpg", "png", "scr", "pok", "txt", "SCR"];
    
    if ($pathIndex > 100) {
        return $filename;
    }
    
    $path_parts = pathinfo($filename);
    $filenamebase = $path_parts['filename'];
    
    $normalizedPath = str_replace("\\", "/", trim($gameDirs[$pathIndex]));
    $DOSprefixPath = "E:/OPT/SPECTRUM/ROMSMINE/";
    $lastSlashPos = strrpos($normalizedPath, $DOSprefixPath);
    
    if ($lastSlashPos !== false) {
        $lastFolder = substr($normalizedPath, $lastSlashPos + strlen($DOSprefixPath));
    } else {
        $lastFolder = $normalizedPath;
    }
    
    $scanDir = $dir . $lastFolder . '/' . $subfolder . '/';
    $files = @scandir($scanDir);
    
    if (!$files) {
        return $filenamebase . ".GIF";
    }
    
    foreach ($files as $file) {
        foreach ($known_ext as $extension) {
            $fullFilename = $filenamebase . '.' . $extension;
            
            if (str_replace(' ', '', strtolower($file)) == str_replace(' ', '', strtolower($fullFilename))) {
                return $file;
            }
        }
    }
    
    return $filenamebase . ".GIF";
}

function getGameFiles($filename, $pathIndex, $gameDirs) {
    $letterDir = resolveLetterDir($pathIndex, $gameDirs);
    
    $actualFilename = getActualFilename("./ROMSMINE/", $filename, $pathIndex, "", $gameDirs);
    $actualInlayFilename = getActualExtrasFilename("./ROMSMINE/", $filename, $pathIndex, "INLAYS", $gameDirs);
    $actualTxtFilename = getActualExtrasFilename("./ROMSMINE/", $filename, $pathIndex, "INFO", $gameDirs);
    $actualSCRSHOTFilename = getActualExtrasFilename("./ROMSMINE/", $filename, $pathIndex, "SCRSHOT", $gameDirs);
    
    $basePath = '';
    $scriptName = $_SERVER['SCRIPT_NAME'];
    $scriptDir = dirname($scriptName);
    $relativeBase = ($scriptDir === '/' || $scriptDir === '.') ? '' : $scriptDir . '/';
    
    $inlayPath = $relativeBase . 'ROMSMINE/' . $letterDir . '/INLAYS/' . $actualInlayFilename;
    $scrshotPath = $relativeBase . 'scr2png.php?scrimg=' . 'ROMSMINE/' . $letterDir . '/SCRSHOT/' . $actualSCRSHOTFilename;
    $txtPath = $relativeBase . 'ROMSMINE/' . $letterDir . '/INFO/' . $actualTxtFilename;
    
    return [
        'success' => true,
        'actualFilename' => $actualFilename,
        'actualInlayFilename' => $actualInlayFilename,
        'actualTxtFilename' => $actualTxtFilename,
        'actualScrshotFilename' => $actualSCRSHOTFilename,
        'letterDir' => $letterDir,
        'basePath' => $basePath,
        'inlayPath' => $inlayPath,
        'scrshotPath' => $scrshotPath,
        'txtPath' => $txtPath,
        'gamePath' => 'ROMSMINE/' . $letterDir
    ];
}

function checkAuth() {
    return isset($_SESSION['authenticated']) && $_SESSION['authenticated'] === true;
}

function login($username, $password) {
    global $EDIT_USERS;
    
    if (isset($EDIT_USERS[$username]) && $EDIT_USERS[$username] === $password) {
        $_SESSION['authenticated'] = true;
        $_SESSION['username'] = $username;
        return ['success' => true, 'username' => $username];
    }
    
    return ['success' => false, 'error' => 'Invalid username or password'];
}

function logout() {
    $_SESSION = [];
    session_destroy();
    return ['success' => true];
}

/*==================================================================================================================================*/
/* API ENDPOINTS                                                                                                                     */
/*==================================================================================================================================*/

if ($_SERVER['REQUEST_METHOD'] === 'GET' && isset($_GET['api'])) {
    header('Content-Type: application/json');
    
    switch ($_GET['api']) {
        case 'list':
            $games = readDatabase();
            $mtime = file_exists(DAT_FILE) ? filemtime(DAT_FILE) : 0;
            echo json_encode(['success' => true, 'games' => $games, 'mtime' => $mtime]);
            exit;
            
        case 'get':
            $index = intval($_GET['index']);
            $games = readDatabase();
            if (isset($games[$index])) {
                echo json_encode(['success' => true, 'game' => $games[$index]]);
            } else {
                echo json_encode(['success' => false, 'error' => 'Game not found']);
            }
            exit;
            
        case 'getFilenames':
            $filename = $_GET['filename'];
            $pathIndex = intval($_GET['pathIndex']);
            $gameDirs = loadGameDirs();
            $result = getGameFiles($filename, $pathIndex, $gameDirs);
            echo json_encode($result);
            exit;
            
        case 'auth':
            echo json_encode(['success' => true, 'authenticated' => checkAuth(), 'username' => $_SESSION['username'] ?? null]);
            exit;
            
        case 'gameDirs':
            echo json_encode(['success' => true, 'dirs' => loadGameDirs()]);
            exit;
            
        default:
            echo json_encode(['success' => false, 'error' => 'Unknown API endpoint']);
            exit;
    }
}

if ($_SERVER['REQUEST_METHOD'] === 'POST') {
    header('Content-Type: application/json');
    $input = json_decode(file_get_contents('php://input'), true);
    
    switch ($_GET['api'] ?? '') {
        case 'login':
            $result = login($input['username'], $input['password']);
            echo json_encode($result);
            exit;
            
        case 'logout':
            $result = logout();
            echo json_encode($result);
            exit;
            
        case 'update':
            if (!checkAuth()) {
                echo json_encode(['success' => false, 'error' => 'Unauthorized']);
                exit;
            }
            
            $index = intval($input['index']);
            $expectedMtime = isset($input['mtime']) ? intval($input['mtime']) : null;
            
            $games = readDatabase();
            
            if ($index < 0 || $index >= count($games)) {
                echo json_encode(['success' => false, 'error' => 'Invalid game index']);
                exit;
            }
            
            foreach ($input['game'] as $field => $value) {
                if (isset($games[$index][$field])) {
                    $games[$index][$field] = $value;
                }
            }
            
            $result = writeDatabase($games, $expectedMtime);
            echo json_encode($result);
            exit;
            
        default:
            echo json_encode(['success' => false, 'error' => 'Unknown API endpoint']);
            exit;
    }
}

?>
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>SGD Web - Spectrum Games Database</title>
    <style>
        * {
            box-sizing: border-box;
            margin: 0;
            padding: 0;
        }
        
        body {
            font-family: 'Courier New', monospace;
            background-color: #f0f0f0;
            display: flex;
            flex-direction: column;
            min-height: 100vh;
        }
        
        header {
            background-color: #2c3e50;
            color: white;
            padding: 15px 20px;
            display: flex;
            justify-content: space-between;
            align-items: center;
            box-shadow: 0 2px 4px rgba(0,0,0,0.1);
        }
        
        header h1 {
            margin: 0;
            font-size: 24px;
        }
        
        #auth-bar {
            display: flex;
            gap: 10px;
            align-items: center;
        }
        
        #auth-bar button {
            padding: 8px 15px;
            border: none;
            border-radius: 4px;
            cursor: pointer;
            background-color: white;
            color: #2c3e50;
            font-weight: bold;
            transition: background-color 0.3s;
        }
        
        #auth-bar button:hover {
            background-color: #ecf0f1;
        }
        
        main {
            display: flex;
            flex: 1;
            padding: 20px;
            gap: 20px;
            overflow: hidden;
        }
        
        #game-list-panel {
            flex: 2;
            background-color: white;
            border-radius: 8px;
            padding: 15px;
            box-shadow: 0 2px 8px rgba(0,0,0,0.1);
            display: flex;
            flex-direction: column;
            overflow: hidden;
        }
        
        #game-detail-panel {
            flex: 1;
            background-color: #e8e8e8;
            border-radius: 8px;
            padding: 15px;
            box-shadow: 0 2px 8px rgba(0,0,0,0.1);
            display: flex;
            flex-direction: column;
            overflow: hidden;
        }
        
        h2 {
            margin-bottom: 15px;
            color: #2c3e50;
            border-bottom: 2px solid #3498db;
            padding-bottom: 5px;
        }
        
        #filters {
            display: flex;
            gap: 10px;
            margin-bottom: 15px;
            flex-wrap: wrap;
        }
        
        #filters input {
            padding: 8px 12px;
            border: 1px solid #ccc;
            border-radius: 4px;
            font-family: 'Courier New', monospace;
            min-width: 120px;
        }
        
        #filters button {
            padding: 8px 15px;
            border: none;
            border-radius: 4px;
            background-color: #e74c3c;
            color: white;
            cursor: pointer;
            transition: background-color 0.3s;
        }
        
        #filters button:hover {
            background-color: #c0392b;
        }
        
        #games-container {
            flex: 1;
            overflow-y: auto;
        }
        
        table {
            width: 100%;
            border-collapse: collapse;
        }
        
        th {
            background-color: #34495e;
            color: white;
            text-align: left;
            padding: 12px 8px;
            position: sticky;
            top: 0;
            z-index: 10;
        }
        
        td {
            padding: 10px 8px;
            border-bottom: 1px solid #ecf0f1;
        }
        
        tbody tr {
            cursor: pointer;
            transition: background-color 0.2s;
        }
        
        tbody tr:hover {
            background-color: #ecf0f1;
        }
        
        tbody tr.selected {
            background-color: #d4edda;
            border-left: 4px solid #28a745;
        }
        
        #detail-container {
            flex: 1;
            overflow-y: auto;
        }
        
        #action-buttons {
            display: flex;
            gap: 10px;
            margin-bottom: 15px;
        }
        
        #action-buttons button {
            padding: 10px 20px;
            border: none;
            border-radius: 4px;
            cursor: pointer;
            font-weight: bold;
            transition: transform 0.1s;
        }
        
        #action-buttons button:active {
            transform: scale(0.98);
        }
        
        #play-btn {
            background-color: #28a745;
            color: white;
            flex: 1;
        }
        
        #save-btn {
            background-color: #007bff;
            color: white;
            flex: 1;
        }
        
        #action-buttons button:disabled {
            background-color: #95a5a6;
            cursor: not-allowed;
            transform: none;
        }
        
        #game-form {
            margin-bottom: 20px;
        }
        
        .form-row {
            display: flex;
            margin-bottom: 10px;
            align-items: center;
            flex-wrap: wrap;
            gap: 5px;
        }
        
        .form-row label {
            min-width: 100px;
            font-weight: bold;
            color: #2c3e50;
        }
        
        .form-row input {
            padding: 6px 8px;
            border: 1px solid #ccc;
            border-radius: 4px;
            font-family: 'Courier New', monospace;
            background-color: white;
            max-width: 200px;
        }
        
        .form-row input[type="text"] {
            min-width: 100px;
        }
        
        .form-row input:disabled {
            background-color: #f8f9fa;
            cursor: not-allowed;
            color: #6c757d;
        }
        
        #path-display {
            background-color: #fff3cd;
            border: 1px solid #ffc107;
            border-radius: 4px;
            padding: 8px;
            margin-bottom: 15px;
            font-size: 13px;
            color: #856404;
        }
        
        #media-panel {
            margin-top: 15px;
            padding-top: 15px;
            border-top: 2px solid #dee2e6;
        }
        
        #inlay-container,
        #scrshot-container {
            margin-bottom: 15px;
        }
        
        #inlay-container img,
        #scrshot-container img {
            width: 300px;
            height: 200px;
            object-fit: cover;
            cursor: pointer;
            border: 2px solid #dee2e6;
            border-radius: 5px;
            transition: all 0.3s;
            display: none;
        }
        
        #inlay-container img:hover,
        #scrshot-container img:hover {
            border-color: #007bff;
            box-shadow: 0 4px 8px rgba(0,0,0,0.2);
        }
        
        #info-container {
            background-color: white;
            padding: 15px;
            border: 1px solid #dee2e6;
            border-radius: 5px;
        }
        
        #info-text {
            white-space: pre-wrap;
            max-height: 200px;
            overflow-y: auto;
            font-family: Arial, sans-serif;
            font-size: 14px;
            line-height: 1.6;
            color: #333;
        }
        
        #error-message {
            background-color: #f8d7da;
            border: 1px solid #f5c6cb;
            border-radius: 4px;
            padding: 12px;
            margin-bottom: 15px;
            color: #721c24;
            display: none;
        }
        
        #success-message {
            background-color: #d4edda;
            border: 1px solid #c3e6cb;
            border-radius: 4px;
            padding: 12px;
            margin-bottom: 15px;
            color: #155724;
            display: none;
        }
        
        .modal-overlay {
            position: fixed;
            top: 0;
            left: 0;
            width: 100%;
            height: 100%;
            background-color: rgba(0,0,0,0.5);
            z-index: 1000;
            display: none;
        }
        
        .modal-content {
            background-color: white;
            margin: 10% auto;
            padding: 25px;
            border-radius: 8px;
            width: 90%;
            max-width: 400px;
            box-shadow: 0 4px 20px rgba(0,0,0,0.3);
            position: fixed;
            top: 10%;
            left: 50%;
            transform: translateX(-50%);
        }
        
        .modal-content.large {
            max-width: 90%;
            width: 90%;
            height: 80vh;
            padding: 15px;
        }
        
        .modal-content h2 {
            margin-top: 0;
            color: #2c3e50;
        }
        
        #login-form input {
            width: 100%;
            padding: 12px;
            margin-bottom: 15px;
            border: 1px solid #ddd;
            border-radius: 4px;
            font-family: 'Courier New', monospace;
            box-sizing: border-box;
        }
        
        #login-form button {
            width: 100%;
            padding: 12px;
            margin-bottom: 10px;
            border: none;
            border-radius: 4px;
            cursor: pointer;
            font-weight: bold;
        }
        
        #login-form button[type="submit"] {
            background-color: #28a745;
            color: white;
        }
        
        #login-form button[type="submit"]:hover {
            background-color: #218838;
        }
        
        #login-form button[type="button"] {
            background-color: #6c757d;
            color: white;
        }
        
        #emulator-frame {
            width: 100%;
            height: calc(100% - 40px);
            border: 2px solid #2c3e50;
            border-radius: 4px;
            background-color: #000;
        }
        
        #close-emulator {
            position: absolute;
            top: 20px;
            right: 20px;
            padding: 10px 20px;
            background-color: #e74c3c;
            color: white;
            border: none;
            border-radius: 4px;
            cursor: pointer;
            font-weight: bold;
        }
        
        #loading {
            text-align: center;
            padding: 40px;
            color: #6c757d;
            font-size: 18px;
        }
        
        #user-display {
            font-weight: bold;
        }
    </style>
</head>
<body>
    <header>
        <h1>SGD Web - Spectrum Games Database</h1>
        <div id="auth-bar">
            <span id="user-display"></span>
            <button id="login-btn">Login</button>
            <button id="logout-btn" style="display:none">Logout</button>
        </div>
    </header>

    <div id="error-message"></div>
    <div id="success-message"></div>

    <main>
        <div id="game-list-panel">
            <h2>Game List</h2>
            <div id="filters">
                <input type="text" id="filter-name" placeholder="Filter by name">
                <input type="text" id="filter-type" placeholder="Filter by type">
                <input type="text" id="filter-year" placeholder="Filter by year">
                <input type="text" id="filter-letter" placeholder="First letter">
                <button onclick="clearFilters()">Clear</button>
            </div>
            <div id="games-container">
                <div id="loading">Loading games...</div>
                <table id="games-table" style="display:none">
                    <thead>
                        <tr>
                            <th>Name</th>
                            <th>Type</th>
                            <th>Year</th>
                            <th>PC-Name</th>
                            <th>Author</th>
                        </tr>
                    </thead>
                    <tbody id="games-body">
                    </tbody>
                </table>
            </div>
        </div>

        <div id="game-detail-panel">
            <h2>Game Details</h2>
            <div id="detail-container">
                <div id="action-buttons">
                    <button id="play-btn" disabled>PLAY (QAOP)</button>
                    <button id="save-btn" style="display:none">SAVE</button>
                </div>

                <form id="game-form">
                    <input type="hidden" id="form-index" value="-1">
                    
                    <div class="form-row">
                        <label>Name:</label>
                        <input type="text" id="form-Name" maxlength="36" disabled>
                    </div>
                    
                    <div class="form-row">
                        <label>Publisher:</label>
                        <input type="text" id="form-Publisher" maxlength="36" disabled>
                    </div>
                    
                    <div class="form-row">
                        <label>Author:</label>
                        <input type="text" id="form-Author" maxlength="100" disabled>
                    </div>
                    
                    <div class="form-row">
                        <label>Year:</label>
                        <input type="text" id="form-Year" maxlength="4" disabled>
                        <label>Memory:</label>
                        <input type="text" id="form-Memory" maxlength="3" disabled>
                    </div>
                    
                    <div class="form-row">
                        <label>Players:</label>
                        <input type="text" id="form-NoOfPlayers" maxlength="1" disabled>
                        <label>Together:</label>
                        <input type="text" id="form-Together" maxlength="1" disabled>
                        <label>Score:</label>
                        <input type="text" id="form-Score" maxlength="3" disabled>
                    </div>
                    
                    <div class="form-row">
                        <label>PC-Name:</label>
                        <input type="text" id="form-PC-Name" maxlength="12" disabled>
                        <label>Type:</label>
                        <input type="text" id="form-Type" maxlength="7" disabled>
                    </div>
                    
                    <div class="form-row">
                        <label>Joysticks:</label>
                        <input type="text" id="form-Joysticks" maxlength="5" disabled>
                    </div>
                    
                    <div class="form-row">
                        <label>Orig_screen:</label>
                        <input type="text" id="form-Orig_screen" maxlength="1" disabled>
                        <label>Origin:</label>
                        <input type="text" id="form-Origin" maxlength="1" disabled>
                    </div>
                    
                    <div class="form-row">
                        <label>FloppyId:</label>
                        <input type="text" id="form-FloppyId" maxlength="4" disabled>
                    </div>
                    
                    <div class="form-row">
                        <label>Emul_override:</label>
                        <input type="text" id="form-Emul_override" maxlength="2" disabled>
                        <label>AYSound:</label>
                        <input type="text" id="form-AYSound" maxlength="1" disabled>
                    </div>
                    
                    <div class="form-row">
                        <label>MultiLoad:</label>
                        <input type="text" id="form-MultiLoad" maxlength="1" disabled>
                        <label>Language:</label>
                        <input type="text" id="form-Language" maxlength="3" disabled>
                    </div>
                    
                    <div class="form-row">
                        <label>PathIndex:</label>
                        <input type="text" id="form-PathIndex" maxlength="3" disabled>
                        <label>FileSize:</label>
                        <input type="text" id="form-FileSize" maxlength="7" disabled>
                    </div>
                    
                    <div id="path-display"></div>
                </form>

                <div id="media-panel">
                    <div id="inlay-container">
                        <img id="inlay-img" alt="Inlay">
                    </div>
                    
                    <div id="scrshot-container">
                        <img id="scrshot-img" alt="Screenshot">
                    </div>
                    
                    <div id="info-container">
                        <pre id="info-text">Select a game to view details</pre>
                    </div>
                </div>
            </div>
        </div>
    </main>

    <div id="login-modal" class="modal-overlay">
        <div class="modal-content">
            <h2>Login</h2>
            <form id="login-form">
                <input type="text" id="username" placeholder="Username" required>
                <input type="password" id="password" placeholder="Password" required>
                <button type="submit">Login</button>
                <button type="button" onclick="closeLoginModal()">Cancel</button>
            </form>
        </div>
    </div>

    <div id="emulator-modal" class="modal-overlay">
        <div class="modal-content large">
            <button id="close-emulator" onclick="closeEmulator()">Close Emulator</button>
            <iframe id="emulator-frame" src="" allowfullscreen></iframe>
        </div>
    </div>

    <script>
    const state = {
        games: [],
        filteredGames: [],
        selectedGame: null,
        isAuthenticated: false,
        username: null,
        gameDirs: [],
        selectedGamePath: null,
        dbMtime: 0
    };

    async function apiCall(endpoint, method = 'GET', data = null) {
        const options = {
            method: method,
            headers: {
                'Content-Type': 'application/json'
            }
        };
        
        if (data) {
            options.body = JSON.stringify(data);
        }
        
        try {
            const response = await fetch('?api=' + endpoint, options);
            return await response.json();
        } catch (error) {
            console.error('API call failed:', error);
            return { success: false, error: 'Network error' };
        }
    }

    function showError(message) {
        const errorDiv = document.getElementById('error-message');
        const successDiv = document.getElementById('success-message');
        
        errorDiv.textContent = message;
        errorDiv.style.display = 'block';
        successDiv.style.display = 'none';
        
        setTimeout(() => {
            errorDiv.style.display = 'none';
        }, 5000);
    }

    function showSuccess(message) {
        const successDiv = document.getElementById('success-message');
        const errorDiv = document.getElementById('error-message');
        
        successDiv.textContent = message;
        successDiv.style.display = 'block';
        errorDiv.style.display = 'none';
        
        setTimeout(() => {
            successDiv.style.display = 'none';
        }, 3000);
    }

    async function init() {
        await loadGames();
        await loadGameDirs();
        await checkAuth();
        renderGames();
    }

    async function loadGames() {
        const response = await apiCall('list');
        if (response.success) {
            state.games = response.games;
            state.filteredGames = [...state.games];
            state.dbMtime = response.mtime || 0;
            
            document.getElementById('loading').style.display = 'none';
            document.getElementById('games-table').style.display = 'table';
        } else {
            showError('Failed to load games: ' + (response.error || 'Unknown error'));
        }
    }

    async function loadGameDirs() {
        const response = await apiCall('gameDirs');
        if (response.success) {
            state.gameDirs = response.dirs;
        }
    }

    async function checkAuth() {
        const response = await apiCall('auth');
        if (response.success) {
            state.isAuthenticated = response.authenticated;
            state.username = response.username;
            updateAuthUI();
        }
    }

    function updateAuthUI() {
        const loginBtn = document.getElementById('login-btn');
        const logoutBtn = document.getElementById('logout-btn');
        const userDisplay = document.getElementById('user-display');
        
        if (state.isAuthenticated) {
            loginBtn.style.display = 'none';
            logoutBtn.style.display = 'inline';
            userDisplay.textContent = 'Welcome, ' + state.username;
            enableEditing(true);
        } else {
            loginBtn.style.display = 'inline';
            logoutBtn.style.display = 'none';
            userDisplay.textContent = '';
            enableEditing(false);
        }
    }

    function enableEditing(enabled) {
        const saveBtn = document.getElementById('save-btn');
        const formInputs = document.querySelectorAll('#game-form input:not([type="hidden"])');
        
        if (enabled && state.selectedGame) {
            saveBtn.style.display = 'inline';
            formInputs.forEach(input => input.disabled = false);
        } else {
            saveBtn.style.display = 'none';
            formInputs.forEach(input => input.disabled = true);
        }
    }

    function renderGames() {
        const tbody = document.getElementById('games-body');
        tbody.innerHTML = '';
        
        state.filteredGames.forEach((game) => {
            const originalIndex = state.games.indexOf(game);
            const tr = document.createElement('tr');
            tr.dataset.index = originalIndex;
            tr.innerHTML = `
                <td><strong>${escapeHtml(game.Name)}</strong></td>
                <td>${escapeHtml(game.Type)}</td>
                <td>${escapeHtml(game.Year)}</td>
                <td>${escapeHtml(game['PC-Name'])}</td>
                <td>${escapeHtml(game.Author)}</td>
            `;
            
            tr.addEventListener('click', () => selectGame(originalIndex));
            tbody.appendChild(tr);
        });
    }

    function escapeHtml(text) {
        const div = document.createElement('div');
        div.textContent = text;
        return div.innerHTML;
    }

    async function selectGame(index) {
        document.querySelectorAll('#games-body tr').forEach(tr => {
            tr.classList.remove('selected');
        });
        
        const selectedRow = document.querySelector('tr[data-index="' + index + '"]');
        if (selectedRow) {
            selectedRow.classList.add('selected');
        }
        
        const game = state.games[index];
        state.selectedGame = game;
        
        Object.keys(game).forEach(field => {
            const input = document.getElementById('form-' + field);
            if (input) {
                input.value = game[field];
            }
        });
        
        document.getElementById('form-index').value = index;
        
        document.getElementById('play-btn').disabled = false;
        if (state.isAuthenticated) {
            enableEditing(true);
        }
        
        await loadGameMedia(game);
    }

    async function loadGameMedia(game) {
        const response = await apiCall(
            'getFilenames&filename=' + encodeURIComponent(game['PC-Name']) + '&pathIndex=' + game['PathIndex']
        );
        
        if (response.success) {
            state.selectedGamePath = response.gamePath;
            
            const pathDisplay = document.getElementById('path-display');
            pathDisplay.textContent = 'Directory: ' + (response.letterDir || 'Unknown');
            
            const inlayImg = document.getElementById('inlay-img');
            if (response.inlayPath) {
                inlayImg.src = response.inlayPath;
                inlayImg.onload = () => inlayImg.style.display = 'block';
                inlayImg.onerror = () => inlayImg.style.display = 'none';
            }
            
            const scrshotImg = document.getElementById('scrshot-img');
            if (response.scrshotPath) {
                scrshotImg.src = response.scrshotPath;
                scrshotImg.onload = () => scrshotImg.style.display = 'block';
                scrshotImg.onerror = () => scrshotImg.style.display = 'none';
            }
            
            const infoText = document.getElementById('info-text');
            if (response.txtPath) {
                try {
                    const infoResponse = await fetch(response.txtPath);
                    const text = await infoResponse.text();
                    infoText.textContent = text;
                } catch (e) {
                    infoText.textContent = 'No description available';
                }
            } else {
                infoText.textContent = 'No description available';
            }
        }
    }

    function filterGames() {
        const nameFilter = document.getElementById('filter-name').value.toLowerCase();
        const typeFilter = document.getElementById('filter-type').value.toLowerCase();
        const yearFilter = document.getElementById('filter-year').value.toLowerCase();
        const letterFilter = document.getElementById('filter-letter').value.toLowerCase();
        
        state.filteredGames = state.games.filter(game => {
            const nameMatch = game.Name.toLowerCase().includes(nameFilter);
            const typeMatch = game.Type.toLowerCase().includes(typeFilter);
            const yearMatch = game.Year.includes(yearFilter);
            const letterMatch = !letterFilter || game.Name.toLowerCase().startsWith(letterFilter);
            
            return nameMatch && typeMatch && yearMatch && letterMatch;
        });
        
        renderGames();
    }

    function clearFilters() {
        document.getElementById('filter-name').value = '';
        document.getElementById('filter-type').value = '';
        document.getElementById('filter-year').value = '';
        document.getElementById('filter-letter').value = '';
        filterGames();
    }

    function showLoginModal() {
        document.getElementById('login-modal').style.display = 'block';
    }

    function closeLoginModal() {
        document.getElementById('login-modal').style.display = 'none';
        document.getElementById('login-form').reset();
    }

    async function handleLogin(event) {
        event.preventDefault();
        
        const username = document.getElementById('username').value;
        const password = document.getElementById('password').value;
        
        const response = await apiCall('login', 'POST', { username: username, password: password });
        
        if (response.success) {
            state.isAuthenticated = true;
            state.username = response.username;
            updateAuthUI();
            closeLoginModal();
            showSuccess('Logged in successfully as ' + response.username);
        } else {
            showError(response.error || 'Login failed');
        }
    }

    async function handleLogout() {
        const response = await apiCall('logout', 'POST');
        if (response.success) {
            state.isAuthenticated = false;
            state.username = null;
            updateAuthUI();
            showSuccess('Logged out successfully');
        }
    }

    async function saveGame() {
        const index = parseInt(document.getElementById('form-index').value);
        
        const gameData = {};
        const inputs = document.querySelectorAll('#game-form input:not([type="hidden"])');
        inputs.forEach(input => {
            const field = input.id.replace('form-', '');
            gameData[field] = input.value;
        });
        
        const response = await apiCall('update', 'POST', {
            index: index,
            mtime: state.dbMtime,
            game: gameData
        });
        
        if (response.success) {
            showSuccess('Game saved successfully!');
            await loadGames();
            renderGames();
            if (state.selectedGame) {
                const newIndex = state.games.findIndex(g => g['PC-Name'] === gameData['PC-Name']);
                if (newIndex >= 0) {
                    selectGame(newIndex);
                }
            }
        } else {
            showError(response.error || 'Save failed');
        }
    }

    function launchEmulator() {
        if (!state.selectedGame || !state.selectedGamePath) {
            showError('No game selected');
            return;
        }
        
        const pcName = document.getElementById('form-PC-Name').value;
        const pathIndex = state.selectedGame['PathIndex'];
        
        const actualFilename = getActualFilenameJS(pcName, pathIndex);
        const gamePath = state.selectedGamePath + '/' + actualFilename;
        
        const emulatorUrl = 'QAOP/qaop.html?#l=' + encodeURIComponent(gamePath);
        
        document.getElementById('emulator-frame').src = emulatorUrl;
        document.getElementById('emulator-modal').style.display = 'block';
    }

    function getActualFilenameJS(filename, pathIndex) {
        return filename;
    }

    function closeEmulator() {
        document.getElementById('emulator-modal').style.display = 'none';
        document.getElementById('emulator-frame').src = '';
    }

    document.addEventListener('DOMContentLoaded', () => {
        init();
        
        document.getElementById('filter-name').addEventListener('input', filterGames);
        document.getElementById('filter-type').addEventListener('input', filterGames);
        document.getElementById('filter-year').addEventListener('input', filterGames);
        document.getElementById('filter-letter').addEventListener('input', filterGames);
        
        document.getElementById('login-btn').addEventListener('click', showLoginModal);
        document.getElementById('logout-btn').addEventListener('click', handleLogout);
        document.getElementById('login-form').addEventListener('submit', handleLogin);
        
        document.getElementById('play-btn').addEventListener('click', launchEmulator);
        document.getElementById('save-btn').addEventListener('click', saveGame);
        
        document.getElementById('inlay-img').addEventListener('click', function() {
            this.style.width = this.style.width === '100%' ? '300px' : '100%';
            this.style.height = this.style.width === '100%' ? 'auto' : '200px';
            this.style.objectFit = this.style.width === '100%' ? 'contain' : 'cover';
        });
        
        document.getElementById('scrshot-img').addEventListener('click', function() {
            this.style.width = this.style.width === '100%' ? '300px' : '100%';
            this.style.height = this.style.width === '100%' ? 'auto' : '200px';
            this.style.objectFit = this.style.width === '100%' ? 'contain' : 'cover';
        });
        
        document.getElementById('login-modal').addEventListener('click', function(e) {
            if (e.target === this) {
                closeLoginModal();
            }
        });
    });
    </script>
</body>
</html>
