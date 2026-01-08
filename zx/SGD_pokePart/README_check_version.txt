check_version.php - Z80 Version Detection Script
============================================

DESCRIPTION:
Detects Z80 snapshot file version (1.45, 2.01, or 3.00) and logs results.

USAGE:
--------
CLI:  php check_version.php file=path/to/file.Z80
Web:  check_version.php?file=path/to/file.Z80&POKES=...

(POKES parameter is currently ignored - for future implementation)


DETECTION LOGIC:
---------------
1. Read Z80 header (first 30 bytes)
2. Check PC register at bytes 6-7:
   - If PC != 0: Version 1.45
   - If PC == 0: Read additional header length at bytes 30-31
     * If length == 23: Version 2.01
     * If length == 54: Version 3.00


Z80 VERSIONS:
-------------

v1.45 (Old Format):
- PC != 0 in main header
- No additional header
- Single 48KB memory block (compressed or uncompressed)
- Supports 48K only
- File size: ~30KB - 48KB

v2.01 (Extended Format):
- PC == 0 in main header (signals extended format)
- Additional header length = 23 bytes
- Supports 48K and 128K memory banking
- Multiple 16KB memory blocks
- File size: ~35KB - 70KB

v3.00 (Extended Format):
- PC == 0 in main header (signals extended format)
- Additional header length = 54 bytes
- Extended v2.01 structure with more hardware details
- Supports 48K, 128K+, and other configurations
- Multiple 16KB memory blocks
- File size: ~50KB - 80KB


TEST RESULTS:
-------------

1. ROMSMINE/F/FIRELORD.Z80
   Size: 41.79KB
   Version: 1.45
   PC: 58429 (0xE43D)
   Status: Valid v1.45 format

2. ROMSMINE/L/LIGHT128.Z80
   Size: 61.53KB
   Version: 3.00
   PC: 0
   Additional Header Length: 54
   Hardware Mode: 5 (128K)
   Status: Valid v3.00 format

3. ROMSMINE/L/LASTDUEL.Z80
   Size: 38.11KB
   Version: 2.01
   PC: 0
   Additional Header Length: 23
   Hardware Mode: 3 (128K)
   Status: Valid v2.01 format


OUTPUT LOG:
-----------
Results are written to: output.log
Format: [YYYY-MM-DD HH:MM:SS] File: path | Size: X.XXKB | Z80 Version: X.XX

Example:
[2026-01-01 22:24:15] File: ROMSMINE/F/FIRELORD.Z80 | Size: 41.79KB | Z80 Version: 1.45
[2026-01-01 22:24:23] File: ROMSMINE/L/LIGHT128.Z80 | Size: 61.53KB | Z80 Version: 3.00
[2026-01-01 22:24:33] File: ROMSMINE/L/LASTDUEL.Z80 | Size: 38.11KB | Z80 Version: 2.01


FEATURES:
---------
- CLI and web mode support
- Path security (prevents directory traversal)
- File existence and readability checks
- Detailed error logging
- File size information included
- Timestamped log entries


ERROR HANDLING:
---------------
- File not found: Logs error and exits
- Invalid Z80 format: Returns "Unknown version" with details
- File read errors: Returns false and logs error


FUTURE ENHANCEMENTS:
--------------------
- POKES parameter parsing and application
- Hardware mode detection and reporting
- Compression status detection
- Memory bank mapping for 128K files
- Batch processing multiple files
