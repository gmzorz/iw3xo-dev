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

		static void MouseEnterItemHook();
		static void MouseMoveHook();
		static void MenuPaintHook();
		static void MenuPaint();
		static void MouseEnteredItemDef(Game::itemDef_s *itemDef);
		static void MouseMove();


		static Game::itemDef_s* getCurrectItemDef();
		static Game::menuDef_t* getCurrentMenuDef();


	};
}

