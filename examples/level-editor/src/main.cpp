#include "mengine/mengine.h"
#include "imgui/imgui.h"

namespace le
{

class LevelEditor : public mrender::AppI
{
public:
	LevelEditor(const char* _name, const char* _description)
		: mrender::AppI(_name, _description)
		, m_width(1280)
		, m_height(720)
		, m_debug(BGFX_DEBUG_NONE)
		, m_reset(BGFX_RESET_NONE)
	{}

	void init(I32 _argc, const char* const* _argv, U32 _width, U32 _height) override
	{
		// Members
		m_width = _width;
		m_height = _height;
		m_debug = BGFX_DEBUG_TEXT;
		m_reset = BGFX_RESET_VSYNC;

		// Create argument context
		mengine::Args args = mengine::Args(_argc, _argv);

		// Create renderer backend
		bgfx::Init init;
		init.type = args.m_type;
		init.vendorId = args.m_pciId;
		init.resolution.width = _width;
		init.resolution.height = _height;
		init.platformData.nwh = mrender::getNativeWindowHandle(mrender::kDefaultWindowHandle);
		init.platformData.ndt = mrender::getNativeDisplayHandle();
		bgfx::init(init);

		// Render commands
		bgfx::setViewClear(0
			, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH
			, 0x202020FF
			, 1.0f
			, 0
		);

		// Create imgui context
		mengine::imguiCreate();
	}

	int shutdown() override
	{
		mengine::imguiDestroy();
		bgfx::shutdown();

		return 0;
	}

	bool update() override
	{
		if (!mrender::processWindowEvents(m_windowState, m_debug, m_reset))
		{
			if (strcmp(m_windowState.m_dropFile.getCPtr(), ".") != 0)
			{
				bx::debugPrintf("pathtest %s\n", m_windowState.m_dropFile.getCPtr());
				m_windowState.m_dropFile.clear();
			}
		}

		if (!mrender::processEvents(m_width, m_height, m_debug, m_reset, &m_mouseState))
		{
			mengine::imguiBeginFrame(m_mouseState.m_mx
			                         , m_mouseState.m_my
			                         , (m_mouseState.m_buttons[mrender::MouseButton::Left] ? IMGUI_MBUT_LEFT : 0)
			                         | (m_mouseState.m_buttons[mrender::MouseButton::Right] ? IMGUI_MBUT_RIGHT : 0)
			                         | (m_mouseState.m_buttons[mrender::MouseButton::Middle] ? IMGUI_MBUT_MIDDLE : 0)
			                         , m_mouseState.m_mz
			                         , U16(m_width)
			                         , U16(m_height)
			);
			imguiRender();
			mengine::imguiEndFrame();

			bgfx::setViewRect(0, 0, 0, U16(m_width), U16(m_height));
			bgfx::touch(0);
			bgfx::frame();

			return true;
		}
		

		return false;
	}

private:
	void imguiRender()
	{
		ImGui::SetNextWindowPos({ 0.0f, 0.0f });
		ImGui::SetNextWindowSize({ (float)m_width, (float)m_height });
		if (ImGui::Begin("##unique_id", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoMove))
		{
			ImGui::Image(0, { (float)m_width, (float)m_height});
			if (ImGui::BeginDragDropTarget())
			{
				const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("AM_OUTPUT_PATH", ImGuiDragDropFlags_SourceExtern);
				const char* path = (const char*)payload->Data;
				bx::debugPrintf("%s\n", path);
				ImGui::EndDragDropTarget();
			}
		}
		ImGui::End();
	}

private:
	U32 m_width;
	U32 m_height;
	U32 m_debug;
	U32 m_reset;
	mrender::MouseState m_mouseState;
	mrender::WindowState m_windowState;
};

} // namespace le

ENTRY_IMPLEMENT_MAIN(
	le::LevelEditor
	, "Level Editor"
	, "An example of a level editor"
);
