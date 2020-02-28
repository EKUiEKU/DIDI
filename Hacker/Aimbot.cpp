#include "Aimbot.h"
#include <d3d9.h>
#include "../Interfaces.h"
#include "../SDK/Entity.h"
#include "../Memory.h"

#include <intrin.h>

#include "../imgui/imgui.h"
#include "../imgui/imgui_internal.h"

#include "../Config.h"

#include < vector> 
#include "../Math.h"

Vector calculateRelativeAngle(const Vector& source, const Vector& destination, const Vector& viewAngles) noexcept
{
	Vector delta = destination - source;
	Vector angles{ radiansToDegrees(atan2f(-delta.z, std::hypotf(delta.x, delta.y))) - viewAngles.x,
				   radiansToDegrees(atan2f(delta.y, delta.x)) - viewAngles.y };
	angles.Normalize();
	return angles;
}

Vector currentViewAngle;

static std::vector<int> bonesIndex;

Vector findPlayer() noexcept {

	Vector bestAimAngle{0,0,0};
	float bestAngle = config->aimbotConfig->aimAngleRange;

	bonesIndex.clear();

	switch (config->aimbotConfig->aimPlace)
	{
	case NEAST:
		//bonesIndex.push_back(2);
		bonesIndex.push_back(8);
		//bonesIndex.push_back(9);

		bonesIndex.push_back(6);
		bonesIndex.push_back(10);
		bonesIndex.push_back(38);

		bonesIndex.push_back(0);
		bonesIndex.push_back(3);
		bonesIndex.push_back(4);
		bonesIndex.push_back(5);
		bonesIndex.push_back(66);
		bonesIndex.push_back(73);

		bonesIndex.push_back(67);
		bonesIndex.push_back(68);
		bonesIndex.push_back(69);
		bonesIndex.push_back(70);
		bonesIndex.push_back(71);
		bonesIndex.push_back(72);

		bonesIndex.push_back(73);
		bonesIndex.push_back(74);
		bonesIndex.push_back(75);
		bonesIndex.push_back(76);
		bonesIndex.push_back(77);
		bonesIndex.push_back(78);
		break;
	case HEAD:
		//bonesIndex.push_back(2);
		bonesIndex.push_back(8);
		//bonesIndex.push_back(9);
		break;
	case CHEAST:
		bonesIndex.push_back(6);
		bonesIndex.push_back(10);
		bonesIndex.push_back(38);
		break;
	case CROTH:
		bonesIndex.push_back(0);
		bonesIndex.push_back(3);
		bonesIndex.push_back(4);
		bonesIndex.push_back(5);
		bonesIndex.push_back(66);
		bonesIndex.push_back(73);
		break;
	}

	auto localPlayer = interfaces->entityList->getEntity(interfaces->engine->getLocalPlayer());


	if (!localPlayer || !localPlayer->isAlive())
		return bestAimAngle;

	auto localOrigin = localPlayer->getAbsOrigin();

	if (fabs(localOrigin.x) < 1e-3 || fabs(localOrigin.y) < 1e-3 || fabs(localOrigin.z) < 1e-3)
		return bestAimAngle;

	auto punchAngle = localPlayer->aimPunchAngle();

	punchAngle.x *= config->aimbotConfig->antPunshX;
	punchAngle.y *= config->aimbotConfig->antPunshY;

	interfaces->engine->getViewAngles(currentViewAngle);

	currentViewAngle += config->aimbotConfig->antPunch ? punchAngle : Vector{};

	auto localHeadPos = localPlayer->getBonePosition(8);

	for (int i = 1; i < interfaces->engine->getMaxClients(); i++)
	{
		auto next = interfaces->entityList->getEntity(i);

		if (!next || !next->isAlive() || next == localPlayer || next->isDormant())
			continue;

		if (next->gunGameImmunity())
			continue;

		if (!config->aimbotConfig->aimTeammate && !memory->isOtherEnemy(localPlayer, next))
			continue;

		auto nextOrigin = next->getAbsOrigin();

		if (fabs(nextOrigin.x) < 1e-3 || fabs(nextOrigin.y) < 1e-3 || fabs(nextOrigin.z) < 1e-3)
			continue;

		auto distance = getDistance3D(localOrigin, nextOrigin) / 100.f;

		if (distance < 1.f)
			distance = 1.f;

		for (auto boneIndex : bonesIndex) {
			auto destPos = next->getBonePosition(boneIndex);
			if (destPos != 0 && localPlayer->visible(destPos))
			{

				if (!config->aimbotConfig->ignoreSmoke && memory->lineGoesThroughSmoke(localHeadPos, destPos, 1))
					continue;


				Vector aimAngle = calculateRelativeAngle(localHeadPos, destPos, currentViewAngle);

				//º∆À„FOV
				//auto delta = aimAngle - currentViewAngle;
				auto fov = std::hypotf(aimAngle.x, aimAngle.y);

				if (fov < bestAngle / distance)
				{
					bestAngle = fov;

					if (config->aimbotConfig->antPunch)
						aimAngle = aimAngle * (1.0f / config->aimbotConfig->smooth) - punchAngle;
					else
						aimAngle = aimAngle * (1.0f / config->aimbotConfig->smooth);

					bestAimAngle = aimAngle;
				}
			}
		}
	}
	return bestAimAngle == 0 ? Vector{0,0,0} : bestAimAngle;
}

void Aimbot::run(ImDrawList* drawList) noexcept
{

	auto localPlayer = interfaces->entityList->getEntity(interfaces->engine->getLocalPlayer());

	if (config->aimbotConfig->enable)
	{
		if (localPlayer && localPlayer->isAlive()) {
			auto bestAngle = findPlayer();

			if (bestAngle != 0)
			{
				auto screenSize = interfaces->engine->getScreenSize();
				DWORD colorState = bestAngle != 0 ? D3DCOLOR_ARGB(255, 0, 255, 0) : D3DCOLOR_ARGB(255, 0, 0, 255);
				drawList->AddCircleFilled({ screenSize.first / 2.f, screenSize.second / 2.f }, 2.5f, colorState);


				if (!config->aimbotConfig->ignoreFlash && localPlayer->flashDuration())
					return;

				int aimType = config->aimbotConfig->aimType;
				if (aimType == MAGNETISM || GetAsyncKeyState(VK_LBUTTON) && aimType == LBUTTON || GetAsyncKeyState(VK_LMENU) && aimType == ALT)
					interfaces->engine->setViewAngles(currentViewAngle + bestAngle);
			}
		}
	}
}

