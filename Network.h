#include "Main.h"
#include "Parser.h"
#include "private.h"
#include <curl\curl.h>

#define IMGUR_API_URL "http://api.imgur.com/2/upload.xml"

size_t writefunction( char *ptr, size_t size, size_t nmemb, void *userdata);
bool ImgurUpload( CURL *pcurl_handle, BYTE *pimage_buffer, unsigned int image_buffer_len );