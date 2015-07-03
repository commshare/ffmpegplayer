#ifndef PLAYER_H_
#define PLAYER_H_

#ifdef WIN32
	#include <windows.h>
	#include <process.h>
	typedef DWORD pthread_t;
	typedef void RETYPE;
#else
	#include <sys/time.h>
	#include <signal.h>
	#include <unistd.h>
	#include <SDL/SDL.h>
	#include <pthread.h>
	#include <time.h>
	#include <sys/types.h>
	typedef void* RETYPE;
#endif

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
#include <iostream>
using namespace std;

extern Queue<AVPacket> g_video_queue;
extern Queue<AVPacket> g_audio_queue;

extern pthread_t g_read_tid;
extern pthread_t g_video_tid;
extern pthread_t g_audio_tid;

extern int g_exit_code;

#ifdef WIN32

#else
	extern pthread_mutex_t p_video_mutex;
	extern pthread_mutex_t p_audio_mutex;
	extern pthread_cond_t p_video_cond;
	extern pthread_cond_t p_audio_cond;
#endif


class Player{
public:
	Player(SDL_Surface* screen);
	~Player();

	int player(const char* filename);

	/**
	* read data packet thread from stream
	*/
	static RETYPE read_packet_thread(void* obj);
	/**
	* codec video thread
	*/
	static RETYPE codec_video_thread(void* obj);
	/**
	* codec audio thread
	*/
	static RETYPE codec_audio_thread(void* obj);

	/**
	* pause player
	*/
	void pause();
	/**
	* resume player while pause
	*/
	void resume();
	/**
	* stop player
	*/
	void stop();
	/**
	* restart player
	*/
	void restart();
	/**
	* next a media
	*/
	int next(const char* name);
	/**
	* next segment
	*/
	void forword(const int value);

	void release();

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


	static int ispause;
	static int isstop;

	/**
	* display
	*/
	SDL_Overlay* p_bmp;
	SDL_Surface* screen;
};

#endif