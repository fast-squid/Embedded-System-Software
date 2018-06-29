#include <jni.h>
#include "android/log.h"

#define LOG_TAG "MyTag"
#define LOGV(...)   __android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG, __VA_ARGS__)

extern int first(int x,int y);
extern int wwww();
extern int cccc();

jint JNICALL Java_org_example_ndk_NDKExam_add(JNIEnv *env, jobject this, jint x, jint y)
{
	LOGV("log test %d", 1234);
	return first(x, y);
}

jint JNICALL Java_org_example_ndk_NDKExam_testString(JNIEnv *env, jobject this, jint mode)
{
	return wwww(mode);
}
void JNICALL Java_org_example_ndk_NDKExam_close(JNIEnv *env, jobject this){
	cccc();
}
