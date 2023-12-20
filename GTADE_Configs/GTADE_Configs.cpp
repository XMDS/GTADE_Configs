#include "GTADE_Configs.h"

#include <jni.h>
#include <unistd.h>
#include <cstdint>

#include "inireader.h"

#include "ScriptCommands.h"
#include "GameVersion.hpp"

#ifdef __USE_AML__
#include "mod/amlmod.h"
MYMOD(XMDS.GTADE.CONFIGS, GTADE_Configs, 1.0, XMDS);
#endif // __USE_AML__

#pragma comment (lib, "libGlossHook.a")

using namespace plugin;
using namespace XB;

static bool bEnableFrameLimit = false; // defaul
static float Frame = 60.0f; // defaul
static uintptr_t _ZN6CTimer8game_FPSE = 0;
static bool bShowCoords = false;

namespace JniFunc
{
	static jstring GetPackageName(JNIEnv* env, jobject jActivity) //获取app包名
	{
		jmethodID method = env->GetMethodID(env->GetObjectClass(jActivity), "getPackageName", "()Ljava/lang/String;");
		return (jstring)env->CallObjectMethod(jActivity, method);
	}

	static jobject GetGlobalContext(JNIEnv* env) //获取app线程的全局对象
	{
		jclass activityThread = env->FindClass("android/app/ActivityThread");
		jmethodID currentActivityThread = env->GetStaticMethodID(activityThread, "currentActivityThread", "()Landroid/app/ActivityThread;");
		jobject at = env->CallStaticObjectMethod(activityThread, currentActivityThread);
		jmethodID getApplication = env->GetMethodID(activityThread, "getApplication", "()Landroid/app/Application;");
		jobject context = env->CallObjectMethod(at, getApplication);
		return context;
	}

	static jobject GetStorageDir(JNIEnv* env) //获取安卓目录 /storage/emulated/0 instead of /sdcard (example)
	{
		jclass classEnvironment = env->FindClass("android/os/Environment");
		return (jstring)env->CallStaticObjectMethod(classEnvironment, env->GetStaticMethodID(classEnvironment, "getExternalStorageDirectory", "()Ljava/io/File;"));
	}

	static jstring GetAbsolutePath(JNIEnv* env, jobject jFile) //转换为绝对路径
	{
		jmethodID method = env->GetMethodID(env->GetObjectClass(jFile), "getAbsolutePath", "()Ljava/lang/String;");
		return (jstring)env->CallObjectMethod(jFile, method);
	}
}

static char* GetPackageName(JNIEnv* env)
{
	static char package_name[1024];
	jstring j_name = JniFunc::GetPackageName(env, JniFunc::GetGlobalContext(env));
	const char* c_name = env->GetStringUTFChars(j_name, NULL);
	strcpy(package_name, c_name);
	env->ReleaseStringUTFChars(j_name, c_name);
	return package_name;
}

static char* GetStoragePath(JNIEnv* env)
{
	static char storage_path[1024];
	jstring j_name = JniFunc::GetAbsolutePath(env, JniFunc::GetStorageDir(env));
	const char* c_name = env->GetStringUTFChars(j_name, NULL);
	strcpy(storage_path, c_name);
	env->ReleaseStringUTFChars(j_name, c_name);
	return storage_path;
}

void* GetGameHandle()
{
	static const char* lib_name = GetGameLibName();

	static gloss_lib handle = GlossOpen(lib_name);
	if (handle == nullptr) {
		XMDS_LOGE("Open game lib '%s' failed!", lib_name);
		return nullptr;
	}
	return handle;
}

static void* (*Orig_UGameterSettings_OnFrameRateLimitSet)(void*, bool) = nullptr;
static void* Proxy_UGameterSettings_OnFrameRateLimitSet(void* _this, bool bEnable)
{
	void* this_setting = UGTASingleton::GetSettings(false, false, false);
	void* default_setting = UGameterSettings::GetPlatformDefaultRules(this_setting);
	WriteMemory<float>((uintptr_t)default_setting + 0x54, Frame);
	WriteMemory<float>((uintptr_t)this_setting + 0x54, Frame);

	return Orig_UGameterSettings_OnFrameRateLimitSet(_this, false);
}

static void (*Orig_OnCScriptEvent)() = nullptr;
static void Proxy_OnCScriptEvent()
{
	float fps = ReadMemory<float>(_ZN6CTimer8game_FPSE);
	
	char text[256];
	memset(text, 0, sizeof(text));
	sprintf(text, "FPS: %.2f", fps);
	uint16_t* wtext = new uint16_t[strlen(text) + 1];
	
	AsciiToGxtChar(text, wtext);
	int width = OS_ScreenGetWidth();
	int height = OS_ScreenGetHeight();
	float scaleX = (float)width / 1920.0f;
	float scaleY = (float)height / 1080.0f;
	float x = 28.0f * scaleX;
	float y = 10.0f * scaleY;

	CFont::SetColor(CRGBA(0, 255, 0, 255));
	CFont::SetDropShadowPosition(0);
	CFont::SetFontStyle(1);
	CFont::SetJustify(1);
	CFont::SetWrapx(1000.0f);
	CFont::SetScale(1.3f);
	CFont::SetOrientation(1);
	CFont::PrintString(x, y, wtext);
	delete wtext;
	
	if (bShowCoords) {
		if (Command<Commands::IS_PLAYER_PLAYING>(0)) {
			CPed* actor = nullptr;
			CVector pos = { 0.0f, 0.0f, 0.0f };
			float angle = 0.0f;
			Command<Commands::GET_PLAYER_CHAR>(0, &actor);
			Command<Commands::GET_CHAR_COORDINATES>(actor, &pos.x, &pos.y, &pos.z);
			Command<Commands::GET_CHAR_HEADING>(actor, &angle);
			memset(text, 0, sizeof(text));
			sprintf(text, "X: %.3f   Y: %.3f   Z: %.3f   A: %.3f", pos.x, pos.y, pos.z, angle);
			uint16_t* wtext2 = new uint16_t[strlen(text) + 1];
			AsciiToGxtChar(text, wtext2);
			x = 28.0f * scaleX;
			y = height - 50.0f * scaleY;
			CFont::SetColor(CRGBA(0, 255, 0, 255));
			CFont::SetDropShadowPosition(0);
			CFont::SetFontStyle(1);
			CFont::SetJustify(0);
			CFont::SetWrapx(1000.0f);
			CFont::SetScale(1.3f);
			CFont::SetOrientation(1);
			CFont::PrintString(x, y, wtext2);
			delete wtext2;
		}
	}
	
	return Orig_OnCScriptEvent();
}

extern "C" void PluginMain(JNIEnv* env)
{
	XMDS_LOGI("'%s' init...", PLUGIN_NAME);
	void* handle = GetGameHandle();
	uintptr_t addr = 0u;

	uintptr_t bias = GlossGetLibBiasEx(handle);
	if (bias == 0u) {
		return XMDS_LOGE("Get game lib bias failed!");
	}

	const char* storage_path = GetStoragePath(env);
	const char* package_name = GetPackageName(env);

	char config_path[512];
	memset(config_path, 0, sizeof(config_path));
	sprintf(config_path, "%s/Android/data/%s/mods/config.ini", storage_path, package_name);
	inireader.SetIniPath(config_path);

	if (access(config_path, F_OK)) {
		inireader.WriteBoolean(PLUGIN_NAME, "Enable", true);
		inireader.WriteString(PLUGIN_NAME, "Author", "XMDS 2841824304@qq.com");
		inireader.WriteBoolean(PLUGIN_NAME, "ShowCoords", bShowCoords);

		inireader.WriteBoolean("FrameLimit", "Enable", bEnableFrameLimit);
		inireader.WriteString("FrameLimit", "FPS", "60.0");
		inireader.WriteBoolean("FrameLimit", "ShowFPS", true);
	}

	if (inireader.ReadBoolean(PLUGIN_NAME, "Enable", false)) {
		bEnableFrameLimit = inireader.ReadBoolean("FrameLimit", "Enable", true);
		Frame = inireader.ReadFloat("FrameLimit", "FPS", 30.0f);

		if (Frame > 0.0f) {
			if (!bEnableFrameLimit) {
				addr = GlossSymbol(handle, "_ZN16UGameterSettings19OnFrameRateLimitSetEb", NULL);
				if (addr == 0u) {
					return XMDS_LOGE("Get game symbol '_ZN16UGameterSettings19OnFrameRateLimitSetEb' failed!");
				}
				GlossHook((void*)addr, (void*)Proxy_UGameterSettings_OnFrameRateLimitSet, (void**)&Orig_UGameterSettings_OnFrameRateLimitSet); // hook frame limit
				WriteMemory<float>(bias + 0x3477B9Cu + 4u, Frame); // ue max frame
			}

			WriteMemory<float>(bias + 0x3477B9Cu, Frame); // ue min frame
			WriteMemory<uint32_t>(bias + 0xA75EA88u, static_cast<uint32_t>(Frame)); // orig game frame limit

			// no need orig game frame limit
			/*
			WriteMemory<uint32_t>(bias + 0x558B304, 0x52800789);
			WriteMemory<uint32_t>(bias + 0x5699DB0, 0x52800789);
			WriteMemory<uint32_t>(bias + 0x57707B4, 0x52800788);
			WriteMemory<uint32_t>(bias + 0x57DDA2C, 0x52800789);
			WriteMemory<uint32_t>(bias + 0x57DDAEC, 0x52800789);
			*/
		}

		GlossHook((void*)(bias + 0x57693E4u), (void*)Proxy_OnCScriptEvent, (void**)&Orig_OnCScriptEvent);

		bShowCoords = inireader.ReadBoolean(PLUGIN_NAME, "ShowCoords", false);
		if (inireader.ReadBoolean("FrameLimit", "ShowFPS", false)) {
			// show fps
			_ZN6CTimer8game_FPSE = GlossSymbol(handle, "_ZN6CTimer8game_FPSE", NULL);
		}
	}
}

extern "C" void OnModPreLoad()
{
	JNIEnv* env = aml->GetJNIEnvironment();
	PluginMain(env);
}

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void* reserved)
{
	JNIEnv* env = nullptr;
	if (vm->GetEnv((void**)&env, JNI_VERSION_1_6) != JNI_OK) {
		return JNI_ERR;
	}

	PluginMain(env);
	return JNI_VERSION_1_6;
}


