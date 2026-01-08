<?php
/**********************************************************************************************************************************/
/* Project      : SGD Web                                                                                                          */
/* Module       : SCR to PNG Converter                                                                                               */
/* Description  : Convert ZX Spectrum SCR files to PNG format                                                                      */
/* Version      : 1.0                                                                                                               */
/* Last changed : 30-12-2025                                                                                                        */
/**********************************************************************************************************************************/

if (!isset($_GET['scrimg'])) {
    header('HTTP/1.0 400 Bad Request');
    exit;
}

$scrFile = $_GET['scrimg'];
$scrPath = realpath($scrFile);

$allowedPaths = [
    realpath('ROMSMINE'),
];

$validPath = false;
foreach ($allowedPaths as $path) {
    if ($scrPath && strpos($scrPath, $path) === 0) {
        $validPath = true;
        break;
    }
}

if (!$validPath || !$scrPath || !file_exists($scrPath)) {
    header('HTTP/1.0 404 Not Found');
    exit;
}

$scrData = file_get_contents($scrPath);
if (strlen($scrData) != 6912) {
    header('Content-Type: image/png');
    $im = imagecreatetruecolor(256, 192);
    $white = imagecolorallocate($im, 255, 255, 255);
    $black = imagecolorallocate($im, 0, 0, 0);
    imagefill($im, 0, 0, $white);
    
    $textColor = imagecolorallocate($im, 0, 0, 0);
    imagestring($im, 3, 10, 90, 'Invalid SCR file (not 6912 bytes)', $textColor);
    
    imagepng($im);
    imagedestroy($im);
    exit;
}

$zxColors = [
    [0, 0, 0],       // 0: Black
    [0, 0, 192],     // 1: Blue
    [192, 0, 0],     // 2: Red
    [192, 0, 192],   // 3: Magenta
    [0, 192, 0],     // 4: Green
    [0, 192, 192],   // 5: Cyan
    [192, 192, 0],   // 6: Yellow
    [192, 192, 192], // 7: White
    [0, 0, 0],       // 8: Black (bright)
    [0, 0, 255],     // 9: Blue (bright)
    [255, 0, 0],     // 10: Red (bright)
    [255, 0, 255],   // 11: Magenta (bright)
    [0, 255, 0],     // 12: Green (bright)
    [0, 255, 255],   // 13: Cyan (bright)
    [255, 255, 0],   // 14: Yellow (bright)
    [255, 255, 255], // 15: White (bright)
];

$im = imagecreatetruecolor(256, 192);
$palette = [];

foreach ($zxColors as $color) {
    $palette[] = imagecolorallocate($im, $color[0], $color[1], $color[2]);
}

for ($y = 0; $y < 192; $y++) {
    for ($x = 0; $x < 256; $x++) {
        $attrY = floor($y / 8);
        $attrX = floor($x / 4);
        $attrAddr = ($attrY * 32) + $attrX + 6144;
        
        $attr = ord($scrData[$attrAddr]);
        $ink = ($attr & 0x07);
        $paper = (($attr & 0x38) >> 3);
        $bright = ($attr & 0x40) >> 6;
        
        if ($bright) {
            $ink += 8;
            $paper += 8;
        }
        
        $pixelAddr = ((($y & 0xC0) << 5) + (($y & 0x07) << 8) + (($y & 0x38) << 2) + floor($x / 8));
        $pixelByte = ord($scrData[$pixelAddr]);
        $pixelBit = 7 - ($x % 8);
        $pixel = ($pixelByte >> $pixelBit) & 0x01;
        
        $color = $pixel ? $palette[$ink] : $palette[$paper];
        imagesetpixel($im, $x, $y, $color);
    }
}

header('Content-Type: image/png');
header('Cache-Control: public, max-age=86400');
imagepng($im);
imagedestroy($im);
?>
