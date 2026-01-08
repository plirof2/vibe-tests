<?php
session_start();
$version="timetable045b - multi-user support with shared teachers/classes";
/*
===========================================
 Timetable Script
===========================================

CHANGES v0.45b:
 - Shared teachers and classes per user (not per slot)
 - Removed "Load from" dropdown (handled by slot buttons)
 - Only timetable files are slot-specific (username#1_timetable.csv, etc.)
 - Teachers: username_teachers.csv (shared across all slots)
 - Classes: username_classes.csv (shared across all slots)

CHANGES v0.45a - Multi-User Support:
 - Added user authentication with users.csv
 - Added session management
 - Added 3 save/load slots per user
 - Added autosave functionality
 - Added copy slot functionality

CHANGES v4.4b:
 - Loading external JS file for printing (like plug-in system)

CHANGES from v4.3 → v4.4a:
 - Messages now show even for empty slots
FIXES APPLIED:
 - Fixed drag and drop state management - proper cleanup of draggedFromSlot
 - Fixed conflict detection race conditions and validation timing
 - Improved error handling for missing/malformed CSV files
 - Fixed status message display consistency across browsers
 - Resolved teacher pool refresh issues after drag operations
 - Fixed backup loading with proper validation and error handling
 - Corrected hours calculation synchronization between UI and data

CHANGES from v4.1 → v4.2 4.3:
 - Fixed move teacher bug: now properly clears old slot when moving to new slot
 - Fixed saving in single class view: automatically switches to "Show All" before saving
 - Added "Load from file" dropdown with timetable backups
 - Added confirmation dialog before loading backup files
 - Added reload functionality via GET parameter after loading backup
 - FIXED: Teacher conflict detection - prevents same teacher in multiple classes at same time
 - ADDED: Visual feedback when dragging - shows blocked slots in gray for the dragged teacher
 - FIXED: Status messages now properly visible when dragging/dropping
 - Added function documentation comments for better readability
 - Default view remains "Show All Classes"

Previous changes (v3 → v4.1):
 - Fixed conflict detection: blocks only the SAME teacher from being double-booked
 - Unavailable days from teachers.csv are enforced with dark gray slots
 - Added tooltips to blocked slots and status messages
 - Added Legend Box under Teacher Pool
 - Creates timestamped backups of timetable.csv on save
==========================================
*/

$days=["Mon","Tue","Wed","Thu","Fri"];
$slots=range(1,9);

// Handle logout
if(isset($_GET['logout'])) {
  session_destroy();
  header("Location: " . strtok($_SERVER["REQUEST_URI"], '?'));
  exit;
}

// Handle login
if($_SERVER['REQUEST_METHOD'] === 'POST' && isset($_POST['username']) && isset($_POST['password'])) {
  $users = read_csv("users.csv");
  $username = $_POST['username'];
  $password = $_POST['password'];
  
  $valid_user = null;
  foreach($users as $u) {
    if($u['username'] === $username && $u['password'] === $password) {
      $valid_user = $u;
      break;
    }
  }
  
  if($valid_user) {
    $_SESSION['user'] = $valid_user;
    $_SESSION['current_slot'] = 1;
    header("Location: " . strtok($_SERVER["REQUEST_URI"], '?') . "?login=success");
    exit;
  } else {
    $login_error = "Invalid username or password";
  }
}

// Check if user is logged in
$is_logged_in = isset($_SESSION['user']);
$current_user = $is_logged_in ? $_SESSION['user'] : null;
$current_slot = isset($_SESSION['current_slot']) ? $_SESSION['current_slot'] : 1;

// Handle slot selection
if(isset($_GET['select_slot']) && $is_logged_in) {
  $slot_num = intval($_GET['select_slot']);
  if($slot_num >= 1 && $slot_num <= 3) {
    $_SESSION['current_slot'] = $slot_num;
    header("Location: " . strtok($_SERVER["REQUEST_URI"], '?') . "?slot_changed=" . $slot_num);
    exit;
  }
}

// Handle save to slot
if($_SERVER['REQUEST_METHOD'] === 'POST' && isset($_POST['save_slot']) && $is_logged_in) {
  $slot_num = intval($_POST['save_slot']);
  if($slot_num >= 1 && $slot_num <= 3) {
    $username = $current_user['username'];
    $data = json_decode($_POST['timetable_json'], true);
    if($data === null) {
      echo "ERROR: Invalid data format";
      exit;
    }
    
    $slot_prefix = $username . "#" . $slot_num . "_";
    
    // Save timetable only (teachers and classes are shared)
    if(!write_csv($slot_prefix . "timetable.csv", ["class_id","day","slot","teacher_id","course_id"], $data)) {
      echo "ERROR: Failed to save timetable";
      exit;
    }
    
    echo "SAVED_TO_SLOT_" . $slot_num;
    exit;
  }
}

// Handle load from slot
if(isset($_GET['load_slot']) && $is_logged_in) {
  $slot_num = intval($_GET['load_slot']);
  if($slot_num >= 1 && $slot_num <= 3) {
    $_SESSION['current_slot'] = $slot_num;
    header("Location: " . strtok($_SERVER["REQUEST_URI"], '?') . "?slot_loaded=" . $slot_num);
    exit;
  }
}

// Handle copy slot
if($_SERVER['REQUEST_METHOD'] === 'POST' && isset($_POST['copy_from_slot']) && isset($_POST['copy_to_slot']) && $is_logged_in) {
  $from_slot = intval($_POST['copy_from_slot']);
  $to_slot = intval($_POST['copy_to_slot']);
  
  if($from_slot >= 1 && $from_slot <= 3 && $to_slot >= 1 && $to_slot <= 3 && $from_slot !== $to_slot) {
    $username = $current_user['username'];
    $from_prefix = $username . "#" . $from_slot . "_";
    $to_prefix = $username . "#" . $to_slot . "_";
    
    // Only copy timetable.csv (teachers and classes are shared)
    $file = "timetable.csv";
    $success = false;
    
    if(file_exists($from_prefix . $file)) {
      $success = copy($from_prefix . $file, $to_prefix . $file);
    }
    
    if($success) {
      header("Location: " . strtok($_SERVER["REQUEST_URI"], '?') . "?copy_success");
    } else {
      header("Location: " . strtok($_SERVER["REQUEST_URI"], '?') . "?copy_failed");
    }
    exit;
  }
}

// Handle autosave
if($_SERVER['REQUEST_METHOD'] === 'POST' && isset($_POST['autosave']) && $is_logged_in) {
  $username = $current_user['username'];
  $autosave_prefix = $username . "_autosave_";
  
  // Only autosave timetable (teachers and classes are managed via crud_csv.php)
  $data = json_decode($_POST['timetable_json'], true);
  if($data !== null) {
    write_csv($autosave_prefix . "timetable.csv", ["class_id","day","slot","teacher_id","course_id"], $data);
  }
  
  echo "AUTOSAVED";
  exit;
}

// Get user-specific file path or default
function getUserFile($filename, $slot = 0) {
  global $is_logged_in, $current_user;
  
  if(!$is_logged_in) {
    return $filename;
  }
  
  $username = $current_user['username'];
  
  // Teachers and classes are shared across all slots for each user
  if($filename === 'teachers.csv' || $filename === 'classes.csv') {
    $user_file = $username . "_" . $filename;
    if(file_exists($user_file)) {
      return $user_file;
    }
    return $filename; // Use default file if user file doesn't exist
  }
  
  // Timetable is slot-specific
  if($filename === 'timetable.csv' && $slot > 0) {
    $user_file = $username . "#" . $slot . "_" . $filename;
    if(file_exists($user_file)) {
      return $user_file;
    }
  }
  
  return $filename;
}

/**
 * Read CSV file and return associative array with better error handling
 */
function read_csv($file){
  if(!file_exists($file)) {
    error_log("CSV file not found: $file");
    return [];
  }
  
  $content = file($file);
  if($content === false) {
    error_log("Failed to read CSV file: $file");
    return [];
  }
  
  $rows = array_map('str_getcsv', $content);
  if(empty($rows)) {
    error_log("Empty CSV file: $file");
    return [];
  }
  
  $header = array_shift($rows);
  if(empty($header)) {
    error_log("Invalid CSV header in file: $file");
    return [];
  }
  
  $out = [];
  foreach($rows as $r){
    if(count($r) == 1 && $r[0] == "") continue; // Skip empty rows
    if(count($r) != count($header)) {
      error_log("Invalid row in CSV file $file: " . implode(',', $r));
      continue;
    }
    $out[] = array_combine($header, $r);
  }
  return $out;
}

/**
 * Write data to CSV file with specified header and error handling
 */
function write_csv($file, $header, $data){
  $fp = fopen($file, "w");
  if($fp === false) {
    error_log("Failed to open file for writing: $file");
    return false;
  }
  
  fputcsv($fp, $header);
  foreach($data as $row){
    $line = [];
    foreach($header as $h) {
      $line[] = $row[$h] ?? "";
    }
    fputcsv($fp, $line);
  }
  fclose($fp);
  return true;
}

/**
 * Get list of all timetable backup files with validation
 */
function get_backup_files(){
  $files = glob("timetable*.csv");
  if($files === false) {
    error_log("Failed to read backup files");
    return [];
  }
  
  // Validate files exist and are readable
  $valid_files = [];
  foreach($files as $file) {
    if(is_readable($file)) {
      $valid_files[] = $file;
    }
  }
  
  sort($valid_files);
  return $valid_files;
}

$teachers_file = getUserFile("teachers.csv", $current_slot);
$classes_file = getUserFile("classes.csv", $current_slot);
$timetable_file = getUserFile("timetable.csv", $current_slot);

$teachers = read_csv($teachers_file);
$classes = read_csv($classes_file);
$timetable = read_csv($timetable_file);

// Validate required data
if(empty($teachers)) {
  error_log("No teachers data loaded");
}
if(empty($classes)) {
  error_log("No classes data loaded");
}

// Handle backup file loading via GET parameter with validation
if(isset($_GET['load_backup']) && $_GET['load_backup']) {
  $backup_file = $_GET['load_backup'];
  
  // Validate backup file
  if(!file_exists($backup_file)) {
    error_log("Backup file not found: $backup_file");
    header("Location: " . strtok($_SERVER["REQUEST_URI"], '?') . "?error=backup_not_found");
    exit;
  }
  
  if(!is_readable($backup_file)) {
    error_log("Backup file not readable: $backup_file");
    header("Location: " . strtok($_SERVER["REQUEST_URI"], '?') . "?error=backup_not_readable");
    exit;
  }
  
  $backup_data = read_csv($backup_file);
  if(empty($backup_data)) {
    error_log("Invalid backup data in file: $backup_file");
    header("Location: " . strtok($_SERVER["REQUEST_URI"], '?') . "?error=invalid_backup");
    exit;
  }
  
  if(write_csv("timetable.csv", ["class_id","day","slot","teacher_id","course_id"], $backup_data)) {
    // Redirect to remove GET parameter and reload fresh data
    header("Location: " . strtok($_SERVER["REQUEST_URI"], '?') . "?success=backup_loaded");
    exit;
  } else {
    error_log("Failed to write backup data to timetable.csv");
    header("Location: " . strtok($_SERVER["REQUEST_URI"], '?') . "?error=backup_write_failed");
    exit;
  }
}

// Handle save operation with better error handling
if($_SERVER['REQUEST_METHOD'] === 'POST' && isset($_POST['timetable_json'])){
  $data = json_decode($_POST['timetable_json'], true);
  if($data === null) {
    error_log("Invalid JSON data received for timetable save");
    echo "ERROR: Invalid data format";
    exit;
  }
  
  // Create backup before saving
  $backup = "timetable_" . date("Ymd_His") . ".csv";
  if(file_exists("timetable.csv")) {
    if(!copy("timetable.csv", $backup)) {
      error_log("Failed to create backup: $backup");
      echo "ERROR: Backup failed";
      exit;
    }
  }
  
  if(write_csv("timetable.csv", ["class_id","day","slot","teacher_id","course_id"], $data)) {
    echo "SAVED";
  } else {
    error_log("Failed to save timetable data");
    echo "ERROR: Save failed";
  }
  exit;
}

// Build timetable map and calculate teacher hours
$map = [];
$hours = [];
foreach($timetable as $r){
  if(isset($r['class_id']) && isset($r['day']) && isset($r['slot']) && isset($r['teacher_id'])) {
    $map[$r['class_id']][$r['day']][$r['slot']] = $r['teacher_id'];
    if($r['teacher_id']) {
      $hours[$r['teacher_id']] = ($hours[$r['teacher_id']] ?? 0) + 1;
    }
  }
}

// Handle URL parameters for user feedback
$message = "";
$message_type = "";
if(isset($_GET['error'])) {
  switch($_GET['error']) {
    case 'backup_not_found':
      $message = "Backup file not found";
      $message_type = "error";
      break;
    case 'backup_not_readable':
      $message = "Backup file cannot be read";
      $message_type = "error";
      break;
    case 'invalid_backup':
      $message = "Invalid backup file format";
      $message_type = "error";
      break;
    case 'backup_write_failed':
      $message = "Failed to load backup data";
      $message_type = "error";
      break;
  }
}
if(isset($_GET['success'])) {
  switch($_GET['success']) {
    case 'backup_loaded':
      $message = "Backup loaded successfully";
      $message_type = "success";
      break;
    case 'login':
      $message = "Logged in as " . htmlspecialchars($current_user['realname']);
      $message_type = "success";
      break;
  }
}
if(isset($_GET['slot_changed'])) {
  $slot_num = intval($_GET['slot_changed']);
  $message = "Switched to Slot " . $slot_num;
  $message_type = "success";
}
if(isset($_GET['slot_loaded'])) {
  $slot_num = intval($_GET['slot_loaded']);
  $message = "Loaded data from Slot " . $slot_num;
  $message_type = "success";
}
if(isset($_GET['copy_success'])) {
  $message = "Slot copied successfully";
  $message_type = "success";
}
if(isset($_GET['copy_failed'])) {
  $message = "Failed to copy slot";
  $message_type = "error";
}
if(isset($_GET['login']) && $_GET['login'] === 'success') {
  $message = "Logged in as " . htmlspecialchars($current_user['realname']);
  $message_type = "success";
}
?>
<!DOCTYPE html>
<html>
<head><meta charset="utf-8"><title>Timetable <?php echo $version; ?></title>
<style>
body{margin:0;font-family:Arial;display:flex;height:100vh;overflow:hidden;}
#pool{width:240px;background:#f8f8f8;border-right:1px solid #aaa;padding:10px;overflow-y:auto;position:sticky;top:0;height:100vh;}
#main{flex:1;display:flex;flex-direction:column;}
#controls{background:#eee;padding:8px;border-bottom:1px solid #ccc;}
#tables{flex:1;overflow:auto;padding:10px;}
.teacher{padding:5px;margin:5px 0;border:1px solid #444;cursor:grab;transition:all 0.2s ease;}
.teacher.maxed{border:2px solid red;}
.teacher.dragging{border:2px solid #00f;background-color:#cce6ff !important;transform:scale(1.05);}
.grid{border-collapse:collapse;margin-bottom:20px;}
.grid th,.grid td{border:1px solid #999;padding:5px;text-align:center;min-width:80px;height:40px;}
.slot{background:#f9f9f9;transition:background-color 0.2s ease;}
.slot.occupied{color:#000;}
.slot.blocked{background:gray;color:#555;cursor:not-allowed;}
/*.slot.blocked{background:#888;color:#555;cursor:not-allowed;}
.slot.unavailable{background:#555;color:#fff;cursor:not-allowed;}*/
.slot.unavailable{background:darkgray;color:#fff;cursor:not-allowed;}
.slot:hover:not(.blocked):not(.unavailable):not(.occupied){background:#e8f4f8;}
button{margin-right:8px;padding:5px 10px;cursor:pointer;}
button:hover{background:#ddd;}
#status,#status2{margin-left:8px;transition:all .3s ease;font-weight:bold;padding:3px 6px;border-radius:3px;}
#legend{margin-top:20px;font-size:13px;}
#legend div{margin:3px 0;display:flex;align-items:center;}
.legend-box{width:18px;height:18px;margin-right:6px;border:1px solid #333;}
#loadControls{margin-top:10px;}
/* Status message styling */
.status-error{color:#ff0000 !important;background:#ffeeee !important;border:1px solid #ffaaaa;}
.status-success{color:#008000 !important;background:#eeffee !important;border:1px solid #aaffaa;}
.system-message{padding:8px;margin:8px 0;border-radius:4px;font-weight:bold;}
.system-message.error{background:#ffeeee;color:#cc0000;border:1px solid #ffaaaa;}
.system-message.success{background:#eeffee;color:#006600;border:1px solid #aaffaa;}
/* Improved drag feedback */
.slot.drag-over{background:#d4edda !important;border:2px dashed #28a745;}
</style>
</head>
<body>
        
<?php if(!$is_logged_in): ?>
<div style="position:fixed;top:0;left:0;width:100%;height:100%;background:rgba(255,255,255,0.98);z-index:9999;display:flex;align-items:center;justify-content:center;">
  <div style="background:#f0f0f0;padding:30px;border-radius:10px;box-shadow:0 0 20px rgba(0,0,0,0.3);max-width:400px;width:90%;">
    <h2 style="text-align:center;margin-top:0;">Login to Timetable</h2>
    <?php if(isset($login_error)): ?>
      <div style="background:#ffeeee;color:#cc0000;padding:10px;margin-bottom:15px;border-radius:5px;border:1px solid #ffaaaa;">
        <?=htmlspecialchars($login_error)?>
      </div>
    <?php endif; ?>
    <form method="POST" style="display:flex;flex-direction:column;gap:15px;">
      <div>
        <label style="display:block;margin-bottom:5px;font-weight:bold;">Username:</label>
        <input type="text" name="username" required style="width:100%;padding:8px;border:1px solid #ccc;border-radius:4px;box-sizing:border-box;">
      </div>
      <div>
        <label style="display:block;margin-bottom:5px;font-weight:bold;">Password:</label>
        <input type="password" name="password" required style="width:100%;padding:8px;border:1px solid #ccc;border-radius:4px;box-sizing:border-box;">
      </div>
      <button type="submit" style="background:#007bff;color:white;padding:10px;border:none;border-radius:4px;cursor:pointer;font-size:16px;font-weight:bold;">Login</button>
    </form>
    <p style="text-align:center;margin-top:15px;color:#666;font-size:12px;">Enter your username and password to access your timetable</p>
  </div>
</div>
<?php endif; ?>

<div id="pool" ondragover="event.preventDefault()" ondrop="removeTeacher(event)">
  <?php if($is_logged_in): ?>
  <div style="background:#e8f4f8;padding:10px;margin-bottom:10px;border-radius:5px;border:1px solid #b8d4e8;">
    <div style="font-weight:bold;margin-bottom:5px;">
      <span style="color:#007bff;">User:</span> <?=htmlspecialchars($current_user['realname'])?>
      <br>
      <span style="font-weight:normal;font-size:12px;"><?=htmlspecialchars($current_user['orgname'])?></span>
    </div>
    <a href="?logout" style="color:#dc3545;text-decoration:none;font-size:13px;">Logout</a>
  </div>
  
  <div style="background:#fff3cd;padding:10px;margin-bottom:10px;border-radius:5px;border:1px solid #ffeeba;">
    <div style="font-weight:bold;margin-bottom:8px;font-size:14px;">Current Slot: <span id="currentSlotDisplay"><?=$current_slot?></span></div>
    <div style="display:flex;gap:5px;flex-wrap:wrap;margin-bottom:8px;">
      <button onclick="switchSlot(1)" class="slot-btn" data-slot="1" style="flex:1;padding:5px;cursor:pointer;<?=$current_slot==1?'background:#007bff;color:white;':''?>">Slot 1</button>
      <button onclick="switchSlot(2)" class="slot-btn" data-slot="2" style="flex:1;padding:5px;cursor:pointer;<?=$current_slot==2?'background:#007bff;color:white;':''?>">Slot 2</button>
      <button onclick="switchSlot(3)" class="slot-btn" data-slot="3" style="flex:1;padding:5px;cursor:pointer;<?=$current_slot==3?'background:#007bff;color:white;':''?>">Slot 3</button>
    </div>
    <div style="display:flex;gap:5px;flex-wrap:wrap;">
      <button onclick="saveToSlot(1)" style="flex:1;padding:5px;cursor:pointer;">Save 1</button>
      <button onclick="saveToSlot(2)" style="flex:1;padding:5px;cursor:pointer;">Save 2</button>
      <button onclick="saveToSlot(3)" style="flex:1;padding:5px;cursor:pointer;">Save 3</button>
    </div>
  </div>
  
  <div style="background:#d1ecf1;padding:10px;margin-bottom:10px;border-radius:5px;border:1px solid #bee5eb;">
    <div style="font-weight:bold;margin-bottom:8px;font-size:14px;">Copy Slot</div>
    <div style="display:flex;gap:5px;align-items:center;">
      <label style="font-size:12px;">From:</label>
      <select id="copyFromSlot" style="flex:1;padding:3px;">
        <option value="1">Slot 1</option>
        <option value="2">Slot 2</option>
        <option value="3">Slot 3</option>
      </select>
      <label style="font-size:12px;">To:</label>
      <select id="copyToSlot" style="flex:1;padding:3px;">
        <option value="1">Slot 1</option>
        <option value="2">Slot 2</option>
        <option value="3">Slot 3</option>
      </select>
      <button onclick="copySlot()" style="padding:5px 10px;cursor:pointer;">Copy</button>
    </div>
  </div>
  <?php endif; ?>
  
  <h3>Teacher Pool</h3>
  <div id="teachers">
    <?php foreach($teachers as $t): 
      $tid = $t['teacher_id'] ?? '';
      $name = htmlspecialchars($t['name'] ?? 'Unknown');
      $color = $t['color'] ?? '#cccccc';
      $max_hours = $t['max_hours'] ?? 0;
      $used = $hours[$tid] ?? 0;
    ?>
      <div class="teacher" draggable="true" data-id="<?=$tid?>" style="background:<?=$color?>;">
        <?=$name?><br>
        <small>(<span id="h-<?=$tid?>" class="hcount"><?=$used?></span>/<?=$max_hours?>)</small>
      </div>
    <?php endforeach;?>
  </div>
  <button id="saveBtn2">Save</button><span id="status2"></span>
  
  <div id="legend">
    <h4>Legend</h4>
    <div><div class="legend-box" style="background:#555"></div>Dark Gray = Unavailable</div>
    <div><div class="legend-box" style="background:#eee"></div>Light Gray = Conflict</div>
    <div><div class="legend-box" style="background:#f9f9f9"></div>Neutral = Free Slot</div>
    <div><div class="legend-box" style="background:skyblue"></div>Colored = Assigned</div>
  </div>
<hr>Timetable <?php echo $version; ?><hr>
</div>

<div id="main">
  <div id="controls">
    <?php if($message): ?>
      <div class="system-message <?=$message_type?>">
        <?=htmlspecialchars($message)?>
      </div>
    <?php endif; ?>
    
    <label>Select Class:
      <select id="classSelect">
        <?php foreach($classes as $c):
          $class_id = $c['class_id'] ?? '';
          $description = htmlspecialchars($c['description'] ?? '');
        ?>
          <option value="<?=$class_id?>"><?=$class_id?> - <?=$description?></option>
        <?php endforeach;?>
      </select>
    </label>
    <button id="showAll">Show All Classes</button>
    <button id="saveBtn">Save</button><span id="status"></span>
  </div>
  <div id="tables"></div>
</div>

<script>
let days = <?=json_encode($days)?>;
let slots = <?=json_encode($slots)?>;
let teachers = <?=json_encode($teachers)?>;
let timetable = <?=json_encode($map)?>;
let teacherMap = {};
teachers.forEach(t => teacherMap[t.teacher_id] = t);
let classes = <?=json_encode($classes)?>;
let showAll = true;
let draggedFromSlot = null; // Track if drag started from a slot
let currentDraggedTeacher = null; // Track which teacher is being dragged
let draggedTeacherElement = null; // Track the DOM element being dragged
let currentSlot = <?=$current_slot?>;
let lastAutosaveTime = Date.now();
let hasChanges = false;

// Slot Management Functions
function switchSlot(slotNum) {
  window.location.href = window.location.pathname + "?select_slot=" + slotNum;
}

function saveToSlot(slotNum) {
  if(!showAll) {
    showMsg("Please switch to 'Show All Classes' view before saving", false, "status2");
    return;
  }
  
  let data = collectData();
  let fd = new FormData();
  fd.append("save_slot", slotNum);
  fd.append("timetable_json", JSON.stringify(data));
  // Teachers and classes are shared, managed via crud_csv.php
  
  fetch(window.location.pathname, {method: "POST", body: fd})
    .then(r => r.text())
    .then(txt => {
      if(txt.includes("SAVED_TO_SLOT")) {
        showMsg("Saved to Slot " + slotNum, true, "status2");
        hasChanges = false;
      } else {
        showMsg("Save failed: " + txt, false, "status2");
      }
    })
    .catch(error => {
      handleError("Network error during save", error);
    });
}

function copySlot() {
  let fromSlot = document.getElementById("copyFromSlot").value;
  let toSlot = document.getElementById("copyToSlot").value;
  
  if(fromSlot === toSlot) {
    showMsg("Cannot copy slot to itself", false, "status2");
    return;
  }
  
  if(confirm("Copy data from Slot " + fromSlot + " to Slot " + toSlot + "? This will overwrite Slot " + toSlot + ".")) {
    let fd = new FormData();
    fd.append("copy_from_slot", fromSlot);
    fd.append("copy_to_slot", toSlot);
    
    fetch(window.location.pathname, {method: "POST", body: fd})
      .then(r => {
        window.location.reload();
      })
      .catch(error => {
        handleError("Network error during copy", error);
      });
  }
}

// Autosave function
function autosave() {
  if(!hasChanges) return;
  
  let data = collectData();
  let fd = new FormData();
  fd.append("autosave", "true");
  fd.append("timetable_json", JSON.stringify(data));
  // Teachers and classes are shared, managed via crud_csv.php
  
  fetch(window.location.pathname, {method: "POST", body: fd})
    .then(r => r.text())
    .then(txt => {
      if(txt === "AUTOSAVED") {
        console.log("Autosaved at", new Date().toLocaleTimeString());
        lastAutosaveTime = Date.now();
      }
    })
    .catch(error => {
      console.error("Autosave failed:", error);
    });
}

// Start autosave interval (every 5 minutes)
setInterval(autosave, 300000);

// Track changes for autosave
function trackChanges() {
  hasChanges = true;
}

// Update slot button styling
function updateSlotButtons() {
  document.querySelectorAll(".slot-btn").forEach(btn => {
    let slot = parseInt(btn.dataset.slot);
    if(slot === currentSlot) {
      btn.style.background = "#007bff";
      btn.style.color = "white";
    } else {
      btn.style.background = "";
      btn.style.color = "";
    }
  });
  document.getElementById("currentSlotDisplay").textContent = currentSlot;
}

// Improved error handling
function handleError(message, error = null) {
  console.error(message, error);
  showMsg(message, false, "status2");
}

/**
 * Initialize drag event listeners for teacher pool
 */
function initializeTeacherDragEvents() {
  document.querySelectorAll(".teacher").forEach(el => {
    el.addEventListener("dragstart", ev => {
      ev.dataTransfer.setData("text/plain", el.dataset.id);
      ev.dataTransfer.effectAllowed = "move";
      draggedFromSlot = null; // Reset slot tracking
      currentDraggedTeacher = el.dataset.id;
      draggedTeacherElement = el;
      el.classList.add("dragging");
      
      // Apply constraints after a small delay to ensure proper state
      setTimeout(() => applyDragConstraints(el.dataset.id), 10);
    });
    
    el.addEventListener("dragend", ev => {
      el.classList.remove("dragging");
      currentDraggedTeacher = null;
      draggedTeacherElement = null;
      clearDragConstraints();
      // Reset drag state completely
      setTimeout(() => {
        draggedFromSlot = null;
      }, 100);
    });
  });
}

/**
 * Main render function - displays either single class or all classes
 */
function render(){
  let cont = document.getElementById("tables");
  cont.innerHTML = "";
  
  if(showAll){
    classes.forEach(c => {
      let d = document.createElement("div");
      d.innerHTML = "<h3>Class " + (c.class_id || 'Unknown') + "</h3>";
      d.appendChild(buildTable(c.class_id));
      cont.appendChild(d);
    });
  } else {
    let cid = document.getElementById("classSelect").value;
    cont.innerHTML = "<h3>Class " + cid + "</h3>";
    cont.appendChild(buildTable(cid));
  }
  
  // Re-initialize drag events after render
  initializeTeacherDragEvents();
  updateHours();
}

/**
 * Build timetable grid for a specific class
 */
function buildTable(cid){
  let t = document.createElement("table");
  t.className = "grid";
  let head = "<tr><th>Period</th>";
  days.forEach(d => head += "<th>" + d + "</th>");
  head += "</tr>";
  t.innerHTML = head;
  
  slots.forEach(s => {
    let tr = document.createElement("tr");
    let th = document.createElement("th");
    th.textContent = "P" + s;
    tr.appendChild(th);
    
    days.forEach(d => {
      let td = document.createElement("td");
      td.className = "slot";
      td.dataset.classId = cid;
      td.dataset.day = d;
      td.dataset.slot = s;
      
      let tid = (timetable[cid] && timetable[cid][d] && timetable[cid][d][s]) || "";
      if(tid && teacherMap[tid]){
        td.textContent = teacherMap[tid].name;
        td.style.background = teacherMap[tid].color;
        td.classList.add("occupied");
        td.dataset.tid = tid;
        td.draggable = true;
      }
      
      // Improved drag event handling
      td.addEventListener("dragover", ev => {
        if(!td.classList.contains("blocked") && !td.classList.contains("unavailable")) {
          //ev.preventDefault();
          ev.dataTransfer.dropEffect = "move";
          td.classList.add("drag-over");
        }
        ev.preventDefault();
      });
      
      td.addEventListener("dragleave", ev => {
        td.classList.remove("drag-over");
      });
      
      td.addEventListener("drop", ev => {
        td.classList.remove("drag-over");
        dropTeacher(ev, td);
      });
      
      td.addEventListener("dragstart", ev => {
        if(td.dataset.tid){
          ev.dataTransfer.setData("text/plain", td.dataset.tid);
          ev.dataTransfer.effectAllowed = "move";
          draggedFromSlot = td; // Store reference to the slot being dragged from
          currentDraggedTeacher = td.dataset.tid;
          draggedTeacherElement = null; // Not from teacher pool
          
          // Apply constraints after a small delay
          setTimeout(() => applyDragConstraints(td.dataset.tid), 10);
        }
      });
      
      td.addEventListener("dragend", ev => {
        if(td.dataset.tid){
          currentDraggedTeacher = null;
          draggedTeacherElement = null;
          clearDragConstraints();
          // Reset drag state after delay
          setTimeout(() => {
            if(!currentDraggedTeacher) {
              draggedFromSlot = null;
            }
          }, 100);
        }
      });
      
      tr.appendChild(td);
    });
    t.appendChild(tr);
  });
  return t;
}

/**
 * Get all slots where a specific teacher is currently assigned (excluding draggedFromSlot)
 */
function getTeacherAssignments(teacherId) {
  let assignments = [];
  document.querySelectorAll(".slot").forEach(slot => {
    if(slot.dataset.tid === teacherId && slot !== draggedFromSlot) {
      assignments.push({
        day: slot.dataset.day,
        slot: slot.dataset.slot,
        class: slot.dataset.classId,
        element: slot
      });
    }
  });
  return assignments;
}

/**
 * Apply visual constraints when dragging a teacher
 */
function applyDragConstraints(teacherId) {
  if(!teacherId || !teacherMap[teacherId]) return;
  
  let teacher = teacherMap[teacherId];
  
  // Get teacher's unavailable days
  let unavailableDays = (teacher.unavailable_days || "").split("|").filter(x => x.trim() !== "");
  
  // Get all current assignments for this teacher
  let assignments = getTeacherAssignments(teacherId);
  
  // Check max hours
  let currentHours = parseInt(document.getElementById("h-" + teacherId).textContent);
  let isAtMaxHours = currentHours >= teacher.max_hours && !draggedFromSlot;
  
  // Apply constraints to all empty slots
  document.querySelectorAll(".slot").forEach(slot => {
    if(!slot.dataset.tid) { // Only check empty slots
      let slotDay = slot.dataset.day;
      let slotPeriod = slot.dataset.slot;
      
      // Reset any previous constraints
      slot.classList.remove("blocked", "unavailable");
      slot.title = "";
      
      // Check unavailable days
      if(unavailableDays.includes(slotDay)) {
        slot.classList.add("unavailable");
        slot.title = teacher.name + " not available on " + slotDay;
      }
      // Check time conflicts - teacher already assigned at this day/period
      else {
        let hasConflict = assignments.some(assignment => 
          assignment.day === slotDay && assignment.slot === slotPeriod
        );
        
        if(hasConflict) {
          let conflictAssignment = assignments.find(assignment => 
            assignment.day === slotDay && assignment.slot === slotPeriod
          );
          slot.classList.add("blocked");
          slot.title = teacher.name + " already teaching class " + conflictAssignment.class + " at " + slotDay + " P" + slotPeriod;
        }
        // Check max hours (only if not moving from existing slot)
        else if(isAtMaxHours) {
          slot.classList.add("blocked");
          slot.title = teacher.name + " has reached max hours (" + teacher.max_hours + ")";
        }
      }
    }
  });
}

/**
 * Clear all drag-specific visual constraints
 */
function clearDragConstraints() {
  document.querySelectorAll(".slot").forEach(slot => {
    if(!slot.dataset.tid) { // Only clear empty slots
      slot.classList.remove("blocked", "unavailable", "drag-over");
      slot.title = "";
    }
  });
}

/**
 * Handle dropping teacher into a slot
 */
function dropTeacher(ev, td){
  ev.preventDefault();
  trackChanges();
  let tid = ev.dataTransfer.getData("text/plain");
  if(!tid) return;
  
  // Clear any previous messages
  clearMessage("status");
  clearMessage("status2");
  
  // Validate teacher exists
  if(!teacherMap[tid]) {
    handleError("Teacher not found: " + tid);
    return;
  }
  
  // Check constraints
  if(td.classList.contains("blocked") || td.classList.contains("unavailable")){
    if(td.classList.contains("unavailable")) {
      showMsg(teacherMap[tid].name + " not available on " + td.dataset.day + " (P" + td.dataset.slot + ")", false, "status2");
    }
    if(td.classList.contains("blocked")) {
      showMsg(teacherMap[tid].name + " already assigned or max hours reached", false, "status2");
    }
    return;
  }
  
  // Double-check for conflicts at drop time
  let assignments = getTeacherAssignments(tid);
  let hasConflict = assignments.some(assignment => 
    assignment.day === td.dataset.day && assignment.slot === td.dataset.slot
  );
  
  if(hasConflict) {
    let conflictAssignment = assignments.find(assignment => 
      assignment.day === td.dataset.day && assignment.slot === td.dataset.slot
    );
    showMsg(teacherMap[tid].name + " already teaching class " + conflictAssignment.class + " at " + td.dataset.day + " P" + td.dataset.slot, false, "status2");
    return;
  }
  
  // Check max hours (only if not moving from existing slot)
  let currentHours = parseInt(document.getElementById("h-" + tid).textContent);
  if(currentHours >= teacherMap[tid].max_hours && !draggedFromSlot) {
    showMsg(teacherMap[tid].name + " has reached max hours (" + teacherMap[tid].max_hours + ")", false, "status2");
    return;
  }
  
  // Clear the old slot if we're moving from a slot to a different slot
  if(draggedFromSlot && draggedFromSlot !== td) {
    clearSlot(draggedFromSlot);
  }
  
  let t = teacherMap[tid];
  td.textContent = t.name;
  td.style.background = t.color;
  td.className = "slot occupied";
  td.dataset.tid = tid;
  td.draggable = true;
  td.title = "";
  
  // Show success message
  showMsg("Teacher " + t.name + " assigned to " + td.dataset.day + " P" + td.dataset.slot, true, "status2");
  
  // Reset drag tracking
  draggedFromSlot = null;
  currentDraggedTeacher = null;
  clearDragConstraints();
  updateHours();
}

/**
 * Handle removing teacher when dragged to pool area
 */
function removeTeacher(ev){
  ev.preventDefault();
  trackChanges();
  let tid = ev.dataTransfer.getData("text/plain");
  if(!tid) return;
  
  // If dragged from a slot, clear that slot
  if(draggedFromSlot) {
    clearSlot(draggedFromSlot);
    if(teacherMap[tid]) {
      showMsg("Teacher " + teacherMap[tid].name + " removed", true, "status2");
    }
    draggedFromSlot = null;
  }
  
  currentDraggedTeacher = null;
  clearDragConstraints();
  updateHours();
}

/**
 * Clear a slot (remove teacher assignment)
 */
function clearSlot(td){
  td.textContent = "";
  td.style.background = "#f9f9f9";
  td.className = "slot";
  delete td.dataset.tid;
  td.draggable = false;
  td.title = "";
}


/**
 * Update teacher hours count and visual constraints
 */
function updateHours(){
  let count = {};
  document.querySelectorAll(".slot").forEach(td => {
    if(td.dataset.tid){
      count[td.dataset.tid] = (count[td.dataset.tid] || 0) + 1;
    }
  });
  
  teachers.forEach(t => {
    let v = count[t.teacher_id] || 0;
    let el = document.getElementById("h-" + t.teacher_id);
    if(el){
      el.textContent = v;
      let box = el.closest(".teacher");
      if(box) {
        if(v >= t.max_hours){
          box.classList.add("maxed");
        } else {
          box.classList.remove("maxed");
        }
      }
    }
  });
}

/**
 * Collect current timetable data for saving
 */
function collectData(){
  let data = [];
  document.querySelectorAll(".slot").forEach(td => {
    if(td.dataset.tid){
      data.push({
        class_id: td.dataset.classId,
        day: td.dataset.day,
        slot: td.dataset.slot,
        teacher_id: td.dataset.tid,
        course_id: ""
      });
    }
  });
  return data;
}

/**
 * Save timetable data to server
 */
function saveData(target){
  // For now, only support saving in "Show All Classes" mode
  if(!showAll) {
    showMsg("Please switch to 'Show All Classes' view before saving", false, target);
    return;
  }
  
  // Collect data from current DOM state (all classes are visible)
  let data = collectData();
  let fd = new FormData();
  fd.append("timetable_json", JSON.stringify(data));
  
  fetch(window.location.pathname, {method: "POST", body: fd})
    .then(r => r.text())
    .then(txt => {
      let isSuccess = txt.includes("SAVED");
      showMsg(txt, isSuccess, target);
      if(!isSuccess) {
        handleError("Save failed: " + txt);
      }
    })
    .catch(error => {
      handleError("Network error during save", error);
    });
}

/**
 * Show status message with better styling and timing
 */
function showMsg(msg, ok = true, target = "status2"){
  let el = document.getElementById(target);
  if(!el) {
    console.log("Status element not found:", target);
    return;
  }
  
  el.textContent = msg;
  el.className = ok ? "status-success" : "status-error";
  el.style.display = "inline-block";
  el.style.opacity = "1";
  
  // Clear message after delay
  setTimeout(() => {
    el.style.opacity = "0";
    setTimeout(() => {
      el.textContent = "";
      el.className = "";
      el.style.display = "";
      el.style.opacity = "";
    }, 300);
  }, 3000);
}

/**
 * Clear message from status element
 */
function clearMessage(target) {
  let el = document.getElementById(target);
  if(el) {
    el.textContent = "";
    el.className = "";
    el.style.display = "";
    el.style.opacity = "";
  }
}

// Event listeners
document.getElementById("saveBtn").onclick = () => saveData("status");
document.getElementById("saveBtn2").onclick = () => saveData("status2");
document.getElementById("showAll").onclick = () => {
  showAll = !showAll;
  document.getElementById("showAll").textContent = showAll ? "Show Single Class" : "Show All Classes";
  render();
};
document.getElementById("classSelect").onchange = () => {
  if(showAll) {
    showAll = false;
    document.getElementById("showAll").textContent = "Show All Classes";
  }
  render();
};

// Initial render
render();

// Update slot button styling
updateSlotButtons();

// Auto-hide system messages after delay
document.addEventListener("DOMContentLoaded", function() {
  let systemMessages = document.querySelectorAll(".system-message");
  systemMessages.forEach(msg => {
    setTimeout(() => {
      msg.style.opacity = "0";
      setTimeout(() => msg.remove(), 300);
    }, 5000);
  });
});
</script>
<script type="text/javascript" src="time_ext_printbuttons.js"></script>
<script type="text/javascript" src="place_teachers.js"></script>

</body></html>