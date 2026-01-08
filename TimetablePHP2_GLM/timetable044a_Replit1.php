<?php
$version="timetable044a_replit_showmsg";
/*
===========================================
 Timetable Script - Version 4.3 FIXED
===========================================
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
===========================================
*/

$days=["Mon","Tue","Wed","Thu","Fri"];
$slots=range(1,9);

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

$teachers = read_csv("teachers.csv");
$classes = read_csv("classes.csv");
$timetable = read_csv("timetable.csv");

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
  }
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
        
<div id="pool" ondragover="event.preventDefault()" ondrop="removeTeacher(event)">
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
  
  <div id="loadControls">
    <label for="backupSelect">Load from:</label>
    <select id="backupSelect">
      <option value="timetable.csv">timetable.csv</option>
      <?php
      $backup_files = get_backup_files();
      foreach($backup_files as $file) {
        if($file !== "timetable.csv") {
          echo "<option value='" . htmlspecialchars($file) . "'>" . htmlspecialchars($file) . "</option>";
        }
      }
      ?>
    </select>
    <button id="loadBtn">Load</button>
  </div>
  
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

/**
 * Load backup file with confirmation and validation
 */
function loadBackup() {
  let selectedFile = document.getElementById("backupSelect").value;
  if(!selectedFile) {
    showMsg("Please select a backup file", false, "status2");
    return;
  }
  
  if(confirm("Load " + selectedFile + "? This will overwrite current timetable data.")) {
    // Show loading message
    showMsg("Loading backup...", true, "status2");
    window.location.href = window.location.pathname + "?load_backup=" + encodeURIComponent(selectedFile);
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
document.getElementById("loadBtn").onclick = loadBackup;

// Initial render
render();

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
</body></html>