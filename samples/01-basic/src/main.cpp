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

			mara::ResourceHandle shaderVert;
			{
				// Create resource
				mara::ShaderCreate create;
				create.mem = graphics::compileShader(0, NULL);
				shaderVert = mara::createResource(create, "vert.shader");
			}

			mara::ResourceHandle shaderFrag;
			{
				// Create resource
				mara::ShaderCreate create;
				create.mem = graphics::compileShader(0, NULL);
				shaderFrag = mara::createResource(create, "frag.shader");
			}

			mara::ResourceHandle material;
			{
				// Material paramters
				mara::MaterialParameters parameters;
				F32 color[4] = { 1.0f, 0.0f, 0.0f, 1.0f };
				parameters.addVec4("u_color", color);

				// Create resource
				mara::MaterialCreate create;
				create.vertShaderPath = "vert.shader";
				create.fragShaderPath = "frag.shader";
				create.parameters = parameters;
				material = mara::createResource(create, "red.mat");
			}

			mara::ResourceHandle geometry;
			{
				// Geometry data
				struct Vertex
				{
					F32 x;
					F32 y;
					F32 z;
				};
				Vertex vertices[] = { { 0.0f,0.0f,0.0f } };
				U16 indices[] = { 0,1,2 };

				// Geometry layout
				graphics::VertexLayout layout;
				layout.begin()
					.add(graphics::Attrib::Position, 3, graphics::AttribType::Float)
					.end();
				
				// Create resource
				mara::GeometryCreate create;
				create.vertices = vertices;
				create.verticesSize = sizeof(Vertex) * 1;
				create.indices = indices;
				create.indicesSize = sizeof(U16) * 3;
				create.layout = layout;
				geometry = mara::createResource(create, "cube.geom");
			}
			
			mara::MeshCreate create;
			create.geometryPath = "cube.geom";
			create.materialPath = "red.mat";
			base::mtxIdentity(create.m_transform);
			m_mesh = mara::createMesh(mara::createResource(create, "cube.mesh"));
		}

		I32 shutdown() override
		{
			mara::destroy(m_mesh);

			mara::shutdown();
			return 0;
		}

		bool update() override
		{
			if (mara::update(GRAPHICS_DEBUG_TEXT, GRAPHICS_RESET_VSYNC))
			{
				graphics::submit(0, m_mesh);

				graphics::frame();
				return true;
			}
			
			return false;
		}

		mara::MeshHandle m_mesh;
	};

} // namespace

ENTRY_IMPLEMENT_MAIN(
	::Empty
	, "00 - Empty"
	, "An example of a empty project using mara engine"
)