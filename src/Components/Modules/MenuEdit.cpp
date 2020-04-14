#include "STDInclude.hpp"
#include "MenuEdit.hpp"

#define PRINTC(x) Game::Com_PrintMessage(0, x, 0);

namespace Components
{


	//Define static variables
	int MenuEdit::selectedMenuDef = 0;
	int MenuEdit::selectedItemDef = -1;
	bool MenuEdit::leftMouseDown = false;
	bool MenuEdit::rightMouseDown = false;
	int MenuEdit::snapGrid = 0;
	glm::vec2 MenuEdit::mousePos;
	glm::vec2 MenuEdit::oldMousePos;
	glm::vec2 MenuEdit::snapPos;
	ContextMenu *MenuEdit::contextMenu = new ContextMenu();
	float MenuEdit::selectColour[4];
	MenuEdit::MenuEdit()
	{
		
		selectColour[0] = 1.0f;
		selectColour[1] = 1.0f;
		selectColour[2] = 1.0f;
		selectColour[3] = 1.0f;

		contextMenu->addButton("Button 1", [this]() {
			if (selectedItemDef != -1) {
				getCurrectItemDef()->window.backColor[0] = 0.0f;
				getCurrectItemDef()->window.backColor[1] = 0.0f;
				getCurrectItemDef()->window.backColor[2] = 1.0f;
				getCurrectItemDef()->window.backColor[3] = 1.0f;
			}
		});

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

			contextMenu->setAlign(getCurrentMenuDef()->window.rectClient.horzAlign, getCurrentMenuDef()->window.rectClient.horzAlign, getCurrentMenuDef()->window.rectClient.x, getCurrentMenuDef()->window.rectClient.y);

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
		Utils::Hook::Nop(0x55494A, 5);
		Utils::Hook(0x55494A, MenuPaintHook, HOOK_JUMP).install()->quick();
	

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

	//Hook at end of menuPaint func
	_declspec(naked) void MenuEdit::MenuPaintHook() {
		const static uint32_t returnPT = 0x55494F;
		const static uint32_t Item_Paint = 0x553C20;
		_asm {
			call	Item_Paint
			call	MenuPaint
			jmp		returnPT
		}
	}

	/*
		Called when mouse enters a itemdef
	*/
	void MenuEdit::MouseEnteredItemDef(Game::itemDef_s *itemDef) {
		if (!leftMouseDown) {
			for (int i = 0; i < itemDef->parent->itemCount; i++) {
				if (itemDef->parent->items[i] == itemDef) {
					selectedItemDef = i;
					break;
				}
			}
		}
	}

	/*
		Called when mouse if moved
	*/
	void MenuEdit::MouseMove() {

		mousePos = glm::vec2(Game::_uiContext->cursor.x, Game::_uiContext->cursor.y);
		Game::rectDef_s *rect = &Game::_uiContext->Menus[selectedMenuDef]->items[selectedItemDef]->window.rectClient;

		if (leftMouseDown) {

			//dont drag if mouse is over context menu
			if (!contextMenu->mouseIntersects()) {
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
		}

		oldMousePos.x = Game::_uiContext->cursor.x;
		oldMousePos.y = Game::_uiContext->cursor.y;
	}

	//might add menu being painted?
	void MenuEdit::MenuPaint() {

		if (!Game::Sys_IsMainThread())
		{
			return;
		}

		if (Game::playerKeys->keys[KEYCATCHER_MOUSE1].down) {
			if (contextMenu->isOpen) {
				if (!contextMenu->mouseIntersects()) {
					contextMenu->close();
				}
				else {
					for (int i = 0; i < contextMenu->buttons->size(); i++) {
						if (contextMenu->buttons->at(i)->mouseIntersects()) {
							drawDebug(10, 50, "hover");
							contextMenu->buttons->at(i)->click();
						}
					}
				}

			}

			leftMouseDown = true;
			//selectedItemDef = -1;
		}

		if (!Game::playerKeys->keys[KEYCATCHER_MOUSE1].down) {
			if (leftMouseDown) {

			}

			leftMouseDown = false;
			//selectedItemDef = -1;
		}

		if (Game::playerKeys->keys[KEYCATCHER_MOUSE2].down) {

			if (!contextMenu->mouseIntersects()) {
				if (contextMenu->isOpen) {
					contextMenu->close();
				}
				contextMenu->open(mousePos.x, mousePos.y);
			}

			rightMouseDown = true;
		}

		if (!Game::playerKeys->keys[KEYCATCHER_MOUSE2].down) {
			rightMouseDown = false;
		}

		//drawItemDefSelect();
		contextMenu->render();


		
	}

	void MenuEdit::drawItemDefSelect() {
		if (selectedItemDef == -1) {
			return;
		}

		float xPos, yPos, width, height;

		//top left
		xPos = getCurrectItemDef()->window.rectClient.x;// -10;
		yPos = getCurrectItemDef()->window.rectClient.y - 10;
		width = getCurrectItemDef()->window.rectClient.w;// +10;
		height = 5.0f;

		
		drawDebug(10, 50, Utils::VA("xPos: %f", xPos));

		//xPos = Components::_UI::ScrPlace_ApplyX(getCurrentMenuDef()->window.rectClient.horzAlign, xPos, 0);

		if (getCurrentMenuDef()->window.rectClient.horzAlign == 0) {
			xPos += (float)(Game::_uiContext->screenWidth - 640) / 2;
		}
		drawDebug(10, 70, Utils::VA("Mousex: %f", Game::_uiContext->cursor.x));


		//adjustPosition(&xPos, &yPos, &width, &height);
		Game::ConDraw_Box(selectColour, xPos, yPos, width, height);
	}

	void MenuEdit::adjustPosition(float *x, float *y, float *w, float *h) {
		*x = Utils::floatToRange(0.0f, 640, 0.0f, (float)Game::_uiContext->screenWidth, *x);
		*y = Utils::floatToRange(0.0f, 480, 0.0f, (float)Game::_uiContext->screenHeight, *y);
		*w = Utils::floatToRange(0.0f, 640, 0.0f, (float)Game::_uiContext->screenWidth, *w);
		*h = Utils::floatToRange(0.0f, 480, 0.0f, (float)Game::_uiContext->screenHeight, *h);
	}

	Game::itemDef_s* MenuEdit::getCurrectItemDef() {
		return getCurrentMenuDef()->items[selectedItemDef];
	}

	Game::menuDef_t* MenuEdit::getCurrentMenuDef() {
		return Game::_uiContext->Menus[selectedMenuDef];
	}

	void MenuEdit::drawDebug(float x, float y, const char *s) {
		void* fontHandle = Game::R_RegisterFont(FONT_NORMAL, sizeof(FONT_NORMAL));
		float colorBackground[4] = { 1.0f, 0.2f, 0.2f, 1.0f };
		Game::R_AddCmdDrawTextASM(s, 0x7FFFFFFF, fontHandle, x, y, 1.4, 1.4, 0, colorBackground, 0);
	}




	MenuEdit::~MenuEdit()
	{
		delete contextMenu;
	}

}
