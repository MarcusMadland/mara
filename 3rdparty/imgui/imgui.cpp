/*
 * Copyright 2014-2015 Daniel Collin. All rights reserved.
 * License: https://github.com/bkaradzic/graphics/blob/master/LICENSE
 */

#include <mara/mara.h>
#include <graphics/graphics.h>
#include <graphics/embedded_shader.h>
#include <base/allocator.h>
#include <base/math.h>
#include <base/timer.h>
#include <imgui/dear-imgui/imgui.h>
#include <imgui/dear-imgui/imgui_internal.h>

#include "imgui.h"

//#define USE_ENTRY 1

#ifndef USE_ENTRY
#	define USE_ENTRY 1
#endif // USE_ENTRY

#if USE_ENTRY
#	include "graphics/entry.h"
#	include "graphics/input.h"
#endif // USE_ENTRY

#include "vs_ocornut_imgui.bin.h"
#include "fs_ocornut_imgui.bin.h"
#include "vs_imgui_image.bin.h"
#include "fs_imgui_image.bin.h"

#include "roboto_regular.ttf.h"
#include "robotomono_regular.ttf.h"
#include "icons_kenney.ttf.h"
#include "icons_font_awesome.ttf.h"

static const graphics::EmbeddedShader s_embeddedShaders[] =
{
	GRAPHICS_EMBEDDED_SHADER(vs_ocornut_imgui),
	GRAPHICS_EMBEDDED_SHADER(fs_ocornut_imgui),
	GRAPHICS_EMBEDDED_SHADER(vs_imgui_image),
	GRAPHICS_EMBEDDED_SHADER(fs_imgui_image),

	GRAPHICS_EMBEDDED_SHADER_END()
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

		graphics::setViewName(m_viewId, "ImGui");
		graphics::setViewMode(m_viewId, graphics::ViewMode::Sequential);

		const graphics::Caps* caps = graphics::getCaps();
		{
			float ortho[16];
			float x = _drawData->DisplayPos.x;
			float y = _drawData->DisplayPos.y;
			float width = _drawData->DisplaySize.x;
			float height = _drawData->DisplaySize.y;

			base::mtxOrtho(ortho, x, x + width, y + height, y, 0.0f, 1000.0f, 0.0f, caps->homogeneousDepth);
			graphics::setViewTransform(m_viewId, NULL, ortho);
			graphics::setViewRect(m_viewId, 0, 0, uint16_t(width), uint16_t(height) );
		}

		const ImVec2 clipPos   = _drawData->DisplayPos;       // (0,0) unless using multi-viewports
		const ImVec2 clipScale = _drawData->FramebufferScale; // (1,1) unless using retina display which are often (2,2)

		// Render command lists
		for (int32_t ii = 0, num = _drawData->CmdListsCount; ii < num; ++ii)
		{
			graphics::TransientVertexBuffer tvb;
			graphics::TransientIndexBuffer tib;

			const ImDrawList* drawList = _drawData->CmdLists[ii];
			uint32_t numVertices = (uint32_t)drawList->VtxBuffer.size();
			uint32_t numIndices  = (uint32_t)drawList->IdxBuffer.size();

			/*
			if (!checkAvailTransientBuffers(numVertices, m_layout, numIndices) )
			{
				// not enough space in transient buffer just quit drawing the rest...
				break;
			}*/

			graphics::allocTransientVertexBuffer(&tvb, numVertices, m_layout);
			graphics::allocTransientIndexBuffer(&tib, numIndices, sizeof(ImDrawIdx) == 4);

			ImDrawVert* verts = (ImDrawVert*)tvb.data;
			base::memCopy(verts, drawList->VtxBuffer.begin(), numVertices * sizeof(ImDrawVert) );

			ImDrawIdx* indices = (ImDrawIdx*)tib.data;
			base::memCopy(indices, drawList->IdxBuffer.begin(), numIndices * sizeof(ImDrawIdx) );

			graphics::Encoder* encoder = graphics::begin();

			for (const ImDrawCmd* cmd = drawList->CmdBuffer.begin(), *cmdEnd = drawList->CmdBuffer.end(); cmd != cmdEnd; ++cmd)
			{
				if (cmd->UserCallback)
				{
					cmd->UserCallback(drawList, cmd);
				}
				else if (0 != cmd->ElemCount)
				{
					uint64_t state = 0
						| GRAPHICS_STATE_WRITE_RGB
						| GRAPHICS_STATE_WRITE_A
						| GRAPHICS_STATE_MSAA
						;

					graphics::TextureHandle th = m_texture;
					graphics::ProgramHandle program = m_program;

					if (NULL != cmd->TextureId)
					{
						union { ImTextureID ptr; struct { graphics::TextureHandle handle; uint8_t flags; uint8_t mip; } s; } texture = { cmd->TextureId };
						state |= 0 != (IMGUI_FLAGS_ALPHA_BLEND & texture.s.flags)
							? GRAPHICS_STATE_BLEND_FUNC(GRAPHICS_STATE_BLEND_SRC_ALPHA, GRAPHICS_STATE_BLEND_INV_SRC_ALPHA)
							: GRAPHICS_STATE_NONE
							;
						th = texture.s.handle;
						if (0 != texture.s.mip)
						{
							const float lodEnabled[4] = { float(texture.s.mip), 1.0f, 0.0f, 0.0f };
							graphics::setUniform(u_imageLodEnabled, lodEnabled);
							program = m_imageProgram;
						}
					}
					else
					{
						state |= GRAPHICS_STATE_BLEND_FUNC(GRAPHICS_STATE_BLEND_SRC_ALPHA, GRAPHICS_STATE_BLEND_INV_SRC_ALPHA);
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
						const uint16_t xx = uint16_t(base::max(clipRect.x, 0.0f) );
						const uint16_t yy = uint16_t(base::max(clipRect.y, 0.0f) );
						encoder->setScissor(xx, yy
								, uint16_t(base::min(clipRect.z, 65535.0f)-xx)
								, uint16_t(base::min(clipRect.w, 65535.0f)-yy)
								);

						encoder->setState(state);
						encoder->setTexture(0, s_tex, th);
						encoder->setVertexBuffer(0, &tvb, cmd->VtxOffset, numVertices);
						encoder->setIndexBuffer(&tib, cmd->IdxOffset, cmd->ElemCount);
						encoder->submit(m_viewId, program);
					}
				}
			}

			graphics::end(encoder);
		}
	}

	void create(float _fontSize, base::AllocatorI* _allocator)
	{
		IMGUI_CHECKVERSION();

		m_allocator = _allocator;

		if (NULL == _allocator)
		{
			static base::DefaultAllocator allocator;
			m_allocator = &allocator;
		}

		m_viewId = 255;
		m_lastScroll = 0;
		m_last = base::getHPCounter();

		ImGui::SetAllocatorFunctions(memAlloc, memFree, NULL);

		m_imgui = ImGui::CreateContext();

		ImGuiIO& io = ImGui::GetIO();

		io.DisplaySize = ImVec2(1280.0f, 720.0f);
		io.DeltaTime   = 1.0f / 60.0f;
		io.IniFilename = NULL;

		setupStyle(true);

		io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;

#if USE_ENTRY
		for (int32_t ii = 0; ii < (int32_t)entry::Key::Count; ++ii)
		{
			m_keyMap[ii] = ImGuiKey_None;
		}

		m_keyMap[entry::Key::Esc]          = ImGuiKey_Escape;
		m_keyMap[entry::Key::Return]       = ImGuiKey_Enter;
		m_keyMap[entry::Key::Tab]          = ImGuiKey_Tab;
		m_keyMap[entry::Key::Space]        = ImGuiKey_Space;
		m_keyMap[entry::Key::Backspace]    = ImGuiKey_Backspace;
		m_keyMap[entry::Key::Up]           = ImGuiKey_UpArrow;
		m_keyMap[entry::Key::Down]         = ImGuiKey_DownArrow;
		m_keyMap[entry::Key::Left]         = ImGuiKey_LeftArrow;
		m_keyMap[entry::Key::Right]        = ImGuiKey_RightArrow;
		m_keyMap[entry::Key::Insert]       = ImGuiKey_Insert;
		m_keyMap[entry::Key::Delete]       = ImGuiKey_Delete;
		m_keyMap[entry::Key::Home]         = ImGuiKey_Home;
		m_keyMap[entry::Key::End]          = ImGuiKey_End;
		m_keyMap[entry::Key::PageUp]       = ImGuiKey_PageUp;
		m_keyMap[entry::Key::PageDown]     = ImGuiKey_PageDown;
		m_keyMap[entry::Key::Print]        = ImGuiKey_PrintScreen;
		m_keyMap[entry::Key::Plus]         = ImGuiKey_Equal;
		m_keyMap[entry::Key::Minus]        = ImGuiKey_Minus;
		m_keyMap[entry::Key::LeftBracket]  = ImGuiKey_LeftBracket;
		m_keyMap[entry::Key::RightBracket] = ImGuiKey_RightBracket;
		m_keyMap[entry::Key::Semicolon]    = ImGuiKey_Semicolon;
		m_keyMap[entry::Key::Quote]        = ImGuiKey_Apostrophe;
		m_keyMap[entry::Key::Comma]        = ImGuiKey_Comma;
		m_keyMap[entry::Key::Period]       = ImGuiKey_Period;
		m_keyMap[entry::Key::Slash]        = ImGuiKey_Slash;
		m_keyMap[entry::Key::Backslash]    = ImGuiKey_Backslash;
		m_keyMap[entry::Key::Tilde]        = ImGuiKey_GraveAccent;
		m_keyMap[entry::Key::F1]           = ImGuiKey_F1;
		m_keyMap[entry::Key::F2]           = ImGuiKey_F2;
		m_keyMap[entry::Key::F3]           = ImGuiKey_F3;
		m_keyMap[entry::Key::F4]           = ImGuiKey_F4;
		m_keyMap[entry::Key::F5]           = ImGuiKey_F5;
		m_keyMap[entry::Key::F6]           = ImGuiKey_F6;
		m_keyMap[entry::Key::F7]           = ImGuiKey_F7;
		m_keyMap[entry::Key::F8]           = ImGuiKey_F8;
		m_keyMap[entry::Key::F9]           = ImGuiKey_F9;
		m_keyMap[entry::Key::F10]          = ImGuiKey_F10;
		m_keyMap[entry::Key::F11]          = ImGuiKey_F11;
		m_keyMap[entry::Key::F12]          = ImGuiKey_F12;
		m_keyMap[entry::Key::NumPad0]      = ImGuiKey_Keypad0;
		m_keyMap[entry::Key::NumPad1]      = ImGuiKey_Keypad1;
		m_keyMap[entry::Key::NumPad2]      = ImGuiKey_Keypad2;
		m_keyMap[entry::Key::NumPad3]      = ImGuiKey_Keypad3;
		m_keyMap[entry::Key::NumPad4]      = ImGuiKey_Keypad4;
		m_keyMap[entry::Key::NumPad5]      = ImGuiKey_Keypad5;
		m_keyMap[entry::Key::NumPad6]      = ImGuiKey_Keypad6;
		m_keyMap[entry::Key::NumPad7]      = ImGuiKey_Keypad7;
		m_keyMap[entry::Key::NumPad8]      = ImGuiKey_Keypad8;
		m_keyMap[entry::Key::NumPad9]      = ImGuiKey_Keypad9;
		m_keyMap[entry::Key::Key0]         = ImGuiKey_0;
		m_keyMap[entry::Key::Key1]         = ImGuiKey_1;
		m_keyMap[entry::Key::Key2]         = ImGuiKey_2;
		m_keyMap[entry::Key::Key3]         = ImGuiKey_3;
		m_keyMap[entry::Key::Key4]         = ImGuiKey_4;
		m_keyMap[entry::Key::Key5]         = ImGuiKey_5;
		m_keyMap[entry::Key::Key6]         = ImGuiKey_6;
		m_keyMap[entry::Key::Key7]         = ImGuiKey_7;
		m_keyMap[entry::Key::Key8]         = ImGuiKey_8;
		m_keyMap[entry::Key::Key9]         = ImGuiKey_9;
		m_keyMap[entry::Key::KeyA]         = ImGuiKey_A;
		m_keyMap[entry::Key::KeyB]         = ImGuiKey_B;
		m_keyMap[entry::Key::KeyC]         = ImGuiKey_C;
		m_keyMap[entry::Key::KeyD]         = ImGuiKey_D;
		m_keyMap[entry::Key::KeyE]         = ImGuiKey_E;
		m_keyMap[entry::Key::KeyF]         = ImGuiKey_F;
		m_keyMap[entry::Key::KeyG]         = ImGuiKey_G;
		m_keyMap[entry::Key::KeyH]         = ImGuiKey_H;
		m_keyMap[entry::Key::KeyI]         = ImGuiKey_I;
		m_keyMap[entry::Key::KeyJ]         = ImGuiKey_J;
		m_keyMap[entry::Key::KeyK]         = ImGuiKey_K;
		m_keyMap[entry::Key::KeyL]         = ImGuiKey_L;
		m_keyMap[entry::Key::KeyM]         = ImGuiKey_M;
		m_keyMap[entry::Key::KeyN]         = ImGuiKey_N;
		m_keyMap[entry::Key::KeyO]         = ImGuiKey_O;
		m_keyMap[entry::Key::KeyP]         = ImGuiKey_P;
		m_keyMap[entry::Key::KeyQ]         = ImGuiKey_Q;
		m_keyMap[entry::Key::KeyR]         = ImGuiKey_R;
		m_keyMap[entry::Key::KeyS]         = ImGuiKey_S;
		m_keyMap[entry::Key::KeyT]         = ImGuiKey_T;
		m_keyMap[entry::Key::KeyU]         = ImGuiKey_U;
		m_keyMap[entry::Key::KeyV]         = ImGuiKey_V;
		m_keyMap[entry::Key::KeyW]         = ImGuiKey_W;
		m_keyMap[entry::Key::KeyX]         = ImGuiKey_X;
		m_keyMap[entry::Key::KeyY]         = ImGuiKey_Y;
		m_keyMap[entry::Key::KeyZ]         = ImGuiKey_Z;

		io.ConfigFlags |= 0
			| ImGuiConfigFlags_NavEnableGamepad
			| ImGuiConfigFlags_NavEnableKeyboard
			;

		m_keyMap[entry::Key::GamepadStart]     = ImGuiKey_GamepadStart;
		m_keyMap[entry::Key::GamepadBack]      = ImGuiKey_GamepadBack;
		m_keyMap[entry::Key::GamepadY]         = ImGuiKey_GamepadFaceUp;
		m_keyMap[entry::Key::GamepadA]         = ImGuiKey_GamepadFaceDown;
		m_keyMap[entry::Key::GamepadX]         = ImGuiKey_GamepadFaceLeft;
		m_keyMap[entry::Key::GamepadB]         = ImGuiKey_GamepadFaceRight;
		m_keyMap[entry::Key::GamepadUp]        = ImGuiKey_GamepadDpadUp;
		m_keyMap[entry::Key::GamepadDown]      = ImGuiKey_GamepadDpadDown;
		m_keyMap[entry::Key::GamepadLeft]      = ImGuiKey_GamepadDpadLeft;
		m_keyMap[entry::Key::GamepadRight]     = ImGuiKey_GamepadDpadRight;
		m_keyMap[entry::Key::GamepadShoulderL] = ImGuiKey_GamepadL1;
		m_keyMap[entry::Key::GamepadShoulderR] = ImGuiKey_GamepadR1;
		m_keyMap[entry::Key::GamepadThumbL]    = ImGuiKey_GamepadL3;
		m_keyMap[entry::Key::GamepadThumbR]    = ImGuiKey_GamepadR3;
#endif // USE_ENTRY

		graphics::RendererType::Enum type = graphics::getRendererType();
		m_program = graphics::createProgram(
			  graphics::createEmbeddedShader(s_embeddedShaders, type, "vs_ocornut_imgui")
			, graphics::createEmbeddedShader(s_embeddedShaders, type, "fs_ocornut_imgui")
			, true
			);

		u_imageLodEnabled = graphics::createUniform("u_imageLodEnabled", graphics::UniformType::Vec4);
		m_imageProgram = graphics::createProgram(
			  graphics::createEmbeddedShader(s_embeddedShaders, type, "vs_imgui_image")
			, graphics::createEmbeddedShader(s_embeddedShaders, type, "fs_imgui_image")
			, true
			);

		m_layout
			.begin()
			.add(graphics::Attrib::Position,  2, graphics::AttribType::Float)
			.add(graphics::Attrib::TexCoord0, 2, graphics::AttribType::Float)
			.add(graphics::Attrib::Color0,    4, graphics::AttribType::Uint8, true)
			.end();

		s_tex = graphics::createUniform("s_tex", graphics::UniformType::Sampler);

		
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

			for (uint32_t ii = 0; ii < BASE_COUNTOF(s_fontRangeMerge); ++ii)
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

		m_texture = graphics::createTexture2D(
			  (uint16_t)width
			, (uint16_t)height
			, false
			, 1
			, graphics::TextureFormat::BGRA8
			, 0
			, graphics::copy(data, width*height*4)
			);

		ImGui::InitDockContext();
	}

	void destroy()
	{
		ImGui::ShutdownDockContext(); 
		ImGui::DestroyContext(m_imgui);

		graphics::destroy(s_tex);
		graphics::destroy(m_texture);

		graphics::destroy(u_imageLodEnabled);
		graphics::destroy(m_imageProgram);
		graphics::destroy(m_program);

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
		, graphics::ViewId _viewId
		)
	{
		m_viewId = _viewId;

		ImGuiIO& io = ImGui::GetIO();
		if (_inputChar >= 0)
		{
			io.AddInputCharacter(_inputChar);
		}

		io.DisplaySize = ImVec2( (float)_width, (float)_height);

		const int64_t now = base::getHPCounter();
		const int64_t frameTime = now - m_last;
		m_last = now;
		const double freq = double(base::getHPFrequency() );
		io.DeltaTime = float(frameTime/freq);

		io.AddMousePosEvent( (float)_mx, (float)_my);
		io.AddMouseButtonEvent(ImGuiMouseButton_Left,   0 != (_button & IMGUI_MBUT_LEFT  ) );
		io.AddMouseButtonEvent(ImGuiMouseButton_Right,  0 != (_button & IMGUI_MBUT_RIGHT ) );
		io.AddMouseButtonEvent(ImGuiMouseButton_Middle, 0 != (_button & IMGUI_MBUT_MIDDLE) );
		io.AddMouseWheelEvent(0.0f, (float)(_scroll - m_lastScroll) );
		m_lastScroll = _scroll;

#if USE_ENTRY
		uint8_t modifiers = inputGetModifiersState();
		io.AddKeyEvent(ImGuiKey_ModShift, 0 != (modifiers & (entry::Modifier::LeftShift | entry::Modifier::RightShift) ) );
		io.AddKeyEvent(ImGuiKey_ModCtrl,  0 != (modifiers & (entry::Modifier::LeftCtrl  | entry::Modifier::RightCtrl ) ) );
		io.AddKeyEvent(ImGuiKey_ModAlt,   0 != (modifiers & (entry::Modifier::LeftAlt   | entry::Modifier::RightAlt  ) ) );
		io.AddKeyEvent(ImGuiKey_ModSuper, 0 != (modifiers & (entry::Modifier::LeftMeta  | entry::Modifier::RightMeta ) ) );
		for (int32_t ii = 0; ii < (int32_t)entry::Key::Count; ++ii)
		{
			io.AddKeyEvent(m_keyMap[ii], inputGetKeyState(entry::Key::Enum(ii) ) );
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
	base::AllocatorI*     m_allocator;
	graphics::VertexLayout  m_layout;
	graphics::ProgramHandle m_program;
	graphics::ProgramHandle m_imageProgram;
	graphics::TextureHandle m_texture;
	graphics::UniformHandle s_tex;
	graphics::UniformHandle u_imageLodEnabled;
	//ImFont* m_font[ImGui::Font::Count];
	int64_t m_last;
	int32_t m_lastScroll;
	graphics::ViewId m_viewId;
#if USE_ENTRY
	ImGuiKey m_keyMap[(int)entry::Key::Count];
#endif // USE_ENTRY
};

static OcornutImguiContext s_ctx;

static void* memAlloc(size_t _size, void* _userData)
{
	BASE_UNUSED(_userData);
	return base::alloc(s_ctx.m_allocator, _size);
}

static void memFree(void* _ptr, void* _userData)
{
	BASE_UNUSED(_userData);
	base::free(s_ctx.m_allocator, _ptr);
}

namespace mara 
{
	void imguiCreate(float _fontSize, base::AllocatorI* _allocator)
	{
		s_ctx.create(_fontSize, _allocator);
	}

	void imguiDestroy()
	{
		s_ctx.destroy();
	}

	void imguiBeginFrame(int _inputChar, graphics::ViewId _viewId)
	{
		const graphics::Stats* stats = graphics::getStats();
		const entry::MouseState* mouseState = mara::getMouseState();

		s_ctx.beginFrame(mouseState->m_mx
			, mouseState->m_my
			, (mouseState->m_buttons[entry::MouseButton::Left] ? IMGUI_MBUT_LEFT : 0)
			| (mouseState->m_buttons[entry::MouseButton::Right] ? IMGUI_MBUT_RIGHT : 0)
			| (mouseState->m_buttons[entry::MouseButton::Middle] ? IMGUI_MBUT_MIDDLE : 0)
			, mouseState->m_mz,
			stats->width,
			stats->height,
			_inputChar, _viewId);
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

BASE_PRAGMA_DIAGNOSTIC_IGNORED_MSVC(4505); // error C4505: '' : unreferenced local function has been removed
BASE_PRAGMA_DIAGNOSTIC_IGNORED_CLANG_GCC("-Wunused-function"); // warning: 'int rect_width_compare(const void*, const void*)' defined but not used
BASE_PRAGMA_DIAGNOSTIC_PUSH();
BASE_PRAGMA_DIAGNOSTIC_IGNORED_CLANG("-Wunknown-pragmas")
BASE_PRAGMA_DIAGNOSTIC_IGNORED_CLANG_GCC("-Wtype-limits"); // warning: comparison is always true due to limited range of data type
#define STBTT_malloc(_size, _userData) memAlloc(_size, _userData)
#define STBTT_free(_ptr, _userData) memFree(_ptr, _userData)
#define STB_RECT_PACK_IMPLEMENTATION
//#include <stb/stb_rect_pack.h>
#define STB_TRUETYPE_IMPLEMENTATION
//#include <stb/stb_truetype.h>
BASE_PRAGMA_DIAGNOSTIC_POP();
