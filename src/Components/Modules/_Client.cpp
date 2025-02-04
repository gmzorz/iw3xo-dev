#include "STDInclude.hpp"

namespace Components
{
	void on_disconnect()
	{ 
		if (Components::active.Mvm)
		{
			const auto& cl_avidemo = Game::Dvar_FindVar("cl_avidemo");

			if (cl_avidemo && cl_avidemo->current.integer)
			{
				Game::Dvar_SetValue(cl_avidemo, 0);
			}

			if (Dvars::cl_avidemo_streams && Dvars::cl_avidemo_streams->current.integer)
			{
				Game::Dvar_SetValue(Dvars::cl_avidemo_streams, 0);
			}
		}

		Game::Globals::mainmenu_fadeDone = false;
		Game::Globals::loaded_MainMenu = false;

		if (Components::active.Gui)
		{
			GET_GGUI.menus[Game::GUI_MENUS::CHANGELOG].menustate = false;
		}
    }

	__declspec(naked) void on_disconnect_stub()
	{
		const static uint32_t rtnPt = 0x4696D5;
		__asm
		{
            // stock op's
            xor     eax, eax;
            cmp     esi, 5;

            pushad;
            call	on_disconnect;
            popad;
			
			jmp		rtnPt;
		}
	}

	// --------

	// actually after time was set
	void on_set_cgame_time()
	{
		if (Components::active.DayNightCycle)
		{
			if (Dvars::r_dayAndNight && Dvars::r_dayAndNight->current.enabled)
			{
				DayNightCycle::set_world_time();
			}
		}
	}

	__declspec(naked) void on_set_cgame_time_stub()
	{
		const static uint32_t CL_SetCGameTime_Func = 0x45C440;
		const static uint32_t rtnPt = 0x46C9FB;
		__asm
		{
			// stock op's
			call	CL_SetCGameTime_Func;

			pushad;
			call	on_set_cgame_time;
			popad;

			jmp		rtnPt;
		}
	}

	_Client::_Client()
	{ 
		// CL_Disconnect, random spot (mvm hooks on first op)
		Utils::Hook(0x4696D0, on_disconnect_stub, HOOK_JUMP).install()->quick();

		// CL_SetCGameTime, called every client frame
		Utils::Hook(0x46C9F6, on_set_cgame_time_stub, HOOK_JUMP).install()->quick();
	}

	_Client::~_Client()
	{ }
}