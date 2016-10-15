# file-magic-finder
- Detect file type based on magic number presents in file header.

- It will detect only below file types
    png, jpeg, bmp, mp4, m4v, wmv, pdf, doc, xls, ppt, docx, xlsx, pptx.
    
### Usage
```sh
const char * checkFileTypeByFile(const char *file)
```
    file - absolute path of the file
    returns filetype if successfully find, else NULL
    
```sh    
const char * checkFileTypeByBuffer(unsigned char *buf, size_t dataLen);
```
    buf - file buffer
    dataLen - buffer length
    returns filetype if successfully find, else NULL
    




