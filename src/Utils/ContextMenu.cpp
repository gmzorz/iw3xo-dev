#include "STDInclude.hpp"
#include "ContextMenu.hpp"


ContextMenu::ContextMenu()
{
	alignH = 0;
	alignV = 0;

	width = 100.0f;
	height = 0.0f;

	buttons = new std::vector<ContextMenuButton*>;
}


ContextMenu::~ContextMenu()
{
	for (int i = 0; i < buttons->size(); i++) {
		delete (*buttons)[i];
	}
	delete buttons;
}

void ContextMenu::setAlign(int _alignH, int _alignV, float x, float y) {
	alignH = _alignH;
	alignV = _alignV;

	menuDefPosition.x = x;
	menuDefPosition.y = y;
}

void ContextMenu::open(float x, float y) {
	position.x = x;
	position.y = y;
	isOpen = true;
}

void ContextMenu::close() {
	isOpen = false;
}

bool ContextMenu::mouseIntersects() {
	if (isOpen) {
		if (Game::_uiContext->cursor.x >= position.x && Game::_uiContext->cursor.x <= position.x + width && Game::_uiContext->cursor.y >= position.y && Game::_uiContext->cursor.y <= position.y + height) {
			return true;
		}
	}
	return false;
}

void ContextMenu::render() {

	if (isOpen) {
		
		glm::vec2 pos = position;
		
		if (pos.x + width > STANDARD_WIDTH) {
			pos.x = STANDARD_WIDTH - width;
		}
		if (pos.y + height > STANDARD_HEIGHT) {
			pos.y = STANDARD_HEIGHT - height;
		}


		drawPosition.x = Utils::floatToRange(0.0f, STANDARD_WIDTH, 0.0f, (float)Game::_uiContext->screenWidth, pos.x);
		drawPosition.y = Utils::floatToRange(0.0f, STANDARD_HEIGHT, 0.0f, (float)Game::_uiContext->screenHeight, pos.y);
		drawWidth = Utils::floatToRange(0.0f, STANDARD_WIDTH, 0.0f, (float)Game::_uiContext->screenWidth, width);
		drawHeight = Utils::floatToRange(0.0f, STANDARD_HEIGHT, 0.0f, (float)Game::_uiContext->screenHeight, height);

		//Game::ConDraw_Box(backgroundColour, drawPosition.x, drawPosition.y, drawWidth, drawHeight);
		

		for (int i = 0; i < buttons->size(); i++) {
			(*buttons)[i]->setPosition(pos.x, pos.y + (i*ContextMenuButton::buttonHeight), width);
			(*buttons)[i]->render();
		}
	}
	
}

void ContextMenu::addButton(const char *text, const std::function<void()> &func) {
	height += ContextMenuButton::buttonHeight;
	buttons->push_back(new ContextMenuButton(text, func));
}
