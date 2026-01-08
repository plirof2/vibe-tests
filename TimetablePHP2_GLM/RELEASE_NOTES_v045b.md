# Timetable System v0.45b - Release Notes

## Overview
Updated multi-user support with shared teachers and classes across all slots.

## Changes from v0.45a

### 1. Removed "Load From" Dropdown
- Removed the "Load from" dropdown in loadControls section
- Slot switching buttons now handle all loading operations
- Cleaner interface with less clutter

### 2. Shared Teachers and Classes
**Previous Behavior (v0.45a):**
- Each slot had its own copy of teachers.csv and classes.csv
- Example: `username#1_teachers.csv`, `username#2_teachers.csv`, etc.
- Example: `username#1_classes.csv`, `username#2_classes.csv`, etc.

**New Behavior (v0.45b):**
- Each user has ONE set of teachers and classes shared across all slots
- Files: `username_teachers.csv` (single file per user)
- Files: `username_classes.csv` (single file per user)
- Only timetable files are slot-specific

### 3. Slot-Specific Timetables Only
**File Structure:**
```
username_teachers.csv       (Shared across all slots)
username_classes.csv        (Shared across all slots)
username#1_timetable.csv    (Slot 1 timetable)
username#2_timetable.csv    (Slot 2 timetable)
username#3_timetable.csv    (Slot 3 timetable)
username_autosave_timetable.csv  (Autosave)
```

### 4. Updated Save Operations
**Save to Slot:**
- Now only saves timetable data
- Teachers and classes are managed via crud_csv.php
- Prevents duplication of data

**Autosave:**
- Only saves timetable data
- Faster autosave operations

**Copy Slot:**
- Only copies timetable.csv between slots
- Teachers and classes are not affected

### 5. Updated crud_csv.php
**Multi-User Support:**
- Added session management
- Redirects to login if not authenticated
- Uses user-specific files: `username_teachers.csv`, `username_classes.csv`
- Creates user files from defaults if they don't exist
- Added user info display with logout link
- Added "Back to Timetable" link

## User Interface Changes

### Pool Section (Left Panel)
- **User Info Display**: Shows current user name and organization
- **Slot Controls**: Slot 1/2/3 buttons for switching
- **Save Controls**: Save 1/2/3 buttons to save to specific slots
- **Copy Slot Controls**: Copy data between slots (timetable only)
- **Logout Link**: User can logout from the system
- **Teacher Pool**: Same as before
- **Legend**: Same as before

### Removed Elements
- "Load from" dropdown (replaced by slot buttons)
- "Load" button (no longer needed)

## crud_csv.php Enhancements

### User Management
- Session-based authentication
- Automatic user file creation from defaults
- User information display
- Logout functionality
- Navigation back to timetable

### File Handling
- Uses `username_teachers.csv` instead of `teachers.csv`
- Uses `username_classes.csv` instead of `classes.csv`
- Copies default files for new users
- All CRUD operations work on user-specific files

## Workflow

### Managing Teachers and Classes
1. Open crud_csv.php
2. Automatically uses your user's teachers and classes
3. Add/Edit/Delete teachers and classes
4. Changes apply to all slots automatically
5. Click "Back to Timetable" to return to main application

### Working with Timetable Slots
1. Switch between slots using Slot 1/2/3 buttons
2. Each slot has different teacher assignments
3. Teachers and classes pool is shared across slots
4. Save to slot stores only timetable assignments

### Example Scenario
**User: jonathan**

1. **Teachers/Classes (Shared):**
   - jonathan_teachers.csv (10 teachers)
   - jonathan_classes.csv (6 classes)
   - Managed via crud_csv.php

2. **Slot 1 (Draft):**
   - jonathan#1_timetable.csv
   - Work on draft timetable

3. **Slot 2 (Alternative):**
   - jonathan#2_timetable.csv
   - Create alternative schedule with same teachers

4. **Slot 3 (Final):**
   - jonathan#3_timetable.csv
   - Final approved schedule

**Benefits:**
- No need to recreate teachers for each slot
- Teachers pool consistent across all timetables
- Classes list consistent across all timetables
- Can test different assignments with same resources

## File Management

### New Users
1. User logs in for the first time
2. System creates `username_teachers.csv` from `teachers.csv`
3. System creates `username_classes.csv` from `classes.csv`
4. User can then modify their own copies

### Existing Users
- Continue using existing user files
- No migration needed
- System automatically detects and uses user files

## Technical Implementation

### PHP Changes (timetable044b.php)
1. Updated `getUserFile()` function:
   - Returns `username_teachers.csv` for teachers
   - Returns `username_classes.csv` for classes
   - Returns `username#N_timetable.csv` for timetable

2. Modified save operations:
   - `saveToSlot()` - only saves timetable
   - `autosave()` - only saves timetable
   - `copySlot()` - only copies timetable

3. Removed:
   - loadControls div with dropdown
   - loadBackup() JavaScript function
   - Event listener for loadBtn

### PHP Changes (crud_csv.php)
1. Added session management
2. User-specific file paths
3. Automatic file creation from defaults
4. User info display in UI
5. Logout functionality

### JavaScript Changes (timetable044b.php)
1. Removed `teachers_json` and `classes_json` from saveToSlot
2. Removed `teachers_json` and `classes_json` from autosave
3. Removed loadBtn event listener
4. Removed loadBackup() function

## Migration Notes

### From v0.45a to v0.45b
**Breaking Change:**
- Existing users may have `username#1_teachers.csv`, `username#2_teachers.csv`, etc.
- These files are no longer used

**Recommended Migration:**
1. Choose the best version of teachers/classes from your slots
2. Manually copy it to `username_teachers.csv` and `username_classes.csv`
3. Delete old slot-specific teacher/class files
4. All slots will now use the shared files

**Example Migration:**
```bash
# Choose Slot 2's teachers as the master
cp jonathan#2_teachers.csv jonathan_teachers.csv

# Choose Slot 1's classes as the master
cp jonathan#1_classes.csv jonathan_classes.csv

# Optionally clean up old files (backup first!)
rm jonathan#1_teachers.csv
rm jonathan#2_teachers.csv
rm jonathan#3_teachers.csv
rm jonathan#1_classes.csv
rm jonathan#2_classes.csv
rm jonathan#3_classes.csv
```

## Testing

### Test Scenarios
1. **Login**: Verify users can login and see their data
2. **Teacher Management**: Use crud_csv.php to add/edit/delete teachers
3. **Class Management**: Use crud_csv.php to add/edit/delete classes
4. **Slot Switching**: Switch between slots, verify teachers/classes remain constant
5. **Save to Slot**: Save timetable to different slots, verify only timetable is saved
6. **Copy Slot**: Copy timetable between slots, verify teachers/classes not affected
7. **Autosave**: Verify autosave only saves timetable data
8. **Logout/Logout**: Verify logout works and session is cleared

### File Verification
After operations, verify files:
```
jonathan_teachers.csv      ✓
jonathan_classes.csv       ✓
jonathan#1_timetable.csv  ✓
jonathan#2_timetable.csv  ✓
jonathan#3_timetable.csv  ✓
jonathan_autosave_timetable.csv  ✓
```

## Files Modified
- `timetable044b.php` - Updated to v0.45b
- `crud_csv.php` - Added multi-user support

## Files Created (Runtime)
- User-specific teachers and classes files
- Slot-specific timetable files
- Autosave timetable files

## Benefits of v0.45b

1. **Cleaner Interface**: Removed redundant "Load from" dropdown
2. **Consistent Resources**: Teachers and classes are shared, no duplication
3. **Better Data Management**: Single source of truth for teachers and classes
4. **Faster Operations**: Saving and copying only affects timetable data
5. **Improved Workflow**: Can test different assignments with same resources
6. **Lower Storage**: Less file duplication
7. **Easier Maintenance**: Update teachers/classes once, apply to all slots

## Limitations

1. **No Shared Resources Between Users**: Each user has their own teachers/classes
2. **Manual Migration**: Users upgrading from v0.45a need manual cleanup
3. **No Resource Templates**: Each user gets copy of default files

## Future Enhancements

1. **Shared Resource Library**: Option to share teachers/classes between users
2. **Resource Templates**: Admin-defined templates for new users
3. **Migration Tool**: Automatic migration from v0.45a to v0.45b
4. **Resource Versioning**: Track changes to teachers/classes over time
5. **Resource Permissions**: Define which users can modify which resources
