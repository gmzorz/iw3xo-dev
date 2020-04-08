#pragma once


namespace Components
{
	class MenuEdit : public Component
	{
	public:
		MenuEdit();
		~MenuEdit();
		const char* getName() override { return "Menu Editor"; };
		

	private:
		bool enabled = false;
		Game::UiContext* uiContext = reinterpret_cast<Game::UiContext*>(0xCAEE200);
	};
}

