#include <iostream>
#include <SDL/SDL.h>

#include "player.h"

using namespace std;

#ifdef WIN32
	#include <windows.h>
	#include <process.h>
	typedef DWORD pthread_t;
	typedef HANDLE pthread_mutex_t;
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

Queue<AVPacket> g_video_queue;
Queue<AVPacket> g_audio_queue;

pthread_t g_read_tid;
pthread_t g_video_tid;
pthread_t g_audio_tid;

int g_exit_code = 0;

#ifdef WIN32

#else
	pthread_mutex_t p_video_mutex;
	pthread_mutex_t p_audio_mutex;
	pthread_cond_t p_video_cond;
	pthread_cond_t p_audio_cond;
#endif



void sig(int sig){
	printf("obtain sig.....");
	if(sig == SIGINT){
		g_exit_code = 1;
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

#ifdef WIN32
	p_video_mutex = CreateMutex(NULL,false,NULL);
	p_audio_mutex = CreateMutex(NULL,false,NULL);
#else
	pthread_mutex_init(&p_audio_mutex,NULL);
	pthread_mutex_init(&p_video_mutex,NULL);
	pthread_cond_init(&p_video_cond,NULL);
	pthread_cond_init(&p_audio_cond,NULL);
#endif


	screen = SDL_SetVideoMode(800,800,0,0);
	if (!screen)
	{
		printf("SDL:Could not set video mode - exiting:%s\n",SDL_GetError());
		return -1;
	}

	
	SDL_WM_SetCaption("Video and Audio Player",NULL);

	

	Player player(screen);
	if(player.player(argc[1]) == -1){
		cout<<"player faile............."<<endl;
		return -1;
	}
	cout<<"player sucess.........."<<endl;

	for(;g_exit_code != 1;){
		SDL_WaitEvent(&event);
		if (event.type == SDL_QUIT)
		{
			/* code */
			g_exit_code = 1;
			break;
		}
		
	}

#ifdef WIN32

#else
	pthread_cancel(g_read_tid);
	pthread_cancel(g_video_tid);
	pthread_cancel(g_audio_tid);
	pthread_join(g_read_tid,NULL);
	pthread_join(g_video_tid,NULL);
	pthread_join(g_audio_tid,NULL);
	pthread_mutex_destroy(&p_audio_mutex);
	pthread_mutex_destroy(&p_video_mutex);
	pthread_cond_destroy(&p_video_cond);
	pthread_cond_destroy(&p_audio_cond);
#endif
	
	
	SDL_Quit();
	
	return 0;
}