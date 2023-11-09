#pragma once

#include <mapp/platform.h>

#ifndef BX_CONFIG_DEBUG
#	error "BX_CONFIG_DEBUG must be defined in build script!"
#endif // BX_CONFIG_DEBUG

#define MENGINE_CONFIG_DEBUG BX_CONFIG_DEBUG

#include "mengine/mengine.h"

namespace mengine {

	struct DataRef
	{
		void* m_data;
		U16 m_refCount;
	};

#if MENGINE_CONFIG_DEBUG
#	define MENGINE_API_FUNC(_func) BX_NO_INLINE _func
#else
#	define MENGINE_API_FUNC(_func) _func
#endif // BGFX_CONFIG_DEBUG

	struct Context
	{
		static constexpr uint32_t kAlignment = 64;

		Context()
		{}

		~Context()
		{}

		bool init(const Init& _init);
		void shutdown();

		MENGINE_API_FUNC(AssetHandle loadGeometry(const bx::FilePath& _filePath))
		{
			U16 idx = m_assetHashMap.find(bx::hash<bx::HashMurmur2A>(_filePath.getCPtr()));
			if (kInvalidHandle != idx)
			{
				AssetHandle handle = { idx };
				return handle;
			}

			AssetHandle handle = { m_assetHandle.alloc() };
			if (!isValid(handle))
			{
				BX_TRACE("Failed to allocate asset handle.");
				return MENGINE_INVALID_HANDLE;
			}
			
			bx::FileReader reader;
			if (!bx::open(&reader, _filePath, bx::ErrorAssert{}))
			{
				BX_TRACE("Failed to load asset at path %s.", _filePath.getCPtr());
				return MENGINE_INVALID_HANDLE;
			}

			GeometryData* geometryData = new GeometryData();

			U32 vertexMemorySize;
			bx::read(&reader, &vertexMemorySize, sizeof(vertexMemorySize), bx::ErrorAssert{});
			geometryData->m_vertexData = bgfx::alloc(vertexMemorySize);
			bx::read(&reader, geometryData->m_vertexData->data, vertexMemorySize, bx::ErrorAssert{});

			U32 indexMemorySize;
			bx::read(&reader, &indexMemorySize, sizeof(indexMemorySize), bx::ErrorAssert{});
			geometryData->m_indexData = bgfx::alloc(indexMemorySize);
			bx::read(&reader, geometryData->m_indexData->data, indexMemorySize, bx::ErrorAssert{});

			size_t layoutSize;
			bx::read(&reader, &layoutSize, sizeof(layoutSize), bx::ErrorAssert{});
			bx::read(&reader, &geometryData->m_layout, (I32)layoutSize, bx::ErrorAssert{});

			bx::close(&reader);

			if (NULL != geometryData)
			{
				m_assets[handle.idx].m_data = geometryData;
				m_assets[handle.idx].m_refCount = 1;

				m_assetHashMap.insert(bx::hash<bx::HashMurmur2A>(_filePath.getCPtr()), handle.idx);

				return handle;
			}

			BX_TRACE("Loaded asset at path %s contains invalid data.", _filePath.getCPtr());
			return MENGINE_INVALID_HANDLE;
		}

		MENGINE_API_FUNC(bool saveGeometry(GeometryData* _geometryData, const bx::FilePath& _filePath))
		{
			bx::makeAll(_filePath.getPath(), bx::ErrorAssert{});

			bx::FileWriter writer;
			if (!bx::open(&writer, _filePath, bx::ErrorAssert{}))
			{
				return false;
			}

			bx::write(&writer, &_geometryData->m_vertexData->size, sizeof(_geometryData->m_vertexData->size), bx::ErrorAssert{});
			bx::write(&writer, _geometryData->m_vertexData->data, (I32)_geometryData->m_vertexData->size, bx::ErrorAssert {});

			bx::write(&writer, &_geometryData->m_indexData->size, sizeof(_geometryData->m_indexData->size), bx::ErrorAssert{});
			bx::write(&writer, _geometryData->m_indexData->data, (I32)_geometryData->m_indexData->size, bx::ErrorAssert {});

			size_t layoutSize = sizeof(_geometryData->m_layout);
			bx::write(&writer, &layoutSize, sizeof(layoutSize), bx::ErrorAssert{});
			bx::write(&writer, &_geometryData->m_layout, (I32)layoutSize, bx::ErrorAssert{});

			bx::close(&writer);
			return true;
		}

		MENGINE_API_FUNC(AssetHandle loadShader(const bx::FilePath& _filePath))
		{
			U16 idx = m_assetHashMap.find(bx::hash<bx::HashMurmur2A>(_filePath.getCPtr()));
			if (kInvalidHandle != idx)
			{
				AssetHandle handle = { idx };
				return handle;
			}

			AssetHandle handle = { m_assetHandle.alloc() };
			if (!isValid(handle))
			{
				BX_TRACE("Failed to allocate asset handle.");
				return MENGINE_INVALID_HANDLE;
			}

			bx::FileReader reader;
			if (!bx::open(&reader, _filePath, bx::ErrorAssert{}))
			{
				BX_TRACE("Failed to load asset at path %s.", _filePath.getCPtr());
				return MENGINE_INVALID_HANDLE;
			}

			ShaderData* shaderData = new ShaderData();

			const I32 fileSize = bx::getSize(&reader);
			shaderData->m_codeData = bgfx::alloc(fileSize);
			bx::read(&reader, shaderData->m_codeData->data, fileSize, bx::ErrorAssert{});

			bx::close(&reader);

			if (NULL != shaderData)
			{
				m_assets[handle.idx].m_data = shaderData;
				m_assets[handle.idx].m_refCount = 1;

				m_assetHashMap.insert(bx::hash<bx::HashMurmur2A>(_filePath.getCPtr()), handle.idx);

				return handle;
			}

			BX_TRACE("Loaded asset at path %s contains invalid data.", _filePath.getCPtr());
			return MENGINE_INVALID_HANDLE;
		}

		MENGINE_API_FUNC(bool saveShader(ShaderData* _shaderData, const bx::FilePath& _filePath))
		{
			bx::makeAll(_filePath.getPath(), bx::ErrorAssert{});

			bx::FileWriter writer;
			if (!bx::open(&writer, _filePath, bx::ErrorAssert{}))
			{
				return false;
			}
			
			bx::write(&writer, _shaderData->m_codeData->data, (I32)_shaderData->m_codeData->size, bx::ErrorAssert {});

			bx::close(&writer);
			return false; 
		}

		bx::HandleAllocT<MENGINE_CONFIG_MAX_ASSETS> m_assetHandle;
		typedef bx::HandleHashMapT<MENGINE_CONFIG_MAX_ASSETS> AssetHashMap;
		AssetHashMap m_assetHashMap;
		DataRef m_assets[MENGINE_CONFIG_MAX_ASSETS];
	};

} // namespace mengine