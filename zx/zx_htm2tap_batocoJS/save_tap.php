<?php
// save_tap.php - Saves TAP file from JavaScript to server and opens emulator

if ($_SERVER['REQUEST_METHOD'] === 'POST') {
    $target_subfolder = "word_saved_data/";
    $fileName = $_POST['fileName'] ?? "program";
    $tapData = $_POST['tapData'] ?? "";
    
    // Decode base64 data
    $tapData = base64_decode($tapData);
    
    // Save TAP file
    $tapFileName = $target_subfolder . $fileName . ".tap";
    file_put_contents($tapFileName, $tapData);
    
    // Save HTML file if provided
    if (isset($_POST['htmlData'])) {
        $htmlData = base64_decode($_POST['htmlData']);
        $htmlFileName = $target_subfolder . $fileName . ".htm";
        file_put_contents($htmlFileName, $htmlData);
    }
    
    // Return success response
    echo json_encode([
        'success' => true,
        'tapFile' => $tapFileName,
        'htmlFile' => $htmlFileName ?? null
    ]);
} else {
    echo json_encode(['success' => false, 'error' => 'Invalid request']);
}
?>
