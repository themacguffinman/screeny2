#include "Network.h"
#include "Errors.h"

size_t writefunction( char *ptr, size_t size, size_t nmemb, void *userdata)
{
	if( size != 1 )
	{
		logger.printf( _T("writefunction() FATAL ERROR: size != 1") );
		return 0;
	}


}

bool ImgurUpload( CURL *pcurl_handle, BYTE *pimage_buffer, unsigned int image_buffer_len )
{
	CURLcode libcurl_result;

	curl_httppost *formpost = NULL;
	curl_httppost *lastptr = NULL;
	curl_slist *headerlist = NULL;
	
	//Define multi-part form to POST
	curl_formadd( &formpost, &lastptr, //Anonymous API key
		CURLFORM_COPYNAME, "key",
		CURLFORM_COPYCONTENTS, IMGUR_ANONYMOUS_API_KEY,
		CURLFORM_CONTENTSLENGTH, strlen(IMGUR_ANONYMOUS_API_KEY),
		CURLFORM_END );

	curl_formadd( &formpost, &lastptr, //image buffer
		CURLFORM_COPYNAME, "image",
		CURLFORM_BUFFER, "screeny.png",
		CURLFORM_BUFFERPTR, pimage_buffer,
		CURLFORM_BUFFERLENGTH, image_buffer_len,
		CURLFORM_END );

	curl_formadd( &formpost, &lastptr, //type
		CURLFORM_COPYNAME, "type",
		CURLFORM_COPYCONTENTS, "file",
		CURLFORM_CONTENTSLENGTH, 4,
		CURLFORM_END );

	//Define POST headers to tell Imgur what I expect
	headerlist = curl_slist_append( headerlist, "Expect: " );

	//Set libCURL parameters, readying for upload POST
	libcurl_result = curl_easy_setopt( pcurl_handle, CURLOPT_URL, IMGUR_API_URL ); //set POST URL
	libcurl_result = curl_easy_setopt( pcurl_handle, CURLOPT_POST, 1 ); //set mode to POST
	libcurl_result = curl_easy_setopt( pcurl_handle, CURLOPT_HTTPPOST, formpost ); //set POST content to multi-part form in formpost
	libcurl_result = curl_easy_setopt( pcurl_handle, CURLOPT_WRITEFUNCTION, writefunction ); //set callback function for received data to writefunction

	libcurl_result = curl_easy_perform( pcurl_handle ); //beams image to the sky, calls writefunction when JSON respones is received

	//memset( &uploadjsonobject, 0, sizeof(uploadjsonobject) );

	//uploadjsonobject.parse( jsonbuf, jsonbuflen ); //call upon my hackery dackery JSON parser and store in jsonbuf

	return true;
}