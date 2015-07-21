#ifndef ERRORCODE_H_
#define ERRORCODE_H_

/**
* status code
*/
#define INIT_START				0x00	
#define PLAY_START 				0x01
#define PLAY_OVER 				0x02

#define PLAY_PAUSED				0x03
#define PLAY_RESUME				0x04
#define PLAY_STOPED				0x04
#define PLAY_NEXT				0x05

#define DIDNOT_INIT				0x99


/**
* error code
*/
#define INIT_FAIL 				0x10
#define INIT_SUCCESS 			0x11
#define CREATE_PLAYER_FAIL		0x12
#define DECODEC_FAIL			0x13
#define OPEN_MEDIA_FAIL			0x14
#define DECODEC_INIT_FAIL		0x15
#define NO_MEDIA_INPUT_INFO		0x16
#define NO_DECODEC				0x17
#define VIDEO_DECODEC_FAIL		0x18
#define AUDIO_DECODEC_FAIL		0x19
#define OPEN_VIDEO_DECODEC_FAIL	0x20
#define OPEN_AUDIO_DECODEC_FAIL	0x21

//media name
#define MEDIA_NAME_NO_SUPPORT	0x50
#define MEDIA_NAME_IS_EMPTY		0x51


#endif