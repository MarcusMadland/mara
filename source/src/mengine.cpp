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
		return true;
	}

	void Context::shutdown()
	{
	}

	Init::Init()
		: allocator(NULL)
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

	bool shutdown()
	{
		return false;
	}

	AssetHandle loadGeometry(const bx::FilePath _filePath)
	{
		if (_filePath.isEmpty())
		{
			BX_TRACE("Filepath is empty.");
			return MENGINE_INVALID_HANDLE;
		}
		return s_ctx->loadGeometry(_filePath);
	}

	bool saveGeometry(GeometryData* _geometryData, const bx::FilePath _filePath)
	{
		if (_filePath.isEmpty())
		{
			BX_TRACE("Filepath is empty.");
			return false;
		}

		if (NULL != _geometryData)
		{
			return s_ctx->saveGeometry(_geometryData, _filePath);
		}
		
		BX_TRACE("Geometrydata is null.");
		return false;
	}

	AssetHandle loadShader(const bx::FilePath _filePath)
	{
		if (_filePath.isEmpty())
		{
			BX_TRACE("Filepath is empty.");
			return MENGINE_INVALID_HANDLE;
		}

		return s_ctx->loadShader(_filePath);
	}

	bool saveShader(ShaderData* _shaderData, const bx::FilePath _filePath)
	{
		if (_filePath.isEmpty())
		{
			BX_TRACE("Filepath is empty.");
			return false;
		}

		if (NULL != _shaderData)
		{
			return s_ctx->saveShader(_shaderData, _filePath);
		}

		BX_TRACE("Shaderdata is null.");
		return false;
	}

} // namespace mengine

namespace bgfx {

	ShaderHandle createShader(mengine::AssetHandle _handle)
	{
		if (!isValid(_handle))
		{
			BX_TRACE("Asset handle is invalid.");
			return BGFX_INVALID_HANDLE;
		}

		mengine::ShaderData* data = (mengine::ShaderData*)mengine::s_ctx->m_assets[_handle.idx].m_data;
		if (data)
		{
			return bgfx::createShader(data->m_codeData);
		}

		BX_TRACE("Asset data is invalid.");
		return BGFX_INVALID_HANDLE;
	}

	VertexBufferHandle createVertexBuffer(mengine::AssetHandle _handle, uint16_t _flags)
	{
		if (!isValid(_handle))
		{
			BX_TRACE("Asset handle is invalid.");
			return BGFX_INVALID_HANDLE;
		}

		mengine::GeometryData* data = (mengine::GeometryData*)mengine::s_ctx->m_assets[_handle.idx].m_data;
		if (data)
		{
			return bgfx::createVertexBuffer(data->m_vertexData, data->m_layout, _flags);
		}

		BX_TRACE("Asset data is invalid.");
		return BGFX_INVALID_HANDLE;
	}

	IndexBufferHandle createIndexBuffer(mengine::AssetHandle _handle, uint16_t _flags)
	{
		if (!isValid(_handle))
		{
			BX_TRACE("Asset handle is invalid.");
			return BGFX_INVALID_HANDLE;
		}

		mengine::GeometryData* data = (mengine::GeometryData*)mengine::s_ctx->m_assets[_handle.idx].m_data;
		if (data)
		{
			return bgfx::createIndexBuffer(data->m_indexData, _flags);
		}

		BX_TRACE("Asset data is invalid.");
		return BGFX_INVALID_HANDLE;
	}

	TextureHandle createTexture2D(uint16_t _width, uint64_t _flags)
	{
		return TextureHandle();
	}

	TextureHandle createTexture2D(mengine::AssetHandle _handle, uint64_t _flags)
	{
		if (!isValid(_handle))
		{
			BX_TRACE("Asset handle is invalid.");
			return BGFX_INVALID_HANDLE;
		}

		mengine::TextureData* data = (mengine::TextureData*)mengine::s_ctx->m_assets[_handle.idx].m_data;
		if (data)
		{
			return bgfx::createTexture2D(data->m_width, data->m_height, data->m_hasMips, data->m_numLayers, data->m_format, _flags, data->m_pixData);
		}

		BX_TRACE("Asset data is invalid.");
		return BGFX_INVALID_HANDLE;
	}

}