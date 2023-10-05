#include "mengine/mengine.h"
#include "mrender/entry.h"

namespace
{

class Game : public mrender::AppI
{
public:
	Game(const char* _name, const char* _description)
		: mrender::AppI(_name, _description)
	{
	}

	void init(I32 _argc, const char* const* _argv, U32 _width, U32 _height) override
	{
		m_width = _width;
		m_height = _height;
		m_debug = BGFX_DEBUG_TEXT;
		m_reset = BGFX_RESET_VSYNC;

		bgfx::Init init;
		init.type = bgfx::RendererType::OpenGL;
		init.resolution.width = _width;
		init.resolution.height = _height;
		init.platformData.nwh = mrender::getNativeWindowHandle(mrender::kDefaultWindowHandle);
		init.platformData.ndt = mrender::getNativeDisplayHandle();
		bgfx::init(init);

		bgfx::setDebug(BGFX_DEBUG_TEXT);

		bgfx::setViewClear(0
			, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH
			, 0x303030FF
			, 1.0f
			, 0
		);
	}

	int shutdown() override
	{
		bgfx::shutdown();

		return 0;
	}

	bool update() override
	{
		if (!mrender::processEvents(m_width, m_height, m_debug, m_reset, &m_mouseState))
		{
			bgfx::setViewRect(0, 0, 0, U16(m_width), U16(m_height));

			bgfx::touch(0);

			bgfx::dbgTextClear();
			bgfx::dbgTextPrintf(0, 1, 0x0f, "Hello world!");

			bgfx::frame();

			return true;
		}

		return false;
	}

	mrender::MouseState m_mouseState;

	U32 m_width;
	U32 m_height;
	U32 m_debug;
	U32 m_reset;
};

} // namespace

ENTRY_IMPLEMENT_MAIN(
	Game
	, "Game"
	, "An example of a game project"
);