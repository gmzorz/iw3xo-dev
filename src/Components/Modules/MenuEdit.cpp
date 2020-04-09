#include "STDInclude.hpp"
#include "MenuEdit.hpp"

#define PRINTC(x) Game::Com_PrintMessage(0, x, 0);

namespace Components
{


	//Define static variables
	int MenuEdit::selectedMenuDef = 0;
	int MenuEdit::selectedItemDef = -1;
	bool MenuEdit::leftMouseDown = false;
	int MenuEdit::snapGrid = 0;
	glm::vec2 MenuEdit::oldMousePos;
	glm::vec2 MenuEdit::snapPos;
	MenuEdit::MenuEdit()
	{
		


		//Add commands
		Command::Add("menuedit", [this](Command::Params) {	
		});

		Command::Add("menuList", [this](Command::Params) {
			for (int i = 0; i < Game::_uiContext->menuCount; i++) {
				Game::Com_PrintMessage(0, Utils::VA("Menu: %s \n", Game::_uiContext->Menus[i]->window.name), 0);
			}
		});

		Command::Add("me_open", [this](Command::Params params) {
			if (params.Length() < 2){
				return;
			}
			const char * name = params[1];
			for (int i = 0; i < Game::_uiContext->menuCount; i++) {
				if (strcmp(Game::_uiContext->Menus[i]->window.name, name) == 0) {//might be wrong
					selectedMenuDef = i;
					break;
				}	
			}

			Game::Key_SetCatcher();
			Game::Menus_CloseAll(Game::_uiContext);
			Game::Menus_OpenByName(name, Game::_uiContext);
		});


		//hook when mouse enters itemDef
		Utils::Hook::Nop(0x5545B8, 5);
		Utils::Hook(0x5545B8, MouseEnterItemHook, HOOK_JUMP).install()->quick();

		//hook when mouse moves
		Utils::Hook::Nop(0x554DEA, 5);
		Utils::Hook(0x554DEA, MouseMoveHook, HOOK_JUMP).install()->quick();

		//hook in MenuPaint function
		Utils::Hook::Nop(0x55489C, 5);
		Utils::Hook(0x55489C, MenuPaintHook, HOOK_JUMP).install()->quick();
	

	}


	//dont do hooks if we are not viewing menu we are editing
	_declspec(naked) void MenuEdit::MouseEnterItemHook() {
		const static uint32_t returnPT = 0x5545BD;
		const static uint32_t Item_MouseEnter = 0x54EB30;
		//esi
		_asm {

			call	Item_MouseEnter
			push	esi
			call	MouseEnteredItemDef
			add		esp, 4h
			jmp		returnPT
		}
	}

	//Hook after mouse positions have been updated and mouse drawn
	_declspec(naked) void MenuEdit::MouseMoveHook() {
		const static uint32_t returnPT = 0x554DEF;
		const static uint32_t Menu_GetFocused = 0x554250;
		_asm {
			call	Menu_GetFocused
			call	MouseMove
			jmp		returnPT
		}
	}

	_declspec(naked) void MenuEdit::MenuPaintHook() {
		const static uint32_t returnPT = 0x5548A1;
		const static uint32_t ui_showMenuOnly = 0x558CF0;
		_asm {
			call	ui_showMenuOnly
			call	MenuPaint
			jmp		returnPT
		}
	}

	/*
		Called when mouse enters a itemdef
	*/
	void MenuEdit::MouseEnteredItemDef(Game::itemDef_s *itemDef) {

		if (Game::playerKeys->keys[KEYCATCHER_MOUSE1].down) {

			if (!leftMouseDown) {
				for (int i = 0; i < itemDef->parent->itemCount; i++) {
					if (itemDef->parent->items[i] == itemDef) {
						selectedItemDef = i;
						break;
					}
				}
				leftMouseDown = true;
			}

		}

	}

	/*
		Called when mouse if moved
	*/
	void MenuEdit::MouseMove() {

		glm::vec2 mousePos(Game::_uiContext->cursor.x, Game::_uiContext->cursor.y);
		Game::rectDef_s *rect = &Game::_uiContext->Menus[selectedMenuDef]->items[selectedItemDef]->window.rectClient;



		if (leftMouseDown) {

			if (selectedItemDef != -1) {

				if (snapGrid == 0) {
					float distancex = Game::_uiContext->cursor.x - oldMousePos.x;
					float distancey = Game::_uiContext->cursor.y - oldMousePos.y;
					rect->x += distancex;
					rect->y += distancey;

				}
				else {
					if (mousePos.x - snapPos.x >= snapGrid) {
						float newX = rect->x + snapGrid;
						rect->x = ceil(newX / snapGrid)*snapGrid;
						snapPos.x = mousePos.x;
					}
					if (mousePos.x - snapPos.x <= -snapGrid) {
						float newX = rect->x - snapGrid;
						rect->x = ceil(newX / snapGrid)*snapGrid;
						snapPos.x = mousePos.x;
					}
					if (mousePos.y - snapPos.y >= snapGrid) {
						float newY = rect->y + snapGrid;
						rect->y = ceil(newY / snapGrid)*snapGrid;
						snapPos.y = mousePos.y;
					}
					if (mousePos.y - snapPos.y <= -snapGrid) {
						float newY = rect->y - snapGrid;
						rect->y = ceil(newY / snapGrid)*snapGrid;
						snapPos.y = mousePos.y;
					}
				}
			}

		}

		oldMousePos.x = Game::_uiContext->cursor.x;
		oldMousePos.y = Game::_uiContext->cursor.y;

	}

	//might add menu being painted? Change to draw after menu is painted so its on top
	void MenuEdit::MenuPaint() {


		if (selectedItemDef != -1) {
			float colour[] = { 1,1,0,1 };

			//top left

			//top right
			
		}


		if (!Game::playerKeys->keys[KEYCATCHER_MOUSE1].down) {
			if (leftMouseDown) {

			}

			leftMouseDown = false;
			selectedItemDef = -1;
		}

	}



	Game::itemDef_s* MenuEdit::getCurrectItemDef() {
		return getCurrentMenuDef()->items[selectedItemDef];
	}

	Game::menuDef_t* MenuEdit::getCurrentMenuDef() {
		return Game::_uiContext->Menus[selectedMenuDef];
	}




	MenuEdit::~MenuEdit()
	{
	}

}
