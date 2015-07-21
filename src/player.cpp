#include "player.h"

#define MAX_BUFFERED_PACKET 300

static int is_finish = 0;
static int onces = 0;
static AVFrame* frist_frame;

int Player::ispause = 0;
int Player::isstop = 0;

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
			g_audio_queue.push(*pPacket);
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
	cout<<"video_queue size:"<<g_video_queue.size()<<endl;
	cout<<"audio_queue size:"<<g_audio_queue.size()<<endl;
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
	int tns, thh, tmm, tss;  
	tns  = (player->get_p_formatCtx()->duration)/1000000;  
	thh  = tns / 3600;  
	tmm  = (tns % 3600) / 60;  
	tss  = (tns % 60);
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
			printf("decoder error.........\n");
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
			pFrameYUV->data[0] = player->p_bmp->pixels[0];
			pFrameYUV->data[1] = player->p_bmp->pixels[2];
			pFrameYUV->data[2] = player->p_bmp->pixels[1];
			pFrameYUV->linesize[0] = player->p_bmp->pitches[0];
			pFrameYUV->linesize[1] = player->p_bmp->pitches[2];
			pFrameYUV->linesize[2] = player->p_bmp->pitches[1];

			sws_scale(player->img_convert_ctx, (const uint8_t* const*)pFrame->data, pFrame->linesize, 0, player->get_p_video_codecCtx()->height, pFrameYUV->data, pFrameYUV->linesize);
			SDL_Rect rect;
			rect.x = 0;
			rect.y = 0;
			rect.w = 800;
			rect.h = 800;

			SDL_DisplayYUVOverlay(player->p_bmp,&rect);

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
			printf("play time:%02d:%02d:%02d  total time:%02d:%02d:%02d  h:%d  w:%d  packet:%d audio:%d\n",nh,nm,ns,thh,tmm,tss,pFrame->height,pFrame->width,g_video_queue.size(),g_audio_queue.size());
		}
		
		
		val.tv_sec= 0;
		val.tv_usec = 40000;
		select(0,NULL,NULL,NULL,&val);
		av_free(packet);
	}
	cout<<"video thread over finish........"<<endl;
	av_free(pFrameYUV);

	return NULL;
}

/**
* player audio thread and function
*/
Uint8  *audio_chunk;   
Uint32  audio_len;   
Uint8  *audio_pos; 

 void  audio_callback(void *udata,Uint8 *stream,int len){   
        /*  Only  play  if  we  have  data  left  */   
    if(audio_len == 0)   
            return;   
        /*  Mix  as  much  data  as  possible  */   
    len = (len>audio_len?audio_len:len);   
    SDL_MixAudio(stream,audio_pos,len,SDL_MIX_MAXVOLUME);  
    audio_pos += len;   
    audio_len -= len;  
    if (audio_len <= 0)
     {
     	/* code */
     	pthread_cond_signal(&p_audio_cond);
     } 
}

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

	SDL_AudioSpec wanted_spec;  
    wanted_spec.freq = player->get_p_audio_codecCtx()->sample_rate;
    cout<<"sample_rate:"<<wanted_spec.freq<<endl;   
    wanted_spec.format = AUDIO_S16SYS;   
    wanted_spec.channels = 1;//player->get_p_audio_codecCtx()->channels;   
    wanted_spec.silence = 0;   
    wanted_spec.samples = 1152;  //mp3 and mav  1152
    wanted_spec.callback = audio_callback;   
    wanted_spec.userdata = player->get_p_audio_codecCtx(); 
    if (SDL_OpenAudio(&wanted_spec, NULL)<0)   
    {   
        printf("can't open audio.\n");   
        return NULL;   
    } 
	AVFrame* pFrame = av_frame_alloc();
	int tns, thh, tmm, tss;  
	tns  = (player->get_p_formatCtx()->duration)/1000000;  
	thh  = tns / 3600;  
	tmm  = (tns % 3600) / 60;  
	tss  = (tns % 60);
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
			cout<<"audio  decoder fail.........."<<endl;
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
			audio_chunk = (Uint8*) pFrame->data[0];   
        	audio_len = pFrame->linesize[0];  
        	audio_pos = audio_chunk;  
            SDL_PauseAudio(0);  
            pthread_cond_wait(&p_audio_cond,&p_audio_mutex);
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
			printf("audio:len:%d  play time:%02d:%02d:%02d  total time:%02d:%02d:%02d  h:%d  w:%d   %d   %d\n",ret,nh,nm,ns,thh,tmm,tss,pFrame->height,pFrame->width,code,g_audio_queue.size());
		}
		av_free(pPacket);
	}
	av_free(pFrame);

	SDL_CloseAudio();

	cout<<"audio thread over..............."<<endl;

	return NULL;
}

Player::Player(SDL_Surface* screen):
m_videoindex(-1),m_audioindex(-1){
	this->screen = screen;
	this->onInit();
}

Player::~Player(){
	this->release();
}

void Player::release(){
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
	//p_formatCtx = NULL;
}

void Player::onInit(){

	av_register_all();
	avcodec_register_all();
	avformat_network_init();

	
	p_video_codecCtx = NULL;
	p_audio_codecCtx = NULL;
	p_formatCtx = NULL;

}

int Player::player(const char* filename){
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

	if(avformat_open_input(&p_formatCtx,filename,NULL,NULL) != 0){
		cout<<"avformat_open_input fail...."<<strerror(errno)<<endl;
		return -1;
	}

	if (avformat_find_stream_info(p_formatCtx,NULL) < 0)
	{
		/* code */
		cout<<"find stream info fail.........."<<endl;
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
		cout<<"Didn't find video stream......."<<p_formatCtx->nb_streams<<"............."<<strerror(errno)<<endl;;
		return -1;
	}

	if(m_videoindex != -1){
		p_video_codecCtx = p_formatCtx->streams[m_videoindex]->codec;
		p_video_codec = avcodec_find_decoder(p_video_codecCtx->codec_id);

		if (!p_video_codec)
		{
			/* code */
			cout<<"Could not found video codec...."<<endl;;
			return -1;
		}
		if (avcodec_open2(p_video_codecCtx,p_video_codec,NULL) < 0)
		{
			cout<<"Could not open video codec......"<<endl;;
			return -1;
		}

		img_convert_ctx = sws_getContext(p_video_codecCtx->width, p_video_codecCtx->height, 
			p_video_codecCtx->pix_fmt, p_video_codecCtx->width, p_video_codecCtx->height, 
			PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL); 

		/**
		* display
		*/
		int screen_w = p_video_codecCtx->width;
		int screen_h = p_video_codecCtx->height;
		p_bmp = SDL_CreateYUVOverlay(screen_w,screen_h,SDL_YV12_OVERLAY,this->screen);
#ifdef WIN32
		g_video_tid = _beginthread(codec_video_thread,0,this);
#else
		pthread_mutex_unlock(&p_video_mutex);
		pthread_create(&g_video_tid,NULL,codec_video_thread,this);
#endif
		
		cout<<"video init sucess...................."<<endl;
	}

	if(m_audioindex != -1){
		p_audio_codecCtx = p_formatCtx->streams[m_audioindex]->codec;
		p_audio_codec = avcodec_find_decoder(p_audio_codecCtx->codec_id);
		
		if (!p_audio_codec)
		{
			cout<<"Could not found audio codec....."<<endl;
			return -1;
		}

		
		if (avcodec_open2(p_audio_codecCtx,p_audio_codec,NULL) < 0)
		{
			cout<<"Could not open audio codec......"<<endl;
			return -1;
		}
		cout<<"audio init  sucess..................."<<endl;
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

	pthread_kill(g_read_tid,SIGKILL);
	pthread_kill(g_video_tid,SIGKILL);
	pthread_kill(g_audio_tid,SIGKILL);
	pthread_join(g_read_tid,NULL);
	pthread_join(g_video_tid,NULL);
	pthread_join(g_audio_tid,NULL);
	cout<<"create thread start...................."<<endl;
	this->release();
	g_video_queue.clear();
	g_audio_queue.clear();

	return this->player(name);
}

void Player::forword(const int value){

}
