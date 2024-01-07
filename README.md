# scan2ocr
For a private user a document management software (DMS) may be a bit too much. The software can change over time, thus files or data can become inaccessible. The most reliable solution is the simplest one, so let the operating system take care of the file organization. Files can be named starting with the year of its creation followed by the month, than a keyword and some following tags like 'invoice.' So every file can quickly be found using the find functions of the operating system.

Scan2ocr takes a pdf file, converts it into black and white, compresses it into storage efficient TIFF G4 encoding, adds an OCR layer using the tesseract API and assists in renaming by guessing an appropiate keyword and date for a possible filename. 

It looks onto the first page of the pdf file and searches for the first word with the largest fontsize which is most likely the most important keyword. It then checks for a date in the file, if it does not find anything it takes the current date. If there is an invoice keyword then it adds it to the the filename.
