#include <iostream>
#include <SDL/SDL.h>

#include "player.h"

using namespace std;

Queue<AVPacket> g_video_queue;
Queue<AVPacket> g_audio_queue;

SDL_mutex* p_video_mutex;
SDL_mutex* p_audio_mutex;	
SDL_cond *p_cond;
SDL_cond *p_audiocond;


void sig(int sig){
	printf("obtain sig.....");
	if(sig == SIGINT){
		exit(0);
	}
}

int main(int argv,const char* argc[]){
	if (argv < 2)
	{
		/* code */
		cout<<"please input file of player..."<<endl;
		return -1;
	}

	signal(SIGINT,sig);
	signal(SIGQUIT,sig);

	/**
	* about screen display
	*/
	SDL_Surface* screen;
	SDL_Rect rect;
	SDL_Thread *video_tid;
	SDL_Event event;

	p_video_mutex = SDL_CreateMutex();
	p_audio_mutex = SDL_CreateMutex();
	p_cond = SDL_CreateCond();
	p_audiocond = SDL_CreateCond();

	screen = SDL_SetVideoMode(800,800,0,0);
	if (!screen)
	{
		printf("SDL:Could not set video mode - exiting:%s\n",SDL_GetError());
		return -1;
	}

	
	SDL_WM_SetCaption("simple SDL test",NULL);

	

	Player player(screen);
	if(player.player(argc[1]) == -1){
		cout<<"player faile............."<<endl;
		return -1;
	}
	cout<<"player sucess.........."<<endl;

	for(;;){
		SDL_WaitEvent(&event);
		if (event.type == SDL_QUIT)
		{
			/* code */
			break;
		}
		
	}
	SDL_DestroyMutex(p_video_mutex);
	SDL_DestroyMutex(p_audio_mutex);
	SDL_DestroyCond(p_cond);
	SDL_DestroyCond(p_audiocond);
	SDL_Quit();
	
	return 0;
}