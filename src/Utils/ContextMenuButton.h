#pragma once

class ContextMenuButton
{
public:
	ContextMenuButton(const char *_text, const std::function<void()> &func),
		~ContextMenuButton();
	void setPosition(float _x, float _y, float _width);
	void render();
	glm::vec2 position;
	float width;
	float buttonColour[4];
	float buttonHeight;
	void click();
	bool mouseIntersects();

private:
	float textColor[4];
	const char *text;
	std::function<void()> function;


};

