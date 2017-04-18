#include <jni.h>

#include <cassert>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <android/log.h>

#include "fluid.h"


bool SetupGraphics(int width, int height) {
	FluidInit(width, height);
	return true;
}

void RenderFrame(float elapsed_time) {
	FluidUpdate(elapsed_time);
}

void PassTouch(bool is_down, float x, float y) {
	FluidTouch(is_down, x, y);
}

void Rotate(int angle) {
	FluidRotate(angle);
}

void SetPrefs(std::string fg_color, std::string bg_color, int iterations, int cursor_size) {
	FluidSetPrefs(fg_color, bg_color, iterations, cursor_size);
}

std::string toString(JNIEnv *env, jstring jstr){
	const char* ch = env->GetStringUTFChars(jstr, 0);
	std::string str = std::string(ch);
	env->ReleaseStringUTFChars(jstr, ch);
	return str;
}

extern "C" 
{
	void SetAssetManager(AAssetManager* pManager);
	
	JNIEXPORT void JNICALL Java_uk_co_mishurov_fluid_ParticlesLib_init(JNIEnv * env, jobject obj,  jint width, jint height) {
		SetupGraphics(width, height);
	}


	JNIEXPORT void JNICALL Java_uk_co_mishurov_fluid_ParticlesLib_step(JNIEnv * env, jobject obj, jfloat elapsed_time) {
		RenderFrame(elapsed_time);
	}

	JNIEXPORT void JNICALL Java_uk_co_mishurov_fluid_ParticlesLib_touch(JNIEnv * env, jobject obj, jboolean is_down, jfloat x, jfloat y) {
		PassTouch(is_down, x, y);
	}

	JNIEXPORT void JNICALL Java_uk_co_mishurov_fluid_ParticlesLib_rotate(JNIEnv * env, jobject obj, jint angle) {
		Rotate(angle);
	}

	JNIEXPORT void JNICALL Java_uk_co_mishurov_fluid_ParticlesLib_settings(JNIEnv * env, jobject obj, jstring fg_color, jstring bg_color, jint iterations, jint cursor_size) {
		SetPrefs(toString(env, fg_color), toString(env, bg_color), iterations, cursor_size);
	}

	JNIEXPORT void JNICALL Java_uk_co_mishurov_fluid_ParticlesLib_createAssetManager(JNIEnv* env, jobject obj, jobject assetManager) {
		AAssetManager* mgr = AAssetManager_fromJava(env, assetManager);
		assert(mgr);
		SetAssetManager(mgr);
	}

}
