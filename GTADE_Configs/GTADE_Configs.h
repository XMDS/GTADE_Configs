#pragma once
#include <android/log.h>

#include "Gloss.h"

#define PLUGIN_NAME "GTADE_Configs"
#define XMDS_LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, PLUGIN_NAME, __VA_ARGS__))
#define XMDS_LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, PLUGIN_NAME, __VA_ARGS__))
#define XMDS_LOGE(...) ((void)__android_log_print(ANDROID_LOG_ERROR, PLUGIN_NAME, __VA_ARGS__))

extern void* GetGameHandle();
