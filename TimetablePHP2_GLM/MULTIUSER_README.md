# Timetable Multi-User Support - Implementation Summary

## Overview
Added multi-user support to the timetable management system (v4.5) with the following features:

## New Features

### 1. User Authentication
- Login system with username/password
- Session management (expires on browser close)
- User information display (real name, organization)
- Logout functionality

### 2. User-Specific Data Storage
- Each user gets their own set of timetable files
- File naming convention: `username#N_filetype.csv`
- Example: `jonathan#1_teachers.csv`, `jonathan#1_classes.csv`, `jonathan#1_timetable.csv`

### 3. Three Save/Load Slots per User
- Each user has 3 independent slots (Slot 1, Slot 2, Slot 3)
- Slot switching loads different sets of data
- Each slot contains complete timetable state (teachers, classes, assignments)

### 4. Slot Management Controls
- **Slot Switching**: Click "Slot 1", "Slot 2", or "Slot 3" to switch between datasets
- **Save to Slot**: "Save 1", "Save 2", "Save 3" buttons to save current state to specific slot
- **Copy Slot**: Copy data from one slot to another (with overwrite confirmation)

### 5. Autosave Functionality
- Automatic save every 5 minutes if changes detected
- Autosave files: `username_autosave_teachers.csv`, `username_autosave_classes.csv`, `username_autosave_timetable.csv`
- Tracks user actions (drag/drop operations) to detect changes

## File Structure

### users.csv
User accounts file with fields:
- username
- password
- date_regged
- expirydate
- realname
- orgname

### User-Specific Files
For each user:
- `username#1_teachers.csv`, `username#1_classes.csv`, `username#1_timetable.csv`
- `username#2_teachers.csv`, `username#2_classes.csv`, `username#2_timetable.csv`
- `username#3_teachers.csv`, `username#3_classes.csv`, `username#3_timetable.csv`
- `username_autosave_teachers.csv`, `username_autosave_classes.csv`, `username_autosave_timetable.csv`

### Default Files
Original files used as templates for new users:
- `teachers.csv`
- `classes.csv`
- `timetable.csv`

## Sample Users

1. **admin** / password123
   - Real Name: Administrator
   - Organization: SchoolAdmin

2. **jonathan** / secret456
   - Real Name: Jonathan Smith
   - Organization: HighSchool

3. **teacher1** / mypass789
   - Real Name: Mary Johnson
   - Organization: MiddleSchool

## User Interface

### Login Screen
- Appears when user is not logged in
- Full-screen overlay
- Username and password fields
- Error messages for invalid credentials

### Pool Section (Left Panel)
- User information display (when logged in)
- Current slot indicator
- Slot selection buttons (Slot 1, 2, 3)
- Save to slot buttons (Save 1, 2, 3)
- Copy slot controls (From/To dropdowns + Copy button)
- Logout link
- Teacher pool (existing)
- Legend (existing)

## Workflow

### Login
1. Open timetable044b.php in browser
2. Enter username and password
3. Click "Login" button
4. System loads data from Slot 1 for that user

### Working with Slots
1. **Switch Slots**: Click "Slot 1", "Slot 2", or "Slot 3" to load different datasets
2. **Save Changes**: Click "Save 1", "Save 2", or "Save 3" to save current state to that slot
3. **Copy Slots**: Use "From" and "To" dropdowns, then click "Copy" to duplicate data

### Autosave
- Automatically saves every 5 minutes if changes detected
- Changes tracked on drag/drop operations
- Logged to browser console with timestamp

### Logout
- Click "Logout" link in user info section
- Session destroyed
- Redirected to login screen

## Technical Implementation

### PHP Changes
1. Added session_start() at beginning
2. Added authentication logic
3. Added getUserFile() function for user-specific file paths
4. Added slot management handlers (save, load, copy)
5. Added autosave endpoint
6. Added login UI with PHP conditionals
7. Modified CSV loading to use user-specific files

### JavaScript Changes
1. Added slot management functions:
   - switchSlot(slotNum)
   - saveToSlot(slotNum)
   - copySlot()
2. Added autosave function
3. Added change tracking (trackChanges())
4. Added updateSlotButtons() for UI updates
5. Integrated trackChanges() into dropTeacher() and removeTeacher()
6. Set up 5-minute autosave interval

### Security Notes
- Passwords stored in plain text (users.csv manually managed)
- No CSRF protection added (consider for production)
- Session expires on browser close
- No password hashing implemented

## Limitations and Future Enhancements

### Current Limitations
1. Manual user management (no registration interface)
2. Plain text passwords
3. No password complexity requirements
4. No account expiration checking (despite field in CSV)
5. No CSRF protection
6. Autosave only tracks drag/drop, not all possible changes

### Suggested Enhancements
1. Add password hashing (bcrypt/password_hash)
2. Add user registration interface
3. Implement account expiration checking
4. Add CSRF tokens
5. Add comprehensive change detection
6. Add password reset functionality
7. Add user role management (admin vs regular user)
8. Add audit logging

## Testing

### Test Scenarios
1. Login with valid credentials
2. Login with invalid credentials (error message)
3. Switch between slots
4. Save to different slots
5. Copy data between slots
6. Verify autosave works
7. Logout and re-login (session persistence)
8. Test with multiple users (different sessions)

### File Verification
After operations, verify files exist with correct naming:
- `jonathan#1_teachers.csv`
- `jonathan_autosave_timetable.csv`
- etc.

## Files Modified
- `timetable044b.php` - Main application file
- `users.csv` - New user accounts file

## Files Created (Runtime)
- User-specific slot files (created when saving)
- Autosave files (created during autosave)
