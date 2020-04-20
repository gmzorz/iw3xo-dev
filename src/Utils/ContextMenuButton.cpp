#include "STDInclude.hpp"
#include "ContextMenuButton.h"

float ContextMenuButton::buttonHeight = 20.0f;
ContextMenuButton::ContextMenuButton(const char *_text, const std::function<void()> &func)
	: buttonColour{ 0.29f,0.29f,0.29f,1.0f }, textColor{ 1.0f,1.0f,1.0f,1.0f }
{
	text = _text;
	function = func;

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

	float fontSize;
	glm::vec2 drawPosition;
	float drawWidth, drawHeight;
	float windowWidth = Game::cgs->refdef.width;
	drawPosition.x = Utils::floatToRange(0.0f, STANDARD_WIDTH, 0.0f, (float)Game::_uiContext->screenWidth, position.x);
	drawPosition.y = Utils::floatToRange(0.0f, STANDARD_HEIGHT, 0.0f, (float)Game::_uiContext->screenHeight, position.y);
	drawWidth = Utils::floatToRange(0.0f, STANDARD_WIDTH, 0.0f, (float)Game::_uiContext->screenWidth, width);
	drawHeight = Utils::floatToRange(0.0f, STANDARD_HEIGHT, 0.0f, (float)Game::_uiContext->screenHeight, buttonHeight);

	void *fontHandle = Game::R_RegisterFont(FONT_CONSOLE, sizeof(FONT_CONSOLE));

	if (windowWidth <= 800)
		fontSize = 0.8f;
	else if (windowWidth <= 1280)
		fontSize = 1.2f;
	else
		fontSize = 1.6f;

	Game::ConDraw_Box(buttonColour, drawPosition.x, drawPosition.y, drawWidth, drawHeight);
	
	Game::R_AddCmdDrawTextASM(text, 0x7FFFFFFF, fontHandle, drawPosition.x + 10, drawPosition.y + drawHeight, fontSize, fontSize, 0, textColor, 0);
}

void ContextMenuButton::click() {
	function();
}

