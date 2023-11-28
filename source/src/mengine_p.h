#pragma once

#include <mapp/platform.h>

#ifndef BX_CONFIG_DEBUG
#	error "BX_CONFIG_DEBUG must be defined in build script!"
#endif // BX_CONFIG_DEBUG

#define MENGINE_CONFIG_DEBUG BX_CONFIG_DEBUG

#include "mengine/mengine.h"

#include <vector> // @todo I don't like this, remove std dependencies 

#if MENGINE_CONFIG_MULTITHREADED
#	define MENGINE_MUTEX_SCOPE(_mutex) bx::MutexScope BX_CONCATENATE(mutexScope, __LINE__)(_mutex)
#else
#	define MENGINE_MUTEX_SCOPE(_mutex) BX_NOOP()
#endif // MENGINE_CONFIG_MULTITHREADED

#define MENGINE_ASSET_TYPE_NONE        UINT64_C(0x0000000000000000) 
#define MENGINE_ASSET_TYPE_GEOMETRY    UINT64_C(0x0000000000000001) 
#define MENGINE_ASSET_TYPE_SHADER      UINT64_C(0x0000000000000002) 

#define MENGINE_ASSET_PACK_VERSION 1
#define MENGINE_CHUNK_MAGIC_MAP BX_MAKEFOURCC('M', 'A', 'P', MENGINE_ASSET_PACK_VERSION)

#define BGFX_SHADER_BIN_VERSION 11
#define BGFX_CHUNK_MAGIC_CSH BX_MAKEFOURCC('C', 'S', 'H', BGFX_SHADER_BIN_VERSION)
#define BGFX_CHUNK_MAGIC_FSH BX_MAKEFOURCC('F', 'S', 'H', BGFX_SHADER_BIN_VERSION)
#define BGFX_CHUNK_MAGIC_VSH BX_MAKEFOURCC('V', 'S', 'H', BGFX_SHADER_BIN_VERSION)

namespace mengine {

	struct EntityRef
	{
		U32 m_mask;
		U16 m_refCount;
	};

	struct ComponentRef
	{
		const bgfx::Memory* m_data;
		U16 m_refCount;
	};

	struct SerializerI
	{
		virtual U32 getSize() = 0;

		virtual U32 write(bx::WriterI* _writer, bx::Error* _err) = 0;
		virtual void read(bx::ReaderSeekerI* _reader, bx::Error* _err) = 0;
	};

	struct GeometryRef : SerializerI
	{
		GeometryRef()
			: m_vbh(MENGINE_INVALID_HANDLE)
			, m_ibh(MENGINE_INVALID_HANDLE)
			, m_vertexData(NULL)
			, m_indexData(NULL)
			, m_refCount(0)
		{}

		U32 getSize() override
		{
			return sizeof(U32) + m_vertexData->size + sizeof(U32) + m_indexData->size + sizeof(U32) + (I32)sizeof(m_layout);
		};

		U32 write(bx::WriterI* _writer, bx::Error* _err) override
		{
			U32 layoutSize = sizeof(m_layout);

			return bx::write(_writer, &m_vertexData->size, sizeof(U32), _err) + 
			bx::write(_writer, m_vertexData->data, (I32)m_vertexData->size, _err) +
			bx::write(_writer, &m_indexData->size, sizeof(U32), _err) +
			bx::write(_writer, m_indexData->data, (I32)m_indexData->size, _err) +
			bx::write(_writer, &layoutSize, sizeof(U32), _err) +
			bx::write(_writer, &m_layout, (I32)layoutSize, _err);
		};

		void read(bx::ReaderSeekerI* _reader, bx::Error* _err) override
		{
			uint32_t vertexDataSize;
			bx::read(_reader, &vertexDataSize, sizeof(vertexDataSize), _err);
			m_vertexData = bgfx::alloc(vertexDataSize);
			bx::read(_reader, m_vertexData->data, vertexDataSize, _err);

			uint32_t indexDataSize;
			bx::read(_reader, &indexDataSize, sizeof(indexDataSize), _err);
			m_indexData = bgfx::alloc(indexDataSize);
			bx::read(_reader, m_indexData->data, indexDataSize, _err);

			U32 layoutSize;
			bx::read(_reader, &layoutSize, sizeof(layoutSize), _err);
			bx::read(_reader, &m_layout, (I32)layoutSize, _err);
		};

		bgfx::VertexBufferHandle m_vbh;
		bgfx::IndexBufferHandle m_ibh;

		const bgfx::Memory* m_vertexData;
		const bgfx::Memory* m_indexData;
		bgfx::VertexLayout m_layout;

		U16 m_refCount;
	};

	struct ShaderRef : SerializerI
	{
		ShaderRef()
			: m_sh(MENGINE_INVALID_HANDLE)
			, m_codeData(NULL)
			, m_refCount(0)
		{}

		U32 getSize() override
		{
			return m_codeData->size;
		};

		U32 write(bx::WriterI* _writer, bx::Error* _err) override
		{ 
			return bx::write(_writer, &m_codeData->size, sizeof(U32), _err) +
				bx::write(_writer, m_codeData->data, m_codeData->size, _err);
		};

		void read(bx::ReaderSeekerI* _reader, bx::Error* _err) override
		{ 
			U32 size;
			bx::read(_reader, &size, sizeof(size), _err);
			m_codeData = bgfx::alloc(size);
			bx::read(_reader, m_codeData->data, size, _err);
		};

		bgfx::ShaderHandle m_sh;

		const bgfx::Memory* m_codeData;

		U16 m_refCount;
	};

	/*
	struct TextureSerializer : SerializeI
	{
		bool serialize(const bx::FilePath& _filePath) override { return false; };
		bool deserialize(const bx::FilePath& _filePath) override { return false; };

		const bgfx::Memory* m_pixData;
		U16 m_width;
		U16 m_height;
		bool m_hasMips;
		U16 m_numLayers;
		bgfx::TextureFormat::Enum m_format;
		U64 m_flags;
	};

	struct MaterialSerializer : SerializeI
	{
		bool serialize(const bx::FilePath& _filePath) override { return false; };
		bool deserialize(const bx::FilePath& _filePath) override { return false; };

		U32 m_shader;
		U32* m_textures;
		U32 m_numTextures;
		// @todo uniform data
	};

	struct MeshSerializer : SerializeI
	{
		bool serialize(const bx::FilePath& _filePath) override { return false; };
		bool deserialize(const bx::FilePath& _filePath) override { return false; };

		U32 m_geometries;
		U32 m_material;
		float m_transform[16];
	};

	struct PrefabSerializer : SerializeI
	{
		bool serialize(const bx::FilePath& _filePath) override { return false; };
		bool deserialize(const bx::FilePath& _filePath) override { return false; };

		U32* m_meshes;
		U32 m_numMeshes;
	};*/

	struct Uniform
	{
		Uniform()
			: name("")
			, type(bgfx::UniformType::Count)
			, num(0)
			, regIndex(0)
			, regCount(0)
			, texComponent(0)
			, texDimension(0)
			, texFormat(0)
		{}

		const char* name;
		bgfx::UniformType::Enum type;
		U8 num;
		U16 regIndex;
		U16 regCount;
		U8 texComponent;
		U8 texDimension;
		U16 texFormat;
	};

	struct AssetPackHeader
	{
		AssetPackHeader()
			: magic(0)
			, hash(0)
			, numEntries(0)
		{}

		U32 magic;
		U32 hash;
		U32 numEntries;
	};

	struct AssetPackEntry
	{
		AssetPackEntry()
			: type(MENGINE_ASSET_TYPE_NONE)
			, hash(0)
			, offset(0)
		{}

		U64 type;
		U32 hash;
		I64 offset;
	};

#if MENGINE_CONFIG_DEBUG
#	define MENGINE_API_FUNC(_func) BX_NO_INLINE _func
#else
#	define MENGINE_API_FUNC(_func) _func
#endif // BGFX_CONFIG_DEBUG

	struct Context
	{
		static constexpr U32 kAlignment = 64;

		Context()
			: m_frameNum(0)
			, m_stats(mengine::Stats())
		{}

		~Context()
		{}

		bool init(const Init& _init);
		void shutdown();
		bool update();

		void componentIncRef(ComponentHandle _handle)
		{
			ComponentRef& sr = m_components[_handle.idx];
			++sr.m_refCount;
		}

		void componentDecRef(ComponentHandle _handle)
		{
			ComponentRef& sr = m_components[_handle.idx];
			U16 refs = --sr.m_refCount;

			if (0 == refs)
			{
				bool ok = m_freeComponents.queue(_handle); BX_UNUSED(ok);
				BX_ASSERT(ok, "Component handle %d is already destroyed!", _handle.idx);

				release(sr.m_data);
				sr.m_data = NULL;
			}
		}

		MENGINE_API_FUNC(ComponentHandle createComponent(void* _data, U32 _size))
		{
			ComponentHandle handle = { m_componentHandle.alloc() };

			if (isValid(handle))
			{
				ComponentRef& sr = m_components[handle.idx];
				sr.m_refCount = 1;

				sr.m_data = bgfx::makeRef(_data, _size);
				
				return handle;
			}

			BX_TRACE("Failed to create component handle.");
			return MENGINE_INVALID_HANDLE;
		}

		MENGINE_API_FUNC(void destroyComponent(ComponentHandle _handle))
		{
			MENGINE_MUTEX_SCOPE(m_resourceApiLock);

			if (!isValid(_handle) && !m_componentHandle.isValid(_handle.idx))
			{
				BX_WARN(false, "Passing invalid component handle to mengine::destroyComponent.");
				return;
			}

			componentDecRef(_handle);
		}

		MENGINE_API_FUNC(void addComponent(EntityHandle _entity, U32 _type, ComponentHandle _component))
		{
			bool ok = m_ecsHashMap[_type].insert(_entity.idx, _component.idx);
			BX_ASSERT(ok, "Entities cannot have duplicated components!", _entity.idx);

			EntityRef& sr = m_entities[_entity.idx];
			sr.m_mask |= _type;
		}

		bool hasComponent(EntityHandle _handle, U32 _type)
		{
			const U16 idx = m_ecsHashMap[_type].find(_handle.idx);
			return idx != kInvalidHandle;
		}

		MENGINE_API_FUNC(void* getComponentData(EntityHandle _handle, U32 _type))
		{
			const U16 idx = m_ecsHashMap[_type].find(_handle.idx);
			if (idx != kInvalidHandle)
			{
				return m_components[idx].m_data->data;
			}
			return NULL;
		}

		MENGINE_API_FUNC(void forEachComponent(U32 _types, SystemFn _systemFn))
		{
			for (U32 i = 0; i < MENGINE_CONFIG_MAX_COMPONENT_TYPES; ++i)
			{
				const auto& typeHashMap = m_ecsHashMap[i];

				for (U16 j = 0; j < typeHashMap.getNumElements(); j++) 
				{
					EntityHandle handle = { j }; // @todo is this correct?

					EntityRef& sr = m_entities[j];
					U32 componentMask = sr.m_mask; 
					if ((componentMask & _types) == _types)
					{
						_systemFn(handle);
					}
				}
			}
		}

		void entityIncRef(EntityHandle _handle)
		{
			EntityRef& sr = m_entities[_handle.idx];
			++sr.m_refCount;
		}

		void entityDecRef(EntityHandle _handle)
		{
			EntityRef& sr = m_entities[_handle.idx];
			U16 refs = --sr.m_refCount;

			if (0 == refs)
			{
				bool ok = m_freeEntities.queue(_handle); BX_UNUSED(ok);
				BX_ASSERT(ok, "Entity handle %d is already destroyed!", _handle.idx);

				sr.m_mask = 0;
			}
		}

		MENGINE_API_FUNC(EntityHandle createEntity())
		{
			EntityHandle handle = { m_entityHandle.alloc() };

			if (isValid(handle))
			{
				EntityRef& sr = m_entities[handle.idx];
				sr.m_refCount = 1;

				return handle;
			}

			BX_TRACE("Failed to create entity handle.");
			return MENGINE_INVALID_HANDLE;
		}

		MENGINE_API_FUNC(void destroyEntity(EntityHandle _handle))
		{
			MENGINE_MUTEX_SCOPE(m_resourceApiLock);

			if (!isValid(_handle) && !m_entityHandle.isValid(_handle.idx))
			{
				BX_WARN(false, "Passing invalid entity handle to mengine::destroyEntity.");
				return;
			}

			entityDecRef(_handle);
		}

		MENGINE_API_FUNC(bool packAssets(const bx::FilePath& _filePath))
		{
			bx::makeAll(_filePath.getPath());

			bx::ErrorAssert err;
			bx::FileWriter writer;
			if (!bx::open(&writer, _filePath, &err))
			{
				BX_TRACE("Failed to open writer at path %s.", _filePath.getCPtr());
				return false;
			}

			// Write asset pack header 
			AssetPackHeader header;
			header.hash = bx::hash<bx::HashMurmur2A>(_filePath.getCPtr());
			header.magic = MENGINE_CHUNK_MAGIC_MAP;
			header.numEntries = m_geometryAssetHashMap.getNumElements() + m_shaderAssetHashMap.getNumElements();
			bx::write(&writer, &header, sizeof(AssetPackHeader), &err);

			// Write asset pack entries
			I64 offset = sizeof(AssetPackHeader) + sizeof(AssetPackEntry) * header.numEntries;
			U16 currGeometry = 0;
			U16 currShader = 0;
			AssetPackEntry entries[3];
			for (U32 i = 0; i < header.numEntries; i++)
			{
				entries[i].offset = offset;

				if (currGeometry < m_geometryAssetHashMap.getNumElements())
				{
					entries[i].type = MENGINE_ASSET_TYPE_GEOMETRY;
					entries[i].hash = m_geometryAssetHashMap.findByHandle(currGeometry);

					offset += m_geometryAssets[currGeometry].getSize();

					currGeometry++;
					continue;
				}

				if (currShader < m_shaderAssetHashMap.getNumElements())
				{
					entries[i].type = MENGINE_ASSET_TYPE_SHADER;
					entries[i].hash = m_shaderAssetHashMap.findByHandle(currShader);

					offset += m_shaderAssets[currShader].getSize();

					currShader++;
					continue;
				}
			}
			bx::write(&writer, &entries, sizeof(AssetPackEntry) * header.numEntries, &err);

			// Write binary blob
			currGeometry = 0;
			currShader = 0;
			for (U32 i = 0; i < header.numEntries; i++)
			{
				if (currGeometry < m_geometryAssetHashMap.getNumElements())
				{
					m_geometryAssets[currGeometry].write(&writer, &err);

					currGeometry++;
					continue;
				}

				if (currShader < m_shaderAssetHashMap.getNumElements())
				{
					m_shaderAssets[currShader].write(&writer, &err);

					currShader++;
					continue;
				}
			}

			bx::close(&writer);
			return true;
		}

		MENGINE_API_FUNC(bool loadAssetPack(const bx::FilePath& _filePath))
		{
			bx::ErrorAssert err;
			bx::FileReader reader;
			if (!bx::open(&reader, _filePath, &err))
			{
				BX_TRACE("Failed to open assetpack at path %s.", _filePath.getCPtr());
				return false;
			}

			// Header
			AssetPackHeader header;
			bx::read(&reader, &header, sizeof(AssetPackHeader), &err);

			// Entries
			AssetPackEntry entries[3];
			bx::read(&reader, &entries, sizeof(AssetPackEntry) * header.numEntries, &err);

			//
			U32 currGeometry = 0;
			U32 currShader = 0;
			for (U32 i = 0; i < header.numEntries; i++)
			{
				if (entries[i].type == MENGINE_ASSET_TYPE_GEOMETRY)
				{
					U16 idx = m_geometryAssetHashMap.find(entries[i].hash);
					if (kInvalidHandle != idx)
					{
						continue;
					}
					GeometryAssetHandle handle = { m_geometryAssetHandle.alloc() };

					if (isValid(handle))
					{
						bool ok = m_geometryAssetHashMap.insert(entries[i].hash, handle.idx);
						BX_ASSERT(ok, "Geometry asset already exists!"); BX_UNUSED(ok);

						GeometryRef& sr = m_geometryAssets[handle.idx];
						sr.read(&reader, &err);
						sr.m_vbh = bgfx::createVertexBuffer(sr.m_vertexData, sr.m_layout);
						sr.m_ibh = bgfx::createIndexBuffer(sr.m_indexData);

						geometryAssetIncRef(handle);
					}
					continue;
				}

				if (entries[i].type == MENGINE_ASSET_TYPE_SHADER)
				{
					U16 idx = m_shaderAssetHashMap.find(entries[i].hash);
					if (kInvalidHandle != idx)
					{
						continue;
					}
					ShaderAssetHandle handle = { m_shaderAssetHandle.alloc() };

					if (isValid(handle))
					{
						bool ok = m_shaderAssetHashMap.insert(entries[i].hash, handle.idx);
						BX_ASSERT(ok, "Shader asset already exists!"); BX_UNUSED(ok);

						ShaderRef& sr = m_shaderAssets[handle.idx];
						sr.read(&reader, &err);
						sr.m_sh = bgfx::createShader(sr.m_codeData);

						shaderAssetIncRef(handle);
					}
					continue;
				}
			}

			bx::close(&reader);
			return true;
		}

		void geometryAssetIncRef(GeometryAssetHandle _handle)
		{
			GeometryRef& sr = m_geometryAssets[_handle.idx];
			++sr.m_refCount;
		}

		void geometryAssetDecRef(GeometryAssetHandle _handle)
		{
			GeometryRef& sr = m_geometryAssets[_handle.idx];
			int32_t refs = --sr.m_refCount;

			if (0 == refs)
			{
				bool ok = m_freeGeometryAssets.queue(_handle); BX_UNUSED(ok);
				BX_ASSERT(ok, "Geometry Asset handle %d is already destroyed!", _handle.idx);

				if (false) // @todo Find a way to see if buffers owns the data or not
				{
					release(sr.m_vertexData);
					sr.m_vertexData = NULL;

					release(sr.m_indexData);
					sr.m_indexData = NULL;
				}
				bgfx::destroy(sr.m_vbh);
				bgfx::destroy(sr.m_ibh);

				sr.m_layout = bgfx::VertexLayout();

				m_geometryAssetHashMap.removeByHandle(_handle.idx);
			}
		}

		MENGINE_API_FUNC(GeometryAssetHandle createGeometry(const void* _vertices, U32 _verticesSize, const void* _indices, U32 _indicesSize, bgfx::VertexLayout _layout, const bx::FilePath& _virtualPath))
		{
			U32 hash = bx::hash<bx::HashMurmur2A>(_virtualPath.getCPtr());;

			U16 idx = m_geometryAssetHashMap.find(hash);
			if (kInvalidHandle != idx)
			{
				GeometryAssetHandle handle = { idx };
				geometryAssetIncRef(handle);
				return handle;
			}
			GeometryAssetHandle handle = { m_geometryAssetHandle.alloc() };

			if (isValid(handle))
			{
				bool ok = m_geometryAssetHashMap.insert(hash, handle.idx);
				BX_ASSERT(ok, "Geometry asset already exists!"); BX_UNUSED(ok);

				GeometryRef& sr = m_geometryAssets[handle.idx];
				sr.m_refCount = 1;
				sr.m_vertexData = bgfx::makeRef(_vertices, _verticesSize, mrender::getAllocator());
				sr.m_indexData = bgfx::makeRef(_indices, _indicesSize, mrender::getAllocator());
				sr.m_layout = _layout;

				sr.m_vbh = bgfx::createVertexBuffer(sr.m_vertexData, sr.m_layout);
				sr.m_ibh = bgfx::createIndexBuffer(sr.m_indexData);

				return handle;
			}

			BX_TRACE("Failed to create asset handle.");
			return MENGINE_INVALID_HANDLE;
		}

		MENGINE_API_FUNC(GeometryAssetHandle loadGeometry(const bx::FilePath& _filePath))
		{
			U32 hash = bx::hash<bx::HashMurmur2A>(_filePath.getCPtr());

			U16 idx = m_geometryAssetHashMap.find(hash);
			if (kInvalidHandle != idx)
			{
				GeometryAssetHandle handle = { idx };
				geometryAssetIncRef(handle);
				return handle;
			}
			GeometryAssetHandle handle = { m_geometryAssetHandle.alloc() };

			bx::Error err;
			bx::FileReader reader;
			if (!bx::open(&reader, _filePath, &err))
			{
				BX_TRACE("Failed to open asset at path %s.", _filePath.getCPtr());
				return MENGINE_INVALID_HANDLE;
			}
			
			if (isValid(handle))
			{
				bool ok = m_geometryAssetHashMap.insert(hash, handle.idx);
				BX_ASSERT(ok, "Geometry asset already exists!"); BX_UNUSED(ok);

				GeometryRef& sr = m_geometryAssets[handle.idx];
				sr.read(&reader, &err);
				sr.m_vbh = bgfx::createVertexBuffer(sr.m_vertexData, sr.m_layout);
				sr.m_ibh = bgfx::createIndexBuffer(sr.m_indexData);

				geometryAssetIncRef(handle);

				bx::close(&reader);
				return handle;
			}

			bx::close(&reader);
			BX_TRACE("Failed to load asset handle.");
			return MENGINE_INVALID_HANDLE;
		}

		MENGINE_API_FUNC(void destroyGeometry(GeometryAssetHandle _handle))
		{
			MENGINE_MUTEX_SCOPE(m_resourceApiLock);

			if (!isValid(_handle) && !m_geometryAssetHandle.isValid(_handle.idx))
			{
				BX_WARN(false, "Passing invalid geometry asset handle to mengine::destroyGeometry.");
				return;
			}

			geometryAssetDecRef(_handle);
		}

		void shaderAssetIncRef(ShaderAssetHandle _handle)
		{
			ShaderRef& sr = m_shaderAssets[_handle.idx];
			++sr.m_refCount;
		}

		void shaderAssetDecRef(ShaderAssetHandle _handle)
		{
			ShaderRef& sr = m_shaderAssets[_handle.idx];
			int32_t refs = --sr.m_refCount;
			if (0 == refs)
			{
				bool ok = m_freeShaderAssets.queue(_handle); BX_UNUSED(ok);
				BX_ASSERT(ok, "Shader Asset handle %d is already destroyed!", _handle.idx);

				if (false) // @todo Find a way to see if buffers owns the data or not
				{
					release(sr.m_codeData);
					sr.m_codeData = NULL;
				}
				bgfx::destroy(sr.m_sh);

				m_shaderAssetHashMap.removeByHandle(_handle.idx);
			}
		}

		MENGINE_API_FUNC(ShaderAssetHandle createShader(const bgfx::Memory* _mem, const bx::FilePath& _virtualPath))
		{
			U32 hash = bx::hash<bx::HashMurmur2A>(_virtualPath.getCPtr());

			U16 idx = m_shaderAssetHashMap.find(hash);
			if (kInvalidHandle != idx)
			{
				ShaderAssetHandle handle = { idx };
				shaderAssetIncRef(handle);
				return handle;
			}

			ShaderAssetHandle handle = { m_shaderAssetHandle.alloc() };
			if (isValid(handle))
			{
				bool ok = m_shaderAssetHashMap.insert(hash, handle.idx);
				BX_ASSERT(ok, "Shader asset already exists!"); BX_UNUSED(ok);

				ShaderRef& sr = m_shaderAssets[handle.idx];
				sr.m_refCount = 1;
				sr.m_codeData = bgfx::makeRef(_mem->data, _mem->size);
				
				sr.m_sh = bgfx::createShader(sr.m_codeData);

				return handle;
			}

			BX_TRACE("Failed to create asset handle.");
			return MENGINE_INVALID_HANDLE;
		}

		MENGINE_API_FUNC(ShaderAssetHandle loadShader(const bx::FilePath& _filePath))
		{
			U32 hash = bx::hash<bx::HashMurmur2A>(_filePath.getCPtr());

			U16 idx = m_shaderAssetHashMap.find(hash);
			if (kInvalidHandle != idx)
			{
				ShaderAssetHandle handle = { idx };
				shaderAssetIncRef(handle);
				return handle;
			}

			bx::Error err;
			bx::FileReader reader;
			if (!bx::open(&reader, _filePath, &err))
			{
				BX_TRACE("Failed to open asset at path %s.", _filePath.getCPtr());
				return MENGINE_INVALID_HANDLE;
			}

			ShaderAssetHandle handle = { m_shaderAssetHandle.alloc() };
			if (isValid(handle))
			{
				bool ok = m_shaderAssetHashMap.insert(hash, handle.idx);
				BX_ASSERT(ok, "Shader asset already exists!"); BX_UNUSED(ok);

				ShaderRef& sr = m_shaderAssets[handle.idx];
				sr.read(&reader, &err);
				sr.m_sh = bgfx::createShader(sr.m_codeData);

				shaderAssetIncRef(handle);

				bx::close(&reader);
				return handle;
			}

			bx::close(&reader);
			BX_TRACE("Failed to load asset handle.");
			return MENGINE_INVALID_HANDLE;
		}

		MENGINE_API_FUNC(void destroyShader(ShaderAssetHandle _handle))
		{
			MENGINE_MUTEX_SCOPE(m_resourceApiLock);

			if (!isValid(_handle) && !m_shaderAssetHandle.isValid(_handle.idx))
			{
				BX_WARN(false, "Passing invalid shader asset handle to mengine::destroyShader.");
				return;
			}

			shaderAssetDecRef(_handle);
		}

		MENGINE_API_FUNC(const bgfx::Memory* compileShader(const char* _shaderCode, ShaderType::Enum _type))
		{
			bx::ErrorAssert err;

			U32 size = sizeof(U32) + sizeof(U32) + sizeof(U32) + sizeof(U16) + sizeof(U32) + (U32)bx::strLen(_shaderCode) + sizeof(U8);
			const bgfx::Memory* mem = bgfx::alloc(size);
			bx::StaticMemoryBlockWriter writer(mem->data, mem->size);

			std::vector<Uniform> uniforms;
			/*
			bx::StringView parse(_shaderCode);
			while (!parse.isEmpty())
			{
				parse = bx::strLTrimSpace(parse);
				bx::StringView eol = bx::strFind(parse, ';');
				if (!eol.isEmpty())
				{
					bx::StringView qualifier = bx::strWord(bx::strLTrimSpace(parse));
					parse = bx::strLTrimSpace(bx::StringView(qualifier.getTerm(), parse.getTerm()));

					if (0 == bx::strCmp(qualifier, "precision", 9))
					{
						// skip precision
						parse.set(eol.getPtr() + 1, parse.getTerm());
						continue;
					}

					if (0 == bx::strCmp(qualifier, "attribute", 9)
						|| 0 == bx::strCmp(qualifier, "varying", 7)
						|| 0 == bx::strCmp(qualifier, "in", 2)
						|| 0 == bx::strCmp(qualifier, "out", 3)
						)
					{
						// skip attributes and varyings.
						parse.set(eol.getPtr() + 1, parse.getTerm());
						continue;
					}

					if (0 == bx::strCmp(qualifier, "flat", 4)
						|| 0 == bx::strCmp(qualifier, "smooth", 6)
						|| 0 == bx::strCmp(qualifier, "noperspective", 13)
						|| 0 == bx::strCmp(qualifier, "centroid", 8)
						)
					{
						// skip interpolation qualifiers
						parse.set(eol.getPtr() + 1, parse.getTerm());
						continue;
					}

					if (0 == bx::strCmp(parse, "tmpvar", 6))
					{
						// skip temporaries
						parse.set(eol.getPtr() + 1, parse.getTerm());
						continue;
					}

					if (0 != bx::strCmp(qualifier, "uniform", 7))
					{
						// end if there is no uniform keyword.
						parse.clear();
						continue;
					}

					bx::StringView precision;

					bx::StringView typen = bx::strWord(bx::strLTrimSpace(parse));
					parse = bx::strLTrimSpace(bx::StringView(typen.getTerm(), parse.getTerm()));

					if (0 == bx::strCmp(typen, "lowp", 4)
						|| 0 == bx::strCmp(typen, "mediump", 7)
						|| 0 == bx::strCmp(typen, "highp", 5))
					{
						precision = typen;
						typen = bx::strWord(bx::strLTrimSpace(parse));
						parse = bx::strLTrimSpace(bx::StringView(typen.getTerm(), parse.getTerm()));
					}

					BX_UNUSED(precision);

					char uniformType[256];

					if (0 == bx::strCmp(typen, "sampler", 7)
						|| 0 == bx::strCmp(typen, "isampler", 8)
						|| 0 == bx::strCmp(typen, "usampler", 8))
					{
						bx::strCopy(uniformType, BX_COUNTOF(uniformType), "int");
					}
					else
					{
						bx::strCopy(uniformType, BX_COUNTOF(uniformType), typen);
					}


					bx::StringView name = bx::strWord(bx::strLTrimSpace(parse));
					parse = bx::strLTrimSpace(bx::StringView(name.getTerm(), parse.getTerm()));

					uint8_t num = 1;
					bx::StringView array = bx::strSubstr(parse, 0, 1);
					if (0 == bx::strCmp(array, "[", 1))
					{
						parse = bx::strLTrimSpace(bx::StringView(parse.getPtr() + 1, parse.getTerm()));

						uint32_t tmp;
						bx::fromString(&tmp, parse);
						num = uint8_t(tmp);
					}

					const char* uniformTypeName[] =
					{
						"int",  "int",
						NULL,   NULL,
						"vec4", "float4",
						"mat3", "float3x3",
						"mat4", "float4x4",
					};

					Uniform un;

					for (uint32_t ii = 0; ii < bgfx::UniformType::Count * 2; ++ii)
					{
						if (NULL != uniformTypeName[ii]
							&& 0 == bx::strCmp(uniformType, uniformTypeName[ii]))
						{
							un.type = bgfx::UniformType::Enum(ii / 2);
							break;
						}
					}

					if (bgfx::UniformType::Count != un.type)
					{
						un.name = static_cast<const char*>(bx::alloc(getAllocator(), (name.getTerm() - name.getPtr() + 1)));
						memcpy(const_cast<char*>(un.name), name.getPtr(), name.getTerm() - name.getPtr());
						const_cast<char*>(un.name)[name.getTerm() - name.getPtr()] = '\0';

						BX_TRACE("name: %s (type %d, num %d)", un.name, un.type, num);

						un.num = num;
						un.regIndex = 0;
						un.regCount = num;
						switch (un.type)
						{
						case bgfx::UniformType::Mat3:
							un.regCount *= 3;
							break;
						case bgfx::UniformType::Mat4:
							un.regCount *= 4;
							break;
						default:
							break;
						}

						uniforms.push_back(un);
					}

					parse = bx::strLTrimSpace(bx::strFindNl(bx::StringView(eol.getPtr(), parse.getTerm())));
				}
			}
			*/

			// Magic
			if (ShaderType::Fragment == _type)
			{
				bx::write(&writer, BGFX_CHUNK_MAGIC_FSH, &err);
			}
			else if (ShaderType::Vertex == _type)
			{
				bx::write(&writer, BGFX_CHUNK_MAGIC_VSH, &err);
			}
			else
			{
				bx::write(&writer, BGFX_CHUNK_MAGIC_CSH, &err);
			}

			// Hashes
			U32 hashIn = 0;
			U32 hashOut = 0;
			bx::write(&writer, hashIn, &err);
			bx::write(&writer, hashOut, &err);

			// Uniforms @todo
			U16 count = 0;
			bx::write(&writer, count, &err);

			// Shader code
			U32 shaderSize = (U32)bx::strLen(_shaderCode);
			bx::write(&writer, shaderSize, &err);
			bx::write(&writer, _shaderCode, shaderSize, &err);
			U8 nul = 0;
			bx::write(&writer, nul, &err);

			// 
			return mem;
		}

		MENGINE_API_FUNC(const Stats* getStats())
		{
			Stats& stats = m_stats;

			stats.numEntities = m_entityHandle.getNumHandles();
			stats.numComponents = m_componentHandle.getNumHandles();
			stats.numGeometryAssets = m_geometryAssetHandle.getNumHandles();
			stats.numShaderAssets = m_shaderAssetHandle.getNumHandles();


			return &stats;
		}

		void release(const bgfx::Memory* _mem);

		// index = type, key = entity, value = component handle
		bx::HandleHashMapT<MENGINE_CONFIG_MAX_COMPONENTS_PER_TYPE> m_ecsHashMap[MENGINE_CONFIG_MAX_COMPONENT_TYPES];

		bx::HandleAllocT<MENGINE_CONFIG_MAX_COMPONENTS> m_componentHandle;
		ComponentRef m_components[MENGINE_CONFIG_MAX_COMPONENTS];

		bx::HandleAllocT<MENGINE_CONFIG_MAX_ENTITIES> m_entityHandle;
		EntityRef m_entities[MENGINE_CONFIG_MAX_ENTITIES];

		bx::HandleAllocT<MENGINE_CONFIG_MAX_GEOMETRY_ASSETS> m_geometryAssetHandle;
		bx::HandleHashMapT<MENGINE_CONFIG_MAX_GEOMETRY_ASSETS> m_geometryAssetHashMap;
		GeometryRef m_geometryAssets[MENGINE_CONFIG_MAX_GEOMETRY_ASSETS];

		bx::HandleAllocT<MENGINE_CONFIG_MAX_SHADER_ASSETS> m_shaderAssetHandle;
		bx::HandleHashMapT<MENGINE_CONFIG_MAX_SHADER_ASSETS> m_shaderAssetHashMap;
		ShaderRef m_shaderAssets[MENGINE_CONFIG_MAX_SHADER_ASSETS];

		template<typename Ty, uint32_t Max>
		struct FreeHandle
		{
			FreeHandle()
				: m_num(0)
			{
			}

			bool isQueued(Ty _handle)
			{
				for (uint32_t ii = 0, num = m_num; ii < num; ++ii)
				{
					if (m_queue[ii].idx == _handle.idx)
					{
						return true;
					}
				}

				return false;
			}

			bool queue(Ty _handle)
			{
				if (BX_ENABLED(MENGINE_CONFIG_DEBUG))
				{
					if (isQueued(_handle))
					{
						return false;
					}
				}

				m_queue[m_num] = _handle;
				++m_num;

				return true;
			}

			void reset()
			{
				m_num = 0;
			}

			Ty get(uint16_t _idx) const
			{
				return m_queue[_idx];
			}

			uint16_t getNumQueued() const
			{
				return m_num;
			}

			Ty m_queue[Max];
			uint16_t m_num;
		};

		FreeHandle<EntityHandle, MENGINE_CONFIG_MAX_ENTITIES>  m_freeEntities;
		FreeHandle<ComponentHandle, MENGINE_CONFIG_MAX_COMPONENTS> m_freeComponents;
		FreeHandle<GeometryAssetHandle, MENGINE_CONFIG_MAX_GEOMETRY_ASSETS>  m_freeGeometryAssets;
		FreeHandle<ShaderAssetHandle, MENGINE_CONFIG_MAX_SHADER_ASSETS> m_freeShaderAssets;

		U32 m_frameNum;
		Stats m_stats;
	};

} // namespace mengine