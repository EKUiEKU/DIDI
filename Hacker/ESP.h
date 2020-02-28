#pragma once

struct ImDrawList;
class EntityList;

namespace ESP {
	void collectMessages() noexcept;
	void render(ImDrawList* drawList) noexcept;
}
