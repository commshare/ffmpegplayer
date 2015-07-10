/*
	ÍŒÏñžñÊœ×ª»»Ïà¹ØµÄº¯Êý
*/

#ifndef __CONVERT_H__
#define __CONVERT_H__

#ifndef WIN32
typedef unsigned short uint16;
typedef unsigned char uint8;
#endif

// Convert a frame of YUV to 16 bit RGB565.
void ConvertYCbCrToRGB565(const uint8* y_buf,
						  const uint8* u_buf,
						  const uint8* v_buf,
						  uint8* rgb_buf,
						  int pic_x,
						  int pic_y,
						  int pic_width,
						  int pic_height,
						  int y_pitch,
						  int uv_pitch,
						  int rgb_pitch,
						  int yuv_type);

void YUV420_C_RGB( char* pYUV, unsigned char* pRGB, int height, int width);

#endif