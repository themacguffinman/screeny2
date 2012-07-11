#include "Main.h"

enum ImgType
{
	JPEG,
	GIF,
	PNG,
	APNG,
	TIFF,
	BMP,
	PDF,
	XCF,
	INVALID
};

struct imgur_image_info
{
	char hash[8];
	char deletehash[16];
	char datetime[32];
	ImgType img_type;
	bool animated;
	unsigned int width;
	unsigned int height;
	unsigned int size;
	unsigned int views;
	unsigned int bandwidth;
};

struct imgur_links_info
{
	char original[32];
	char imgur_page[32];
	char delete_page[48];
	char small_square[32];
	char large_square[32];
};

struct imgur_upload_info
{
	imgur_image_info image;
	imgur_links_info links;
};

struct imgur_error_info
{
	char message[128];
	char request[32];
	char method[4];
	char format[4];
};

struct imgur_xml_obj
{
	char root[32];

	union
	{
		imgur_upload_info upload;
		imgur_error_info error;
	};
};

bool parse_imgur_xml( char *xml_str, unsigned int xml_strlen, imgur_xml_obj *presponse );