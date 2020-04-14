#include "STDInclude.hpp"
#include "ContextMenuButton.h"


ContextMenuButton::ContextMenuButton(const char *_text, const std::function<void()> &func)
	: buttonColour{ 1.0f,0.0f,0.0f,1.0f }, textColor{ 0.0f,1.0f,0.0f,1.0f }
{
	text = _text;
	function = func;
	buttonHeight = 40.0f;

}


ContextMenuButton::~ContextMenuButton()
{
}

void ContextMenuButton::setPosition(float _x, float _y, float _width) {
	position.x = _x;
	position.y = _y;
	width = _width;
}

bool ContextMenuButton::mouseIntersects() {
	if (Game::_uiContext->cursor.x >= position.x && Game::_uiContext->cursor.x <= position.x + width && Game::_uiContext->cursor.y >= position.y && Game::_uiContext->cursor.y <= position.y + buttonHeight) {
		return true;
	}
	return false;
}

void ContextMenuButton::render() {

	glm::vec2 drawPosition;
	float drawWidth, drawHeight;
	drawPosition.x = Utils::floatToRange(0.0f, STANDARD_WIDTH, 0.0f, (float)Game::_uiContext->screenWidth, position.x);
	drawPosition.y = Utils::floatToRange(0.0f, STANDARD_HEIGHT, 0.0f, (float)Game::_uiContext->screenHeight, position.y);
	drawWidth = Utils::floatToRange(0.0f, STANDARD_WIDTH, 0.0f, (float)Game::_uiContext->screenWidth, width);
	drawHeight = Utils::floatToRange(0.0f, STANDARD_HEIGHT, 0.0f, (float)Game::_uiContext->screenHeight, buttonHeight);

	void *fontHandle = Game::R_RegisterFont(FONT_NORMAL, sizeof(FONT_NORMAL));
	Game::ConDraw_Box(buttonColour, drawPosition.x, drawPosition.y, drawWidth, drawHeight);
	Game::R_AddCmdDrawTextASM(text, 0x7FFFFFFF, fontHandle, drawPosition.x, drawPosition.y+ (drawHeight /2), 1.6f, 1.6f, 0, textColor, 0);
}

void ContextMenuButton::click() {
	function();
}

