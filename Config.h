#pragma once
#include <iostream>
#include <d3d9.h>



class ESPConfig {
public:

	bool enable;

	void change(DWORD color, float target[4]) {
		int alpha = (color & 0xff000000) >> 24;
		int red = (color & 0x00ff0000) >> 16;
		int green = (color & 0x0000ff00) >> 8;
		int blue = (color & 0x000000ff);

		target[0] = red / 255.f;
		target[1] = green / 255.f;
		target[2] = blue / 255.f;
		target[3] = alpha / 255.f;
	}


	ESPConfig(bool enable, DWORD visiable,DWORD invisiable) {
		this->enable = true;

		this->enable = enable;
		change(visiable, visiableColor);
		change(invisiable, invisiableColor);
	}



	float visiableColor[4] = {0.f, 1.f,0.f,1.f};
	float invisiableColor[4] = { 0.f, 1.f,0.f,1.f };

	DWORD getVisiableColor() {
		return D3DCOLOR_ARGB((int)(visiableColor[3] * 255), (int)(visiableColor[2] * 255), (int)(visiableColor[1] * 255), (int)(visiableColor[0] * 255));
	}


	DWORD getInvisiableColor() {
		return D3DCOLOR_ARGB((int)(invisiableColor[3] * 255), (int)(invisiableColor[2] * 255), (int)(invisiableColor[1] * 255), (int)(invisiableColor[0] * 255));
	}

};

enum ESP_TYPE { BOX, HEALTH_BAR, HEALTH_VALUE, LINE }; 
enum AIMBOT_PLACE { NEAST ,HEAD,CHEAST ,CROTH};
enum AIMBOT_TYPE { LBUTTON, MAGNETISM, ALT };

class AimbotConfig {
public:
 	bool enable = true;
	int aimType = MAGNETISM;
	int aimPlace = NEAST;

	bool  antPunch = true;
	float antPunshX = 1.7f;
	float antPunshY = 1.7f;

	float smooth = 5.f;

	//float accelerateX;
	//float accelerateY;

	float aimAngleRange = 5.f; 

	int style = 0;

	bool ignoreSmoke = false;
	bool ignoreFlash = false;

	bool aimTeammate = false;
	
};

class Config {
public:
	bool espEnable = false;

	bool boxEnable = true;
	ESPConfig* BoxTeammate = new ESPConfig(false, D3DCOLOR_ARGB(255, 102, 205, 170), D3DCOLOR_ARGB(255, 60, 179, 113));
	ESPConfig* BoxEnemy = new ESPConfig(true, D3DCOLOR_ARGB(255, 0, 255, 0), D3DCOLOR_ARGB(255, 238, 0, 0));

	bool healthBarEnable = true;
	ESPConfig* HealthBarTeammate = new ESPConfig(false, D3DCOLOR_ARGB(255, 0, 255, 0), D3DCOLOR_ARGB(255, 0, 200, 0));
	ESPConfig* HealthBarEnemy = new ESPConfig(true, D3DCOLOR_ARGB(255, 0, 255, 0), D3DCOLOR_ARGB(255, 0, 200, 0));

	bool healthValueEnable = false;
	ESPConfig* HealthValueTeammate = new ESPConfig(false, D3DCOLOR_ARGB(255, 99, 184, 255), D3DCOLOR_ARGB(255, 99, 184, 255));
	ESPConfig* HealthValueEnemy = new ESPConfig(false, D3DCOLOR_ARGB(255, 255, 0, 0), D3DCOLOR_ARGB(255, 255, 255, 113));

	bool lineEnable = true;
	ESPConfig* LineTeammate = new ESPConfig(false, D3DCOLOR_ARGB(255, 102, 205, 170), D3DCOLOR_ARGB(255, 60, 179, 113));
	ESPConfig* LineEnemy = new ESPConfig(true, D3DCOLOR_ARGB(255, 102, 205, 0), D3DCOLOR_ARGB(255, 154, 205, 50));

	DWORD getEspColor(ESP_TYPE type, bool isEnemy, bool isVisiable) {
		ESPConfig* temp;

		switch (type)
		{
		case BOX:
			temp = isEnemy ? BoxEnemy : BoxTeammate;
			break;
		case HEALTH_BAR:
			temp = isEnemy ? HealthBarEnemy : HealthBarTeammate;
			break;
		case HEALTH_VALUE:
			temp = isEnemy ? HealthValueEnemy : HealthValueTeammate;
			break;
		case LINE:
			temp = isEnemy ? LineEnemy : LineTeammate;
			break;
		default:
			return D3DCOLOR_ARGB(0, 0, 0, 0);
			break;
		}

		return isVisiable ? temp->getVisiableColor() : temp->getInvisiableColor();
	}


	//aimbot config
	AimbotConfig* aimbotConfig = new AimbotConfig();
};

inline std::unique_ptr<Config> config;
