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

bool PassTouch(float x, float y) {
	FluidTouch(x, y);
	return true;
}

bool Rotate(int angle) {
	FluidRotate(angle);
	return true;
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

	JNIEXPORT void JNICALL Java_uk_co_mishurov_fluid_ParticlesLib_touch(JNIEnv * env, jobject obj, jfloat x, jfloat y) {
		PassTouch(x, y);
	}

	JNIEXPORT void JNICALL Java_uk_co_mishurov_fluid_ParticlesLib_rotate(JNIEnv * env, jobject obj, jint angle) {
		Rotate(angle);
	}

	JNIEXPORT void JNICALL Java_uk_co_mishurov_fluid_ParticlesLib_createAssetManager(JNIEnv* env, jobject obj, jobject assetManager) {
		AAssetManager* mgr = AAssetManager_fromJava(env, assetManager);
		assert(mgr);
		SetAssetManager(mgr);
	}

}
