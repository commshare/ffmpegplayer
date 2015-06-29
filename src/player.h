#ifndef PLAYER_H_
#define PLAYER_H_

#include <iostream>
#include <sys/time.h>
#include <signal.h>
#include <unistd.h>
#include <SDL/SDL.h>

extern "C"
{
	#include <libavutil/opt.h>
	#include <libswscale/swscale.h>
	#include <libavutil/imgutils.h>
	#include <libavutil/mathematics.h>
	#include <libavformat/avformat.h>
	#include <libavcodec/avcodec.h>
}

#include "queue.cpp"

using namespace std;


extern Queue<AVPacket> g_video_queue;
extern Queue<AVPacket> g_audio_queue;

extern SDL_mutex* p_video_mutex;
extern SDL_mutex* p_audio_mutex;	
extern SDL_cond *p_cond;
extern SDL_cond *p_audiocond;

class Player{
public:
	Player(SDL_Surface* screen);
	~Player();

	int player(const char* filename);

	/**
	* read data packet thread from stream
	*/
	static int read_packet_thread(void* obj);
	/**
	* codec video thread
	*/
	static int codec_video_thread(void* obj);
	/**
	* codec audio thread
	*/
	static int codec_audio_thread(void* obj);

	inline   AVFormatContext* get_p_formatCtx(){
		return p_formatCtx;
	}

	inline  AVCodecContext* get_p_video_codecCtx(){
		return p_video_codecCtx;
	}

	inline  AVCodecContext* get_p_audio_codecCtx(){
		return p_audio_codecCtx;
	}

private:
	void onInit();

private:
	AVFormatContext* p_formatCtx;
	AVCodecContext* p_video_codecCtx;
	AVCodecContext* p_audio_codecCtx;
	struct SwsContext *img_convert_ctx;
	AVCodec* p_video_codec;
	AVCodec* p_audio_codec;


	int m_sign;

public:
	int m_videoindex;
	int m_audioindex;



	/**
	* display
	*/
	SDL_Overlay* p_bmp;
	SDL_Surface* screen;
};

#endif
