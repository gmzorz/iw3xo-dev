#include "STDInclude.hpp"
#include "ContextMenu.hpp"


ContextMenu::ContextMenu()
{
	alignH = 0;
	alignV = 0;

	backgroundColour[0] = 1.0f;
	backgroundColour[1] = 0.0f;
	backgroundColour[2] = 0.0f;
	backgroundColour[3] = 1.0f;

	width = 100.0f;
	height = 300.0f;
}


ContextMenu::~ContextMenu()
{
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

void ContextMenu::render() {

	if (isOpen) {
		
		float xPos = Components::_UI::ScrPlace_ApplyX( alignH, position.x, menuDefPosition.x);

		Game::ConDraw_Box(backgroundColour, xPos, position.y, width, height);
	}
	
}
