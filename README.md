# file-type-detector-without-extension
- Detect file type based on magic number presents in file header.

- It detects the following file types
    png, jpeg, bmp, mp4, m4v, wmv, pdf, doc, xls, ppt, docx, xlsx, pptx.
    
### Usage
```sh
const char * checkFileTypeByFile(const char *file)

    file - absolute path of the file
    returns filetype if successfully find, else NULL
```

```sh    
const char * checkFileTypeByBuffer(unsigned char *buf, size_t dataLen);
    buf - file buffer
    dataLen - buffer length
    returns filetype if successfully find, else NULL
```

| Magic Number reference for different file types |
| ------ | 
| [https://gist.github.com/leommoore/f9e57ba2aa4bf197ebc5][PlDb]
| [https://gist.github.com/Qti3e/6341245314bf3513abb080677cd1c93b][PlGh] |
    




