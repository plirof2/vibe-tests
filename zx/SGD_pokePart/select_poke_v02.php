<?php
$romfolder="ROMSMINE";
function scanRomFiles() {
    $results = [];
    
    $romsDir = __DIR__ . '/ROMSMINE';
    
    $iterator = new RecursiveIteratorIterator(
        new RecursiveDirectoryIterator($romsDir, RecursiveDirectoryIterator::SKIP_DOTS),
        RecursiveIteratorIterator::SELF_FIRST
    );
    
    foreach ($iterator as $file) {
        if ($file->isFile()) {
            $extension = strtolower($file->getExtension());
            
            if ($extension === 'z80' || $extension === 'sna') {
                $filePath = $file->getPathname();
                $pathInfo = pathinfo($filePath);
                $fileName = $pathInfo['filename'];
                $fileDir = $pathInfo['dirname'];
                
                $pokFilePaths = [
                    $fileDir . '/POKES/' . $fileName . '.POK',
                    $fileDir . '/POKES/' . $fileName . '.pok'
                ];
                
                $pokFilePath = null;
                foreach ($pokFilePaths as $path) {
                    if (file_exists($path)) {
                        $pokFilePath = $path;
                        break;
                    }
                }
                
                if ($pokFilePath) {
                    $relativePath = str_replace($romsDir . '/', '', $filePath);
                    $results[] = [
                        'file' => $relativePath,
                        'full_path' => $filePath,
                        'pok_file' => $pokFilePath,
                        'name' => $fileName
                    ];
                }
            }
        }
    }
    
    return $results;
}

function parsePOKFile($pokFilePath) {
    $pokeGroups = [];
    
    if (!file_exists($pokFilePath)) {
        return $pokeGroups;
    }
    
    $lines = file($pokFilePath, FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);
    
    if ($lines === false) {
        return $pokeGroups;
    }
    
    $currentName = 'Unnamed POKE';
    $currentPokes = [];
    
    foreach ($lines as $line) {
        $line = trim($line);
        
        if (empty($line)) {
            continue;
        }
        
        $firstChar = substr($line, 0, 1);
        
        if ($firstChar === 'N') {
            if (!empty($currentPokes)) {
                $pokeGroups[] = [
                    'name' => $currentName,
                    'pokes' => $currentPokes
                ];
                $currentPokes = [];
            }
            $currentName = trim(substr($line, 1));
            continue;
        }
        
        if ($firstChar === 'Y') {
            if (!empty($currentPokes)) {
                $pokeGroups[] = [
                    'name' => $currentName,
                    'pokes' => $currentPokes
                ];
            }
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
                
                if ($address >= 16384 && $address <= 65535) {
                    $currentPokes[] = [
                        'address' => $address,
                        'value' => $set,
                        'original' => $org
                    ];
                }
            }
        }
    }
    
    if (!empty($currentPokes)) {
        $pokeGroups[] = [
            'name' => $currentName,
            'pokes' => $currentPokes
        ];
    }
    
    return $pokeGroups;
}

function getDescription($pokFilePath) {
    if (!file_exists($pokFilePath)) {
        return '';
    }
    
    $lines = file($pokFilePath, FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);
    
    if ($lines === false) {
        return '';
    }
    
    foreach ($lines as $line) {
        $line = trim($line);
        
        if (substr($line, 0, 1) === 'N') {
            return trim(substr($line, 1));
        }
    }
    
    return '';
}

$romFiles = scanRomFiles();
$selectedFile = $_GET['file'] ?? '';
$selectedPokes = $_GET['selected_pokes'] ?? '';

if ($selectedFile) {
    foreach ($romFiles as $rom) {
        if ($rom['file'] === $selectedFile) {
            $pokes = parsePOKFile($rom['pok_file']);
            break;
        }
    }
}
?>
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <title>Select POKE</title>
    <style>
        body { font-family: Arial, sans-serif; margin: 20px; }
        h1 { color: #333; }
        table { border-collapse: collapse; width: 100%; margin-top: 20px; }
        th, td { border: 1px solid #ddd; padding: 8px; text-align: left; }
        th { background-color: #4CAF50; color: white; }
        tr:hover { background-color: #f5f5f5; cursor: pointer; }
        .selected { background-color: #ffff99; }
        .container { display: flex; gap: 20px; }
        .rom-list { flex: 1; }
        .poke-details { flex: 1; }
        .poke-item { padding: 10px; margin: 5px 0; background-color: #f9f9f9; border-radius: 4px; }
        .poke-item input[type="checkbox"] { margin-right: 10px; }
        .url-output { margin-top: 20px; padding: 15px; background-color: #e7f3fe; border-left: 6px solid #2196F3; }
        .url-output a { color: #2196F3; text-decoration: none; font-weight: bold; }
        .url-output a:hover { text-decoration: underline; }
        .description { color: #666; font-style: italic; margin-bottom: 5px; }
        .no-pokes { color: #999; font-style: italic; }
    </style>
</head>
<body>
    <h1>Select POKE for ZX Spectrum Games</h1>
    
    <?php if ($selectedFile && isset($pokes)): ?>
        <p><strong>Selected:</strong> <?php echo htmlspecialchars($selectedFile); ?></p>
        <p><a href="select_poke.php">← Back to list</a></p>
        
        <?php if (!empty($pokes)): ?>
            <h2>Select POKES</h2>
            <form method="GET" action="select_poke.php">
                <input type="hidden" name="file" value="<?php echo htmlspecialchars($selectedFile); ?>">
                
                <?php $pokeIndex = 0; ?>
                <?php foreach ($pokes as $group): ?>
                    <h3 style="color: #2196F3; margin-top: 20px;">
                        <input type="checkbox" 
                               id="group_<?php echo $pokeIndex; ?>"
                               onchange="toggleGroup(<?php echo $pokeIndex; ?>)">
                        <label for="group_<?php echo $pokeIndex; ?>" style="cursor: pointer;">
                            <?php echo htmlspecialchars($group['name']); ?>
                        </label>
                    </h3>
                    
                    <?php foreach ($group['pokes'] as $poke): ?>
                        <div class="poke-item">
                            <input type="checkbox" 
                                   name="poke_<?php echo $pokeIndex; ?>" 
                                   id="poke_<?php echo $pokeIndex; ?>"
                                   data-group="<?php echo $pokeIndex; ?>"
                                   <?php echo isset($_GET['poke_' . $pokeIndex]) ? 'checked' : ''; ?>
                                   onchange="updateURL()">
                            <label for="poke_<?php echo $pokeIndex; ?>">
                                <strong>Address:</strong> <?php echo $poke['address']; ?>, 
                                <strong>Value:</strong> <?php echo $poke['value']; ?>
                                <?php if ($poke['original'] > 0): ?>
                                    (Original: <?php echo $poke['original']; ?>)
                                <?php endif; ?>
                            </label>
                        </div>
                        <?php $pokeIndex++; ?>
                    <?php endforeach; ?>
                <?php endforeach; ?>
                
                <div class="url-output">
                    <strong>URL:</strong><br>
                    <a id="generatedURL" href="" target="_blank">Generate URL by selecting POKES above</a>
                </div>
            </form>
            
            <script>
            const pokeData = [
                <?php 
                $pokeIndex = 0;
                $pokeData = [];
                foreach ($pokes as $group) {
                    foreach ($group['pokes'] as $poke) {
                        $pokeData[] = "{ index: $pokeIndex, address: {$poke['address']}, value: {$poke['value']} }";
                        $pokeIndex++;
                    }
                }
                echo implode(",\n                ", $pokeData);
                ?>
            ];

            function toggleGroup(groupIndex) {
                const groupCheckbox = document.getElementById('group_' + groupIndex);
                const checkboxes = document.querySelectorAll('input[data-group="' + groupIndex + '"]');
                
                checkboxes.forEach(cb => {
                    cb.checked = groupCheckbox.checked;
                });
                
                updateURL();
            }
            
            function updateURL() {
                const params = [];
                let pokeCount = 0;
                
                pokeData.forEach(poke => {
                    const checkbox = document.getElementById('poke_' + poke.index);
                    if (checkbox && checkbox.checked) {
                        params.push(poke.address + ',' + poke.value);
                        pokeCount++;
                    }
                });
                
                const pokesString = params.join(':');
                const url = 'pokesgd.php?file=<?php echo $romfolder."/";echo $selectedFile; ?>&POKES=' + pokesString;
                
                const linkElement = document.getElementById('generatedURL');
                if (pokeCount > 0) {
                    linkElement.href = url;
                    linkElement.textContent = url;
                } else {
                    linkElement.href = '';
                    linkElement.textContent = 'Generate URL by selecting POKES above';
                }
            }
            
            document.addEventListener('DOMContentLoaded', updateURL);
            </script>
        <?php else: ?>
            <p class="no-pokes">No POKES found in this file.</p>
        <?php endif; ?>
        
    <?php else: ?>
        <h2>Available ROM Files with POKES</h2>
        
        <?php if (empty($romFiles)): ?>
            <p class="no-pokes">No ROM files with associated POKES found in ROMSMINE/ directory.</p>
        <?php else: ?>
            <table>
                <thead>
                    <tr>
                        <th>Game Name</th>
                        <th>File</th>
                        <th>Type</th>
                    </tr>
                </thead>
                <tbody>
                    <?php foreach ($romFiles as $rom): ?>
                        <tr onclick="window.location.href='?file=<?php echo urlencode($rom['file']); ?>'">
                            <td>
                                <strong><?php echo htmlspecialchars($rom['name']); ?></strong><br>
                                <span class="description"><?php echo htmlspecialchars(getDescription($rom['pok_file'])); ?></span>
                            </td>
                            <td><?php echo htmlspecialchars($rom['file']); ?></td>
                            <td><?php echo strtoupper(pathinfo($rom['file'], PATHINFO_EXTENSION)); ?></td>
                        </tr>
                    <?php endforeach; ?>
                </tbody>
            </table>
            <p>Total files found: <?php echo count($romFiles); ?></p>
        <?php endif; ?>
    <?php endif; ?>
    
</body>
</html>