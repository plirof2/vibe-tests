# SGD Web - Spectrum Games Database (web_sgd_glm01.php)

Single-file web application for managing ZX Spectrum game database.

## Features

- **Game Database**: View and search Spectrum games from DAT file
- **Game Details**: Display all 19 game fields (Name, Year, Publisher, Author, etc.)
- **Media Display**: Show inlays, screenshots, and game descriptions
- **Emulator Integration**: Launch games in QAOP.js emulator
- **Filtering**: Filter by name, type, year, or first letter
- **Authentication**: 
  - Read-only access for all visitors
  - Edit access for authenticated users
- **Backup**: Automatic timestamped backups before edits
- **Concurrency Protection**: Optimistic locking to prevent conflicts

## Files

- `web_sgd_glm01.php` - Main application (single-file SPA)
- `scr2png.php` - Spectrum SCR to PNG converter
- `HAD_ZX.DAT` - Game database file
- `ROMSMINE/` - Game files directory structure
- `ROMSMINE/_SGD304/sgd.ini` - Configuration file

## Configuration

Edit variables at top of `web_sgd_glm01.php`:

```php
define('DAT_FILE', 'HAD_ZX.DAT');           // Database file
define('INI_FILE', 'ROMSMINE/_SGD304/sgd.ini');  // Config file
define('QAOP_PATH', 'QAOP/qaop.html');      // Emulator path
define('ROMSMINE_PATH', 'ROMSMINE/');       // Games directory
```

### Authentication

Edit users array in `web_sgd_glm01.php`:

```php
$EDIT_USERS = [
    'admin' => 'admin123',
    'editor' => 'editor456'
];
```

Passwords are stored in plain text as requested.

## Usage

### Viewing Games

1. Open `http://localhost/SGDsrc/web_sgd_glm01.php` in browser
2. Games load automatically from HAD_ZX.DAT
3. Click any game to view details
4. Use filters to search games

### Playing Games

1. Select a game from the list
2. Click "PLAY (QAOP)" button
3. Game opens in emulator overlay

### Editing Games (Authenticated Users Only)

1. Click "Login" button
2. Enter username and password
3. Select a game
4. Edit fields in the form
5. Click "SAVE" button
6. Backup is created automatically before save

## File Structure

```
ROMSMINE/
├── A/              # Games starting with A
│   ├── *.SNA       # Game files
│   ├── *.Z80
│   ├── *.TAP
│   ├── *.TZX
│   ├── INLAYS/     # Box scans
│   │   └── *.GIF
│   ├── SCRSHOT/    # Screenshots
│   │   └── *.SCR
│   └── INFO/       # Descriptions
│       └── *.TXT
├── B/              # Games starting with B
├── C/
└── ...
```

## Backup System

When saving edits:
- Original file copied to `{filename}_YYMMDD_HHMM.dat`
- Example: `HAD_ZX.DAT_251230_1435.dat`
- Backups stored in same directory as original

## Concurrency Handling

- Application checks file modification time before saving
- If file was modified by another user, error message displayed
- User must reload page to see latest changes

## Field Definitions

All 19 fields from DAT format supported:

1. Name (36 chars) - Game name
2. Year (4 chars) - Release year
3. Publisher (36 chars) - Publisher name
4. Author (100 chars) - Developer
5. Memory (3 chars) - 48/128K
6. NoOfPlayers (1 char) - Player count
7. Together (1 char) - Simultaneous play
8. Joysticks (5 chars) - Joystick support
9. PC-Name (12 chars) - File name
10. Type (7 chars) - Game type
11. PathIndex (3 chars) - Directory index
12. FileSize (7 chars) - File size
13. Orig_screen (1 char) - Original screen flag
14. Origin (1 char) - Country of origin
15. FloppyId (4 chars) - Disk ID
16. Emul_override (2 chars) - Emulator override
17. AYSound (1 char) - AY sound support
18. MultiLoad (1 char) - Multi-load flag
19. Language (3 chars) - Language code
20. Score (3 chars) - User rating

## Image Features

- **Click to expand**: Images resize from 300px to full width
- **Auto-hide**: Missing images hidden automatically
- **SCR conversion**: Spectrum SCR files converted to PNG on-the-fly

## Error Handling

- In-page error messages (no browser alerts)
- Success messages auto-hide after 3 seconds
- Error messages auto-hide after 5 seconds
- Files allowed to edit even if game files missing

## Security

- Session-based authentication
- Path validation for file access
- SCR file conversion restricted to ROMSMINE directory
- Plain text passwords (as requested)

## Requirements

- PHP 8.x
- Apache web server
- GD library for image processing
- QAOP.js emulator (for game playing)

## Troubleshooting

### Games not loading
- Check HAD_ZX.DAT file exists and is readable
- Check PHP file permissions

### Images not displaying
- Check ROMSMINE directory structure exists
- Verify INLAYS/SCRSHOT/INFO subdirectories
- Check file permissions

### Emulator not loading
- Verify QAOP_PATH is correct
- Ensure QAOP/qaop.html exists
- Check browser console for errors

### Login not working
- Verify username/password in $EDIT_USERS array
- Check session.save_path in php.ini
- Clear browser cookies

## API Endpoints

The application uses internal API endpoints:

### GET
- `?api=list` - Get all games (includes mtime for concurrency)
- `?api=get&index=N` - Get specific game
- `?api=getFilenames&filename=X&pathIndex=N` - Get file paths
- `?api=auth` - Check authentication status
- `?api=gameDirs` - Get game directories from sgd.ini

### POST
- `?api=login` - Login (requires username, password)
- `?api=logout` - Logout
- `?api=update` - Update game entry (requires authentication)
  - Parameters: index, mtime, game object

## License

Based on original SGD (Spectrum Games Database) by Martijn van der Heide.
Web implementation 2025.

## Support

For issues or questions, refer to original SGD documentation in SGD.DOC.
