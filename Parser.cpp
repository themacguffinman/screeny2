#include "Parser.h"

bool parse_xml_tag( char *xml_str, unsigned int xml_strlen, char **pptag_name, char **pptag_data, char **pprest_of_string )
{
	if( xml_strlen < 7 )
		return false;

	char tempbuf[128]; //I'm assuming the max strlen for a tag name is 128 chars long

	for( unsigned int tn_start = 0; tn_start + 1 < xml_strlen; tn_start++ )
	{
		//check for beginning of opening tag && make sure it's not actually a closing tag
		if( xml_str[tn_start] == '<' && xml_str[tn_start+1] != '/' )
		{
			for( unsigned int tn_end = tn_start; tn_end + 1 < xml_strlen; tn_end++ )
			{
				//check for '/' to see if the tag has no data (<emptytag/>)
				if( xml_str[tn_end] == '/' )
				{
					(*pptag_name) = &xml_str[tn_start+1];
					xml_str[tn_end] = NULL;

					*pptag_data = NULL;
					break;
				}

				//check for end of opening tag
				if( xml_str[tn_end] == '>' )
				{
					(*pptag_name) = &xml_str[tn_start+1];
					xml_str[tn_end] = NULL;
					
					for( unsigned int td_end = tn_end; td_end + 1 < xml_strlen; td_end++ )
					{
						//check for '</' the start of a closing tag
						if( xml_str[td_end] == '/' && xml_str[td_end-1] == '<' )
						{
							for( unsigned int ct_end = td_end; ct_end < xml_strlen; ct_end++ )
							{
								//check for '>' the end of a closing tag
								if( xml_str[ct_end] == '>' )
								{
									//check if the tag name in the closing tag is the same as the opening tag
									strncpy( tempbuf, &xml_str[td_end+1], ct_end - td_end - 1 );
									tempbuf[ ct_end-td_end-1 ] = NULL; //strncpy() does not NULL terminate
									if( NULL == strcmp( *pptag_name, tempbuf ) )
									{
										(*pptag_data) = &xml_str[tn_end+1];
										xml_str[td_end-1] = NULL;

										//output the rest of the string
										if( ct_end + 1 < xml_strlen )
											(*pprest_of_string) = &xml_str[ct_end+1];
										else
											(*pprest_of_string) = NULL;

										return true;
									}
									break;
								}
							}
							
						}
					}
					break;
				}
			}

			break;
		}
	}

	return false;
}

bool parse_imgur_xml( char *xml_str, unsigned int xml_strlen, imgur_xml_obj *presponse )
{
	bool result;
	char *proot = NULL;
	char *proot_data = NULL;
	char *prest_of_string = NULL;

	//extract root node
	result = parse_xml_tag( xml_str, xml_strlen, &proot, &proot_data, &prest_of_string );
	if( proot_data == NULL || result == false )
		return false;

	if( NULL == strcmp( proot, "upload" ) )
	{
		strcpy( presponse->root, proot );
		
		char *pimage;
		char *pimage_data;
		char *ptemp;
		char *ptemp_data;

		//extract image node
		parse_xml_tag( proot_data, strlen(proot_data), &pimage, &pimage_data, &prest_of_string );

		//discard name, title and caption
		parse_xml_tag( pimage_data, strlen(pimage_data), &ptemp, &ptemp_data, &prest_of_string );
		parse_xml_tag( prest_of_string, strlen(prest_of_string), &ptemp, &ptemp_data, &prest_of_string );
		parse_xml_tag( prest_of_string, strlen(prest_of_string), &ptemp, &ptemp_data, &prest_of_string );

		//glean hash
		parse_xml_tag( prest_of_string, strlen(prest_of_string), &ptemp, &ptemp_data, &prest_of_string );
		strcpy( presponse->upload.image.hash, ptemp_data );

		//glean deletehash
		parse_xml_tag( prest_of_string, strlen(prest_of_string), &ptemp, &ptemp_data, &prest_of_string );
		strcpy( presponse->upload.image.deletehash, ptemp_data );

		//glean datetime
		parse_xml_tag( prest_of_string, strlen(prest_of_string), &ptemp, &ptemp_data, &prest_of_string );
		strcpy( presponse->upload.image.datetime, ptemp_data );

		//glean image type
		parse_xml_tag( prest_of_string, strlen(prest_of_string), &ptemp, &ptemp_data, &prest_of_string );
		for( int i = 0; i + 1 < strlen(ptemp_data); i++ )
		{
			if( ptemp_data[i] == '/' )
			{
				ptemp_data = &ptemp_data[i+1];
				break;
			}
		}
		if( !strcmp( ptemp_data, "png" ) )
			presponse->upload.image.img_type = PNG;
		else if( !strcmp( ptemp_data, "bmp" ) )
			presponse->upload.image.img_type = BMP;
		else if( !strcmp( ptemp_data, "jpeg" ) )
			presponse->upload.image.img_type = JPEG;
		else if( !strcmp( ptemp_data, "gif" ) )
			presponse->upload.image.img_type = GIF;
		else if( !strcmp( ptemp_data, "apng" ) )
			presponse->upload.image.img_type = APNG;
		else if( !strcmp( ptemp_data, "tiff" ) )
			presponse->upload.image.img_type = TIFF;
		else if( !strcmp( ptemp_data, "pdf" ) )
			presponse->upload.image.img_type = PDF;
		else if( !strcmp( ptemp_data, "xcf" ) )
			presponse->upload.image.img_type = XCF;
		else presponse->upload.image.img_type = INVALID;

		//glean animated
		parse_xml_tag( prest_of_string, strlen(prest_of_string), &ptemp, &ptemp_data, &prest_of_string );
		if( !strcmp( ptemp_data, "true" ) )
			presponse->upload.image.animated = true;
		else
			presponse->upload.image.animated = false;

		//glean width
		parse_xml_tag( prest_of_string, strlen(prest_of_string), &ptemp, &ptemp_data, &prest_of_string );
		presponse->upload.image.width = atoi( ptemp_data );

		//glean height
		parse_xml_tag( prest_of_string, strlen(prest_of_string), &ptemp, &ptemp_data, &prest_of_string );
		presponse->upload.image.height = atoi( ptemp_data );

		//glean size
		parse_xml_tag( prest_of_string, strlen(prest_of_string), &ptemp, &ptemp_data, &prest_of_string );
		presponse->upload.image.size = atoi( ptemp_data );

		//glean views
		parse_xml_tag( prest_of_string, strlen(prest_of_string), &ptemp, &ptemp_data, &prest_of_string );
		presponse->upload.image.views = atoi( ptemp_data );

		//glean bandwidh
		parse_xml_tag( prest_of_string, strlen(prest_of_string), &ptemp, &ptemp_data, &prest_of_string );
		presponse->upload.image.bandwidth = atoi( ptemp_data );
	} else if ( NULL == strcmp( proot, "error" ) )
	{
		strcpy( presponse->root, proot );
	} else {
		return false;
	}

	return true;
}