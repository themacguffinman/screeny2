#include "Main.h"
#include "Parser.h"
#include <vector>
#include <curl\curl.h>

#define IMGUR_API_URL "http://api.imgur.com/2/upload.xml"

struct image_bufobj
{
	BYTE *pimage_buffer;
	ULARGE_INTEGER image_size;
};

struct CaptureScreenThread_in
{
	std::queue<image_bufobj> *bufq;
};

size_t writefunction( char *ptr, size_t size, size_t nmemb, void *userdata);
bool ImgurUpload( BYTE *pimage_buffer, unsigned int image_buffer_len );

extern std::vector<imgur_xml_obj> imgur_uploads;