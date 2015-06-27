#include <iostream>
#include <SDL/SDL.h>

#include "player.h"

using namespace std;

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

	screen = SDL_SetVideoMode(800,800,0,0);
	if (!screen)
	{
		printf("SDL:Could not set video mode - exiting:%s\n",SDL_GetError());
		return -1;
	}

	
	SDL_WM_SetCaption("simple SDL test",NULL);

	Player player(screen);
	if(!player.player(argc[1])){
		cout<<"player faile............."<<endl;
		return -1;
	}
	//Player::read_packet_thread(&player);
	cout<<"player sucess.........."<<endl;

	//cout<<"video_queue size:"<<player.m_video_queue.size()<<endl;
	//cout<<"audio_queue size:"<<player.m_audio_queue.size()<<endl;
	for(;;){
		SDL_WaitEvent(&event);
		if (event.type == SDL_QUIT)
		{
			/* code */
			break;
		}
		
	}
	return 0;
}