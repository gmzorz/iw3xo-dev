#pragma once

class ContextMenu
{
public:
	ContextMenu();
	~ContextMenu();

	bool isOpen = false;

	void open(float x, float y);
	void close();
	void render();
	void setAlign(int _alignH, int _alignV, float x, float y);

private:
	int alignH;
	int alignV;
	glm::vec2 menuDefPosition;
	glm::vec2 position;
	float width;
	float height;
	float backgroundColour[4];
};


