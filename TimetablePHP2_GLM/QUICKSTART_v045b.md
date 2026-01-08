# Timetable v0.45b - Quick Start Guide

## What's New in v0.45b?

### Teachers and Classes are Now Shared
- You have ONE set of teachers and classes that work across ALL your slots
- No need to recreate teachers for each timetable version
- Use **crud_csv.php** to manage teachers and classes

### Slots Are for Timetables Only
- Each slot (1, 2, 3) stores a different teacher assignment
- Same teachers and classes, different assignments

## Getting Started

### First Time Login
1. Open `timetable044b.php` in your browser
2. Enter your username and password
3. Your teachers and classes are automatically created from defaults
4. You can customize them using **crud_csv.php**

### Managing Teachers and Classes

1. **Open crud_csv.php**
   - Click the link or navigate to crud_csv.php
   - You'll see your user info at the top

2. **Add Teachers**
   - Fill in teacher details
   - Click "Save (Add/Update)"
   - Teacher appears in your pool for all slots

3. **Add Classes**
   - Fill in class details
   - Click "Save (Add/Update)"
   - Class appears in your timetables

4. **Edit/Delete**
   - Click "Edit" to modify existing entries
   - Click "Delete" to remove entries
   - Use ↑/↓ arrows to reorder

5. **Return to Timetable**
   - Click "← Back to Timetable"
   - Your teachers and classes are ready to use

## Working with Slots

### Switching Slots
- Click "Slot 1", "Slot 2", or "Slot 3"
- Each slot has a different timetable assignment
- Teachers and classes remain the same across all slots

### Saving to Slots
- Click "Save 1", "Save 2", or "Save 3"
- Saves your current timetable assignments to that slot
- Teachers and classes are NOT saved (they're shared)
- You must be in "Show All Classes" view to save

### Copying Slots
- Use "Copy Slot" to duplicate a timetable
- Select "From" slot (source)
- Select "To" slot (destination)
- Click "Copy"
- Only the timetable is copied, not teachers/classes

## Example Workflow

### Scenario: Testing Different Schedules

**Setup (do once):**
1. Login to timetable
2. Go to crud_csv.php
3. Add your teachers: John, Mary, Robert, etc.
4. Add your classes: Class A, Class B, etc.
5. Return to timetable

**Create Draft (Slot 1):**
1. Click "Slot 1" to start fresh
2. Drag teachers to create draft schedule
3. Click "Save 1" to save draft

**Create Alternative (Slot 2):**
1. Click "Slot 2"
2. Drag teachers to create alternative schedule
3. Click "Save 2" to save alternative

**Create Final (Slot 3):**
1. Click "Slot 3"
2. Drag teachers to create final schedule
3. Click "Save 3" to save final

**Review All:**
1. Click "Slot 1" to see draft
2. Click "Slot 2" to see alternative
3. Click "Slot 3" to see final
4. Teachers and classes are the same in all slots!

## Quick Reference

### Slot Buttons
- **Slot 1 / 2 / 3**: Switch to that slot's timetable
- **Save 1 / 2 / 3**: Save current timetable to that slot
- **Copy Slot**: Copy timetable from one slot to another

### Navigation
- **← Back to Timetable**: From crud_csv.php back to main app
- **Logout**: End your session

### File Locations (Your User: jonathan)
```
jonathan_teachers.csv       # Your teachers (shared)
jonathan_classes.csv        # Your classes (shared)
jonathan#1_timetable.csv    # Slot 1 assignments
jonathan#2_timetable.csv    # Slot 2 assignments
jonathan#3_timetable.csv    # Slot 3 assignments
```

## Common Tasks

### Add a New Teacher
1. Go to crud_csv.php
2. Scroll to "Add New Teacher" form
3. Fill in: teacher_id, name, max_hours, color, etc.
4. Click "Save (Add/Update)"
5. Return to timetable
6. Teacher appears in pool for all slots

### Change Teacher Details
1. Go to crud_csv.php
2. Find teacher in table
3. Click "Edit"
4. Modify details
5. Click "Save (Add/Update)"
6. Changes apply to ALL slots automatically

### Create a New Timetable Version
1. Switch to a slot (Slot 1, 2, or 3)
2. Drag and drop teachers to create schedule
3. Click "Save 1" (or 2 or 3) to save
4. Switch to another slot to create another version
5. Same teachers, different assignments

### Compare Two Versions
1. Click "Slot 1" to see first version
2. Note the assignments
3. Click "Slot 2" to see second version
4. Compare assignments
5. Teachers and classes are identical, only assignments differ

## Tips

### Best Practices
1. **Manage Resources in One Place**: Use crud_csv.php for all teacher/class changes
2. **Use Slots for Experiments**: Test different schedules without affecting the original
3. **Save Frequently**: Click save buttons after making changes
4. **Use Copy Slot**: Duplicate a good version before making major changes
5. **Autosave**: System saves every 5 minutes if you make changes

### File Management
- **Don't Delete User Files**: `jonathan_teachers.csv` and `jonathan_classes.csv` are your master files
- **Slot Files Can Be Replaced**: Timetable slots can be overwritten by saving or copying
- **Autosave**: Check `jonathan_autosave_timetable.csv` if you lose unsaved changes

## Troubleshooting

### Teachers Not Appearing
1. Go to crud_csv.php
2. Verify teachers are listed
3. If not, add them
4. Return to timetable
5. Refresh page (F5)

### Changes Not Saving to All Slots
1. This is normal! Teachers/classes are shared, but assignments are per-slot
2. Teachers/classes changes appear in all slots
3. Timetable assignments must be saved to each slot separately

### Can't Save to Slot
1. Make sure you're in "Show All Classes" view
2. Click "Show All Classes" button if needed
3. Then click save button

### Wrong Teachers in a Slot
1. Check which teachers are selected
2. Go to crud_csv.php to verify master teacher list
3. Changes to master list apply to all slots
4. Return to timetable to refresh

## Sample Accounts

- **admin** / password123
- **jonathan** / secret456
- **teacher1** / mypass789

## Support

For issues or questions:
1. Check RELEASE_NOTES_v045b.md for detailed information
2. Review this quick start guide
3. Contact your system administrator

---

**Version**: v0.45b
**Release Date**: December 25, 2025
**Changes**: Multi-user support with shared teachers/classes
