#include "STDInclude.hpp"

namespace Components
{
	void RB_GaussianFilterImage(float radius, Game::GfxRenderTargetId srcRenderTargetId, Game::GfxRenderTargetId dstRenderTargetId, float _dstWidth = 0.0, float _dstHeight = 0.0)
	{
		int srcWidth, srcHeight, dstWidth, dstHeight;
		float radiusX, radiusY;

		radiusY = *Game::wnd_SceneHeight * radius / 480.0f; // sceneHeight
		radiusX = *Game::wnd_SceneAspect * radiusY; // sceneHeight * AspectRatio(1.7778) / sceneWidth

		srcWidth = Game::RenderTargetWidth[5 * srcRenderTargetId];
		srcHeight = Game::RenderTargetHeight[5 * srcRenderTargetId];

		dstWidth = (std::int32_t)(Game::RenderTargetWidth[5 * dstRenderTargetId] * 0.25);
		dstHeight = (std::int32_t)(Game::RenderTargetHeight[5 * dstRenderTargetId] * 0.25);

		Game::GfxImageFilter filter;
		filter.sourceImage = (Game::GfxImage *)Game::RenderTargetArray[5 * srcRenderTargetId];
		filter.finalTarget = dstRenderTargetId;

		// generates 1 pass if src is 2x bigger then dest
		filter.passCount = Game::RB_GenerateGaussianFilterChain( radiusX, radiusY, srcWidth, srcHeight, dstWidth, dstHeight, filter.passes );

		if (filter.passCount) 
		{
			Game::RB_FilterImage(&filter);
		}
	}

	void DrawToRendertarget(Game::GfxRenderTargetId Rendertarget, const char *materialName, float x, float y, float width, float height)
	{
		Game::R_Set2D();
		Game::R_SetRenderTarget(Rendertarget);

		auto material = Game::Material_RegisterHandle(materialName, 3);

		if (material) 
		{
			Game::RB_DrawStretchPic(material, x, y, width, height, 0.0, 0.0, 1.0, 1.0);
		}

		Game::RB_EndTessSurface();
	}

	void RB_DrawCustomShaders(const char *shader)
	{
		if (!Game::gfxCmdBufSourceState)
		{
			return;
		}

		if (Game::tess->indexCount)
		{
			Game::RB_EndTessSurface();
		}

		float _SSAO_DBG_MATSCALE = 0.33f;

		float fullscreenX = Game::scrPlace->realViewableMax[0];
		float fullscreenY = Game::scrPlace->realViewableMax[1];
		
		// POSTFX_SSAO || POSTFX_SSAO_NORMAL
		if (Dvars::xo_shaderoverlay->current.integer == Game::XO_SHADEROVERLAY::Z_SHADER_SSAO || Dvars::xo_ssao_debugnormal->current.enabled)
		{
			// SSAO DVARS WITHIN SHADER CONSTANT FILTERTAP

			// FilterTap 0
			Game::gfxCmdBufSourceState->input.consts[Game::ShaderCodeConstants::CONST_SRC_CODE_FILTER_TAP_0][0] = Dvars::xo_ssao_noisescale->current.value;
			Game::gfxCmdBufSourceState->input.consts[Game::ShaderCodeConstants::CONST_SRC_CODE_FILTER_TAP_0][1] = Dvars::xo_ssao_quality->current.value;
			Game::gfxCmdBufSourceState->input.consts[Game::ShaderCodeConstants::CONST_SRC_CODE_FILTER_TAP_0][2] = fullscreenX; // DONT USE FilterTap[0][2] for dvars -> Used as TextureSize.x
			Game::gfxCmdBufSourceState->input.consts[Game::ShaderCodeConstants::CONST_SRC_CODE_FILTER_TAP_0][3] = fullscreenY; // DONT USE FilterTap[0][3] for dvars -> Used as TextureSize.y

			// FilterTap 1
			Game::gfxCmdBufSourceState->input.consts[Game::ShaderCodeConstants::CONST_SRC_CODE_FILTER_TAP_1][0] = Dvars::xo_ssao_radius->current.value;
			Game::gfxCmdBufSourceState->input.consts[Game::ShaderCodeConstants::CONST_SRC_CODE_FILTER_TAP_1][2] = Game::Dvar_FindVar("cg_fov")->current.value / Game::Dvar_FindVar("cg_fovScale")->current.value;

			// FilterTap 2
			Game::gfxCmdBufSourceState->input.consts[Game::ShaderCodeConstants::CONST_SRC_CODE_FILTER_TAP_2][0] = Dvars::xo_ssao_contrast->current.value;
			Game::gfxCmdBufSourceState->input.consts[Game::ShaderCodeConstants::CONST_SRC_CODE_FILTER_TAP_2][1] = Dvars::xo_ssao_attenuation->current.value;
			Game::gfxCmdBufSourceState->input.consts[Game::ShaderCodeConstants::CONST_SRC_CODE_FILTER_TAP_2][2] = Dvars::xo_ssao_angleBias->current.value;

			// Camera Angles / Axis
			Dvars::xo_camDir0->current.value = Game::gfxCmdBufSourceState->input.data->viewParms->axis[0][0];
			Dvars::xo_camDir1->current.value = Game::gfxCmdBufSourceState->input.data->viewParms->axis[0][1];
			Dvars::xo_camDir2->current.value = Game::gfxCmdBufSourceState->input.data->viewParms->axis[0][2];

			// PASS 1 :: CREATE DEPTH
			DrawToRendertarget(Game::GfxRenderTargetId::R_RENDERTARGET_FLOAT_Z, "z_shader_ssao_depth", 0.0f, 0.0f, fullscreenX, fullscreenY);

			// DEBUG :: Fullscreen Normals
			if (Dvars::xo_ssao_debugnormal->current.enabled)
			{
				DrawToRendertarget(Game::GfxRenderTargetId::R_RENDERTARGET_FRAME_BUFFER, "z_shader_ssao_normal", 0.0f, 0.0f, fullscreenX, fullscreenY);
				return;

				// only here if we want to look what values are within BufState
				// const auto GfxCmdBufState = reinterpret_cast<Game::GfxCmdBufState*>(0xD5404F0); 
			}

			// Draw SSAO if not using fullscreen Normals
			else
			{
				// PASS 2 :: CALCULATE AO :: CREATE NORMALS FROM DEPTH
				DrawToRendertarget(Game::GfxRenderTargetId::R_RENDERTARGET_RESOLVED_POST_SUN, "z_shader_ssao", 0.0f, 0.0f, fullscreenX, fullscreenY);

				// DEBUG :: Draw Normals & Non Blured AO
				if (Dvars::xo_ssao_debugTargets->current.enabled)
				{
					Game::gfxCmdBufSourceState->input.consts[Game::ShaderCodeConstants::CONST_SRC_CODE_FILTER_TAP_0][2] = _SSAO_DBG_MATSCALE * fullscreenX; // DONT USE FilterTap[0][2] for dvars -> Used as TextureSize.x
					Game::gfxCmdBufSourceState->input.consts[Game::ShaderCodeConstants::CONST_SRC_CODE_FILTER_TAP_0][3] = _SSAO_DBG_MATSCALE * fullscreenY; // DONT USE FilterTap[0][3] for dvars -> Used as TextureSize.y

					// DEBUG :: Draw Normals
					DrawToRendertarget(Game::GfxRenderTargetId::R_RENDERTARGET_FRAME_BUFFER, "z_shader_ssao_normal", 0.0f, 0.0f, _SSAO_DBG_MATSCALE * fullscreenX, _SSAO_DBG_MATSCALE * fullscreenY);

					// DEBUG :: Non Blured AO
					DrawToRendertarget(Game::GfxRenderTargetId::R_RENDERTARGET_FRAME_BUFFER, "z_shader_ssao", 0.0f, _SSAO_DBG_MATSCALE * fullscreenY, _SSAO_DBG_MATSCALE * fullscreenX, _SSAO_DBG_MATSCALE * fullscreenY);

					Game::gfxCmdBufSourceState->input.consts[Game::ShaderCodeConstants::CONST_SRC_CODE_FILTER_TAP_0][2] = fullscreenX; // DONT USE FilterTap[0][2] for dvars -> Used as TextureSize.x
					Game::gfxCmdBufSourceState->input.consts[Game::ShaderCodeConstants::CONST_SRC_CODE_FILTER_TAP_0][3] = fullscreenY; // DONT USE FilterTap[0][3] for dvars -> Used as TextureSize.y
				}

				// PASS 3 :: BLUR AO
				DrawToRendertarget(Game::GfxRenderTargetId::R_RENDERTARGET_FLOAT_Z, "z_shader_ssao_blur", 0.0f, 0.0f, fullscreenX, fullscreenY);

				// DEBUG :: Blured AO
				if (Dvars::xo_ssao_debugTargets->current.enabled)
				{
					Game::gfxCmdBufSourceState->input.consts[Game::ShaderCodeConstants::CONST_SRC_CODE_FILTER_TAP_0][2] = _SSAO_DBG_MATSCALE * fullscreenX; // DONT USE FilterTap[0][2] for dvars -> Used as TextureSize.x
					Game::gfxCmdBufSourceState->input.consts[Game::ShaderCodeConstants::CONST_SRC_CODE_FILTER_TAP_0][3] = _SSAO_DBG_MATSCALE * fullscreenY; // DONT USE FilterTap[0][3] for dvars -> Used as TextureSize.y

					DrawToRendertarget(Game::GfxRenderTargetId::R_RENDERTARGET_FRAME_BUFFER, "z_shader_ssao_blur", 0.0f, 2.0f * _SSAO_DBG_MATSCALE * fullscreenY, _SSAO_DBG_MATSCALE * fullscreenX, _SSAO_DBG_MATSCALE * fullscreenY);

					Game::gfxCmdBufSourceState->input.consts[21][2] = fullscreenX; // DONT USE FilterTap[0][2] for dvars -> Used as TextureSize.x
					Game::gfxCmdBufSourceState->input.consts[21][3] = fullscreenY; // DONT USE FilterTap[0][3] for dvars -> Used as TextureSize.y
				}

				// PASS 4 :: APPLY AO MULTIPLY MATERIAL
				else
				{
					DrawToRendertarget(Game::GfxRenderTargetId::R_RENDERTARGET_FRAME_BUFFER, "z_shader_ssao_apply", 0.0f, 0.0f, fullscreenX, fullscreenY);
				}
				
			}
		}

		// POSTFX_OUTLINER
		else if (Dvars::xo_shaderoverlay->current.integer == Game::XO_SHADEROVERLAY::Z_SHADER_POSTFX_OUTLINER)
		{
			// Depth prepass
			DrawToRendertarget(Game::GfxRenderTargetId::R_RENDERTARGET_FLOAT_Z, "floatz_display", 0.0f, 0.0f, fullscreenX, fullscreenY);

			// FilterTap 0
			Game::gfxCmdBufSourceState->input.consts[Game::ShaderCodeConstants::CONST_SRC_CODE_FILTER_TAP_0][0] = Dvars::xo_outliner_scale->current.value;
			Game::gfxCmdBufSourceState->input.consts[Game::ShaderCodeConstants::CONST_SRC_CODE_FILTER_TAP_0][1] = Dvars::xo_outliner_depthDiffScale->current.value;
			Game::gfxCmdBufSourceState->input.consts[Game::ShaderCodeConstants::CONST_SRC_CODE_FILTER_TAP_0][2] = Dvars::xo_outliner_depthThreshold->current.value;

			// FilterTap 2
			Game::gfxCmdBufSourceState->input.consts[Game::ShaderCodeConstants::CONST_SRC_CODE_FILTER_TAP_2][0] = Dvars::xo_outliner_toonEnable->current.enabled ? 1.0f : 0.0f;
			Game::gfxCmdBufSourceState->input.consts[Game::ShaderCodeConstants::CONST_SRC_CODE_FILTER_TAP_2][1] = Dvars::xo_outliner_toonShades->current.value;

			// Draw Outliner
			DrawToRendertarget(Game::GfxRenderTargetId::R_RENDERTARGET_FRAME_BUFFER, "z_shader_outliner", 0.0f, 0.0f, fullscreenX, fullscreenY);
		}

		else 
		{
			// If using other overlay shaders
			DrawToRendertarget(Game::GfxRenderTargetId::R_RENDERTARGET_FRAME_BUFFER, shader, 0.0f, 0.0f, fullscreenX, fullscreenY);
		}
	}

	void RB_DrawDebugPostEffects(const Game::GfxViewInfo* viewInfo)
	{
		auto r_showFbColorDebug = Game::Dvar_FindVar("r_showFbColorDebug");
		auto r_showFloatZDebug	= Game::Dvar_FindVar("r_showFloatZDebug");
		auto sc_showDebug		= Game::Dvar_FindVar("sc_showDebug");
		
		if (r_showFbColorDebug && r_showFbColorDebug->current.integer == 1)
		{
			Game::RB_ShowFbColorDebug_Screen();
		}

		else if (r_showFbColorDebug && r_showFbColorDebug->current.integer == 2)
		{
			Game::RB_ShowFbColorDebug_Feedback();
		}

		else if (r_showFloatZDebug && r_showFloatZDebug->current.enabled)
		{
			Game::RB_ShowFloatZDebug();
		}

		else if (sc_showDebug && sc_showDebug->current.enabled)
		{
			Game::RB_ShowShadowsDebug();
		}

		else if (Dvars::xo_shaderoverlay && Dvars::xo_ssao_debugnormal && (Dvars::xo_shaderoverlay->current.integer != 0 || Dvars::xo_ssao_debugnormal->current.enabled))
		{
			auto r_zFeather		= Game::Dvar_FindVar("r_zFeather");
			auto r_distortion	= Game::Dvar_FindVar("r_distortion");
			auto r_glow_allowed = Game::Dvar_FindVar("r_glow_allowed");
			
			// force depthbuffer
			if (r_zFeather && !r_zFeather->current.enabled)
			{
				Game::Cmd_ExecuteSingleCommand(0, 0, "r_zFeather 1\n");
			}

			// enable distortion (its rendertarget is needed)
			if (r_distortion && !r_distortion->current.enabled)
			{
				Game::Cmd_ExecuteSingleCommand(0, 0, "r_distortion 1\n");
			}

			// disable glow as it uses the backbuffer exclusively 
			if (r_glow_allowed && r_glow_allowed->current.enabled)
			{
				Game::Cmd_ExecuteSingleCommand(0, 0, "r_glow_allowed 0\n");
			}

			// use a custom material
			if (Dvars::xo_shaderoverlay_custom && Dvars::xo_shaderoverlay->current.integer == 5)
			{
				RB_DrawCustomShaders(Dvars::xo_shaderoverlay_custom->current.string);
			}

			// use pre-defined materials
			else
			{
				RB_DrawCustomShaders(Game::Dvar_EnumToString(Dvars::xo_shaderoverlay));
			}
		}

		if (Dvars::xo_shaderdbg_matrix && Dvars::xo_shaderdbg_matrix->current.enabled)
		{
			// GfxCodeMatrices matrices;
			memcpy(&Game::Globals::viewMatrix, &Game::gfxCmdBufSourceState->input.data->viewParms->viewMatrix, sizeof(Game::GfxMatrix));
			memcpy(&Game::Globals::projectionMatrix, &Game::gfxCmdBufSourceState->input.data->viewParms->projectionMatrix, sizeof(Game::GfxMatrix));
			memcpy(&Game::Globals::viewProjectionMatrix, &Game::gfxCmdBufSourceState->input.data->viewParms->viewProjectionMatrix, sizeof(Game::GfxMatrix));
			memcpy(&Game::Globals::inverseViewProjectionMatrix, &Game::gfxCmdBufSourceState->input.data->viewParms->inverseViewProjectionMatrix, sizeof(Game::GfxMatrix));
		}


		// *
		// overwrite resolvedPostSun to include filmTweaks
		
		//DrawToRendertarget
		Game::R_Set2D();
		Game::R_SetRenderTarget(Game::GfxRenderTargetId::R_RENDERTARGET_RESOLVED_POST_SUN);

		auto rgp = reinterpret_cast<Game::r_global_permanent_t*>(0xCC98280);
		Game::Material* t_material = rgp->postFxColorMaterial; // works without glow/blur/dof

		/*if (!viewInfo->film.enabled) {
			t_material = rgp->feedbackBlendMaterial;
		}*/

		if (t_material)
		{
			Game::RB_DrawStretchPic(t_material, 0.0f, 0.0f, Game::scrPlace->realViewableMax[0], Game::scrPlace->realViewableMax[1], 0.0, 0.0, 1.0, 1.0);
		}

		Game::RB_EndTessSurface();
	}

	__declspec(naked) void RB_DrawDebugPostEffects_Pre()
	{
		__asm
		{
			pushad;
			push	esi; // GfxViewInfo
			call	RB_DrawDebugPostEffects;
			add		esp, 4;
			popad;

			push	0x64AD75;
			retn;
		}
	}

	void RB_ShaderOverlays::Register_StringDvars()
	{
		Dvars::xo_shaderoverlay_custom = Game::Dvar_RegisterString(
			/* name		*/ "xo_shaderoverlay_custom",
			/* desc		*/ "<postfx material name>",
			/* default	*/ "default",
			/* flags	*/ Game::dvar_flags::none);
	}

	RB_ShaderOverlays::RB_ShaderOverlays()
	{
		// -----
		// Dvars

		 Dvars::r_dumpShaders = Game::Dvar_RegisterBool(
			/* name		*/ "r_dumpShaders",
			/* desc		*/ "dump loaded shaders at runtime",
			/* default	*/ false,
			/* flags	*/ Game::dvar_flags::none);

		Dvars::xo_shaderdbg_matrix = Game::Dvar_RegisterBool(
			/* name		*/ "xo_shaderdbg_matrix",
			/* desc		*/ "debug matrices",
			/* default	*/ false,
			/* flags	*/ Game::dvar_flags::none);

		static std::vector <const char*> xo_shaderoverlayEnum =
		{
			"NONE",
			"Z_SHADER_SSAO",
			"Z_SHADER_POSTFX_CELLSHADING",
			"Z_SHADER_POSTFX_OUTLINER",
			"FLOATZ_DISPLAY",
			"CUSTOM",
		};

		Dvars::xo_shaderoverlay = Game::Dvar_RegisterEnum(
			/* name		*/ "xo_shaderoverlay",
			/* desc		*/ "fullscreen shaderoverlays. <CUSTOM> uses the material defined with \"xo_shaderoverlay_custom\"",
			/* default	*/ 0,
			/* enumSize	*/ xo_shaderoverlayEnum.size(),
			/* enumData */ xo_shaderoverlayEnum.data(),
			/* flags	*/ Game::dvar_flags::none);

		Dvars::xo_ssao_debugnormal = Game::Dvar_RegisterBool(
			/* name		*/ "xo_ssao_debugnormal",
			/* desc		*/ "draw reconstructed normals from depth",
			/* default	*/ false,
			/* flags	*/ Game::dvar_flags::none);

		Dvars::xo_ssao_debugTargets = Game::Dvar_RegisterBool(
			/* name		*/ "xo_ssao_debugTargets",
			/* desc		*/ "draw each ssao pass",
			/* default	*/ false,
			/* flags	*/ Game::dvar_flags::none);

		Dvars::xo_ssao_depthprepass = Game::Dvar_RegisterBool(
			/* name		*/ "xo_ssao_depthprepass",
			/* desc		*/ "use a custom floatz pass for ssao",
			/* default	*/ true,
			/* flags	*/ Game::dvar_flags::none);

		// SSAO FilterTap 0
		Dvars::xo_ssao_noisescale	= Game::Dvar_RegisterFloat("xo_ssao_noisescale", "hlsl constant filtertap[0][0] :: _NOISESCALE :: scale of noisemap", 1.0f, 0.0f, 100.0f, Game::dvar_flags::none);
		Dvars::xo_ssao_quality		= Game::Dvar_RegisterFloat("xo_ssao_quality", "hlsl constant filtertap[0][1] :: _QUALITY :: 0 = Low, 1 = High", 0.0f, 0.0f, 1.0f, Game::dvar_flags::none);

		// SSAO FilterTap 1
		Dvars::xo_ssao_radius		= Game::Dvar_RegisterFloat("xo_ssao_radius", "hlsl constant filtertap[1][0] :: _RADIUS :: sample radius", 0.75f, 0.0f, 20.0f, Game::dvar_flags::none);

		// SSAO FilterTap 2
		Dvars::xo_ssao_contrast		= Game::Dvar_RegisterFloat("xo_ssao_contrast", "hlsl constant filtertap[2][0] :: _CONTRAST :: ao contrast", 0.6f, 0.0f, 20.0f, Game::dvar_flags::none);
		Dvars::xo_ssao_attenuation	= Game::Dvar_RegisterFloat("xo_ssao_attenuation", "hlsl constant filtertap[2][1] :: _ATTENUATION :: ao attenuation", 25.0f, -200.0f, 200.0f, Game::dvar_flags::none);
		Dvars::xo_ssao_angleBias	= Game::Dvar_RegisterFloat("xo_ssao_angleBias", "hlsl constant filtertap[2][2] :: _ANGLEBIAS :: in degrees", 30.0f, 0.0f, 90.0f, Game::dvar_flags::none);

		// SSAO Misc
		Dvars::xo_camDir0 = Game::Dvar_RegisterFloat("xo_camDir0", "viewMatrix camera dir x :: read-only", 0.0f, -1000.0f, 1000.0f, Game::dvar_flags::read_only);
		Dvars::xo_camDir1 = Game::Dvar_RegisterFloat("xo_camDir1", "viewMatrix camera dir y :: read-only", 0.0f, -1000.0f, 1000.0f, Game::dvar_flags::read_only);
		Dvars::xo_camDir2 = Game::Dvar_RegisterFloat("xo_camDir2", "viewMatrix camera dir z :: read-only", 0.0f, -1000.0f, 1000.0f, Game::dvar_flags::read_only);

		// Outliner FilterTap 0
		Dvars::xo_outliner_scale			= Game::Dvar_RegisterFloat("xo_outliner_scale", "hlsl constant filtertap[0][0] :: edge sample radius / scale", 2.0f, 0.0f, 50.0f, Game::dvar_flags::none);
		Dvars::xo_outliner_depthDiffScale	= Game::Dvar_RegisterFloat("xo_outliner_depthDiffScale", "hlsl constant filtertap[0][1] :: scale when edges with x depth difference get detected", 45.0f, 0.0f, 200.0f, Game::dvar_flags::none);
		Dvars::xo_outliner_depthThreshold	= Game::Dvar_RegisterFloat("xo_outliner_depthThreshold", "hlsl constant filtertap[0][2] :: Edges are drawn between areas where the sampled depth is greater than depthThreshold", 6.0f, -10.0f, 60.0f, Game::dvar_flags::none);

		// Outliner FilterTap 2
		Dvars::xo_outliner_toonEnable = Game::Dvar_RegisterBool("xo_outliner_toonEnable", "hlsl constant filtertap[2][0] :: toggle use of a simple toon shader", false, Game::dvar_flags::none);
		Dvars::xo_outliner_toonShades = Game::Dvar_RegisterFloat("xo_outliner_toonShades", "hlsl constant filtertap[2][1] :: Max amount of shades / colors used by the toon shader", 6.0f, 0.0f, 64.0f, Game::dvar_flags::none);


		// -----
		// Hooks

		// Disable Change from FLOATZ to SHADOWCOOKIE, to prevent AO lag or freezing
		Utils::Hook::Nop(0x658FB2, 2);

		// Rewrite RB_DrawDebugPostEffects (Entry for custom post-effects)
		//Utils::Hook(0x64AD70, RB_DrawDebugPostEffects, HOOK_CALL).install()->quick();
		Utils::Hook(0x64AD70, RB_DrawDebugPostEffects_Pre, HOOK_JUMP).install()->quick();
	}

	RB_ShaderOverlays::~RB_ShaderOverlays()
	{ }
}
