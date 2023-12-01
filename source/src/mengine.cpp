#include "mengine/mengine.h"
#include "mengine_p.h"

#include <mapp/commandline.h>
#include <mapp/allocator.h>
#include <mapp/endian.h>
#include <mapp/math.h>
#include <mapp/readerwriter.h>
#include <mapp/string.h>

namespace mengine {

	bx::AllocatorI* g_allocator = NULL;

	static Context* s_ctx = NULL;

	bool Context::init(const Init& _init)
	{
		bgfx::Init bgfxInit;
		bgfxInit.type = _init.graphicsApi;
		bgfxInit.vendorId = _init.vendorId;
		bgfxInit.resolution = _init.resolution;
		bgfxInit.platformData.nwh = mrender::getNativeWindowHandle(mrender::kDefaultWindowHandle);
		bgfxInit.platformData.ndt = mrender::getNativeDisplayHandle();
		bgfxInit.allocator = mrender::getAllocator();
		if (bgfx::init(bgfxInit))
		{
			bgfx::setViewRect(0, 0, 0, U16(_init.resolution.width), U16(_init.resolution.height));
			bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x000000FF, 1.0f, 0);
			bgfx::touch(0);

			return true;
		}

		return false;
	}

	void Context::shutdown()
	{
		bgfx::shutdown();
	}

	bool Context::update(U32 _debug, U32 _reset)
	{
		bgfx::setDebug(_debug);

		U32 width, height;
		if (!mrender::processEvents(width, height, _debug, _reset, &m_mouseState))
		{
			bgfx::setViewRect(0, 0, 0, U16(width), U16(height));

			for (uint16_t ii = 0, num = m_freeGeometryAssets.getNumQueued(); ii < num; ++ii)
			{
				m_geometryAssetHandle.free(m_freeGeometryAssets.get(ii).idx);
			}

			for (uint16_t ii = 0, num = m_freeShaderAssets.getNumQueued(); ii < num; ++ii)
			{
				m_shaderAssetHandle.free(m_freeShaderAssets.get(ii).idx);
			}

			m_freeGeometryAssets.reset();
			m_freeShaderAssets.reset();

			return true;
		}

		return false;
	}

	void Context::release(const bgfx::Memory* _mem)
	{
		BX_ASSERT(NULL != _mem, "_mem can't be NULL");

		bgfx::Memory* mem = const_cast<bgfx::Memory*>(_mem);
		bx::free(mrender::getAllocator(), mem);
	}

	Init::Init()
		: allocator(NULL)
		, graphicsApi(bgfx::RendererType::Count)
		, vendorId(BGFX_PCI_ID_NONE)
	{

	}

	bool init(const Init& _init)
	{
		if (NULL != s_ctx)
		{
			BX_TRACE("mengine is already initialized.");
			return false;
		}
		Init init = _init;

		if (NULL != init.allocator)
		{
			g_allocator = init.allocator;
		}
		else
		{
			bx::DefaultAllocator allocator;
			g_allocator = BX_NEW(&allocator, bx::DefaultAllocator);
		}

		BX_TRACE("Init...")

		// mengine 1.104.7082
		//      ^ ^^^ ^^^^
		//      | |   +--- Commit number  (https://github.com/marcusmadland/mengine / git rev-list --count HEAD)
		//      | +------- API version    (from https://github.com/marcusmadland/mengine/blob/master/scripts/mengine.idl#L4)
		//      +--------- Major revision (always 1)
		BX_TRACE("Version 1.%d.%d (commit: " MENGINE_REV_SHA1 ")", MENGINE_API_VERSION, MENGINE_REV_NUMBER)

		s_ctx = BX_ALIGNED_NEW(g_allocator, Context, Context::kAlignment);
		if (s_ctx->init(init))
		{
			BX_TRACE("Init complete.");
			return true;
		}

		BX_TRACE("Init failed.");
		return false;
	}

	void shutdown()
	{
		if (NULL != s_ctx)
		{
			s_ctx->shutdown();
		}
	}

	bool update(U32 _debug, U32 _reset)
	{
		if (NULL != s_ctx)
		{
			return s_ctx->update(_debug, _reset);
		}

		BX_TRACE("Calling update before initializing.");
		return false;
	}

	ComponentHandle createComponent(void* _data, U32 _size)
	{
		return s_ctx->createComponent(_data, _size);
	}

	void destroy(ComponentHandle _handle)
	{
		s_ctx->destroyComponent(_handle);
	}

	void addComponent(EntityHandle _entity, U32 _type, ComponentHandle _component)
	{
		s_ctx->addComponent(_entity, _type, _component);
	}

	void* getComponentData(EntityHandle _entity, U32 _type)
	{
		return s_ctx->getComponentData(_entity, _type);
	}

	EntityQuery* queryEntities(U32 _types)
	{
		return s_ctx->queryEntities(_types);
	}

	EntityHandle createEntity()
	{
		return s_ctx->createEntity();
	}

	void destroy(EntityHandle _handle)
	{
		s_ctx->destroyEntity(_handle);
	}

	bool packAssets(const bx::FilePath& _filePath)
	{
		return s_ctx->packAssets(_filePath);
	}

	bool loadAssetPack(const bx::FilePath& _filePath)
	{
		return s_ctx->loadAssetPack(_filePath);
	}

	GeometryAssetHandle createGeometry(const void* _vertices, U32 _verticesSize, const void* _indices, U32 _indicesSize, bgfx::VertexLayout _layout, const bx::FilePath _virtualPath)
	{
		if (NULL != _vertices && NULL != _indices)
		{
			return s_ctx->createGeometry(_vertices, _verticesSize, _indices, _indicesSize, _layout, _virtualPath);
		}

		BX_TRACE("Data is null.");
		return MENGINE_INVALID_HANDLE;
	}

	GeometryAssetHandle loadGeometry(const bx::FilePath _filePath)
	{
		if (!_filePath.isEmpty())
		{
			return s_ctx->loadGeometry(_filePath);
		}

		BX_TRACE("Filepath is empty.");
		return MENGINE_INVALID_HANDLE;
	}

	void destroy(GeometryAssetHandle _handle)
	{
		s_ctx->destroyGeometry(_handle);
	}

	ShaderAssetHandle createShader(const bgfx::Memory* _mem, const bx::FilePath _virtualPath)
	{
		if (NULL != _mem)
		{
			return s_ctx->createShader(_mem, _virtualPath);
		}

		BX_TRACE("Data is null.");
		return MENGINE_INVALID_HANDLE;
	}

	ShaderAssetHandle loadShader(const bx::FilePath _filePath)
	{
		if (!_filePath.isEmpty())
		{
			return s_ctx->loadShader(_filePath);
		}

		BX_TRACE("Filepath is empty.");
		return MENGINE_INVALID_HANDLE;
	}

	void destroy(ShaderAssetHandle _handle)
	{
		s_ctx->destroyShader(_handle);
	}

	const bgfx::Memory* compileShader(const char* _shaderCode, ShaderType::Enum _type)
	{
		if (NULL != _shaderCode)
		{
			return s_ctx->compileShader(_shaderCode, _type);
		}

		BX_TRACE("Data is null.");
		return NULL;
	}

	const mrender::MouseState* getMouseState()
	{
		return s_ctx->getMouseState();
	}

	const Stats* getStats()
	{
		return s_ctx->getStats();
	}

	bx::AllocatorI* getAllocator()
	{
		return g_allocator;
	}

} // namespace mengine

namespace bgfx {

	ProgramHandle createProgram(mengine::ShaderAssetHandle _vsah, mengine::ShaderAssetHandle _fsah)
	{
		if (!isValid(_vsah) || !isValid(_fsah))
		{
			BX_TRACE("Asset handle is invalid.");
			return BGFX_INVALID_HANDLE;
		}

		mengine::ShaderRef& vsr = mengine::s_ctx->m_shaderAssets[_vsah.idx];
		mengine::ShaderRef& fsr = mengine::s_ctx->m_shaderAssets[_fsah.idx];
		return bgfx::createProgram(vsr.m_sh, fsr.m_sh);
	}

	void setGeometry(mengine::GeometryAssetHandle _handle)
	{
		if (!isValid(_handle))
		{
			BX_TRACE("Asset handle is invalid.");
		}

		mengine::GeometryRef& sr = mengine::s_ctx->m_geometryAssets[_handle.idx];
		bgfx::setVertexBuffer(0, sr.m_vbh);
		bgfx::setIndexBuffer(sr.m_ibh);
	}
}