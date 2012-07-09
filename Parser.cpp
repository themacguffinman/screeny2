#include "Parser.h"

void parse_xml_tag( char *xml_str, unsigned int xml_strlen, char **pptag_name, char **pptag_data )
{
	char tempbuf[128]; //I'm assuming the max strlen for a tag name is 64 chars long

	for( unsigned int tn_start = 0; tn_start < xml_strlen; tn_start++ )
	{
		//check for beginning of opening tag
		if( xml_str[tn_start] == '<' )
		{
			for( unsigned int tn_end = tn_start; tn_end < xml_strlen; tn_end++ )
			{
				//check for '/' to see if the tag has no data (<emptytag/>)
				if( xml_str[tn_end] == '/' )
				{
					(*pptag_name) = &xml_str[tn_start+1]; //I'm not liking this +1 business...
					xml_str[tn_end] = NULL;

					*pptag_data = NULL;
					break;
				}

				//check for end of opening tag
				if( xml_str[tn_end] == '>' )
				{
					(*pptag_name) = &xml_str[tn_start+1];
					xml_str[tn_end] = NULL;
					
					for( unsigned int td_end = tn_end; td_end < xml_strlen; td_end++ )
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
									tempbuf[ ct_end-td_end-1 ] = NULL;
									if( NULL == strcmp( *pptag_name, tempbuf ) )
									{
										(*pptag_data) = &xml_str[tn_end+1];
										xml_str[td_end] = NULL;
										break;
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
}

void parse_xml( char *xml_str, unsigned int xml_strlen, imgur_xml_obj *presponse )
{
	
}