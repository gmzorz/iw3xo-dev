#pragma once
#include "Utils/ContextMenuButton.h"
#define STANDARD_WIDTH 640.0f
#define STANDARD_HEIGHT 480.0f

class ContextMenu
{
public:
	ContextMenu();
	~ContextMenu();

	bool isOpen = false;

	void open(float x, float y);
	void close();
	void render();
	bool mouseIntersects();
	void setAlign(int _alignH, int _alignV, float x, float y);
	void addButton(const char *text, const std::function<void()> &func);
	std::vector<ContextMenuButton*> *buttons;

private:
	int alignH;
	int alignV;
	glm::vec2 menuDefPosition;
	glm::vec2 position;
	glm::vec2 drawPosition;
	float width = 0;
	float height = 0;
	float drawWidth = 0;
	float drawHeight = 0;
	
};


