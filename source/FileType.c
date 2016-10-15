/*
 Copyright © 2016 Suresh Kumar. All rights reserved.
 
 ( MIT License )
 
 Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "FileType.h"
#include <ctype.h>
#include <string.h>
#include <limits.h>

static size_t dataLen;
static endian_t byteOrder = unknown_endian;

static file_type isPNG(uint8_t* data);
static file_type isBMP(uint8_t* data);
static file_type isJPEG(uint8_t* data);
static file_type isMP4(uint8_t* data);
static file_type isWMV(uint8_t* data);
static file_type isOOXML(uint8_t* data);
static file_type isPDF(uint8_t* data);
static file_type isMSOfficeDocument(uint8_t* data);

static uint16_t byteswap_2 (uint16_t* n);
static short memoryCompare(uint8_t *data, int offset, const void *cmpStr, size_t n);
static int xmlOffset(uint8_t *data, int sOffset, int eOffset);
static int pkOffset(uint8_t *data, int sOffset, int eOffset);
static endian_t getByteOrder();


/*
 #PNG Image Format
 0	string		\x89PNG			image/png
 */
file_type isPNG(uint8_t* data){
    return memoryCompare(data, 0, "\x89PNG", 4)?png_file:unknown_file;
}
/*
 # PC bitmaps (OS/2, Windoze BMP files)  (Greg Roelofs, newt@uchicago.edu)
 0	string		BM		image/bmp
 */
file_type isBMP(uint8_t* data)
{
    return memoryCompare(data, 0, "BM" , 2)?bmp_file:unknown_file;
}
/*
 # JPEG images
 0	beshort		0xffd8		image/jpeg
 */
file_type isJPEG(uint8_t* data){
    unsigned int ofs = 0;
    uint16_t m,tmp;
    unsigned int n;
    unsigned long tmp1;
    if ((tmp1 = strtoul("0xffd8",NULL,0)) == ULONG_MAX)
        return 0;
    tmp = (uint16_t)tmp1;
    endian_t byteOrder = getByteOrder();
    if (byteOrder == little_endian)
        byteswap_2 (&tmp);
    n = tmp;
    
    if(dataLen < 2) return unknown_file;
    // get 16 bit big endian short from buffer
    m = ((uint16_t)data[ofs] << 8) | ((uint16_t)data[ofs+1]);
    //make sure it's in the native byte order
    if (byteOrder == little_endian)
        byteswap_2(&m);
    
    return n == m ?jpeg_file:unknown_file;
}


/*4	string		ftyp
 >8	string		isom	      video/mp4
 >8	string		mp41	      video/mp4
 >8	string		mp42	      video/mp4
 >8	string/B	jp2	      image/jp2
 >8	string		3gp	      video/3gpp
 >8      string          avc1          video/3gpp
 >8	string		mmp4	      video/mp4
 >8	string/B	M4A	      audio/mp4
 >8	string/B	qt	      video/quicktime
 #CAS added from /usr/share/file/magic/animation
 >8      string/W        M4V             video/mp4
 */
file_type isMP4(uint8_t* data)
{
    if(!memoryCompare(data, 4, "ftyp", 4)) return unknown_file;
    
    if (memoryCompare(data, 8, "isom", 4) ||
        memoryCompare(data, 8, "mp41", 4) ||
        memoryCompare(data, 8, "mp42", 4) ||
        memoryCompare(data, 8, "M4A", 3) ||
        memoryCompare(data, 8, "mmp4", 4) ||
        memoryCompare(data, 8, "M4V", 3)) {
        return mp4_file;
    }
    return unknown_file;
}
/*
 # Acrobat
 # (due to clamen@cs.cmu.edu)
 0	string		%PDF-		application/pdf
 */
file_type isPDF(uint8_t* data){
    return memoryCompare(data, 0, "%PDF-", 4)?pdf_file:unknown_file;
}
/*
 #------------------------------------------------------------------------------
 # $File: msooxml,v 1.1 2011/01/25 18:36:19 christos Exp $
 # msooxml:  file(1) magic for Microsoft Office XML
 # From: Ralf Brown <ralf.brown@gmail.com>
 
 # .docx, .pptx, and .xlsx are XML plus other files inside a ZIP
 #   archive.  The first member file is normally "[Content_Types].xml".
 # Since MSOOXML doesn't have anything like the uncompressed "mimetype"
 #   file of ePub or OpenDocument, we'll have to scan for a filename
 #   which can distinguish between the three types
 
 # start by checking for ZIP local file header signature
 0               string          PK\003\004
 # make sure the first file is correct
 >0x1E           string          [Content_Types].xml
 # skip to the second local file header
 #   since some documents include a 520-byte extra field following the file
 #   header,  we need to scan for the next header
 >>(18.l+49)     search/2000     PK\003\004
 # now skip to the *third* local file header; again, we need to scan due to a
 #   520-byte extra field following the file header
 >>>&26          search/1000     PK\003\004
 # and check the subdirectory name to determine which type of OOXML
 #   file we have
 >>>>&26         string          word/           Microsoft Word 2007+
 !:mime application/msword
 >>>>&26         string          ppt/            Microsoft PowerPoint 2007+
 !:mime application/vnd.ms-powerpoint
 >>>>&26         string          xl/             Microsoft Excel 2007+
 !:mime application/vnd.ms-excel
 >>>>&26         default         x               Microsoft OOXML
 !:strength +10
 */
file_type isOOXML(uint8_t* data)
{
    //check for ZIP local file header signature
    if(!memoryCompare(data, 0, "PK\003\004", 4)) return unknown_file;
    
    int offset = -1;
    //check for *.xml within first 100 bytes
    offset = xmlOffset(data, 30, 100);
    if(offset == -1) return unknown_file;
    
    //Google docs
    if(memoryCompare(data, 30, "word/", 5))
        return docx_file;//docx
    if(memoryCompare(data, 30, "xl/", 3))
        return xlsx_file;//xlsx
    if(memoryCompare(data, 30, "ppt/", 4))
        return pptx_file;//pptx
    
    //Find next "PK\003\004"
    offset += 4;
    offset = pkOffset(data, offset, offset+2000);
    if(offset == -1) return unknown_file;
    //Find next "PK\003\004"
    offset += 4;
    offset = pkOffset(data, offset,offset+1000);
    if(offset == -1) return unknown_file;
    
    offset = offset+4;
    //Find "word/" or "xl/" "ppt/"
    uint8_t *w = (uint8_t *)"w";
    uint8_t *o = (uint8_t *)"o";
    uint8_t *r = (uint8_t *)"r";
    uint8_t *d = (uint8_t *)"d";
    uint8_t *x = (uint8_t *)"x";
    uint8_t *l = (uint8_t *)"l";
    uint8_t *p = (uint8_t *)"p";
    uint8_t *t = (uint8_t *)"t";
    uint8_t *slash = (uint8_t *)"/";
    
    int i = offset;
    for (; i < offset+100; i++) {
        if(dataLen < i) return unknown_file;
        if (data[i] == *w && data[i+1] == *o && data[i+2] == *r && data[i+3] == *d && data[i+4] == *slash) {
            return docx_file;
        }
        else if (data[i] == *x && data[i+1] == *l && data[i+2] == *slash){
            return xlsx_file;
        }
        else if (data[i] == *p && data[i+1] == *p && data[i+2] ==  *t && data[i+3] ==  *slash)
            return pptx_file;
    }
    
    return unknown_file;
}
file_type isMSOfficeDocument(uint8_t* data){
    if (memoryCompare(data,0,"\xD0\xCF\x11\xE0\xA1\xB1\x1A\xE1", 8)) {
        //xls
        if (memoryCompare(data,512, "\x09\x08\x10\x00\x00\x06\x05\x00", 8) ||
            memoryCompare(data,512, "\xFD\xFF\xFF\xFF\x10", 5) ||
            memoryCompare(data,512, "\xFD\xFF\xFF\xFF\x1F", 5) ||
            memoryCompare(data,512, "\xFD\xFF\xFF\xFF\x22", 5) ||
            memoryCompare(data,512, "\xFD\xFF\xFF\xFF\x23", 5) ||
            memoryCompare(data,512, "\xFD\xFF\xFF\xFF\x28", 5) ||
            memoryCompare(data,512, "\xFD\xFF\xFF\xFF\x29", 5)) {
            return xls_file;
        }
        //ppt
        else if (memoryCompare(data, 512, "\x00\x6E\x1E\xF0", 4) ||
                 memoryCompare(data, 512, "\x0F\x00\xE8\x03", 4) ||
                 memoryCompare(data, 512, "\xA0\x46\x1D\xF0", 4) ||
                 memoryCompare(data, 512, "\xFD\xFF\xFF\xFF\x0E\x00\x00\x00", 8) ||
                 memoryCompare(data, 512, "\xFD\xFF\xFF\xFF\x1C\x00\x00\x00", 8) ||
                 memoryCompare(data, 512, "\xFD\xFF\xFF\xFF\x43\x00\x00\x00", 8)){
            return ppt_file;
        }
        //msword
        else if (memoryCompare(data, 0, "\376\067\0\043", 4) ||
                 memoryCompare(data, 0, "\333\245-\0\0\0", 5) ||
                 memoryCompare(data, 512, "\xEC\xA5\xC1\x00", 4))//equivalent to "\354\245\301\0" in octal
            return doc_file;
        else
            return ppt_file;
    }
    return unknown_file;
    
}
static file_type isWMV(uint8_t* data){
    return memoryCompare(data, 0, "\x30\x26\xB2\x75\x8E\x66\xCF\x11", 8)?wmv_file:unknown_file;
}
const char * checkFileTypeByBuffer(unsigned char *buf, size_t bufLen){
    int result;
    dataLen = bufLen;
    //find the endianess of the device for later use.
    if (byteOrder == unknown_endian)
        byteOrder = getByteOrder();
    
    if(isPNG(buf)) return "png";
    else if (isJPEG(buf)) return "jpeg";
    else if (isBMP(buf)) return "bmp";
    else if (isPDF(buf)) return "pdf";
    else if (isMP4(buf)) return "mp4";
    else if (isWMV(buf)) return "wmv";
    else if ((result = isMSOfficeDocument(buf))){
        if(result == doc_file)
            return "doc";
        else if (result == xls_file)
            return "xls";
        else
            return "ppt";
    }
    else if ((result = isOOXML(buf))){
        if(result == docx_file)
            return "docx";
        else if (result == xlsx_file)
            return "xlsx";
        else
            return "pptx";
    }
    else return NULL;
    
}
const char * checkFileTypeByFile(const char *file)
{
    FILE *fp;
    uint8_t *buf;
    //unsigned char *buf;
    size_t buflen;
    if ((buf = malloc(BYTES_TO_READ)) == NULL)
        return NULL;
    
    if ((fp = fopen(file,"rb")) == NULL)
        return NULL;
    buflen = fread(buf, 1,BYTES_TO_READ, fp);
    fclose(fp);
    /* resize the buffer so we don't waste memory */
    if ((buf = realloc(buf, buflen)) == NULL)
        return NULL;
    
    return checkFileTypeByBuffer(buf, buflen);
}
#pragma mark - utility functions
static uint16_t byteswap_2 (uint16_t* n)
{
    uint16_t t;
    t = *n;
    *n = (t>>8) | ((t&0xFF)<<8);
    
    return *n;
}
static short memoryCompare(uint8_t *data, int offset, const void *cmpStr, size_t n){
    if(dataLen < offset+n) return 0;
    return memcmp(&data[offset], cmpStr, n) == 0?1:0;
}
static int xmlOffset(uint8_t *data, int sOffset, int eOffset){
    if(dataLen < eOffset) return -1;
    uint8_t *c = (uint8_t *)"x";
    //check the header contains .xml
    int i = sOffset;
    for (; i< eOffset; i++) {
        if(data[i] == *c && memoryCompare(data, i-1,".xml",4))
            return i-1;
    }
    
    return -1;
    
}
static int pkOffset(uint8_t *data, int sOffset, int eOffset){
    if(dataLen < eOffset) return -1;
    uint8_t *c = (uint8_t *)"P";
    int i = sOffset;
    //check the header contains "PK\003\004"
    for (i = sOffset; i< eOffset; i++) {
        if(data[i] == *c && memoryCompare(data, i,"PK\003\004",4))
            return i;
    }
    
    return -1;
}

static endian_t getByteOrder(){
    unsigned int i = 1;
    char *c = (char*)&i;
    if (*c)
        return little_endian;
    else
        return big_endian;
}