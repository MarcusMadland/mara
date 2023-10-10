#include "mengine/mengine.h"

namespace
{

struct PosColorVertex
{
	float x;
	float y;
	float z;
	uint32_t abgr;
};

static PosColorVertex cube_vertices[] = {
	{-1.0f, 1.0f, 1.0f, 0xff000000},   {1.0f, 1.0f, 1.0f, 0xff0000ff},
	{-1.0f, -1.0f, 1.0f, 0xff00ff00},  {1.0f, -1.0f, 1.0f, 0xff00ffff},
	{-1.0f, 1.0f, -1.0f, 0xffff0000},  {1.0f, 1.0f, -1.0f, 0xffff00ff},
	{-1.0f, -1.0f, -1.0f, 0xffffff00}, {1.0f, -1.0f, -1.0f, 0xffffffff},
};

static const U16 cube_tri_list[] = {
	0, 1, 2, 1, 3, 2, 4, 6, 5, 5, 6, 7, 0, 2, 4, 4, 2, 6,
	1, 5, 3, 5, 7, 3, 0, 4, 1, 4, 5, 1, 2, 3, 6, 6, 3, 7,
};

class Game : public mrender::AppI
{
public:
	Game(const char* _name, const char* _description)
		: mrender::AppI(_name, _description)
	{}

	void init(I32 _argc, const char* const* _argv, U32 _width, U32 _height) override
	{
		mengine::Args args(_argc, _argv);

		m_width = _width;
		m_height = _height;
		m_debug = BGFX_DEBUG_TEXT;
		m_reset = BGFX_RESET_VSYNC;

		bgfx::Init init;
		init.type = args.m_type;
		init.vendorId = args.m_pciId;
		init.type = bgfx::RendererType::OpenGL; // @todo temp
		init.resolution.width = _width;
		init.resolution.height = _height;
		init.platformData.nwh = mrender::getNativeWindowHandle(mrender::kDefaultWindowHandle);
		init.platformData.ndt = mrender::getNativeDisplayHandle();
		bgfx::init(init);

		// Renderer
		bgfx::setViewClear(0
			, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH
			, 0xFF00FFFF
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