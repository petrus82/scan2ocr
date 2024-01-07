# scan2ocr
For a private user document management software is a rather complicated process to take care of incoming pdf files. DMS can change over time, thus files or data can become inaccessible. Therefore the most simplest solution is to just put all incoming files into a data directory and renaming them into a simple scheme like "yyyy mm keyword.pdf". 

Scan2ocr takes a pdf file, converts it into black and white, compresses it into storage efficient TIFF G4 encoding, adds an OCR layer using the tesseract API and assists in renaming by guessing an appropiate keyword and date for a possible filename. 

It looks onto the first page of the pdf file and searches for the first word with the largest fontsize which is most likely the most important keyword. It then checks for a date in the file, if it does not find anything it takes the current date. If there is an invoice keyword then it adds it to the the filename.
