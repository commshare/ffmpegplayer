#include "player.h"

void Player::read_packet_thread(void* obj){
	if (!obj)
	{
		/* code */
		return;
	}
	Player* player = (Player*)obj;
	AVPacket *pPacket = (AVPacket*)av_malloc(sizeof(AVPacket));
	AVFrame *pFrame = av_frame_alloc();
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
				/*
				SDL_LockYUVOverlay(bmp);
				pFrameYUV->data[0] = bmp->pixels[0];
				pFrameYUV->data[1] = bmp->pixels[1];
				pFrameYUV->data[2] = bmp->pixels[2];
				pFrameYUV->linesize[0] = bmp->pitches[0];
				pFrameYUV->linesize[1] = bmp->pitches[1];
				pFrameYUV->linesize[2] = bmp->pitches[2];

				sws_scale(img_convert_ctx, (const uint8_t* const*)pFrame->data, pFrame->linesize, 0, pCodecCtx->height, pFrameYUV->data, pFrameYUV->linesize);

				SDL_UnlockYUVOverlay(bmp);
				SDL_Rect rect;
				rect.x = 0;
				rect.y = 0;
				rect.w = 400;
				rect.h = 400;

				SDL_DisplayYUVOverlay(bmp,&rect);
				*/
				player->m_video_queue.push(*pPacket);
			}
			int nts,nh,nm,ns;
			nts = pPacket->pts / 1000000;
			nh = nts / 3600;
			nm = (nts % 3600) / 60;
			ns = nts % 60;
			printf("video:len:%d  play time:%02d:%02d:%02d  total time:%02d:%02d:%02d  h:%d  w:%d\n",ret,nh,nm,ns,thh,tmm,tss,pFrame->height,pFrame->width);
			//SDL_Delay(50);
		}else if(pPacket->stream_index == player->m_audioindex){
			int got = 0;
			int ret = avcodec_decode_audio4(player->get_p_audio_codecCtx(),pFrame,&got,pPacket);
			if (got)
			{
				/* code */
				player->m_audio_queue.push(*pPacket);
			}

			int nts,nh,nm,ns;
			nts = pPacket->pts / 1000000;
			nh = nts / 3600;
			nm = (nts % 3600) / 60;
			ns = nts % 60;
			printf("audio:len:%d  play time:%02d:%02d:%02d  total time:%02d:%02d:%02d  h:%d  w:%d\n",ret,nh,nm,ns,thh,tmm,tss,pFrame->height,pFrame->width);
		}
		usleep(40000);
	}
	av_free(pPacket);
	av_free(pFrame);
}

void Player::codec_video_thread(void* obj){

}

void Player::codec_audio_thread(void *obj){

}

Player::Player():
m_videoindex(-1),m_audioindex(-1){
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

	av_dump_format(p_formatCtx,0,filename,0);

	return 1;
}