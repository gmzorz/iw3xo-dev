#include "STDInclude.hpp"
#include "MenuEdit.hpp"


namespace Components
{
	MenuEdit::MenuEdit()
	{

		//Add commands
		Command::Add("menuedit", [this](Command::Params) {
			enabled = !enabled;		
		});

		Command::Add("menuList", [this](Command::Params) {
			for (int i = 0; i < uiContext->menuCount; i++) {
				Game::Com_PrintMessage(0, Utils::VA("Menu: %s \n", uiContext->Menus[i]->window.name), 0);
			}
		});


	}


	MenuEdit::~MenuEdit()
	{
	}
}
