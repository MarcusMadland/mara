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

		// Offline
#if false
		mengine::GeometryAsset cube = mengine::GeometryAsset(
			bgfx::makeRef(cube_vertices, sizeof(PosColorVertex) * 8),
			bgfx::makeRef(cube_tri_list, sizeof(U16) * 36));
		cube.serialize("meshes/my_cube.bin");
#endif

		// Other
		m_timeOffset = bx::getHPCounter();

		// Renderer
		bgfx::setDebug(BGFX_DEBUG_STATS);
		bgfx::setViewClear(0
			, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH
			, 0x303030FF
			, 1.0f
			, 0
		);

		// Program
		mengine::ShaderAsset* vertexShader = new mengine::ShaderAsset();
		mengine::ShaderAsset* fragmentShader = new mengine::ShaderAsset();
		if (vertexShader->deserialize("shaders/glsl/vs_mesh.bin") &&
			fragmentShader->deserialize("shaders/glsl/fs_mesh.bin"))
		{
			bgfx::ShaderHandle vsh = bgfx::createShader(vertexShader->m_memory);
			bgfx::ShaderHandle fsh = bgfx::createShader(fragmentShader->m_memory);
			m_program = bgfx::createProgram(vsh, fsh, true);
		}

		// Mesh
		mengine::GeometryAsset* geom = new mengine::GeometryAsset();
		if (geom->deserialize("meshes/my_cube.bin"))
		{
			bgfx::VertexLayout layout;
			layout.begin()
				.add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
				.add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true)
				.end();

			m_vbh = bgfx::createVertexBuffer(geom->m_vertexMemory, layout);
			m_ibh = bgfx::createIndexBuffer(geom->m_indexMemory);
		}
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

			// Time
			F32 time = (F32)((bx::getHPCounter() - m_timeOffset) / F64(bx::getHPFrequency()));

			// Debug
			bgfx::dbgTextClear();
			bgfx::dbgTextPrintf(2, 1, 0x0f, "Hello world!");

			// Camera
			const bx::Vec3 at = { 0.0f, 0.0f,  0.0f };
			const bx::Vec3 eye = { 0.0f, 2.0f, -5.0f };
			{
				float view[16];
				bx::mtxLookAt(view, eye, at);

				float proj[16];
				bx::mtxProj(proj, 60.0f, F32(m_width) / F32(m_height), 0.1f, 100.0f, bgfx::getCaps()->homogeneousDepth);
				bgfx::setViewTransform(0, view, proj);

				bgfx::setViewRect(0, 0, 0, U16(m_width), U16(m_height));
			}

			// Mesh
			F32 mtx[16];
			bx::mtxRotateXY(mtx
				, 0.0f
				, time * 0.37f
			);
			bgfx::setTransform(mtx);
			
			bgfx::setVertexBuffer(0, m_vbh);
			bgfx::setIndexBuffer(m_ibh);

			bgfx::submit(0, m_program);

			// Swap buffers
			bgfx::frame();

			return true;
		}

		return false;
	}

	I64 m_timeOffset;

	mrender::MouseState m_mouseState;

	U32 m_width;
	U32 m_height;
	U32 m_debug;
	U32 m_reset;

	bgfx::ProgramHandle m_program;
	bgfx::VertexBufferHandle m_vbh;
	bgfx::IndexBufferHandle m_ibh;
};

} // namespace

ENTRY_IMPLEMENT_MAIN(
	Game
	, "Game"
	, "An example of a game project"
);