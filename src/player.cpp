#include "player.h"

int Player::read_packet_thread(void* obj){
	if (!obj)
	{
		/* code */
		return -1;
	}
	Player* player = (Player*)obj;
	AVPacket *pPacket = (AVPacket*)av_malloc(sizeof(AVPacket));
	AVFrame *pFrame = av_frame_alloc();
	AVFrame *pFrameYUV = av_frame_alloc();
	int tns, thh, tmm, tss;  
	tns  = (player->get_p_formatCtx()->duration)/1000000;  
	thh  = tns / 3600;  
	tmm  = (tns % 3600) / 60;  
	tss  = (tns % 60);
	while(av_read_frame(player->get_p_formatCtx(),pPacket) >= 0){
		if (pPacket->stream_index == player->m_videoindex){
			/* code */
			int got = 0;
			int ret = avcodec_decode_video2(player->get_p_video_codecCtx(),pFrame,&got,pPacket);
			if (ret < 0)
			{
				/* code */
				printf("decoder error.........\n");
				break ;
			}
			if (got)
			{
				/* code */
				/**
				SDL_LockYUVOverlay(player->p_bmp);
				pFrameYUV->data[0] = player->p_bmp->pixels[0];
				pFrameYUV->data[1] = player->p_bmp->pixels[1];
				pFrameYUV->data[2] = player->p_bmp->pixels[2];
				pFrameYUV->linesize[0] = player->p_bmp->pitches[0];
				pFrameYUV->linesize[1] = player->p_bmp->pitches[1];
				pFrameYUV->linesize[2] = player->p_bmp->pitches[2];

				sws_scale(player->img_convert_ctx, (const uint8_t* const*)pFrame->data, pFrame->linesize, 0, player->get_p_video_codecCtx()->height, pFrameYUV->data, pFrameYUV->linesize);

				SDL_UnlockYUVOverlay(player->p_bmp);
				SDL_Rect rect;
				rect.x = 0;
				rect.y = 0;
				rect.w = 800;
				rect.h = 800;

				SDL_DisplayYUVOverlay(player->p_bmp,&rect);
				*/
				SDL_LockMutex(p_video_mutex);
				g_video_queue.push(*pFrame);
				SDL_CondSignal(p_cond);
				SDL_UnlockMutex(p_video_mutex);
			}
			int nts,nh,nm,ns;
			nts = pPacket->pts / 1000000;
			nh = nts / 3600;
			nm = (nts % 3600) / 60;
			ns = nts % 60;
			//printf("video:len:%d  play time:%02d:%02d:%02d  total time:%02d:%02d:%02d  h:%d  w:%d\n",ret,nh,nm,ns,thh,tmm,tss,pFrame->height,pFrame->width);
			SDL_Delay(1);
		}else if(pPacket->stream_index == player->m_audioindex){
			int got = 0;
			int ret = avcodec_decode_audio4(player->get_p_audio_codecCtx(),pFrame,&got,pPacket);
			if (got)
			{
				/* code */
				SDL_LockMutex(p_audio_mutex);
				g_audio_queue.push(*pFrame);
				SDL_UnlockMutex(p_audio_mutex);
			}

			int nts,nh,nm,ns;
			nts = pPacket->pts / 1000000;
			nh = nts / 3600;
			nm = (nts % 3600) / 60;
			ns = nts % 60;
			//printf("audio:len:%d  play time:%02d:%02d:%02d  total time:%02d:%02d:%02d  h:%d  w:%d\n",ret,nh,nm,ns,thh,tmm,tss,pFrame->height,pFrame->width);
			SDL_Delay(1);
		}
	}
	cout<<"video_queue size:"<<g_video_queue.size()<<endl;
	cout<<"audio_queue size:"<<g_audio_queue.size()<<endl;
	av_free(pPacket);
	av_free(pFrame);
	av_free(pFrameYUV);

	return 0;
}

int Player::codec_video_thread(void* obj){
	if (!obj)
	{
		/* code */
		return -1;
	}
	SDL_Delay(20);

	Player* player = (Player*)obj;
	AVFrame* pFrameYUV = av_frame_alloc();
	for(;;){
		SDL_LockMutex(p_video_mutex);
		if (g_video_queue.size() <= 0)
		{
			/* code */
			//SDL_CondWait(p_cond,p_video_mutex);
			SDL_UnlockMutex(p_video_mutex);
			continue;
		}

		
		AVFrame* frame = av_frame_alloc();
		*frame = g_video_queue.queue();
		g_video_queue.pop();
		cout<<"..............................."<<frame->linesize[0]<<" \t"<<g_video_queue.size()<<endl;
		pFrameYUV->data[0] = player->p_bmp->pixels[0];
		pFrameYUV->data[1] = player->p_bmp->pixels[1];
		pFrameYUV->data[2] = player->p_bmp->pixels[2];
		pFrameYUV->linesize[0] = player->p_bmp->pitches[0];
		pFrameYUV->linesize[1] = player->p_bmp->pitches[1];
		pFrameYUV->linesize[2] = player->p_bmp->pitches[2];

		

		sws_scale(player->img_convert_ctx, (const uint8_t* const*)frame->data, frame->linesize, 0, player->get_p_video_codecCtx()->height, pFrameYUV->data, pFrameYUV->linesize);

		SDL_UnlockMutex(p_video_mutex);

		SDL_Rect rect;
		rect.x = 0;
		rect.y = 0;
		rect.w = 800;
		rect.h = 800;

		SDL_DisplayYUVOverlay(player->p_bmp,&rect);
		SDL_Delay(50);
	}
	av_free(pFrameYUV);

	return 0;
}

int Player::codec_audio_thread(void *obj){
	if (!obj)
	{
		/* code */
		return -1;
	}
	SDL_Delay(20);

	return 0;

	Player* player = (Player*)obj;

	AVFrame* pFrameYUV = av_frame_alloc();
	for(;;){
		if (g_audio_queue.size() <= 0)
		{
			/* code */
			continue;
		}
		
		SDL_LockMutex(p_audio_mutex);
		AVFrame frame = g_audio_queue.queue();

		g_audio_queue.pop();
		SDL_UnlockMutex(p_audio_mutex);
	}
	av_free(pFrameYUV);

	return 0;
}

Player::Player(SDL_Surface* screen):
m_videoindex(-1),m_audioindex(-1){
	this->screen = screen;
	this->onInit();
}

Player::~Player(){
	if (p_video_codecCtx)
	{
		/* code */
		avcodec_close(p_video_codecCtx);
	}

	if (p_audio_codecCtx)
	{
		/* code */
		avcodec_close(p_audio_codecCtx);
	}

	//m_video_queue.clear();
	//m_audio_queue.clear();
	
	avformat_close_input(&p_formatCtx);
}

void Player::onInit(){

	av_register_all();
	avcodec_register_all();

	p_formatCtx = avformat_alloc_context();
	if (!p_formatCtx)
	{
		/* code */
		m_sign = -1;
		return;
	}
	p_video_codecCtx = NULL;
	p_audio_codecCtx = NULL;

}

int Player::player(const char* filename){
	if (m_sign == -1 || !filename)
	{
		/* code */
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
			//break;
		}
		if (p_formatCtx->streams[ii]->codec->codec_type == AVMEDIA_TYPE_AUDIO)
		{
			/* code */
			m_audioindex = ii;
		}
	}
	if (m_videoindex == -1 || m_audioindex == -1)
	{
		/* code */
		cout<<"Didn't find video stream......."<<endl;;
		return -1;
	}

	p_video_codecCtx = p_formatCtx->streams[m_videoindex]->codec;
	p_audio_codecCtx = p_formatCtx->streams[m_audioindex]->codec;

	p_video_codec = avcodec_find_decoder(p_video_codecCtx->codec_id);
	p_audio_codec = avcodec_find_decoder(p_audio_codecCtx->codec_id);
	if (!p_video_codec)
	{
		/* code */
		cout<<"Could not found video codec...."<<endl;;
		return -1;
	}

	if (!p_audio_codec)
	{
		/* code */
		cout<<"Could not found audio codec....."<<endl;
		return -1;
	}

	if (avcodec_open2(p_video_codecCtx,p_video_codec,NULL) < 0)
	{
		/* code */
		cout<<"Could not open video codec......"<<endl;;
		return -1;
	}

	if (avcodec_open2(p_audio_codecCtx,p_audio_codec,NULL) < 0)
	{
		/* code */
		cout<<"Could not open audio codec......"<<endl;
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



	av_dump_format(p_formatCtx,0,filename,0);
	SDL_CreateThread(read_packet_thread,this);
	SDL_CreateThread(codec_video_thread,this);
	SDL_CreateThread(codec_audio_thread,this);

	return 1;
}