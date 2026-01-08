<?php
session_start();
$version="v006b_251225 - multi-user support";

// Check if user is logged in
if(!isset($_SESSION['user'])) {
  header("Location: timetable044b.php");
  exit;
}

$current_user = $_SESSION['user'];
//ini_set('display_errors', 1);
//error_reporting(E_ALL);
/*
NOTE: MOVE works - just online version has refresh issues. Same code works locally

v005e 251003- Disabled caching so that move works on remote server
v005d 250930- Initial placement/generation of timetable when I press button
v004 added css
v003
v002
v001_250928a
*/

// Paths - user-specific files
$username = $current_user['username'];
$classesFile = $username . '_classes.csv';
$teachersFile = $username . '_teachers.csv';

// If user files don't exist, use default files as templates
if(!file_exists($classesFile) && file_exists('classes.csv')) {
  copy('classes.csv', $classesFile);
}
if(!file_exists($teachersFile) && file_exists('teachers.csv')) {
  copy('teachers.csv', $teachersFile);
}

$classname=[];

header("Cache-Control: no-store, no-cache, must-revalidate, max-age=0");
header("Pragma: no-cache");
header("Expires: 0");

// Load CSV, ignoring the header
function load_csv_ignore_header($file) {
    if (!file_exists($file)) return [];
    $rows = [];
    if (($handle = fopen($file, "r")) !== false) {
        $header = fgetcsv($handle); // read header and ignore
        while (($data = fgetcsv($handle)) !== false) {
            $rows[] = $data;
        }

        fclose($handle);
        return ['header' => $header, 'rows' => $rows];
    }
    return ['header' => [], 'rows' => []];
}

// Save CSV with header
function save_csv_with_header($file, $header, $rows) {
    if (($handle = fopen($file, "w")) !== false) {
        fputcsv($handle, $header);
        foreach ($rows as $row) {
            fputcsv($handle, $row);
        }
        fclose($handle);
        return true;
    }
    return false;
}

// Load data
$classData = load_csv_ignore_header($classesFile);
//print_r($classData);
$teacherData = load_csv_ignore_header($teachersFile);
//print_r($teacherData);
$classes = $classData['rows'];
$teachers = $teacherData['rows'];
$classHeader = $classData['header'];
$teacherHeader = $teacherData['header'];

// Get params
$action = $_GET['action'] ?? '';
$type = $_GET['type'] ?? '';
$id = $_GET['id'] ?? '';
$move_dir = $_GET['move'] ?? '';

// Helper: move entry
function move_entry(&$array, $id, $direction) {
    $index = null;
    foreach ($array as $i => $row) {
        if ($row[0] == $id) { $index = $i; break; }
    }
    if ($index === null) return;
    if ($direction == 'up' && $index > 0) {
        $temp = $array[$index - 1];
        $array[$index - 1] = $array[$index];
        $array[$index] = $temp;
    } elseif ($direction == 'down' && $index < count($array) - 1) {
        $temp = $array[$index + 1];
        $array[$index + 1] = $array[$index];
        $array[$index] = $temp;
    }
}

// Handle move
if ($action == 'move' && in_array($type, ['class', 'teacher']) && in_array($move_dir, ['up', 'down'])) {
    if ($type == 'class') {
        move_entry($classes, $id, $move_dir);
        save_csv_with_header($classesFile, $classHeader, $classes);
    } elseif ($type == 'teacher') {
        move_entry($teachers, $id, $move_dir);
        save_csv_with_header($teachersFile, $teacherHeader, $teachers);
    }
    header("Location: " . $_SERVER['PHP_SELF']);
    exit;
}

// Handle delete
if ($action == 'delete') {
    if ($type == 'class') {
        //$classes = array_filter($classes, fn($c) => $c[0] != $id); //php 7.4+
        $classes = array_filter($classes, function($c) use ($id) { return $c[0] != $id;        }); //older PHP
        save_csv_with_header($classesFile, $classHeader, array_values($classes));
    } elseif ($type == 'teacher') {
        //$teachers = array_filter($teachers, fn($t) => $t[0] != $id);//php 7.4+
        $teachers = array_filter($teachers, function($t) use ($id) { return $t[0] != $id;        }); //older PHP
        save_csv_with_header($teachersFile, $teacherHeader, array_values($teachers));
    }
    header("Location: " . $_SERVER['PHP_SELF']);
    exit;
}

// Handle add/update
if ($_SERVER['REQUEST_METHOD'] === 'POST') {
    if ($_POST['type'] == 'class') {
        $class_id = trim($_POST['class_id']);
        $description = trim($_POST['description']);
        $original_id = $_POST['original_class_id'] ?? '';

        if ($original_id) {
            // Update existing
            foreach ($classes as &$c) {
                if ($c[0] == $original_id) {
                    $c[0] = $class_id;
                    $c[1] = $description;
                    break;
                }
            }
        } else {
            // Add new
            $classes[] = [$class_id, $description];
        }
        save_csv_with_header($classesFile, $classHeader, $classes);
    } elseif ($_POST['type'] == 'teacher') {
        $teacher_id = trim($_POST['teacher_id']);
        $name = trim($_POST['name']);
        $max_hours = intval($_POST['max_hours']);
        $unavailable_days = isset($_POST['unavailable_days']) ? implode('|', $_POST['unavailable_days']) : '';
        $color = trim($_POST['color']);
        $eidikotita = trim($_POST['eidikotita']);
        $original_id = $_POST['original_teacher_id'] ?? '';

        if ($original_id) {
            foreach ($teachers as &$t) {
                if ($t[0] == $original_id) {
                    $t[0] = $teacher_id;
                    $t[1] = $name;
                    $t[2] = $max_hours;
                    $t[3] = $unavailable_days;
                    $t[4] = $color;
                    $t[5] = $eidikotita;
                    break;
                }
            }
        } else {
            $teachers[] = [$teacher_id, $name, $max_hours, $unavailable_days, $color, $eidikotita];
        }
        save_csv_with_header($teachersFile, $teacherHeader, $teachers);
    }
    header("Location: " . $_SERVER['PHP_SELF']);
    exit;
}

// Find for editing
function find_in_array($array, $id) {
    foreach ($array as $row) {
        if ($row[0] == $id) return $row;
    }
    return null;
}

// Define days of the week for checkboxes
$daysOfWeek = ['Mon', 'Tue', 'Wed', 'Thu', 'Fri'];
$selectedDays = [];
if ($type == 'teacher' && isset($id)) {
    $teacher = find_in_array($teachers, $id);
    if ($teacher && isset($teacher[3])) {
        $selectedDays = explode('|', $teacher[3]);
    }
}
?>
<!DOCTYPE html>
<html>
<head>
    <title>Manage Classes & Teachers</title>
    
    
    <style>
        /* Added CSS for a prettier look */

        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            background-color: #f4f6f8;
            color: #333;
            margin: 20px;
            line-height: 1.6;
        }

        h1 {
            text-align: center;
            color: #2c3e50;
            margin-bottom: 30px;
        }

        h2 {
            margin-top: 40px;
            color: #34495e;
            border-bottom: 2px solid #ccc;
            padding-bottom: 5px;
        }

        table {
            width: 100%;
            border-collapse: collapse;
            margin-bottom: 20px;
            background-color: #fff;
            box-shadow: 0 2px 5px rgba(0,0,0,0.1);
        }

        th, td {
            border: 1px solid #ddd;
            padding: 12px 15px;
            text-align: center;
        }

        th {
            background-color: #ecf0f1;
            font-weight: 600;
        }

        tr:hover {
            background-color: #fafafa;
        }

        a {
            text-decoration: none;
            color: #2980b9;
            margin: 0 8px;
            font-weight: 500;
        }

        a:hover {
            text-decoration: underline;
        }

        form {
            background-color: #fff;
            padding: 20px;
            margin-bottom: 40px;
            border-radius: 8px;
            box-shadow: 0 2px 8px rgba(0,0,0,0.1);
            max-width: 600px;
            margin-left: auto;
            margin-right: auto;
        }

        label {
            display: block;
            margin-top: 10px;
            font-weight: 600;
        }

        input[type="text"],
        input[type="number"] {
            width: 100%;
            padding: 8px 10px;
            margin-top: 4px;
            border: 1px solid #ccc;
            border-radius: 4px;
            box-sizing: border-box;
            font-size: 14px;
        }

        input[type="text"]:focus,
        input[type="number"]:focus {
            border-color: #2980b9;
            outline: none;
            box-shadow: 0 0 5px rgba(41, 128, 185, 0.3);
        }

        button {
            margin-top: 15px;
            padding: 10px 20px;
            background-color: #2980b9;
            color: #fff;
            border: none;
            border-radius: 4px;
            cursor: pointer;
            font-size: 16px;
            transition: background-color 0.3s ease;
        }

        button:hover {
            background-color: #3498db;
        }

        /* Style for checkboxes */
        .checkbox-group {
            margin-top: 10px;
        }

        .checkbox-group label {
            display: inline-block;
            margin-right: 15px;
            font-weight: normal;
        }

        /* Responsive adjustments */
        @media(max-width: 768px) {
            table, form {
                font-size: 14px;
            }
        }
    </style>
</head>
<body>
<h1>Manage Classes & Teachers</h1>

<div style="background:#e8f4f8;padding:10px;margin-bottom:20px;border-radius:5px;border:1px solid #b8d4e8;">
  <div style="font-weight:bold;">
    <span style="color:#007bff;">User:</span> <?=htmlspecialchars($current_user['realname'])?>
    <span style="font-weight:normal;font-size:12px;">(<?=htmlspecialchars($current_user['orgname'])?>)</span>
  </div>
  <a href="timetable044b.php" style="color:#007bff;text-decoration:none;font-size:13px;">← Back to Timetable</a>
  <span style="margin:0 10px;">|</span>
  <a href="timetable044b.php?logout" style="color:#dc3545;text-decoration:none;font-size:13px;">Logout</a>
</div>



<h2>Teachers</h2>
<table>
<tr><th>teacher_id</th><th>name</th><th>max_hours</th><th>unavailable_days</th><th>color</th><th>eidikotita</th><th>Actions</th></tr>
<?php foreach ($teachers as $t): ?>
<tr>
<td><?= htmlspecialchars($t[0]) ?></td>
<td><?= htmlspecialchars($t[1]) ?></td>
<td><?= htmlspecialchars($t[2]) ?></td>
<td><?= htmlspecialchars($t[3]) ?></td>
<td><?= htmlspecialchars($t[4]) ?></td>
<td><?= htmlspecialchars($t[5]) ?></td>
<td>
    <a href="?action=edit&type=teacher&id=<?= urlencode($t[0]) ?>">Edit</a>
    <a href="?action=delete&type=teacher&id=<?= urlencode($t[0]) ?>" onclick="return confirm('Delete this teacher?')">Delete</a>
    <a href="?action=move&type=teacher&id=<?= urlencode($t[0]) ?>&move=up">↑</a>
    <a href="?action=move&type=teacher&id=<?= urlencode($t[0]) ?>&move=down">↓</a>
</td>
</tr>
<?php endforeach; ?>
</table>

<h3><?= ($type=='teacher' && isset($id)) ? 'Edit Teacher' : 'Add New Teacher' ?></h3>
<form method="post" action="">
    <?php
    $edit_teacher = null;
    if ($type=='teacher' && isset($id)) {
        $edit_teacher = find_in_array($teachers, $id);
    }
    ?>
    <input type="hidden" name="type" value="teacher" />
    <?php if ($edit_teacher): ?>
        <input type="hidden" name="original_teacher_id" value="<?= htmlspecialchars($edit_teacher[0]) ?>" />
    <?php endif; ?>
    <label>teacher_id: <input type="text" name="teacher_id" value="<?= $edit_teacher ? htmlspecialchars($edit_teacher[0]) : '' ?>" required></label>
    <label>name: <input type="text" name="name" value="<?= $edit_teacher ? htmlspecialchars($edit_teacher[1]) : '' ?>" required></label>
    <label>max_hours: <input type="number" name="max_hours" value="<?= $edit_teacher ? htmlspecialchars($edit_teacher[2]) : '' ?>" required></label>
    <label>unavailable_days:</label>
    <div class="checkbox-group">
        <?php foreach ($daysOfWeek as $day): ?>
            <label>
                <input type="checkbox" name="unavailable_days[]" value="<?= $day ?>"
                    <?= in_array($day, $selectedDays) ? 'checked' : '' ?>>
                <?= $day ?>
            </label>
        <?php endforeach; ?>
    </div>
    <label>color: <input type="text" name="color" value="<?= $edit_teacher ? htmlspecialchars($edit_teacher[4]) : '' ?>"></label>
    <label>eidikotita: <input type="text" name="eidikotita" value="<?= $edit_teacher ? htmlspecialchars($edit_teacher[5]) : '' ?>"></label>
    <button type="submit">Save (Add/Update)</button>
</form>

<h2>Classes</h2>
<table>
<tr><th>class_id</th><th>description</th><th>Actions</th></tr>
<?php foreach ($classes as $c): ?>
<tr>
<td><?= htmlspecialchars($c[0]) ?></td>
<td><?= htmlspecialchars($c[1]) ?></td>
<td>
    <a href="?action=edit&type=class&id=<?= urlencode($c[0]) ?>">Edit</a>
    <a href="?action=delete&type=class&id=<?= urlencode($c[0]) ?>" onclick="return confirm('Delete this class?')">Delete</a>
    <a href="?action=move&type=class&id=<?= urlencode($c[0]) ?>&move=up">↑</a>
    <a href="?action=move&type=class&id=<?= urlencode($c[0]) ?>&move=down">↓</a>
</td>
</tr>
<?php endforeach; ?>
</table>

<h3><?= ($type=='class' && isset($id)) ? 'Edit Class' : 'Add New Class' ?></h3>
<form method="post" action="">
    <?php
    $edit_class = null;
    if ($type=='class' && isset($id)) {
        $edit_class = find_in_array($classes, $id);
    }
    ?>
    <input type="hidden" name="type" value="class" />
    <?php if ($edit_class): ?>
        <input type="hidden" name="original_class_id" value="<?= htmlspecialchars($edit_class[0]) ?>" />
    <?php endif; ?>
    <label>class_id: <input type="text" name="class_id" value="<?= $edit_class ? htmlspecialchars($edit_class[0]) : '' ?>" required></label>
    <label>description: <input type="text" name="description" value="<?= $edit_class ? htmlspecialchars($edit_class[1]) : '' ?>" required></label>
    <button type="submit">Save (Add/Update)</button>
</form>



<hr>





<!-- //+++++++++++++randomly place teachers +++++++++++ -->
<button onclick="window.location.href='?generate_initial=1'">Generate Initial timetable</button>
<div id="status"></div>


<?php
$inital_placement_flag = isset($_GET['generate_initial']) ? true : false;
$generation_message = '';

if ($inital_placement_flag) {
        // Generate timetable data
    $timetable = [];
    $counter=0;


    foreach ($classes as $class) {
        $class_id = $class[0]; // class ID
        $teacher_id = isset($teachers[$counter]) ? $teachers[$counter][0] : null; // fixed teacher
        $counter++;

        foreach ($daysOfWeek as $day) {
            // Initialize slots: 1 to 6
            $full_slots = array_fill(1, 6, '');

            // Select 2 empty slots first with constraints
            do {
                $indices = range(1, 6);
                shuffle($indices);
                $empty_positions = array_slice($indices, 0, 2);
                sort($empty_positions);

                $diff = $empty_positions[1] - $empty_positions[0];

                $both_start = ($empty_positions[0] == 1 && $empty_positions[1] == 2);
                $both_end = ($empty_positions[0] == 5 && $empty_positions[1] == 6);

                $consecutive = ($diff == 1) && !($both_start || $both_end);
            } while ($consecutive);

            // Set empty slots
            foreach ($empty_positions as $pos) {
                $full_slots[$pos] = '';
            }

            // Assign remaining 4 slots to the teacher
            $teacher_slots = array_diff(range(1, 6), $empty_positions);
            shuffle($teacher_slots);

            // Assign teacher to these 4 slots
            foreach ($teacher_slots as $slotNum) {
                $full_slots[$slotNum] = $teacher_id;
            }

            // Save each slot
            foreach ($full_slots as $slotNum => $teacherInSlot) {
                $timetable[] = [
                    'class_id' => $class_id,
                    'day' => $day,
                    'slot' => $slotNum,
                    'teacher_id' => $teacherInSlot,
                    'additional_info' => ''
                ];
            }
        }
    } // end of  foreach ($classes as $class) {

    // Generate filename
    $dateStr = date('ymd_His');
    $filename = "timetable_{$dateStr}_INITIAL.csv";

    $fh = fopen($filename, 'w');
    fputcsv($fh, ['class_id', 'day', 'slot', 'teacher_id', 'course_id']);
    foreach ($timetable as $row) {
        fputcsv($fh, $row);
    }
    fclose($fh);

    echo "New timetable generated: {$filename}";


    // Output the timetable
    print_r($timetable);

}// END of if ($inital_placement_flag) {    
?>


<?php
//-----------randomly place teachers -----------------


// Save CSV

?>    
    
</body>
</html>