/*
 Copyright © 2016 Suresh Kumar. All rights reserved.
 
 ( MIT License )
 
 Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef FileType_h
#define FileType_h
#define BYTES_TO_READ 0xBB8 //BB8(hexadecimal) = 3000(octal) bytes
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

typedef enum {
    unknown_endian, native_endian, big_endian, little_endian
} endian_t;

typedef enum {
    unknown_file, png_file, jpeg_file, bmp_file, mp4_file, m4v_file, wmv_file, pdf_file, doc_file, xls_file, ppt_file, docx_file, xlsx_file, pptx_file
} file_type;


#if defined __cplusplus
extern "C" {
#endif
    const char * checkFileTypeByFile(const char *file);
    const char * checkFileTypeByBuffer(unsigned char *buf, size_t dataLen);
#if defined __cplusplus
};
#endif

#endif /* MagicFinder_h */
