/*
* MIT License
*
* Copyright (c) 2023 BEATLESS
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*/

// create by XMDS on 2023-10-01
// A separate header file for the game version, so that it can be used independently in other projects.

#ifndef __XBEATLESS_GTA_ANDROID_GAME_VERSION_HPP__
#define __XBEATLESS_GTA_ANDROID_GAME_VERSION_HPP__

#include <cstdio>
#include <cstdint>
#include <cstring>
#include <android/log.h>

// need GLossHook: https://github.com/XMDS/GlossHook
#include "Gloss.h"

#ifndef XB_GTA_TAG
#define XB_GTA_TAG "XB_GTA_GAME"
#define XB_GTA_LOGE(text) __android_log_write(ANDROID_LOG_ERROR, XB_GTA_TAG, text)
#define XB_GTA_LOGE2(text, ...) __android_log_print(ANDROID_LOG_ERROR, XB_GTA_TAG, text, __VA_ARGS__)
#endif // !XB_TAG

namespace XB
{
	enum GAME_ID : uint8_t
	{
		GTA3,
		GTAVC,
		GTASA,
		GTALCS,
		GTACTW,

		GTA3_DE, // netflix or RStar
		GTAVC_DE, // netflix or RStar
		GTASA_DE, // netflix or RStar
		MAX_GAME = 8
	};

	enum GAME_VER : uint8_t
	{
		// 32
		III_1_8,
		VC_1_09,
		SA_2_00,
		LCS_2_4,
		CTW_1_04,

		// hign ver
		III_1_9, // 32 or 64
		VC_1_12, // 32 or 64
		SA_2_10, // 32 or 64

		// de ver
		III_DE_1_72,
		VC_DE_1_72,
		SA_DE_1_72,
		
		MAX_VER = 11
	};


	static const char* game_name[] = { "GTA3", "GTAVC", "GTASA", "GTALCS", "GTACTW", "GTA3_DE", "GTAVC_DE", "GTASA_DE" };
	
	static const char* package_name[] = { "com.rockstargames.gta3", "com.rockstargames.gtavc", "com.rockstargames.gtasa", "com.rockstargames.gtalcs", "com.rockstargames.gtactw",
		"com.rockstargames.gta3.de", "com.rockstargames.gtavc.de", "com.rockstargames.gtasa.de",
		"com.netflix.NGP.GTAIIIDefinitiveEdition", "com.netflix.NGP.GTAViceCityDefinitiveEdition", "com.netflix.NGP.GTASanAndreasDefinitiveEdition" };

	static const char* game_version[] = { "III 1.8", "VC 1.09", "SA 2.00", "LCS 2.4", "CTW 1.04", "III 1.9", "VC 1.12", "SA 2.10", 
		"III_DE 1.72.42919648", "VC_DE 1.72.42919648", "SA_DE 1.72.42919648"};

	static const char* game_lib[] = { "libR1.so", "libGTAVC.so", "libGTASA.so", "LibGTALcs.so", "LibCTW.so" , "libUE4.so"};

	inline const char* GetGameProcessName()
	{
		static char name[255];
		memset(name, 0, sizeof(name));
		FILE* cmd_file = fopen("/proc/self/cmdline", "r");
		if (cmd_file) {
			if (fgets(name, sizeof(name) - 1, cmd_file)) {
				fclose(cmd_file);
				return name;
			}	
		}
		return NULL;
	}
	
	inline GAME_VER GetGameVersionId()
	{
		const char* p_name = GetGameProcessName();
		if (p_name) {
			char line[2048];
			memset(line, 0, sizeof(line));

			FILE* maps_file = fopen("/proc/self/maps", "r");
			if (maps_file) {
				while (fgets(line, sizeof(line) - 1, maps_file)) {
					const char* lib_path = strstr(line, "/data/");
					if (lib_path && strstr(line, p_name) && strstr(line, ".so")) {
						uintptr_t plt_addr = GlossGetLibSection(lib_path, ".plt", NULL);
						uintptr_t re_addr = plt_addr - GlossGetLibBase(lib_path, -1);

						if (re_addr && re_addr != plt_addr) {
							switch (re_addr)
							{
							case 0x00189D44:
								return GAME_VER::SA_2_00;
							case 0x000AFBA0:
								return GAME_VER::VC_1_09;
							case 0x000970B8:
								return GAME_VER::III_1_8;
							case 0x00109784:
								return GAME_VER::LCS_2_4;
							case 0x002987F8:
								return GAME_VER::CTW_1_04;
							case 0x00189CCC:
							case 0x00218E90: // 64
								return GAME_VER::SA_2_10;
							case 0x000BAE98:
							case 0x000FFD90: // 64
								return GAME_VER::VC_1_12;
							case 0x00097140:
							case 0x000CE100: // 64
								return GAME_VER::III_1_9;
							case 0x09A80F90:
								return GAME_VER::SA_DE_1_72;
							default:
								break;
							}
						}
					}
				}
				fclose(maps_file);
			}
		}

		XB_GTA_LOGE("GetGameVersionId error, Unknown Game version.");
		return GAME_VER::MAX_VER;
	}

	inline const char* GetGameVersion()
	{
		auto id = GetGameVersionId();
		return id == GAME_VER::MAX_VER ? NULL : game_version[id];
	}
	
	
	inline GAME_ID GetGameId()
	{
		const char* p_name = GetGameProcessName();
		if (p_name) {
			for (uint8_t i = GAME_ID::GTA3; i < GAME_ID::MAX_GAME; i++) {
				if (i == GAME_ID::GTA3_DE || i == GAME_ID::GTAVC_DE || i == GAME_ID::GTASA_DE) {
					if (strcasecmp(p_name, package_name[i]) == 0 || strcasecmp(p_name, package_name[i + 3]) == 0)
						return (GAME_ID)i;
				}
				else {
					if (strcasecmp(p_name, package_name[i]) == 0) {
					return (GAME_ID)i;
					}
				}
			}
		}
		
		// if the process name(package name) is modified, try to load lib
		for (uint8_t i = GAME_ID::GTA3; i < GTA3_DE; i++) {
			auto lib = GlossOpen(game_lib[i]);
			if (lib) {
				GlossClose(lib, false);
				return (GAME_ID)i;
			}
		}

		// if the process name(package name) and lib name are modified, try to load lib
		auto ver = GetGameVersionId();
		switch (ver)
		{
		case GAME_VER::III_1_8:
		case GAME_VER::III_1_9:
			return GAME_ID::GTA3;
		case GAME_VER::VC_1_09:
		case GAME_VER::VC_1_12:
			return GAME_ID::GTAVC;
		case GAME_VER::SA_2_00:
		case GAME_VER::SA_2_10:
			return GAME_ID::GTASA;
		case GAME_VER::LCS_2_4:
			return GAME_ID::GTALCS;
		case GAME_VER::CTW_1_04:
			return GAME_ID::GTACTW;
		case GAME_VER::III_DE_1_72:
			return GAME_ID::GTA3_DE;
		case GAME_VER::VC_DE_1_72:
			return GAME_ID::GTAVC_DE;
		case GAME_VER::SA_DE_1_72:
			return GAME_ID::GTASA_DE;
		}
		
		XB_GTA_LOGE("GetGameId error, Unknown Game.");
		return GAME_ID::MAX_GAME;
	}
	
	inline const char* GetGameName()
	{
		auto id = GetGameId();
		return id == GAME_ID::MAX_GAME ? NULL : game_name[id];
	}
	
	inline const char* GetGameLibName()
	{
		auto id = GetGameId();
		if (id == GAME_ID::MAX_GAME) {
			return NULL;
		}
		else if (id == GAME_ID::GTA3_DE || id == GAME_ID::GTAVC_DE || id == GAME_ID::GTASA_DE) {
			return game_lib[GAME_ID::GTA3_DE];
		}
		else {
			return game_lib[id];
		}
	}

}

#endif // !__XBEATLESS_GTA_ANDROID_GAME_VERSION_HPP__

