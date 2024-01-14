#include <mara/mara.h>

namespace 
{
	class Empty : public entry::AppI
	{
	public:
		Empty(const char* _name, const char* _description)
			: entry::AppI(_name, _description)
		{
			entry::setWindowTitle(entry::kDefaultWindowHandle, _name);
		}

		void init(I32 _argc, const char* const* _argv, U32 _width, U32 _height) override
		{
			mara::Init maraInit;
			maraInit.resolution.width = _width;
			maraInit.resolution.height = _height;
			maraInit.graphicsApi = graphics::RendererType::Direct3D11;
			mara::init(maraInit);
		}

		I32 shutdown() override
		{
			mara::shutdown();
			return 0;
		}

		bool update() override
		{
			if (mara::update(GRAPHICS_DEBUG_TEXT, GRAPHICS_RESET_VSYNC))
			{
				graphics::frame();
				return true;
			}
			
			return false;
		}
	};

} // namespace

ENTRY_IMPLEMENT_MAIN(
	::Empty
	, "00 - Empty"
	, "An example of a empty project using mara engine"
)