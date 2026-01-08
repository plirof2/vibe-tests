const VERSION_NUMBER = "0.0.10";
const DEFAULT_OUTPUT = "out.tap";

const SinclairBasicKeywords = {
    "RND": 165, "INKEY$": 166, "PI": 167, "FN": 168, "POINT": 169, "SCREEN$": 170, "ATTR": 171,
    "AT": 172, "TAB": 173, "VAL$": 174, "CODE": 175, "VAL": 176, "LEN": 177, "SIN": 178, "COS": 179,
    "TAN": 180, "ASN": 181, "ACS": 182, "ATN": 183, "LN": 184, "EXP": 185, "INT": 186, "SQR": 187,
    "SGN": 188, "ABS": 189, "PEEK": 190, "IN": 191, "USR": 192, "STR$": 193, "CHR$": 194,
    "NOT": 195, "BIN": 196, "OR": 197, "AND": 198, "LINE": 202, "THEN": 203, "TO": 204, "STEP": 205,
    "DEF FN": 206, "CAT": 207, "FORMAT": 208, "MOVE": 209, "ERASE": 210, "OPEN #": 211, "CLOSE #": 212,
    "MERGE": 213, "VERIFY": 214, "BEEP": 215, "CIRCLE": 216, "INK": 217, "PAPER": 218, "FLASH": 219,
    "BRIGHT": 220, "INVERSE": 221, "OVER": 222, "OUT": 223, "LPRINT": 224, "LLIST": 225, "STOP": 226,
    "READ": 227, "DATA": 228, "RESTORE": 229, "NEW": 230, "BORDER": 231, "CONTINUE": 232, "DIM": 233,
    "REM": 234, "FOR": 235, "GO TO": 236, "GOTO": 236, "GO SUB": 237, "GOSUB": 237, "INPUT": 238,
    "LOAD": 239, "LIST": 240, "LET": 241, "PAUSE": 242, "NEXT": 243, "POKE": 244, "PRINT": 245,
    "PLOT": 246, "RUN": 247, "SAVE": 248, "RANDOMIZE": 249, "RANDOMISE": 249, "IF": 250, "CLS": 251,
    "DRAW": 252, "CLEAR": 253, "RETURN": 254, "COPY": 255
};

const SinclairBasic = {
    "PRINT comma": 6, "Edit": 7, "Backspace": 12, "Enter": 13, "number": 14, "Not used": 15,
    "INK control": 16, "PAPER control": 17, "FLASH control": 18, "BRIGHT control": 19, "INVERSE control": 20,
    "OVER control": 21, "AT control": 22, "TAB control": 23, "SPACE": 32, " ": 32, "!": 33, "\"": 34,
    "#": 35, "$": 36, "%": 37, "&": 38, "'": 39, "(": 40, ")": 41, "*": 42, "+": 43, ",": 44, "-": 45,
    ".": 46, "/": 47, "0": 48, "1": 49, "2": 50, "3": 51, "4": 52, "5": 53, "6": 54, "7": 55,
    "8": 56, "9": 57, ":": 58, ";": 59, "<": 60, "=": 61, ">": 62, "?": 63, "@": 64, "A": 65,
    "B": 66, "C": 67, "D": 68, "E": 69, "F": 70, "G": 71, "H": 72, "I": 73, "J": 74, "K": 75, "L": 76,
    "M": 77, "N": 78, "O": 79, "P": 80, "Q": 81, "R": 82, "S": 83, "T": 84, "U": 85, "V": 86,
    "W": 87, "X": 88, "Y": 89, "Z": 90, "[": 91, "\\": 92, "]": 93, "^": 94, "_": 95, "£": 96,
    "a": 97, "b": 98, "c": 99, "d": 100, "e": 101, "f": 102, "g": 103, "h": 104, "i": 105,
    "j": 106, "k": 107, "l": 108, "m": 109, "n": 110, "o": 111, "p": 112, "q": 113, "r": 114,
    "s": 115, "t": 116, "u": 117, "v": 118, "w": 119, "x": 120, "y": 121, "z": 122, "{": 123,
    "|": 124, "}": 125, "~": 126, "©": 127, "BLANKBLANK": 128, "BLANK'": 129, "'BLANK": 130,
    "''": 131, "BLANK.": 132, "BLANK:": 133, "'.": 134, "':": 135, ".BLANK": 136, ".'": 137,
    ":BLANK": 138, ":'": 139, "..": 140, ".:": 141, ":.": 142, "::": 143, "(a)": 144, "(b)": 145,
    "(c)": 146, "(d)": 147, "(e)": 148, "(f)": 149, "(g)": 150, "(h)": 151, "(i)": 152, "(j)": 153,
    "(k)": 154, "(l)": 155, "(m)": 156, "(n)": 157, "(o)": 158, "(p)": 159, "(q)": 160, "(r)": 161,
    "(s)": 162, "(t)": 163, "(u)": 164, "<=": 199, ">=": 200, "<>": 201
};

function strReplaceAssoc(replace, subject) {
    let result = subject;
    for (const key in replace) {
        result = result.split(key).join(replace[key]);
    }
    return result;
}

function strReplaceAssoci(replace, subject) {
    let result = subject;
    for (const key in replace) {
        const regex = new RegExp(key, 'gi');
        result = result.replace(regex, replace[key]);
    }
    return result;
}

function greeklish(str) {
    const greekWords = ['ΤΥΠΩΣΕ', 'ΧΑΡΤΙ', 'ΜΕΛΑΝΙ', 'ΦΛΑΣ', 'ΠΕΡΙΘΩΡΙΟ', 'ΓΙΑ', 'ΕΠΟΜΕΝΟ', 'ΠΗΓΑΙΝΕ', 'ΠΑΝΕΕΛΑ', 'ΕΠΙΣΤΡΟΦΗ'];
    const englishWords = ['PRINT', 'PAPER', 'INK', 'FLASH', 'BORDER', 'FOR', 'NEXT', 'GOTO', 'GOSUB', 'RETURN'];
    
    let result = str;
    for (let i = 0; i < greekWords.length; i++) {
        result = result.split(greekWords[i]).join(englishWords[i]);
    }
    
    const greek = ['α', 'ά', 'Ά', 'Α', 'β', 'Β', 'γ', 'Γ', 'δ', 'Δ', 'ε', 'έ', 'Ε', 'Έ', 'ζ', 'Ζ', 'η', 'ή', 'Η', 'θ', 'Θ',
        'ι', 'ί', 'ϊ', 'ΐ', 'Ι', 'Ί', 'κ', 'Κ', 'λ', 'Λ', 'μ', 'Μ', 'ν', 'Ν', 'ξ', 'Ξ', 'ο', 'ό', 'Ο', 'Ό', 'π', 'Π', 'ρ', 'Ρ', 'σ',
        'ς', 'Σ', 'τ', 'Τ', 'υ', 'ύ', 'Υ', 'Ύ', 'φ', 'Φ', 'χ', 'Χ', 'ψ', 'Ψ', 'ω', 'ώ', 'Ω', 'Ώ', ' '];
    const english = ['a', 'a', 'A', 'A', 'b', 'B', 'g', 'G', 'd', 'D', 'e', 'e', 'E', 'E', 'z', 'Z', 'i', 'i', 'I', 'th', 'Th',
        'i', 'i', 'i', 'i', 'I', 'I', 'k', 'K', 'l', 'L', 'm', 'M', 'n', 'N', 'x', 'X', 'o', 'o', 'O', 'O', 'p', 'P', 'r', 'R', 's',
        's', 'S', 't', 'T', 'u', 'u', 'Y', 'Y', 'f', 'F', 'x', 'X', 'ps', 'Ps', 'o', 'o', 'O', 'O', ' '];
    
    for (let i = 0; i < greek.length; i++) {
        result = result.split(greek[i]).join(english[i]);
    }
    return result;
}

function floatToBytes(value) {
    const buffer = new ArrayBuffer(4);
    const view = new DataView(buffer);
    view.setFloat32(0, value, true);
    return new Uint8Array(buffer);
}

function frexp(number) {
    const returnArray = [];
    
    if (Number.isInteger(number) && number >= -65535 && number <= 65535) {
        returnArray.push(0x0E);
        returnArray.push(0x00);
        if (number < 0) {
            returnArray.push(0xFF);
        } else {
            returnArray.push(0x00);
        }
        returnArray.push(number & 0xff);
        returnArray.push((number & 0xff00) >> 8);
        returnArray.push(0x00);
    } else {
        const exponent = Math.floor(Math.log2(number)) + 1;
        if (exponent < -128 || exponent > 127) {
            throw new Error("Exponent out of range (number too big).");
        }
        const mantissa = number * Math.pow(2, -exponent);
        const mantissaBytes = floatToBytes(mantissa);
        
        returnArray.push(0x0E);
        returnArray.push(0x80 + exponent);
        returnArray.push(mantissaBytes[3]);
        returnArray.push(mantissaBytes[2]);
        returnArray.push(mantissaBytes[1]);
        returnArray.push(mantissaBytes[0]);
    }
    return returnArray;
}

function prependPlus3Header(outputData, autostartLine) {
    const fileSize = outputData.length + 128;
    
    const header = [];
    header.push("P".charCodeAt(0), "L".charCodeAt(0), "U".charCodeAt(0), "S".charCodeAt(0));
    header.push("3".charCodeAt(0), "D".charCodeAt(0), "O".charCodeAt(0), "S".charCodeAt(0));
    header.push(0x1A, 0x01, 0x00);
    header.push(fileSize & 0xFF, (fileSize & 0xFF00) >> 8, (fileSize & 0xFF0000) >> 16, (fileSize & 0xFF000000) >> 24);
    header.push(0x00);
    
    const dataSize = fileSize - 128;
    header.push(dataSize & 0x00FF, (dataSize & 0xFF00) >> 8);
    header.push(autostartLine & 0x00FF, (autostartLine & 0xFF00) >> 8);
    header.push(dataSize & 0x00FF, (dataSize & 0xFF00) >> 8);
    
    while (header.length < 127) header.push(0x00);
    
    let checksum = 0;
    for (let i = 0; i < 127; i++) checksum += header[i];
    header.push(checksum & 0xFF);
    
    return new Uint8Array([...header, ...outputData]);
}

function prependSpectrumTapeHeader(outputData, spectrumFilename, autostartLine) {
    const fileSize = outputData.length;
    
    const header = [];
    header.push(0x00);
    
    while (header.length < 11) header.push(0x20);
    
    let strPtr = 0;
    while (strPtr < Math.min(spectrumFilename.length, 9)) {
        header[strPtr + 1] = spectrumFilename.charCodeAt(strPtr);
        strPtr++;
    }
    
    header.push(fileSize & 0x00FF, (fileSize & 0xFF00) >> 8);
    header.push(autostartLine & 0x00FF, (autostartLine & 0xFF00) >> 8);
    header.push(fileSize & 0x00FF, (fileSize & 0xFF00) >> 8);
    
    let checksum = 0;
    for (let i = 1; i < 17; i++) {
        checksum ^= header[i];
    }
    header.push(checksum & 0xFF);
    
    const tapeHeader = [0x13, 0x00, 0x00, ...header];
    tapeHeader.push((fileSize + 2) & 0x00FF, ((fileSize + 2) & 0xFF00) >> 8, 0xFF);
    
    checksum = 0xFF;
    for (let i = 0; i < outputData.length; i++) {
        checksum ^= outputData[i];
    }
    
    return new Uint8Array([...tapeHeader, ...outputData, checksum & 0xFF]);
}

function processBasic(basicLines, options) {
    let linenum = 0;
    let currentLinenum = 0;
    const labelNumberArray = {};
    
    if (options.useLabels) linenum = options.setLabelsModeStartLineNumber;
    
    basicLines = basicLines.filter((currentLine, index) => {
        if (currentLine.trim().length === 0) return false;
        
        const trimmedLine = currentLine.trimLeft();
        const firstChar = trimmedLine[0];
        
        if (/^[1-9]$/.test(firstChar)) {
            if (options.useLabels) {
                throw new Error("Line " + index + " Line number used in label mode.");
            }
        } else if (firstChar === '#') {
            return false;
        } else if (firstChar === '@') {
            if (options.useLabels) {
                const tempPos = currentLine.indexOf(":");
                if (tempPos === -1) {
                    throw new Error("Line " + index + " Incomplete token definition, label should close with an end tag ':'");
                }
                const atPos = currentLine.indexOf('@');
                if (tempPos < atPos) {
                    throw new Error("Line " + index + " Label end tag ':' occurs before label start tag '@'");
                }
                
                let tempBuffer = ['@'];
                let ptr = atPos + 1;
                while (ptr < currentLine.length && /[a-zA-Z]/.test(currentLine[ptr])) {
                    tempBuffer.push(currentLine[ptr]);
                    ptr++;
                }
                const label = tempBuffer.join('');
                if (labelNumberArray[label] !== undefined) {
                    throw new Error("Line " + index + " Attempt to redefine label " + label);
                }
                labelNumberArray[label] = linenum;
            }
            return false;
        } else {
            if (options.useLabels) {
                basicLines[index] = linenum + " " + currentLine;
                linenum += options.setLabelModeIncrement;
            }
        }
        return true;
    });
    
    if (options.useLabels) {
        basicLines = basicLines.map(line => strReplaceAssoci(labelNumberArray, line));
    }
    
    currentLinenum = 0;
    linenum = 0;
    const tempBuffer = [];
    
    for (const currentLine of basicLines) {
        currentLinenum++;
        const lastLinenum = linenum;
        
        if (currentLine.trim().length === 0) continue;
        
        let ptr = 0;
        while (/\s/.test(currentLine[ptr])) ptr++;
        
        if (!/\d/.test(currentLine[ptr])) {
            throw new Error("line: " + currentLinenum + " missing line number");
        }
        
        const tempNumBuffer = [];
        while (/\d/.test(currentLine[ptr]) && ptr < currentLine.length) {
            tempNumBuffer.push(currentLine[ptr]);
            ptr++;
        }
        linenum = parseInt(tempNumBuffer.join(''));
        
        if (linenum <= lastLinenum) {
            throw new Error("line: " + currentLinenum + " line no. not greater than previous one");
        }
        
        if (linenum < 0 || linenum > 9999) {
            throw new Error("line: " + currentLinenum + " line no. out of range, should be 1 - 9999, found " + linenum);
        }
        
        tempBuffer.push((linenum & 0xff00) >> 8, linenum & 0xff, 0, 0);
        
        while (/\s/.test(currentLine[ptr])) ptr++;
        
        let inString = false;
        let textBuffer = "";
        
        while (ptr < currentLine.length) {
            if (currentLine[ptr] === '"') {
                inString = !inString;
                tempBuffer.push(SinclairBasic['"']);
                ptr++;
                continue;
            }
            
            if (inString) {
                if (currentLine[ptr] === '\\' && ptr + 1 < currentLine.length) {
                    const nextChar = currentLine[ptr + 1];
                    if (/[a-zA-Z]/.test(nextChar) && !/[V-Zv-z]/.test(nextChar)) {
                        tempBuffer.push(SinclairBasic["(" + nextChar + ")"]);
                        ptr += 2;
                        continue;
                    } else {
                        switch (nextChar) {
                            case '\\':
                            case '@':
                                tempBuffer.push(SinclairBasic[nextChar]);
                                ptr += 2;
                                continue;
                            case '*':
                                tempBuffer.push(SinclairBasic['@']);
                                ptr += 2;
                                continue;
                            case "'": case '.': case ':': case ' ':
                                const char1 = currentLine[ptr + 1] === " " ? "BLANK" : currentLine[ptr + 1];
                                const char2 = currentLine[ptr + 2] === " " ? "BLANK" : currentLine[ptr + 2];
                                tempBuffer.push(SinclairBasic[char1 + char2]);
                                ptr += 3;
                                continue;
                            default:
                                console.warn("line: " + currentLinenum + " unknown escape character " + currentLine[ptr] + nextChar + " inserting literally");
                                tempBuffer.push(currentLine.charCodeAt(ptr), currentLine.charCodeAt(ptr + 1));
                                ptr += 2;
                                continue;
                        }
                    }
                }
                
                if (SinclairBasic[currentLine[ptr]] !== undefined) {
                    tempBuffer.push(SinclairBasic[currentLine[ptr]]);
                }
            }
            
            if (/[a-zA-Z]/.test(currentLine[ptr]) && !inString && ptr !== currentLine.length - 1) {
                textBuffer = "";
                let textBuffer2 = "";
                
                while ((/[a-zA-Z]/.test(currentLine[ptr]) || currentLine[ptr] === '$') && ptr < currentLine.length) {
                    textBuffer += currentLine[ptr];
                    ptr++;
                }
                
                if (textBuffer.toUpperCase() === "REM") {
                    tempBuffer.push(SinclairBasicKeywords[textBuffer.toUpperCase()]);
                    ptr++;
                    while (ptr < currentLine.length) {
                        tempBuffer.push(SinclairBasic[currentLine[ptr]]);
                        ptr++;
                    }
                    continue;
                }
                
                if (textBuffer.toUpperCase() === "GO") {
                    const tempPtr = ptr;
                    ptr++;
                    while (!/[a-zA-Z]/.test(currentLine[ptr]) && ptr < currentLine.length) ptr++;
                    
                    while (/[a-zA-Z]/.test(currentLine[ptr]) && ptr < currentLine.length) {
                        textBuffer2 += currentLine[ptr];
                        ptr++;
                    }
                    
                    if (textBuffer2.toUpperCase() === "TO" || textBuffer2.toUpperCase() === "SUB") {
                        textBuffer += textBuffer2;
                        if (SinclairBasicKeywords[textBuffer.toUpperCase()] !== undefined) {
                            tempBuffer.push(SinclairBasicKeywords[textBuffer.toUpperCase()]);
                        }
                        ptr++;
                        continue;
                    } else {
                        ptr = tempPtr;
                    }
                }
                
                if (textBuffer.toUpperCase() === "OPEN" || textBuffer.toUpperCase() === "CLOSE") {
                    const tempPtr = ptr;
                    while (currentLine[ptr] !== '#' && ptr < currentLine.length) ptr++;
                    
                    if (currentLine[ptr] === '#') {
                        textBuffer += '#';
                        if (SinclairBasicKeywords[textBuffer.toUpperCase()] !== undefined) {
                            tempBuffer.push(SinclairBasicKeywords[textBuffer.toUpperCase()]);
                        }
                        continue;
                    } else {
                        ptr = tempPtr;
                    }
                }
                
                if (textBuffer.toUpperCase() === "BIN") {
                    if (SinclairBasicKeywords[textBuffer.toUpperCase()] !== undefined) {
                        tempBuffer.push(SinclairBasicKeywords[textBuffer.toUpperCase()]);
                    }
                    ptr++;
                    
                    while (/\s/.test(currentLine[ptr])) ptr++;
                    
                    if (currentLine[ptr] !== '0' && currentLine[ptr] !== '1') {
                        throw new Error("line: " + currentLinenum + " Bad BIN number");
                    }
                    
                    let tempBinNumber = 0;
                    
                    if (currentLine[ptr] === '0' && ptr + 1 < currentLine.length && currentLine[ptr + 1].toUpperCase() === 'X') {
                        let tempHexNumber = "";
                        ptr += 2;
                        while (/[0-9A-Fa-f]/.test(currentLine[ptr]) && ptr < currentLine.length) {
                            tempHexNumber += currentLine[ptr];
                            ptr++;
                        }
                        
                        const chars = parseInt(tempHexNumber, 16).toString(2).split('');
                        for (const char of chars) {
                            tempBinNumber *= 2;
                            tempBinNumber += parseInt(char);
                            if (SinclairBasic[char] !== undefined) {
                                tempBuffer.push(SinclairBasic[char]);
                            }
                        }
                    } else {
                        while ((currentLine[ptr] === '0' || currentLine[ptr] === '1') && ptr < currentLine.length) {
                            tempBinNumber *= 2;
                            tempBinNumber += parseInt(currentLine[ptr]);
                            if (SinclairBasic[currentLine[ptr]] !== undefined) {
                                tempBuffer.push(SinclairBasic[currentLine[ptr]]);
                            }
                            ptr++;
                        }
                    }
                    
                    const exponentMantissaArray = frexp(tempBinNumber);
                    for (const element of exponentMantissaArray) {
                        tempBuffer.push(element);
                    }
                    continue;
                }
                
                if (textBuffer.toUpperCase() === "DEF") {
                    const tempPtr = ptr;
                    ptr++;
                    while (!/[a-zA-Z]/.test(currentLine[ptr]) && ptr < currentLine.length) ptr++;
                    
                    while (/[a-zA-Z]/.test(currentLine[ptr]) && ptr < currentLine.length) {
                        textBuffer2 += currentLine[ptr];
                        ptr++;
                    }
                    
                    if (textBuffer2.toUpperCase() === "FN") {
                        textBuffer += " " + textBuffer2;
                        if (SinclairBasicKeywords[textBuffer.toUpperCase()] !== undefined) {
                            tempBuffer.push(SinclairBasicKeywords[textBuffer.toUpperCase()]);
                        }
                        ptr++;
                        continue;
                    } else {
                        ptr = tempPtr;
                    }
                }
                
                if (SinclairBasicKeywords[textBuffer.toUpperCase()] !== undefined) {
                    tempBuffer.push(SinclairBasicKeywords[textBuffer.toUpperCase()]);
                    textBuffer = "";
                    if (/\s/.test(currentLine[ptr])) ptr++;
                    continue;
                }
                
                const chars = textBuffer.split('');
                for (const char of chars) {
                    if (SinclairBasic[char] !== undefined) {
                        tempBuffer.push(SinclairBasic[char]);
                    }
                }
                
                textBuffer = "";
                
                while ((/[a-zA-Z]/.test(currentLine[ptr]) || /\d/.test(currentLine[ptr]) || /\s/.test(currentLine[ptr])) && ptr < currentLine.length) {
                    if (!/\s/.test(currentLine[ptr])) {
                        textBuffer += currentLine[ptr];
                    } else {
                        if (SinclairBasicKeywords[textBuffer.toUpperCase()] !== undefined) {
                            tempBuffer.push(SinclairBasicKeywords[textBuffer.toUpperCase()]);
                            textBuffer = "";
                            if (ptr + 1 < currentLine.length) ptr++;
                            continue;
                        } else {
                            const varChars = textBuffer.split('');
                            for (const char of varChars) {
                                if (SinclairBasic[char] !== undefined) {
                                    tempBuffer.push(SinclairBasic[char]);
                                }
                            }
                            textBuffer = "";
                        }
                    }
                    ptr++;
                    
                    if (ptr === currentLine.length) break;
                }
            }
            
            const chars = textBuffer.split('');
            for (const char of chars) {
                if (SinclairBasic[char] !== undefined) {
                    tempBuffer.push(SinclairBasic[char]);
                }
            }
            textBuffer = "";
            
            if (!inString && (/\d/.test(currentLine[ptr]) || currentLine[ptr] === '.')) {
                let tempNumberBuffer = "";
                while ((/\d/.test(currentLine[ptr]) || currentLine[ptr] === '.' || currentLine[ptr] === 'e') && ptr < currentLine.length) {
                    tempNumberBuffer += currentLine[ptr];
                    if (SinclairBasic[currentLine[ptr]] !== undefined) {
                        tempBuffer.push(SinclairBasic[currentLine[ptr]]);
                    }
                    ptr++;
                }
                
                const exponentMantissaArray = frexp(parseFloat(tempNumberBuffer));
                for (const element of exponentMantissaArray) {
                    tempBuffer.push(element);
                }
                continue;
            }
            
            if (!inString) {
                if (currentLine[ptr] === '<') {
                    if (currentLine[ptr + 1] === '>') {
                        tempBuffer.push(SinclairBasic['<>']);
                        ptr += 2;
                        continue;
                    } else if (currentLine[ptr + 1] === '=') {
                        tempBuffer.push(SinclairBasic['<=']);
                        ptr += 2;
                        continue;
                    }
                } else if (currentLine[ptr] === '>') {
                    if (currentLine[ptr + 1] === '=') {
                        tempBuffer.push(SinclairBasic['>=']);
                        ptr += 2;
                        continue;
                    }
                }
                
                if (!/\s/.test(currentLine[ptr]) && SinclairBasic[currentLine[ptr]] !== undefined) {
                    tempBuffer.push(SinclairBasic[currentLine[ptr]]);
                }
            }
            ptr++;
        }
        
        tempBuffer.push(0x0D);
        
        const tempLineLength = tempBuffer.length - 4;
        tempBuffer[2] = tempLineLength & 0xff;
        tempBuffer[3] = (tempLineLength & 0xff00) >> 8;
    }
    
    return new Uint8Array(tempBuffer);
}

function convertToTAP(basicCode, options) {
    let basicLines = basicCode.split(/\r?\n/);
    
    basicLines = basicLines.map(line => line.replace(/\r/g, '').replace(/\t/g, ' ').trim());
    basicLines = basicLines.filter(line => line.length > 0);
    
    if (options.greekglishConvert) {
        basicLines = basicLines.map(line => greeklish(line));
    }
    
    let outputData = processBasic(basicLines, options);
    
    switch (options.outputFormat) {
        case "TAP":
            outputData = prependSpectrumTapeHeader(outputData, options.spectrumFilename, options.autostartLine);
            break;
        case "RAW":
            break;
        case "+3":
            outputData = prependPlus3Header(outputData, options.autostartLine);
            break;
        case "P81":
            break;
    }
    
    return outputData;
}

function downloadFile(data, filename) {
    const blob = new Blob([data], { type: 'application/octet-stream' });
    const url = URL.createObjectURL(blob);
    const a = document.createElement('a');
    a.href = url;
    a.download = filename;
    document.body.appendChild(a);
    a.click();
    document.body.removeChild(a);
    URL.revokeObjectURL(url);
}

function saveToLocalStorage(data, key) {
    localStorage.setItem(key, JSON.stringify(Array.from(data)));
}

function loadFromLocalStorage(key) {
    const data = localStorage.getItem(key);
    if (data) {
        return new Uint8Array(JSON.parse(data));
    }
    return null;
}

window.convertBasic = function() {
    try {
        let inputCode;
        if (typeof editAreaLoader !== 'undefined') {
            inputCode = editAreaLoader.getValue('text_entered');
        } else {
            inputCode = document.getElementById('text_entered').value;
        }
        
        const schoolname = document.getElementById('schoolname').value || "school";
        const taksi = document.getElementById('taksi').value;
        const tmima = document.getElementById('tmima').value;
        const pc = document.getElementById('pc').value;
        const name = document.getElementById('name').value || "student";
        const date = new Date().toISOString().slice(0, 10).replace(/-/g, '');
        
        const fileBase = "sch" + schoolname + "-" + taksi + tmima + "-" + date + "-PC" + pc + "-" + name;
        const fileNameSafe = fileBase.replace(/\s+/g, '_');
        
        const options = {
            spectrumFilename: "ZMB-TEST",
            outputFormat: "TAP",
            autostartLine: 0x8000,
            useLabels: false,
            setLabelModeIncrement: 2,
            setLabelsModeStartLineNumber: 10,
            greekglishConvert: true,
            verboseMode: false
        };
        
        const outputData = convertToTAP(inputCode, options);
        
        // Convert to base64 for sending to PHP
        const tapDataBase64 = btoa(String.fromCharCode.apply(null, outputData));
        const htmlDataBase64 = btoa(inputCode);
        
        // Send to PHP script for saving
        const formData = new FormData();
        formData.append('fileName', fileNameSafe);
        formData.append('tapData', tapDataBase64);
        formData.append('htmlData', htmlDataBase64);
        
        fetch('save_tap.php', {
            method: 'POST',
            body: formData
        })
        .then(response => response.json())
        .then(data => {
            if (data.success) {
                document.getElementById('status').style.display = 'block';
                document.getElementById('status').textContent = "Success! File saved: " + data.tapFile;
                document.getElementById('status').style.color = "green";
                
                // Open emulator with the saved file
                setTimeout(() => {
                    const emulatorWindow = window.open('', 'emulator_output');
                    if (emulatorWindow) {
                        emulatorWindow.location.href = 'qaop.html?#l=' + data.tapFile;
                    } else {
                        alert("Popup blocked. Please allow popups and try again.");
                    }
                }, 1000);
            } else {
                throw new Error(data.error || 'Unknown error');
            }
        })
        .catch(error => {
            throw error;
        });
        
    } catch (error) {
        document.getElementById('status').style.display = 'block';
        document.getElementById('status').textContent = "Error: " + error.message;
        document.getElementById('status').style.color = "red";
        console.error(error);
    }
};
