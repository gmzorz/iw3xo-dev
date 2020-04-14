#pragma once
#include "Utils/ContextMenu.hpp"

namespace Components
{
		
	class MenuEdit : public Component
	{
	public:
		MenuEdit();
		~MenuEdit();
		const char* getName() override { return "Menu Editor"; };	
		static void adjustPosition(float *x, float *y, float *w, float *h);
		

	private:
		static int selectedMenuDef;
		static int selectedItemDef;
		static bool leftMouseDown;
		static bool rightMouseDown;
		static int snapGrid;
		static glm::vec2 mousePos;
		static glm::vec2 oldMousePos;
		static glm::vec2 snapPos;
		static ContextMenu *contextMenu;
		static float selectColour[4];

		static void MouseEnterItemHook();
		static void MouseMoveHook();
		static void MenuPaintHook();
		static void MenuPaint();
		static void MouseEnteredItemDef(Game::itemDef_s *itemDef);
		static void MouseMove();
		static void drawItemDefSelect();


		static Game::itemDef_s* getCurrectItemDef();
		static Game::menuDef_t* getCurrentMenuDef();

		static void MenuEdit::drawDebug(float x, float y, const char *s);


	};
}

