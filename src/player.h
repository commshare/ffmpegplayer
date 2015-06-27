#ifndef PLAYER_H_
#define PLAYER_H_

#include <iostream>
#include <sys/time.h>
#include <signal.h>
#include <unistd.h>

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


class Player{
public:
	Player();
	~Player();

	int player(const char* filename);

	/**
	* read data packet thread from stream
	*/
	static void read_packet_thread(void* obj);
	/**
	* codec video thread
	*/
	static void codec_video_thread(void* obj);
	/**
	* codec audio thread
	*/
	static void codec_audio_thread(void* obj);

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
	AVCodec* p_video_codec;
	AVCodec* p_audio_codec;


	int m_sign;

public:
	int m_videoindex;
	int m_audioindex;


	Queue<AVPacket> m_video_queue;
	Queue<AVPacket> m_audio_queue;

};

#endif
