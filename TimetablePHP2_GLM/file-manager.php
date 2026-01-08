<?php
// if ($_SERVER['REQUEST_METHOD'] === 'POST') { echo "11111111111111111111111111";}

//++++++++++++++++++++++++ File Management+++++++++++++++++++++++++++++++++
 echo "<h3>";print_r($_REQUEST);echo "</h3>";
function displayTimetableManagement($folderPath) {
    // Handle form submissions
    if ($_SERVER['REQUEST_METHOD'] === 'POST') {
        //if ($_SERVER['REQUEST_METHOD'] === 'POST') { echo "<hr>222222222222222222222222<hr>";}
        // Get the list of selected files
        $selectedFiles = isset($_POST['selected_files']) ? $_POST['selected_files'] : [];
        echo "<h3>";print_r($_REQUEST);echo "</h3>";
        // Identify which button was pressed
        if (isset($_POST['delete'])) {
            // Delete selected files
            foreach ($selectedFiles as $filename) {
                $filePath = $folderPath . '/' . basename($filename);
                if (file_exists($filePath)) {
                    unlink($filePath);
                }
            }
        } elseif (isset($_POST['add_postfix'])) {
            $postfix = trim($_POST['postfix']);
            if ($postfix !== '' && !empty($selectedFiles)) {
                foreach ($selectedFiles as $filename) {
                    $originalPath = $folderPath . '/' . basename($filename);
                    if (file_exists($originalPath)) {
                        $pathInfo = pathinfo($originalPath);
                        $newName = $pathInfo['filename'] . '_' . $postfix . '.' . $pathInfo['extension'];
                        rename($originalPath, $folderPath . '/' . $newName);
                    }
                }
            }
        } elseif (isset($_POST['clone'])) {
            $clonePostfix = trim($_POST['postfix']);
            if ($clonePostfix !== '' && !empty($selectedFiles)) {
                foreach ($selectedFiles as $filename) {
                    $originalPath = $folderPath . '/' . basename($filename);
                    if (file_exists($originalPath)) {
                        $pathInfo = pathinfo($originalPath);
                        $newName = $pathInfo['filename'] . '_' . $clonePostfix . '.' . $pathInfo['extension'];
                        copy($originalPath, $folderPath . '/' . $newName);
                    }
                }
            }
        }
    }

    // List matching files
    $files = glob($folderPath . '/timetable_*.csv');

    // Generate HTML
    echo '<h2>Timetables Files</h2>';

    echo '<form method="post">';
    echo '<table border="1" cellpadding="5" cellspacing="0">';
    echo '<tr>
            <th>Select</th>
            <th>Filename</th>
          </tr>';

    foreach ($files as $file) {
        $filename = basename($file);
        echo '<tr>
                <td>
                    <input type="checkbox" name="selected_files[]" value="' . htmlspecialchars($filename) . '">
                </td>
                <td>' . htmlspecialchars($filename) . '</td>
              </tr>';
    }
    echo '</table>';

    // Buttons for actions
    echo '<br>';

    // Delete button
    echo '<button type="submit" name="delete">Delete Selected</button>';

    // Add postfix section
    echo '<h3>Add Postfix to Files</h3>';
    echo '<input type="text" name="postfix" value="_v02" placeholder="Postfix">';
    echo '<button type="submit" name="add_postfix">Add Postfix</button>';

    echo '<button type="submit" name="clone">Clone Selected Files</button>';

    echo '</form>';
}

displayTimetableManagement(".");