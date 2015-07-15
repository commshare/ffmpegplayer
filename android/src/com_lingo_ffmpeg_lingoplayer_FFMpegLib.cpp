#include "com_lingo_ffmpeg_lingoplayer_FFMpegLib.h"
#include <string.h>
#include <unistd.h>
#include "player.h"
#include "log.h"
#ifdef __cplusplus
extern "C" 
{
#endif
#include <jni.h>
#ifdef __cplusplus
}
#endif

Player* g_player = NULL;
JavaVM *g_jvm = NULL;
JNIEnv* g_env = NULL;
jobject g_job = NULL;
jmethodID g_method;

JNIEXPORT jint JNICALL Java_com_lingo_ffmpeg_lingoplayer_FFMpegLib_initplayer
  (JNIEnv *env, jobject job){
	g_player = Player::getInstance();
	if(g_player == NULL){
		return 0;
	}

	
	g_job = env->NewGlobalRef(job);
  g_env = env;

	env->GetJavaVM(&g_jvm);

  jclass cls = env->GetObjectClass(g_job); 
  g_method = env->GetMethodID(cls,"statusCallback","(I)V"); 

	return 1;
}

JNIEXPORT jint JNICALL Java_com_lingo_ffmpeg_lingoplayer_FFMpegLib_player
  (JNIEnv *env, jobject job, jstring jstr){
  	const char* name = (char*)env->GetStringUTFChars(jstr,0);
  	LOGE("filename is :%s",name);
  	if (g_player != NULL)
  	{
  		/* code */
		int ret = g_player->player(name);
		return ret;
  	}
  	
	return 0;
}

JNIEXPORT jint JNICALL Java_com_lingo_ffmpeg_lingoplayer_FFMpegLib_pause
  (JNIEnv *env, jobject job){
  	if (g_player != NULL)
  	{
  		/* code */
  		g_player->pause();
  		return 1;
  	}

	return 0;
}

JNIEXPORT jint JNICALL Java_com_lingo_ffmpeg_lingoplayer_FFMpegLib_resume
  (JNIEnv *env, jobject job){

  	if (g_player != NULL)
  	{
  		/* code */
  		g_player->resume();
  		return 1;
  	}

	return 0;
}

JNIEXPORT jint JNICALL Java_com_lingo_ffmpeg_lingoplayer_FFMpegLib_stop
  (JNIEnv *env, jobject job){
  	if (g_player != NULL)
  	{
  		/* code */
  		g_player->stop();
  		return 1;
  	}

	return 0;
}

JNIEXPORT jint JNICALL Java_com_lingo_ffmpeg_lingoplayer_FFMpegLib_release
  (JNIEnv *env, jobject job){
  	if (g_player != NULL)
  	{
  		/* code */
  		Player::releaseInstance();
  		return 1;
  	}

  	return 0;
}

JNIEXPORT jint JNICALL Java_com_lingo_ffmpeg_lingoplayer_FFMpegLib_next
  (JNIEnv *env, jobject job, jstring jstr){
    LOGE("....................next.................");
  	const char* name = env->GetStringUTFChars(jstr,false);
  	LOGE("next url is :%s",name);
  	if (g_player != NULL)
  	{
  		/* code */
		  int ret = g_player->next(name);
		  return ret;
  	}

	return 5;
}

