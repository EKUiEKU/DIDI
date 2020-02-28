#include "ESP.h"

#include <iostream>
#include <Windows.h>
#include <tlhelp32.h>
#include <dwmapi.h>
#include <string_view>
#include <sstream>
#include <vector>

#include "../imgui/imgui.h"
#include "../imgui/imgui_internal.h"
#include <d3d9.h>

#include "../Config.h"

#include "../Interfaces.h"

#include "../SDK/Entity.h"

#include "../Math.h"


class BoundingBox {
public:
	ImVec2 min, max;
	ImVec2 vertices[8];
};

Entity* localPlayer;
D3DMATRIX viewMatrix;
Entity* observeTarget;
std::pair<int, int> screenSize;


struct PlayerMessage {
	BoundingBox bbox;
	bool isVisiable;
	bool isEnemy;
	Vector absOrigin;
	int healthValue;
};

void draw(PlayerMessage msg, ImDrawList* drawList);
boolean getBoundingBox(Entity* entity, BoundingBox& bbox);

std::vector<PlayerMessage> playerMessages;

void ESP::collectMessages() noexcept
{
	playerMessages.clear();

	int localPlayerID = interfaces->engine->getLocalPlayer();
	localPlayer = interfaces->entityList->getEntity(localPlayerID);

	if (localPlayer)
	{
		observeTarget = localPlayer->getObserverTarget();

		viewMatrix = interfaces->engine->worldToScreenMatrix();
		screenSize = interfaces->engine->getScreenSize();

		for (size_t i = 1; i < interfaces->engine->getMaxClients(); i++)
		{
			auto entity = interfaces->entityList->getEntity(i);

			if (!entity || entity == localPlayer || observeTarget == localPlayer
				|| !entity->isAlive() || entity->isDormant())
				continue;

			PlayerMessage msg;

			msg.absOrigin = entity->getAbsOrigin();

			if (fabs(msg.absOrigin.x) < 1e-3 || fabs(msg.absOrigin.y) < 1e-3 || fabs(msg.absOrigin.z) < 1e-3)
				continue;

			msg.isVisiable = localPlayer->visibleTo(entity);
			msg.isEnemy = memory->isOtherEnemy(entity, localPlayer);
			msg.healthValue = entity->health();
			
			if(getBoundingBox(entity, msg.bbox))
				playerMessages.push_back(msg);
		}
	}
}

void ESP::render(ImDrawList* drawList) noexcept
{
	if (config->espEnable)
		for (auto msg : playerMessages)
			draw(msg, drawList);
}



static bool worldToScreen(const Vector& in, ImVec2& out) noexcept
{
	const auto& matrix = viewMatrix;

	float w = matrix._41 * in.x + matrix._42 * in.y + matrix._43 * in.z + matrix._44;

	if (w > 0.001f) {
		const auto [width, height] = screenSize;
		out.x = width / 2 * (1 + (matrix._11 * in.x + matrix._12 * in.y + matrix._13 * in.z + matrix._14) / w);
		out.y = height / 2 * (1 - (matrix._21 * in.x + matrix._22 * in.y + matrix._23 * in.z + matrix._24) / w);
		return true;
	}
	return false;

}

boolean getBoundingBox(Entity* entity, BoundingBox& bbox) {

	bbox.min.x = static_cast<float>(screenSize.first * 2);
	bbox.min.y = static_cast<float>(screenSize.second * 2);
	bbox.max.x = -static_cast<float>(screenSize.first * 2);
	bbox.max.y = -static_cast<float>(screenSize.second * 2);


	auto mCollideable = entity->getCollideable();

	Vector mins = mCollideable->obbMins();
	Vector maxs = mCollideable->obbMaxs();

	Vector points[8];

	for (int i = 0; i < 8; ++i) {
		const Vector point{ i & 1 ? maxs.x : mins.x,
							i & 2 ? maxs.y : mins.y,
							i & 4 ? maxs.z : mins.z };


		if (!worldToScreen(point.transform(entity->coordinateFrame()), bbox.vertices[i]))
			return false;


		if (bbox.min.x > bbox.vertices[i].x)
			bbox.min.x = bbox.vertices[i].x;

		if (bbox.min.y > bbox.vertices[i].y)
			bbox.min.y = bbox.vertices[i].y;

		if (bbox.max.x < bbox.vertices[i].x)
			bbox.max.x = bbox.vertices[i].x;

		if (bbox.max.y < bbox.vertices[i].y)
			bbox.max.y = bbox.vertices[i].y;

	}

	return true;
}

void draw(PlayerMessage msg, ImDrawList* drawList) {
	BoundingBox bbox = msg.bbox;

	bool isVisiable = msg.isVisiable;
	bool isEnemy = msg.isEnemy;

	if (config->boxEnable && isEnemy ? config->BoxEnemy->enable : config->BoxTeammate->enable)
	{
		//画立体体

		DWORD color = config->getEspColor(BOX,isEnemy,isVisiable);

		for (int i = 0; i < 8; ++i) {
			for (int j = 1; j <= 4; j <<= 1) {
				if (!(i & j))
					drawList->AddLine({ bbox.vertices[i].x, bbox.vertices[i].y }, { bbox.vertices[i + j].x, bbox.vertices[i + j].y }, color, 1.);
			}
		}
	}

	
	
	//画血条
	float distance = getDistance3D(localPlayer->getAbsOrigin(), msg.absOrigin);
	int healthValue = msg.healthValue;

	float healthBarWidth = bbox.max.x - bbox.min.x;
	float healthBarHeight = healthBarWidth / 20.f;
	float healtnVerOffset = 20;

	float outLineWidth = 1.f;
	if (config->healthBarEnable && isEnemy ? config->HealthBarEnemy->enable : config->HealthBarTeammate->enable)
	{
		if (distance > 150)
		{
			DWORD color = config->getEspColor(HEALTH_BAR, isEnemy, isVisiable);

			drawList->AddRectFilled({ bbox.min.x - outLineWidth, bbox.min.y - healtnVerOffset - outLineWidth }, { bbox.min.x + healthBarWidth + outLineWidth, bbox.min.y - healtnVerOffset + healthBarHeight + outLineWidth }, D3DCOLOR_ARGB(0xff, 0x00, 0x00, 0x00));
			drawList->AddRectFilled({ bbox.min.x, bbox.min.y - healtnVerOffset }, { bbox.min.x + healthBarWidth , bbox.min.y - healtnVerOffset + healthBarHeight }, D3DCOLOR_ARGB(0xff, 0x77, 0x77, 0x77));

			drawList->AddRectFilled({ bbox.min.x , bbox.min.y - healtnVerOffset }, { bbox.min.x + healthBarWidth * (healthValue / 100.f), bbox.min.y - healtnVerOffset + healthBarHeight }, color);
		}
	}

	std::string text = std::to_string(healthValue);
	const auto fontSize = healthBarHeight * 2;

	if (config->healthValueEnable && isEnemy ? config->HealthValueEnemy->enable : config->HealthValueTeammate->enable)
	{
		if (distance > 150) {
			DWORD color = config->getEspColor(HEALTH_VALUE, isEnemy, isVisiable);

			drawList->AddText(NULL, fontSize, { bbox.min.x + healthBarWidth + 5, bbox.min.y - healtnVerOffset - fontSize / 4 }, color, std::to_string(healthValue).c_str());
		}
	}


	if (config->lineEnable && isEnemy ? config->LineEnemy->enable : config->LineTeammate->enable)
	{
		//画线条

		if (distance > 100) {
			ImVec2 pointBound, pointHeadAbove;

			pointBound = { screenSize.first / 2.f, 0 };

			Vector pointHeadAbovePosition = msg.absOrigin;
			pointHeadAbovePosition.z += 90;

			ImVec2 screen2D;
			if (!worldToScreen(pointHeadAbovePosition, screen2D))
				return;

			pointHeadAbove = { screen2D.x, screen2D.y };

			DWORD color = config->getEspColor(LINE, isEnemy, isVisiable);

			
			drawList->AddLine(pointBound, pointHeadAbove, color, 1.);
		
		}
	}
	
	
	/*
	for (size_t i = 0; i < 85; i++)
	{
		auto p = entity->getBonePosition(i);
		if (p) {
			ImVec2 s;
			worldToScreen(p, s);

			drawList->AddText(s, D3DCOLOR_ARGB(255, 255, 255, 255), std::to_string(i).c_str());
		}
	}
	*/

}