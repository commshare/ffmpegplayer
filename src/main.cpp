#include <iostream>

#include "player.h"

using namespace std;

int main(int argv,const char* argc[]){
	if (argv < 2)
	{
		/* code */
		cout<<"please input file of player..."<<endl;
		return -1;
	}
	Player player;
	if(!player.player(argc[1])){
		cout<<"player faile............."<<endl;
		return -1;
	}
	Player::read_packet_thread(&player);
	cout<<"player sucess.........."<<endl;

	cout<<"video_queue size:"<<player.m_video_queue.size()<<endl;
	cout<<"audio_queue size:"<<player.m_audio_queue.size()<<endl;
	return 0;
}