/*
 * Copyright 2014-2015 Daniel Collin. All rights reserved.
 * License: https://github.com/bkaradzic/bgfx/blob/master/LICENSE
 */

#include <mrender/bgfx.h>
#include <mrender/embedded_shader.h>
#include <mapp/allocator.h>
#include <mapp/math.h>
#include <mapp/timer.h>
#include <imgui/dear-imgui/imgui.h>
#include <imgui/dear-imgui/imgui_internal.h>

#include "imgui.h"

//#define USE_ENTRY 1

#ifndef USE_ENTRY
#	define USE_ENTRY 1
#endif // USE_ENTRY

#if USE_ENTRY
#	include "mrender/entry.h"
#	include "mrender/input.h"
#endif // USE_ENTRY

#include "vs_ocornut_imgui.bin.h"
#include "fs_ocornut_imgui.bin.h"
#include "vs_imgui_image.bin.h"
#include "fs_imgui_image.bin.h"

#include "roboto_regular.ttf.h"
#include "robotomono_regular.ttf.h"
#include "icons_kenney.ttf.h"
#include "icons_font_awesome.ttf.h"

static const bgfx::EmbeddedShader s_embeddedShaders[] =
{
	BGFX_EMBEDDED_SHADER(vs_ocornut_imgui),
	BGFX_EMBEDDED_SHADER(fs_ocornut_imgui),
	BGFX_EMBEDDED_SHADER(vs_imgui_image),
	BGFX_EMBEDDED_SHADER(fs_imgui_image),

	BGFX_EMBEDDED_SHADER_END()
};

static void* memAlloc(size_t _size, void* _userData);
static void memFree(void* _ptr, void* _userData);

struct OcornutImguiContext
{
	void render(ImDrawData* _drawData)
	{
		// Avoid rendering when minimized, scale coordinates for retina displays (screen coordinates != framebuffer coordinates)
		int fb_width = (int)(_drawData->DisplaySize.x * _drawData->FramebufferScale.x);
		int fb_height = (int)(_drawData->DisplaySize.y * _drawData->FramebufferScale.y);
		if (fb_width <= 0 || fb_height <= 0)
			return;

		bgfx::setViewName(m_viewId, "ImGui");
		bgfx::setViewMode(m_viewId, bgfx::ViewMode::Sequential);

		const bgfx::Caps* caps = bgfx::getCaps();
		{
			float ortho[16];
			float x = _drawData->DisplayPos.x;
			float y = _drawData->DisplayPos.y;
			float width = _drawData->DisplaySize.x;
			float height = _drawData->DisplaySize.y;

			bx::mtxOrtho(ortho, x, x + width, y + height, y, 0.0f, 1000.0f, 0.0f, caps->homogeneousDepth);
			bgfx::setViewTransform(m_viewId, NULL, ortho);
			bgfx::setViewRect(m_viewId, 0, 0, uint16_t(width), uint16_t(height) );
		}

		const ImVec2 clipPos   = _drawData->DisplayPos;       // (0,0) unless using multi-viewports
		const ImVec2 clipScale = _drawData->FramebufferScale; // (1,1) unless using retina display which are often (2,2)

		// Render command lists
		for (int32_t ii = 0, num = _drawData->CmdListsCount; ii < num; ++ii)
		{
			bgfx::TransientVertexBuffer tvb;
			bgfx::TransientIndexBuffer tib;

			const ImDrawList* drawList = _drawData->CmdLists[ii];
			uint32_t numVertices = (uint32_t)drawList->VtxBuffer.size();
			uint32_t numIndices  = (uint32_t)drawList->IdxBuffer.size();

			/*
			if (!checkAvailTransientBuffers(numVertices, m_layout, numIndices) )
			{
				// not enough space in transient buffer just quit drawing the rest...
				break;
			}*/

			bgfx::allocTransientVertexBuffer(&tvb, numVertices, m_layout);
			bgfx::allocTransientIndexBuffer(&tib, numIndices, sizeof(ImDrawIdx) == 4);

			ImDrawVert* verts = (ImDrawVert*)tvb.data;
			bx::memCopy(verts, drawList->VtxBuffer.begin(), numVertices * sizeof(ImDrawVert) );

			ImDrawIdx* indices = (ImDrawIdx*)tib.data;
			bx::memCopy(indices, drawList->IdxBuffer.begin(), numIndices * sizeof(ImDrawIdx) );

			bgfx::Encoder* encoder = bgfx::begin();

			for (const ImDrawCmd* cmd = drawList->CmdBuffer.begin(), *cmdEnd = drawList->CmdBuffer.end(); cmd != cmdEnd; ++cmd)
			{
				if (cmd->UserCallback)
				{
					cmd->UserCallback(drawList, cmd);
				}
				else if (0 != cmd->ElemCount)
				{
					uint64_t state = 0
						| BGFX_STATE_WRITE_RGB
						| BGFX_STATE_WRITE_A
						| BGFX_STATE_MSAA
						;

					bgfx::TextureHandle th = m_texture;
					bgfx::ProgramHandle program = m_program;

					if (NULL != cmd->TextureId)
					{
						union { ImTextureID ptr; struct { bgfx::TextureHandle handle; uint8_t flags; uint8_t mip; } s; } texture = { cmd->TextureId };
						state |= 0 != (IMGUI_FLAGS_ALPHA_BLEND & texture.s.flags)
							? BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_INV_SRC_ALPHA)
							: BGFX_STATE_NONE
							;
						th = texture.s.handle;
						if (0 != texture.s.mip)
						{
							const float lodEnabled[4] = { float(texture.s.mip), 1.0f, 0.0f, 0.0f };
							bgfx::setUniform(u_imageLodEnabled, lodEnabled);
							program = m_imageProgram;
						}
					}
					else
					{
						state |= BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_INV_SRC_ALPHA);
					}

					// Project scissor/clipping rectangles into framebuffer space
					ImVec4 clipRect;
					clipRect.x = (cmd->ClipRect.x - clipPos.x) * clipScale.x;
					clipRect.y = (cmd->ClipRect.y - clipPos.y) * clipScale.y;
					clipRect.z = (cmd->ClipRect.z - clipPos.x) * clipScale.x;
					clipRect.w = (cmd->ClipRect.w - clipPos.y) * clipScale.y;

					if (clipRect.x <  fb_width
					&&  clipRect.y <  fb_height
					&&  clipRect.z >= 0.0f
					&&  clipRect.w >= 0.0f)
					{
						const uint16_t xx = uint16_t(bx::max(clipRect.x, 0.0f) );
						const uint16_t yy = uint16_t(bx::max(clipRect.y, 0.0f) );
						encoder->setScissor(xx, yy
								, uint16_t(bx::min(clipRect.z, 65535.0f)-xx)
								, uint16_t(bx::min(clipRect.w, 65535.0f)-yy)
								);

						encoder->setState(state);
						encoder->setTexture(0, s_tex, th);
						encoder->setVertexBuffer(0, &tvb, cmd->VtxOffset, numVertices);
						encoder->setIndexBuffer(&tib, cmd->IdxOffset, cmd->ElemCount);
						encoder->submit(m_viewId, program);
					}
				}
			}

			bgfx::end(encoder);
		}
	}

	void create(float _fontSize, bx::AllocatorI* _allocator)
	{
		IMGUI_CHECKVERSION();

		m_allocator = _allocator;

		if (NULL == _allocator)
		{
			static bx::DefaultAllocator allocator;
			m_allocator = &allocator;
		}

		m_viewId = 255;
		m_lastScroll = 0;
		m_last = bx::getHPCounter();

		ImGui::SetAllocatorFunctions(memAlloc, memFree, NULL);

		m_imgui = ImGui::CreateContext();

		ImGuiIO& io = ImGui::GetIO();

		io.DisplaySize = ImVec2(1280.0f, 720.0f);
		io.DeltaTime   = 1.0f / 60.0f;
		io.IniFilename = NULL;

		setupStyle(true);

		io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;

#if USE_ENTRY
		for (int32_t ii = 0; ii < (int32_t)mrender::Key::Count; ++ii)
		{
			m_keyMap[ii] = ImGuiKey_None;
		}

		m_keyMap[mrender::Key::Esc]          = ImGuiKey_Escape;
		m_keyMap[mrender::Key::Return]       = ImGuiKey_Enter;
		m_keyMap[mrender::Key::Tab]          = ImGuiKey_Tab;
		m_keyMap[mrender::Key::Space]        = ImGuiKey_Space;
		m_keyMap[mrender::Key::Backspace]    = ImGuiKey_Backspace;
		m_keyMap[mrender::Key::Up]           = ImGuiKey_UpArrow;
		m_keyMap[mrender::Key::Down]         = ImGuiKey_DownArrow;
		m_keyMap[mrender::Key::Left]         = ImGuiKey_LeftArrow;
		m_keyMap[mrender::Key::Right]        = ImGuiKey_RightArrow;
		m_keyMap[mrender::Key::Insert]       = ImGuiKey_Insert;
		m_keyMap[mrender::Key::Delete]       = ImGuiKey_Delete;
		m_keyMap[mrender::Key::Home]         = ImGuiKey_Home;
		m_keyMap[mrender::Key::End]          = ImGuiKey_End;
		m_keyMap[mrender::Key::PageUp]       = ImGuiKey_PageUp;
		m_keyMap[mrender::Key::PageDown]     = ImGuiKey_PageDown;
		m_keyMap[mrender::Key::Print]        = ImGuiKey_PrintScreen;
		m_keyMap[mrender::Key::Plus]         = ImGuiKey_Equal;
		m_keyMap[mrender::Key::Minus]        = ImGuiKey_Minus;
		m_keyMap[mrender::Key::LeftBracket]  = ImGuiKey_LeftBracket;
		m_keyMap[mrender::Key::RightBracket] = ImGuiKey_RightBracket;
		m_keyMap[mrender::Key::Semicolon]    = ImGuiKey_Semicolon;
		m_keyMap[mrender::Key::Quote]        = ImGuiKey_Apostrophe;
		m_keyMap[mrender::Key::Comma]        = ImGuiKey_Comma;
		m_keyMap[mrender::Key::Period]       = ImGuiKey_Period;
		m_keyMap[mrender::Key::Slash]        = ImGuiKey_Slash;
		m_keyMap[mrender::Key::Backslash]    = ImGuiKey_Backslash;
		m_keyMap[mrender::Key::Tilde]        = ImGuiKey_GraveAccent;
		m_keyMap[mrender::Key::F1]           = ImGuiKey_F1;
		m_keyMap[mrender::Key::F2]           = ImGuiKey_F2;
		m_keyMap[mrender::Key::F3]           = ImGuiKey_F3;
		m_keyMap[mrender::Key::F4]           = ImGuiKey_F4;
		m_keyMap[mrender::Key::F5]           = ImGuiKey_F5;
		m_keyMap[mrender::Key::F6]           = ImGuiKey_F6;
		m_keyMap[mrender::Key::F7]           = ImGuiKey_F7;
		m_keyMap[mrender::Key::F8]           = ImGuiKey_F8;
		m_keyMap[mrender::Key::F9]           = ImGuiKey_F9;
		m_keyMap[mrender::Key::F10]          = ImGuiKey_F10;
		m_keyMap[mrender::Key::F11]          = ImGuiKey_F11;
		m_keyMap[mrender::Key::F12]          = ImGuiKey_F12;
		m_keyMap[mrender::Key::NumPad0]      = ImGuiKey_Keypad0;
		m_keyMap[mrender::Key::NumPad1]      = ImGuiKey_Keypad1;
		m_keyMap[mrender::Key::NumPad2]      = ImGuiKey_Keypad2;
		m_keyMap[mrender::Key::NumPad3]      = ImGuiKey_Keypad3;
		m_keyMap[mrender::Key::NumPad4]      = ImGuiKey_Keypad4;
		m_keyMap[mrender::Key::NumPad5]      = ImGuiKey_Keypad5;
		m_keyMap[mrender::Key::NumPad6]      = ImGuiKey_Keypad6;
		m_keyMap[mrender::Key::NumPad7]      = ImGuiKey_Keypad7;
		m_keyMap[mrender::Key::NumPad8]      = ImGuiKey_Keypad8;
		m_keyMap[mrender::Key::NumPad9]      = ImGuiKey_Keypad9;
		m_keyMap[mrender::Key::Key0]         = ImGuiKey_0;
		m_keyMap[mrender::Key::Key1]         = ImGuiKey_1;
		m_keyMap[mrender::Key::Key2]         = ImGuiKey_2;
		m_keyMap[mrender::Key::Key3]         = ImGuiKey_3;
		m_keyMap[mrender::Key::Key4]         = ImGuiKey_4;
		m_keyMap[mrender::Key::Key5]         = ImGuiKey_5;
		m_keyMap[mrender::Key::Key6]         = ImGuiKey_6;
		m_keyMap[mrender::Key::Key7]         = ImGuiKey_7;
		m_keyMap[mrender::Key::Key8]         = ImGuiKey_8;
		m_keyMap[mrender::Key::Key9]         = ImGuiKey_9;
		m_keyMap[mrender::Key::KeyA]         = ImGuiKey_A;
		m_keyMap[mrender::Key::KeyB]         = ImGuiKey_B;
		m_keyMap[mrender::Key::KeyC]         = ImGuiKey_C;
		m_keyMap[mrender::Key::KeyD]         = ImGuiKey_D;
		m_keyMap[mrender::Key::KeyE]         = ImGuiKey_E;
		m_keyMap[mrender::Key::KeyF]         = ImGuiKey_F;
		m_keyMap[mrender::Key::KeyG]         = ImGuiKey_G;
		m_keyMap[mrender::Key::KeyH]         = ImGuiKey_H;
		m_keyMap[mrender::Key::KeyI]         = ImGuiKey_I;
		m_keyMap[mrender::Key::KeyJ]         = ImGuiKey_J;
		m_keyMap[mrender::Key::KeyK]         = ImGuiKey_K;
		m_keyMap[mrender::Key::KeyL]         = ImGuiKey_L;
		m_keyMap[mrender::Key::KeyM]         = ImGuiKey_M;
		m_keyMap[mrender::Key::KeyN]         = ImGuiKey_N;
		m_keyMap[mrender::Key::KeyO]         = ImGuiKey_O;
		m_keyMap[mrender::Key::KeyP]         = ImGuiKey_P;
		m_keyMap[mrender::Key::KeyQ]         = ImGuiKey_Q;
		m_keyMap[mrender::Key::KeyR]         = ImGuiKey_R;
		m_keyMap[mrender::Key::KeyS]         = ImGuiKey_S;
		m_keyMap[mrender::Key::KeyT]         = ImGuiKey_T;
		m_keyMap[mrender::Key::KeyU]         = ImGuiKey_U;
		m_keyMap[mrender::Key::KeyV]         = ImGuiKey_V;
		m_keyMap[mrender::Key::KeyW]         = ImGuiKey_W;
		m_keyMap[mrender::Key::KeyX]         = ImGuiKey_X;
		m_keyMap[mrender::Key::KeyY]         = ImGuiKey_Y;
		m_keyMap[mrender::Key::KeyZ]         = ImGuiKey_Z;

		io.ConfigFlags |= 0
			| ImGuiConfigFlags_NavEnableGamepad
			| ImGuiConfigFlags_NavEnableKeyboard
			;

		m_keyMap[mrender::Key::GamepadStart]     = ImGuiKey_GamepadStart;
		m_keyMap[mrender::Key::GamepadBack]      = ImGuiKey_GamepadBack;
		m_keyMap[mrender::Key::GamepadY]         = ImGuiKey_GamepadFaceUp;
		m_keyMap[mrender::Key::GamepadA]         = ImGuiKey_GamepadFaceDown;
		m_keyMap[mrender::Key::GamepadX]         = ImGuiKey_GamepadFaceLeft;
		m_keyMap[mrender::Key::GamepadB]         = ImGuiKey_GamepadFaceRight;
		m_keyMap[mrender::Key::GamepadUp]        = ImGuiKey_GamepadDpadUp;
		m_keyMap[mrender::Key::GamepadDown]      = ImGuiKey_GamepadDpadDown;
		m_keyMap[mrender::Key::GamepadLeft]      = ImGuiKey_GamepadDpadLeft;
		m_keyMap[mrender::Key::GamepadRight]     = ImGuiKey_GamepadDpadRight;
		m_keyMap[mrender::Key::GamepadShoulderL] = ImGuiKey_GamepadL1;
		m_keyMap[mrender::Key::GamepadShoulderR] = ImGuiKey_GamepadR1;
		m_keyMap[mrender::Key::GamepadThumbL]    = ImGuiKey_GamepadL3;
		m_keyMap[mrender::Key::GamepadThumbR]    = ImGuiKey_GamepadR3;
#endif // USE_ENTRY

		bgfx::RendererType::Enum type = bgfx::getRendererType();
		m_program = bgfx::createProgram(
			  bgfx::createEmbeddedShader(s_embeddedShaders, type, "vs_ocornut_imgui")
			, bgfx::createEmbeddedShader(s_embeddedShaders, type, "fs_ocornut_imgui")
			, true
			);

		u_imageLodEnabled = bgfx::createUniform("u_imageLodEnabled", bgfx::UniformType::Vec4);
		m_imageProgram = bgfx::createProgram(
			  bgfx::createEmbeddedShader(s_embeddedShaders, type, "vs_imgui_image")
			, bgfx::createEmbeddedShader(s_embeddedShaders, type, "fs_imgui_image")
			, true
			);

		m_layout
			.begin()
			.add(bgfx::Attrib::Position,  2, bgfx::AttribType::Float)
			.add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
			.add(bgfx::Attrib::Color0,    4, bgfx::AttribType::Uint8, true)
			.end();

		s_tex = bgfx::createUniform("s_tex", bgfx::UniformType::Sampler);

		
		uint8_t* data;
		int32_t width;
		int32_t height;
		/*
		{
			ImFontConfig config;
			config.FontDataOwnedByAtlas = false;
			config.MergeMode = false;
//			config.MergeGlyphCenterV = true;

			const ImWchar* ranges = io.Fonts->GetGlyphRangesCyrillic();
			m_font[ImGui::Font::Regular] = io.Fonts->AddFontFromMemoryTTF( (void*)s_robotoRegularTtf,     sizeof(s_robotoRegularTtf),     _fontSize,      &config, ranges);
			m_font[ImGui::Font::Mono   ] = io.Fonts->AddFontFromMemoryTTF( (void*)s_robotoMonoRegularTtf, sizeof(s_robotoMonoRegularTtf), _fontSize-3.0f, &config, ranges);

			config.MergeMode = true;
			config.DstFont   = m_font[ImGui::Font::Regular];

			for (uint32_t ii = 0; ii < BX_COUNTOF(s_fontRangeMerge); ++ii)
			{
				const FontRangeMerge& frm = s_fontRangeMerge[ii];

				io.Fonts->AddFontFromMemoryTTF( (void*)frm.data
						, (int)frm.size
						, _fontSize-3.0f
						, &config
						, frm.ranges
						);
			}
		}*/

		io.Fonts->GetTexDataAsRGBA32(&data, &width, &height);

		m_texture = bgfx::createTexture2D(
			  (uint16_t)width
			, (uint16_t)height
			, false
			, 1
			, bgfx::TextureFormat::BGRA8
			, 0
			, bgfx::copy(data, width*height*4)
			);

		ImGui::InitDockContext();
	}

	void destroy()
	{
		ImGui::ShutdownDockContext(); 
		ImGui::DestroyContext(m_imgui);

		bgfx::destroy(s_tex);
		bgfx::destroy(m_texture);

		bgfx::destroy(u_imageLodEnabled);
		bgfx::destroy(m_imageProgram);
		bgfx::destroy(m_program);

		m_allocator = NULL;
	}

	void setupStyle(bool _dark)
	{
		ImGuiStyle& style = ImGui::GetStyle();
		if (_dark)
		{
			ImGui::StyleColorsDark(&style);
		}
		else
		{
			ImGui::StyleColorsLight(&style);
		}

		style.FrameRounding    = 4.0f;
		style.WindowBorderSize = 0.0f;
	}

	void beginFrame(
		  int32_t _mx
		, int32_t _my
		, uint8_t _button
		, int32_t _scroll
		, int _width
		, int _height
		, int _inputChar
		, bgfx::ViewId _viewId
		)
	{
		m_viewId = _viewId;

		ImGuiIO& io = ImGui::GetIO();
		if (_inputChar >= 0)
		{
			io.AddInputCharacter(_inputChar);
		}

		io.DisplaySize = ImVec2( (float)_width, (float)_height);

		const int64_t now = bx::getHPCounter();
		const int64_t frameTime = now - m_last;
		m_last = now;
		const double freq = double(bx::getHPFrequency() );
		io.DeltaTime = float(frameTime/freq);

		io.AddMousePosEvent( (float)_mx, (float)_my);
		io.AddMouseButtonEvent(ImGuiMouseButton_Left,   0 != (_button & IMGUI_MBUT_LEFT  ) );
		io.AddMouseButtonEvent(ImGuiMouseButton_Right,  0 != (_button & IMGUI_MBUT_RIGHT ) );
		io.AddMouseButtonEvent(ImGuiMouseButton_Middle, 0 != (_button & IMGUI_MBUT_MIDDLE) );
		io.AddMouseWheelEvent(0.0f, (float)(_scroll - m_lastScroll) );
		m_lastScroll = _scroll;

#if USE_ENTRY
		uint8_t modifiers = inputGetModifiersState();
		io.AddKeyEvent(ImGuiKey_ModShift, 0 != (modifiers & (mrender::Modifier::LeftShift | mrender::Modifier::RightShift) ) );
		io.AddKeyEvent(ImGuiKey_ModCtrl,  0 != (modifiers & (mrender::Modifier::LeftCtrl  | mrender::Modifier::RightCtrl ) ) );
		io.AddKeyEvent(ImGuiKey_ModAlt,   0 != (modifiers & (mrender::Modifier::LeftAlt   | mrender::Modifier::RightAlt  ) ) );
		io.AddKeyEvent(ImGuiKey_ModSuper, 0 != (modifiers & (mrender::Modifier::LeftMeta  | mrender::Modifier::RightMeta ) ) );
		for (int32_t ii = 0; ii < (int32_t)mrender::Key::Count; ++ii)
		{
			io.AddKeyEvent(m_keyMap[ii], inputGetKeyState(mrender::Key::Enum(ii) ) );
			io.SetKeyEventNativeData(m_keyMap[ii], 0, 0, ii);
		}
#endif // USE_ENTRY

		ImGui::NewFrame();

		//ImGuizmo::BeginFrame();  @todo
	}

	void endFrame()
	{
		ImGui::Render();
		render(ImGui::GetDrawData() );
	}

	ImGuiContext*       m_imgui;
	bx::AllocatorI*     m_allocator;
	bgfx::VertexLayout  m_layout;
	bgfx::ProgramHandle m_program;
	bgfx::ProgramHandle m_imageProgram;
	bgfx::TextureHandle m_texture;
	bgfx::UniformHandle s_tex;
	bgfx::UniformHandle u_imageLodEnabled;
	//ImFont* m_font[ImGui::Font::Count];
	int64_t m_last;
	int32_t m_lastScroll;
	bgfx::ViewId m_viewId;
#if USE_ENTRY
	ImGuiKey m_keyMap[(int)mrender::Key::Count];
#endif // USE_ENTRY
};

static OcornutImguiContext s_ctx;

static void* memAlloc(size_t _size, void* _userData)
{
	BX_UNUSED(_userData);
	return bx::alloc(s_ctx.m_allocator, _size);
}

static void memFree(void* _ptr, void* _userData)
{
	BX_UNUSED(_userData);
	bx::free(s_ctx.m_allocator, _ptr);
}

namespace mengine {
	void imguiCreate(float _fontSize, bx::AllocatorI* _allocator)
	{
		s_ctx.create(_fontSize, _allocator);
	}

	void imguiDestroy()
	{
		s_ctx.destroy();
	}

	void imguiBeginFrame(int32_t _mx, int32_t _my, uint8_t _button, int32_t _scroll, uint16_t _width, uint16_t _height, int _inputChar, bgfx::ViewId _viewId)
	{
		s_ctx.beginFrame(_mx, _my, _button, _scroll, _width, _height, _inputChar, _viewId);
	}

	void imguiEndFrame()
	{
		s_ctx.endFrame();
	}
}
/*
namespace ImGui
{
	void PushFont(Font::Enum _font)
	{
		PushFont(s_ctx.m_font[_font]);
	}

	void PushEnabled(bool _enabled)
	{
		extern void PushItemFlag(int option, bool enabled);
		PushItemFlag(ImGuiItemFlags_Disabled, !_enabled);
		PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * (_enabled ? 1.0f : 0.5f) );
	}

	void PopEnabled()
	{
		extern void PopItemFlag();
		PopItemFlag();
		PopStyleVar();
	}

} // namespace ImGui*/

BX_PRAGMA_DIAGNOSTIC_IGNORED_MSVC(4505); // error C4505: '' : unreferenced local function has been removed
BX_PRAGMA_DIAGNOSTIC_IGNORED_CLANG_GCC("-Wunused-function"); // warning: 'int rect_width_compare(const void*, const void*)' defined but not used
BX_PRAGMA_DIAGNOSTIC_PUSH();
BX_PRAGMA_DIAGNOSTIC_IGNORED_CLANG("-Wunknown-pragmas")
BX_PRAGMA_DIAGNOSTIC_IGNORED_CLANG_GCC("-Wtype-limits"); // warning: comparison is always true due to limited range of data type
#define STBTT_malloc(_size, _userData) memAlloc(_size, _userData)
#define STBTT_free(_ptr, _userData) memFree(_ptr, _userData)
#define STB_RECT_PACK_IMPLEMENTATION
//#include <stb/stb_rect_pack.h>
#define STB_TRUETYPE_IMPLEMENTATION
//#include <stb/stb_truetype.h>
BX_PRAGMA_DIAGNOSTIC_POP();