#include "player.h"
#include "convert.h"

#include "log.h"
#include <jni.h>

#define MAX_BUFFERED_PACKET 100

Queue<AVPacket> g_video_queue;
Queue<AVPacket> g_audio_queue;

pthread_t g_read_tid;
pthread_t g_video_tid;
pthread_t g_audio_tid;

pthread_mutex_t p_video_mutex;
pthread_mutex_t p_audio_mutex;
pthread_cond_t p_video_cond;
pthread_cond_t p_audio_cond;

static int is_finish = 0;
static int onces = 0;
static AVFrame* frist_frame;
int g_exit_code = 0;

int Player::ispause = 0;
int Player::isstop = 0;
Player* Player::play = NULL;

Player* Player::getInstance(){
	if (play == NULL)
	{
		/* code */
		play = new Player;
	}

	return play;
}

RETYPE Player::read_packet_thread(void* obj){
	if (!obj)
	{
		/* code */
		return NULL;
	}
	Player* player = (Player*)obj;
	AVPacket *pPacket = (AVPacket*)av_malloc(sizeof(AVPacket));
	struct timeval val;
	while(av_read_frame(player->get_p_formatCtx(),pPacket) >= 0 && g_exit_code != 1){
		if (Player::ispause == 1)
		{
			/* code */
			while(Player::ispause == 1);
		}
		if (Player::isstop == 1)
		{
			/* code */
			break;
		}
		if (pPacket->stream_index == player->m_videoindex){
			/* code */
#ifdef WIN32
			WaitForSingleObject(p_video_mutex,INFINITE);
#else
			pthread_mutex_lock(&p_video_mutex);
#endif
			if (g_video_queue.size() <= MAX_BUFFERED_PACKET)
			{
				/* code */
				g_video_queue.push(*pPacket);
			}
			
			pthread_cond_signal(&p_video_cond);
#ifdef WIN32
			ReleaseMutex(p_video_mutex);
#else
			pthread_mutex_unlock(&p_video_mutex);
#endif
			if(g_video_queue.size() >= MAX_BUFFERED_PACKET){
				val.tv_sec= 0;
				val.tv_usec = 40000;
				select(-1,NULL,NULL,NULL,&val);
			}
			val.tv_sec= 0;
			val.tv_usec = 1000;
			select(-1,NULL,NULL,NULL,&val);
		}else if(pPacket->stream_index == player->m_audioindex){
			/* code */
#ifdef WIN32
			WaitForSingleObject(p_audio_mutex,INFINITE);
#else
			pthread_mutex_lock(&p_audio_mutex);
#endif
			//g_audio_queue.push(*pPacket);
			if(g_audio_queue.size() == 1){
				pthread_cond_signal(&p_audio_cond);
			}
#ifdef WIN32
			ReleaseMutex(p_audio_mutex);
#else
			pthread_mutex_unlock(&p_audio_mutex);
#endif
			val.tv_sec= 0;
			val.tv_usec = 1000;
			select(0,NULL,NULL,NULL,&val);
		}
	}
	is_finish = 1;
	LOGI("video_queue size:%s",g_video_queue.size());
	LOGI("audio_queue size:%s",g_audio_queue.size());
	av_free(pPacket);

	return NULL;
}

/**
* player video thread
*/
RETYPE Player::codec_video_thread(void* obj){
	if (!obj)
	{
		/* code */
		return NULL;
	}
	/**
	* delay 50ms start player
	*/
	struct timeval val;
	val.tv_sec= 0;
	val.tv_usec = 40000;
	select(0,NULL,NULL,NULL,&val);

	Player* player = (Player*)obj;
	AVFrame *pFrame = av_frame_alloc();
	AVFrame* pFrameYUV = av_frame_alloc();
	
	int temp = 0;

	for(;g_exit_code != 1;){

		if (Player::ispause == 1)
		{
			/* code */
			while(Player::ispause == 1);
		}
		if (Player::isstop == 1)
		{
			/* code */
			break;
		}

#ifdef WIN32
		WaitForSingleObject(p_video_mutex,INFINITE);
#else
		pthread_mutex_lock(&p_video_mutex);
#endif

		if (g_video_queue.size() <= 0)
		{
			/* code */
			if (is_finish == 1)
			{
				/* code */
				break;
			}

			pthread_cond_wait(&p_video_cond,&p_video_mutex);
#ifdef WIN32
			ReleaseMutex(p_video_mutex);
#else
			pthread_mutex_unlock(&p_video_mutex);
#endif
			continue;
		}
		
		AVPacket* packet = (AVPacket*)av_malloc(sizeof(AVPacket));
		*packet = g_video_queue.queue();
		
		int got = 0;
		int ret = avcodec_decode_video2(player->get_p_video_codecCtx(),pFrame,&got,packet);
		
		if (onces == 0)
		{
			onces = 1;
		}

		if (ret < 0)
		{
			/* code */
			char buf[1024] = {0};
			av_strerror(ret,buf,1024);
			LOGE("decoder error....%d.....%s    %d",ret,buf,g_video_queue.size());
#ifdef WIN32
			ReleaseMutex(p_video_mutex);
#else
			pthread_mutex_unlock(&p_video_mutex);
#endif
			av_free(packet);
			break ;
		}
		if (got)
		{
			//will frame change to byte data//
			if(g_jvm != NULL){
				//http://www.xuebuyuan.com/1469657.html
				unsigned char *decode_yuv420pBuf = NULL;
				int decode_buffLen = player->get_p_video_codecCtx()->height * player->get_p_video_codecCtx()->width*3/2;
				//int decode_buffLen = pFrame->height * pFrame->width;
				decode_yuv420pBuf = (unsigned char *)malloc(decode_buffLen * sizeof(unsigned char));
				 memset(decode_yuv420pBuf, 0, player->get_p_video_codecCtx()->height * player->get_p_video_codecCtx()->width * 3 / 2);
				int i, j, nDataLen ;
				for (i=0, nDataLen=0; i<3; i++)
				{
					int nShift = (i==0)?0:1;
					unsigned char *pYUVData = (unsigned char *)pFrame->data[i];
					for (j=0; j< (player->get_p_video_codecCtx()->height>>nShift); j++)
					{
						memcpy(&decode_yuv420pBuf[nDataLen], pYUVData, (player->get_p_video_codecCtx()->width>>nShift));
						pYUVData += pFrame->linesize[i];
						int sum = (player->get_p_video_codecCtx()->width>>nShift);
						nDataLen += (player->get_p_video_codecCtx()->width>>nShift);
					}
				}
			
				JNIEnv* env;
				jclass jcls;
				g_jvm->AttachCurrentThread(&env, NULL);  //必须AttachCurrentThread，env只能在自己的线程里面使用
				//http://www.cnblogs.com/lovingprince/archive/2008/08/19/2166366.html
				jbyteArray RtnArr = NULL;  //下面一系列操作把btPath转成jbyteArray 返回出去
    			RtnArr =env->NewByteArray(nDataLen);
    			env->SetByteArrayRegion(RtnArr, 0, nDataLen, (jbyte*)decode_yuv420pBuf);

				jclass cls = env->GetObjectClass(g_job); 
 				jmethodID methodid = env->GetMethodID(cls,"disPlayer","([BII)V"); 
				env->CallVoidMethod(g_job,methodid,RtnArr,pFrame->height,pFrame->width);
				g_jvm->DetachCurrentThread();
				free(decode_yuv420pBuf);
				//free(buf);
			}
			
		}
		ret = g_video_queue.pop();
#ifdef WIN32
			ReleaseMutex(p_video_mutex);
#else
			pthread_mutex_unlock(&p_video_mutex);
#endif

		/**
		* output media times
		*/
		int nts,nh,nm,ns;
		nts = packet->pts / 100000;
		nh = nts / 3600;
		nm = (nts % 3600) / 60;
		ns = nts % 60;
		int min = temp < ns ? temp : ns;
		int max = temp > ns ? temp : ns;
		if (max - min >= 1)
		{
			/* code */
			temp = ns;
			LOGI("play time:%02d:%02d:%02d  total time:%02d:%02d:%02d  h:%d  w:%d  packet:%d audio:%d\n",nh,nm,ns,player->getHH(),player->getMM(),player->getSS()
				,pFrame->height,pFrame->width,g_video_queue.size(),g_audio_queue.size());
		}
		
		
		val.tv_sec= 0;
		val.tv_usec = 30000;
		select(0,NULL,NULL,NULL,&val);
		av_free(packet);
	}
	LOGI("video thread over finish........");
	av_free(pFrameYUV);

	return NULL;
}

/**
* player audio thread and function
*/
RETYPE Player::codec_audio_thread(void *obj){
	if (!obj)
	{
		/* code */
		return NULL;
	}
	/**
	* delay 50ms start player
	*/
	struct timeval val;
	val.tv_sec= 0;
	val.tv_usec = 40000;
	select(-1,NULL,NULL,NULL,&val);

	//return 0;

	Player* player = (Player*)obj;

	AVFrame* pFrame = av_frame_alloc();

	int temp = 0;

	for(; g_exit_code != 1;){
		if (Player::ispause == 1)
		{
			/* code */
			while(Player::ispause == 1);
		}

		if (Player::isstop == 1)
		{
			/* code */						
			break;
		}
#ifdef WIN32
		WaitForSingleObject(p_audio_mutex,INFINITE);
#else
		pthread_mutex_lock(&p_audio_mutex);
#endif
		if (g_audio_queue.size() <= 0)
		{
			/* code */
			pthread_cond_wait(&p_audio_cond,&p_audio_mutex);
#ifdef WIN32
			ReleaseMutex(p_audio_mutex);
#else
			pthread_mutex_unlock(&p_audio_mutex);
#endif
			continue;
		}

		int got = 0;
		AVPacket* pPacket = (AVPacket*)av_malloc(sizeof(AVPacket));
		*pPacket = g_audio_queue.queue();
		int ret = avcodec_decode_audio4(player->get_p_audio_codecCtx(),pFrame,&got,pPacket);

		if (ret < 0)
		{
			/* code */
			LOGE("audio  decoder fail..........");
#ifdef WIN32
			ReleaseMutex(p_audio_mutex);
#else
			pthread_mutex_unlock(&p_audio_mutex);
#endif
			av_free(pPacket);
			break;
		}
		if (got)
		{
			/* code */
            //pthread_cond_wait(&p_audio_cond,&p_audio_mutex);
		}
	
		int code = g_audio_queue.pop();
#ifdef WIN32
			ReleaseMutex(p_audio_mutex);
#else
			pthread_mutex_unlock(&p_audio_mutex);
#endif

		/**
		* output media times
		*/
		int nts,nh,nm,ns;
		nts = pPacket->pts / 1000000;
		nh = nts / 3600;
		nm = (nts % 3600) / 60;
		ns = nts % 60;
		int min = temp < ns ? temp : ns;
		int max = temp > ns ? temp : ns;
		if (max - min >= 1)
		{
			/* code */
			temp = ns;
			LOGI("audio:len:%d  play time:%02d:%02d:%02d  total time:%02d:%02d:%02d  h:%d  w:%d   %d   %d\n",ret,nh,nm,ns,player->getHH(),
				player->getMM(),player->getSS(),pFrame->height,pFrame->width,code,g_audio_queue.size());
		}
		av_free(pPacket);
	}
	av_free(pFrame);

	LOGI("audio thread over...............");

	return NULL;
}

Player::Player():
m_videoindex(-1),m_audioindex(-1){
	this->onInit();

#ifdef WIN32
	p_video_mutex = CreateMutex(NULL,false,NULL);
	p_audio_mutex = CreateMutex(NULL,false,NULL);
#else
	 pthread_mutex_init(&p_audio_mutex,NULL);
	 pthread_mutex_init(&p_video_mutex,NULL);
	 pthread_cond_init(&p_video_cond,NULL);
	 pthread_cond_init(&p_audio_cond,NULL);
#endif
}

Player::~Player(){
	this->release();
#ifdef WIN32

#else
	// pthread_kill(g_read_tid,SIGKILL);
	// pthread_kill(g_video_tid,SIGKILL);
	// pthread_kill(g_audio_tid,SIGKILL);
	// pthread_join(g_read_tid,NULL);
	// pthread_join(g_video_tid,NULL);
	// pthread_join(g_audio_tid,NULL);
	pthread_mutex_destroy(&p_audio_mutex);
	pthread_mutex_destroy(&p_video_mutex);
	pthread_cond_destroy(&p_video_cond);
	pthread_cond_destroy(&p_audio_cond);
#endif
}

void Player::release(){

	g_exit_code = 1;
	pthread_kill(g_read_tid,SIGKILL);
	pthread_kill(g_video_tid,SIGKILL);
	pthread_kill(g_audio_tid,SIGKILL);
	pthread_join(g_read_tid,NULL);
	pthread_join(g_video_tid,NULL);
	pthread_join(g_audio_tid,NULL);
	
	if (p_video_codecCtx)
	{
		/* code */
		avcodec_close(p_video_codecCtx);
		p_video_codecCtx = NULL;
	}

	if (p_audio_codecCtx)
	{
		/* code */
		avcodec_close(p_audio_codecCtx);
		p_audio_codecCtx = NULL;
	}
	
	avformat_close_input(&p_formatCtx);
	sws_freeContext(img_convert_ctx);
	//p_formatCtx = NULL;
}

void Player::onInit(){

	av_register_all();
	avcodec_register_all();
	avformat_network_init();

	
	p_video_codecCtx = NULL;
	p_audio_codecCtx = NULL;
	p_formatCtx = NULL;
	LOGI("init all success........................");
}

int Player::player(const char* filename){
	LOGI("Player start..........................");
	if (m_sign == -1 || !filename)
	{
		/* code */
		return -1;
	}

	p_formatCtx = avformat_alloc_context();
	if (!p_formatCtx)
	{
		/* code */
		m_sign = -1;
		return -1;
	}
	
	int av = 0;
	if((av = avformat_open_input(&p_formatCtx,filename,NULL,NULL)) != 0){
		char buf[1024] = {0};
		av_strerror(av,buf,1024);
		LOGE("avformat_open_input fail........%d........%s",av,buf);
		return -1;
	}

	if (avformat_find_stream_info(p_formatCtx,NULL) < 0)
	{
		/* code */
		LOGE("find stream info fail.............");
		return -1;
	}

	ispause = 0;
	isstop = 0;
	is_finish = 0;

	m_videoindex = -1;
	m_audioindex = -1;
	int ii = 0;


	for (ii = 0; ii < p_formatCtx->nb_streams; ++ii)
	{
		/* code */
		if (p_formatCtx->streams[ii]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
		{
			/* code */
			m_videoindex = ii;
		}
		if (p_formatCtx->streams[ii]->codec->codec_type == AVMEDIA_TYPE_AUDIO)
		{
			/* code */
			m_audioindex = ii;
		}
	}

	av_dump_format(p_formatCtx,0,filename,0);

	if (m_videoindex == -1 && m_audioindex == -1)
	{
		/* code */
		LOGE("Didn't find video stream.......%s",strerror(errno));
		return -1;
	}

	int tns  = get_p_formatCtx()->duration/1000000;  
	this->m_hh  = tns / 3600;  
 	this->m_mm  = (tns % 3600) / 60;  
	this->m_ss  = (tns % 60);

	if(m_videoindex != -1){
		p_video_codecCtx = p_formatCtx->streams[m_videoindex]->codec;
		p_video_codec = avcodec_find_decoder(p_video_codecCtx->codec_id);

		if (!p_video_codec)
		{
			/* code */
			LOGE("Could not found video codec....");
			return -1;
		}
		if (avcodec_open2(p_video_codecCtx,p_video_codec,NULL) < 0)
		{
			LOGE("Could not open video codec......");
			return -1;
		}

		img_convert_ctx = sws_getContext(p_video_codecCtx->width, p_video_codecCtx->height, 
			p_video_codecCtx->pix_fmt, p_video_codecCtx->width, p_video_codecCtx->height, 
			PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL); 

#ifdef WIN32
		g_video_tid = _beginthread(codec_video_thread,0,this);
#else
		pthread_mutex_unlock(&p_video_mutex);
		pthread_create(&g_video_tid,NULL,codec_video_thread,this);
#endif
		
		LOGI("video init sucess....................");
	}

	if(m_audioindex != -1){
		p_audio_codecCtx = p_formatCtx->streams[m_audioindex]->codec;
		p_audio_codec = avcodec_find_decoder(p_audio_codecCtx->codec_id);
		
		if (!p_audio_codec)
		{
			LOGE("Could not found audio codec.....");
			return -1;
		}

		
		if (avcodec_open2(p_audio_codecCtx,p_audio_codec,NULL) < 0)
		{
			LOGE("Could not open audio codec......");
			return -1;
		}
		LOGI("audio init  sucess...................");
#ifdef WIN32
		g_audio_tid = _beginthread(codec_audio_thread,0,this);
#else
		pthread_mutex_unlock(&p_audio_mutex);
		pthread_create(&g_audio_tid,NULL,codec_audio_thread,this);
#endif
		
	}

#ifdef WIN32
	g_read_tid = _beginthread(read_packet_thread,0,this);

#else
	pthread_create(&g_read_tid,NULL,read_packet_thread,this);
#endif
	
	return 1;
}

void Player::pause(){
	ispause = 1;
}

void Player::resume(){
	ispause = 0;
}

void Player::stop(){
	isstop = 1;
}

void Player::restart(){
	ispause = 0;
	isstop = 0;
}

int Player::next(const char* name){
	ispause = 0;
	isstop = 1;

	// pthread_kill(g_read_tid,SIGKILL);
	// pthread_kill(g_video_tid,SIGKILL);
	// pthread_kill(g_audio_tid,SIGKILL);
	// pthread_join(g_read_tid,NULL);
	// pthread_join(g_video_tid,NULL);
	// pthread_join(g_audio_tid,NULL);

	this->release();
	g_video_queue.clear();
	g_audio_queue.clear();

	return this->player(name);
}

void Player::forword(const int value){

}

void Player::open_audio(){

}