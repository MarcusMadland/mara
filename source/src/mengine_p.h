/*
 * Copyright 2023 Marcus Madland. All rights reserved.
 * License: https://github.com/MarcusMadland/mengine/blob/main/LICENSE
 */

#ifndef MENGINE_P_H_HEADER_GUARD
#define MENGINE_P_H_HEADER_GUARD

#include <mapp/platform.h>

#ifndef BX_CONFIG_DEBUG
#	error "BX_CONFIG_DEBUG must be defined in build script!"
#endif // BX_CONFIG_DEBUG

#define MENGINE_CONFIG_DEBUG BX_CONFIG_DEBUG

#include "mengine/mengine.h"
#include "config.h"

 // Check handle, cannot be bgfx::kInvalidHandle and must be valid.
#define MENGINE_CHECK_HANDLE(_desc, _handleAlloc, _handle) \
	BX_ASSERT(isValid(_handle)                          \
		&& _handleAlloc.isValid(_handle.idx)            \
		, "Invalid handle. %s handle: %d (max %d)"      \
		, _desc                                         \
		, _handle.idx                                   \
		, _handleAlloc.getMaxHandles()                  \
		)

// Check handle, it's ok to be bgfx::kInvalidHandle or must be valid.
#define MENGINE_CHECK_HANDLE_INVALID_OK(_desc, _handleAlloc, _handle) \
	BX_ASSERT(!isValid(_handle)                                    \
		|| _handleAlloc.isValid(_handle.idx)                       \
		, "Invalid handle. %s handle: %d (max %d)"                 \
		, _desc                                                    \
		, _handle.idx                                              \
		, _handleAlloc.getMaxHandles()                             \
		)

#if MENGINE_CONFIG_MULTITHREADED
#	define MENGINE_MUTEX_SCOPE(_mutex) bx::MutexScope BX_CONCATENATE(mutexScope, __LINE__)(_mutex)
#else
#	define MENGINE_MUTEX_SCOPE(_mutex) BX_NOOP()
#endif // MENGINE_CONFIG_MULTITHREADED

#include <mapp/bx.h>
#include <mapp/timer.h>
#include <mapp/math.h>
#include <mapp/bounds.h>
#include <mapp/pixelformat.h>
#include <mapp/string.h>
#include <mapp/allocator.h>
#include <mapp/file.h>
#include <mapp/handlealloc.h>
#include <mapp/hash.h>
#include <mapp/commandline.h>

#define MENGINE_ASSET_PACK_VERSION 1
#define MENGINE_CHUNK_MAGIC_MAP BX_MAKEFOURCC('M', 'A', 'P', MENGINE_ASSET_PACK_VERSION)

#define BGFX_SHADER_BIN_VERSION 11
#define BGFX_CHUNK_MAGIC_CSH BX_MAKEFOURCC('C', 'S', 'H', BGFX_SHADER_BIN_VERSION)
#define BGFX_CHUNK_MAGIC_FSH BX_MAKEFOURCC('F', 'S', 'H', BGFX_SHADER_BIN_VERSION)
#define BGFX_CHUNK_MAGIC_VSH BX_MAKEFOURCC('V', 'S', 'H', BGFX_SHADER_BIN_VERSION)

namespace mengine 
{
	struct AssetType
	{
		enum Enum
		{
			None,
			Geometry,
			Shader,
			Texture,
			Material,
			Mesh,
		};
	};

	struct EntityRef
	{
		EntityRef() // @todo I don't like to use constructors like this, the code depends on it. Fix this
			: m_mask(0)
			, m_refCount(0)
		{}

		U32 m_mask;
		U16 m_refCount;
	};

	struct ComponentRef
	{
		ComponentRef()
			: m_data(NULL)
			, m_refCount(0)
		{}

		ComponentI* m_data;
		U16 m_refCount;
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

		void write(bx::WriterI* _writer, bx::Error* _err) override
		{
			U32 layoutSize = sizeof(m_layout);

			bx::write(_writer, &m_vertexData->size, sizeof(U32), _err);
			bx::write(_writer, m_vertexData->data, (I32)m_vertexData->size, _err);
			bx::write(_writer, &m_indexData->size, sizeof(U32), _err);
			bx::write(_writer, m_indexData->data, (I32)m_indexData->size, _err);
			bx::write(_writer, &layoutSize, sizeof(U32), _err);
			bx::write(_writer, &m_layout, (I32)layoutSize, _err);
		};

		void read(bx::ReaderSeekerI* _reader, bx::Error* _err) override
		{
			U32 vertexDataSize;
			bx::read(_reader, &vertexDataSize, sizeof(vertexDataSize), _err);
			m_vertexData = bgfx::alloc(vertexDataSize);
			bx::read(_reader, m_vertexData->data, vertexDataSize, _err);

			U32 indexDataSize;
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

		void write(bx::WriterI* _writer, bx::Error* _err) override
		{ 
			bx::write(_writer, &m_codeData->size, sizeof(U32), _err);
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

	struct TextureRef : SerializerI
	{
		TextureRef()
			: m_th(BGFX_INVALID_HANDLE)
			, width(0)
			, height(0)
			, hasMips(false)
			, format(bgfx::TextureFormat::Count)
			, flags(BGFX_TEXTURE_NONE)
			, mem(NULL)
			, m_refCount(0)
		{}

		U32 getSize() override
		{
			return sizeof(width) + sizeof(height) + sizeof(hasMips) + sizeof(format) + sizeof(flags)
				+ sizeof(mem->size) + mem->size;
		};

		void write(bx::WriterI* _writer, bx::Error* _err) override
		{
			bx::write(_writer, &width, sizeof(width), _err);
			bx::write(_writer, &height, sizeof(height), _err);
			bx::write(_writer, &hasMips, sizeof(hasMips), _err);
			bx::write(_writer, &format, sizeof(format), _err);
			bx::write(_writer, &flags, sizeof(flags), _err);

			bx::write(_writer, &mem->size, sizeof(mem->size), _err);
			bx::write(_writer, mem->data, mem->size, _err);
		};

		void read(bx::ReaderSeekerI* _reader, bx::Error* _err) override
		{
			bx::read(_reader, &width, sizeof(width), _err);
			bx::read(_reader, &height, sizeof(height), _err);
			bx::read(_reader, &hasMips, sizeof(hasMips), _err);
			bx::read(_reader, &format, sizeof(format), _err);
			bx::read(_reader, &flags, sizeof(flags), _err);

			U32 size;
			bx::read(_reader, &size, sizeof(size), _err);
			mem = bgfx::alloc(size);
			bx::read(_reader, mem->data, size, _err);
		};

		bgfx::TextureHandle m_th;

		U16 width;
		U16 height;
		bool hasMips;
		bgfx::TextureFormat::Enum format;
		U64 flags;
		const bgfx::Memory* mem;

		U16 m_refCount;
	};

	struct MaterialRef : SerializerI
	{
		MaterialRef()
			: m_ph(BGFX_INVALID_HANDLE)
			, m_vertHash(0)
			, m_fragHash(0)
			, m_refCount(0)
		{}

		U32 getSize() override
		{
			return sizeof(m_vertHash) + sizeof(m_fragHash);
		};

		void write(bx::WriterI* _writer, bx::Error* _err) override
		{
			bx::write(_writer, &m_vertHash, sizeof(m_vertHash), _err);
			bx::write(_writer, &m_fragHash, sizeof(m_fragHash), _err);
		};

		void read(bx::ReaderSeekerI* _reader, bx::Error* _err) override
		{
			bx::read(_reader, &m_vertHash, sizeof(m_vertHash), _err);
			bx::read(_reader, &m_fragHash, sizeof(m_fragHash), _err);
		};

		bgfx::ProgramHandle m_ph;

		U32 m_vertHash;
		U32 m_fragHash;

		U16 m_refCount;
	};

	struct AssetPackEntry
	{
		AssetPackEntry()
			: type(AssetType::Enum::None)
			, hash(0)
			, offset(0)
		{}

		AssetType::Enum type;
		U32 hash;
		I64 offset;
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

#if MENGINE_CONFIG_DEBUG
#	define MENGINE_API_FUNC(_func) BX_NO_INLINE _func
#else
#	define MENGINE_API_FUNC(_func) _func
#endif // BGFX_CONFIG_DEBUG

	struct Context
	{
		Context()
			: m_stats(mengine::Stats())
		{
		}

		~Context()
		{
		}

		bool init(const Init& _init);
		void shutdown();
		bool update(U32 _debug, U32 _reset);

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

				// @todo We are using delete because we are using virtual destructors. Can we do this manually?
				delete sr.m_data; 
				sr.m_data = NULL;
			}
		}

		MENGINE_API_FUNC(ComponentHandle createComponent(ComponentI* _data))
		{
			ComponentHandle handle = { m_componentHandle.alloc() };

			if (isValid(handle))
			{
				ComponentRef& sr = m_components[handle.idx];
				sr.m_refCount = 1;

				sr.m_data = _data;
				
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
			bool ok = m_componentHashMap[_type].insert(_entity.idx, _component.idx);
			BX_ASSERT(ok, "Entities cannot have duplicated components!", _entity.idx);

			m_entityHashMap[_type].insert(m_entityHashMap[_type].getNumElements(), _entity.idx);

			EntityRef& sr = m_entities[_entity.idx];
			sr.m_mask |= _type;
		}

		bool hasComponent(EntityHandle _handle, U32 _type)
		{
			const U16 idx = m_componentHashMap[_type].find(_handle.idx);
			return idx != kInvalidHandle;
		}

		MENGINE_API_FUNC(void* getComponentData(EntityHandle _handle, U32 _type))
		{
			const U16 idx = m_componentHashMap[_type].find(_handle.idx);
			if (idx != kInvalidHandle)
			{
				void* data = m_components[idx].m_data;
				if (NULL != data)
				{
					return data;
				}

				BX_ASSERT(true, "Component data is NULL but entity contains component type")
				return NULL;
			}

			BX_ASSERT(true, "Component handle is invalid")
			return NULL;
		}

		MENGINE_API_FUNC(EntityQuery* queryEntities(U32 _types))
		{
			// @todo What if- instead of allocating an entity query here, we have a global entity query we change instead. 
			// Then we can allocate and deallocate that ourself without leaving that respnsibility to the user.
			EntityQuery* query = (EntityQuery*)bx::alloc(mrender::getAllocator(), sizeof(EntityQuery));
			query->m_count = 0;

			for (U32 i = 0; i < 32; ++i)
			{
				for (U16 j = 0; j < m_componentHashMap[i].getNumElements(); j++) // @todo We are looping over the same entity multiple times
				{
					EntityHandle handle = { m_entityHashMap[i].find(j) };
					if (!isValid(handle))
					{
						continue;
					}

					EntityRef& sr = m_entities[handle.idx];
					if ((sr.m_mask & _types) == _types)
					{
						query->m_entities[query->m_count] = handle;
						query->m_count++;
					}
				}
			}

			return query;
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

			if (!isValid(_handle))
			{
				BX_WARN(false, "Passing invalid entity handle to mengine::destroyEntity.");
				return;
			}

			EntityRef& sr = m_entities[_handle.idx]; // @todo make destroying components optional maybe
			
			// Loop over all bit mask components
			for (U32 i = 0; i < 32; i++)
			{
				// If entity got that component find it in the component hash map and get the component reference id
				if (sr.m_mask & (1 << i))
				{
					const U16 idx = m_componentHashMap[1 << i].find(_handle.idx);
					if (kInvalidHandle != idx)
					{
						// Destroy component
						destroyComponent({ idx });
						m_componentHashMap[1 << i].removeByKey(_handle.idx);
						m_entityHashMap[1 << i].removeByHandle(i);
					}
				}
			}

			entityDecRef(_handle);
		}

		// @todo Asset packing needs refactoring, currently loads entire pack into memory.
		// Implement load on demand. Also dont like the use of 'new' use alloc instead.
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
			header.numEntries = 
				m_geometryAssetHashMap.getNumElements() + 
				m_shaderAssetHashMap.getNumElements() +
				m_textureAssetHashMap.getNumElements() + 
				m_materialAssetHashMap.getNumElements();
			bx::write(&writer, &header, sizeof(AssetPackHeader), &err);

			// Write asset pack entries
			I64 offset = sizeof(AssetPackHeader) + sizeof(AssetPackEntry) * header.numEntries;
			U16 currGeometry = 0;
			U16 currShader = 0;
			U16 currTexture = 0;
			U16 currMaterial = 0;
			AssetPackEntry* entries = new AssetPackEntry[header.numEntries];
			for (U32 i = 0; i < header.numEntries; i++)
			{
				entries[i].offset = offset;

				if (currGeometry < m_geometryAssetHashMap.getNumElements())
				{
					entries[i].type = AssetType::Geometry;
					entries[i].hash = m_geometryAssetHashMap.findByHandle(currGeometry);

					offset += m_geometryAssets[currGeometry].getSize();

					currGeometry++;
					continue;
				}

				if (currShader < m_shaderAssetHashMap.getNumElements())
				{
					entries[i].type = AssetType::Shader;
					entries[i].hash = m_shaderAssetHashMap.findByHandle(currShader);

					offset += m_shaderAssets[currShader].getSize();

					currShader++;
					continue;
				}

				if (currTexture < m_textureAssetHashMap.getNumElements())
				{
					entries[i].type = AssetType::Texture;
					entries[i].hash = m_textureAssetHashMap.findByHandle(currTexture);

					offset += m_textureAssets[currTexture].getSize();

					currTexture++;
					continue;
				}

				if (currMaterial < m_materialAssetHashMap.getNumElements())
				{
					entries[i].type = AssetType::Material;
					entries[i].hash = m_materialAssetHashMap.findByHandle(currMaterial);

					offset += m_materialAssets[currMaterial].getSize();

					currMaterial++;
					continue;
				}
			}
			bx::write(&writer, entries, sizeof(AssetPackEntry) * header.numEntries, &err);

			// Write binary blob
			currGeometry = 0;
			currShader = 0;
			currTexture = 0;
			currMaterial = 0;
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

				if (currTexture < m_textureAssetHashMap.getNumElements())
				{
					m_textureAssets[currTexture].write(&writer, &err);

					currTexture++;
					continue;
				}

				if (currMaterial < m_materialAssetHashMap.getNumElements())
				{
					m_materialAssets[currMaterial].write(&writer, &err);

					currMaterial++;
					continue;
				}
			}

			delete[] entries;
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
			AssetPackEntry* entries = new AssetPackEntry[header.numEntries];
			bx::read(&reader, entries, sizeof(AssetPackEntry) * header.numEntries, &err);

			//
			for (U32 i = 0; i < header.numEntries; i++)
			{
				switch (entries[i].type)
				{
					case AssetType::Geometry:
					{
						U16 idx = m_geometryAssetHashMap.find(entries[i].hash);
						if (kInvalidHandle != idx)
						{
							break;
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
						break;
					}
					
					case AssetType::Shader:
					{
						U16 idx = m_shaderAssetHashMap.find(entries[i].hash);
						if (kInvalidHandle != idx)
						{
							break;
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
						break;
					}

					case AssetType::Texture:
					{
						U16 idx = m_textureAssetHashMap.find(entries[i].hash);
						if (kInvalidHandle != idx)
						{
							break;
						}
						TextureAssetHandle handle = { m_textureAssetHandle.alloc() };

						if (isValid(handle))
						{
							bool ok = m_textureAssetHashMap.insert(entries[i].hash, handle.idx);
							BX_ASSERT(ok, "Texture asset already exists!"); BX_UNUSED(ok);

							TextureRef& sr = m_textureAssets[handle.idx];
							sr.read(&reader, &err);
							sr.m_th = bgfx::createTexture2D(sr.width, sr.height, sr.hasMips, 1, sr.format, sr.flags, sr.mem);

							textureAssetIncRef(handle);
						}
						break;
					}

					case AssetType::Material:
					{
						U16 idx = m_materialAssetHashMap.find(entries[i].hash);
						if (kInvalidHandle != idx)
						{
							break;
						}
						MaterialAssetHandle handle = { m_materialAssetHandle.alloc() };

						if (isValid(handle))
						{
							bool ok = m_materialAssetHashMap.insert(entries[i].hash, handle.idx);
							BX_ASSERT(ok, "Material asset already exists!"); BX_UNUSED(ok);

							MaterialRef& sr = m_materialAssets[handle.idx];
							sr.read(&reader, &err);

							ShaderAssetHandle vert = { m_shaderAssetHashMap.find(sr.m_vertHash) };
							ShaderAssetHandle frag = { m_shaderAssetHashMap.find(sr.m_fragHash) };
							shaderAssetIncRef(vert);
							shaderAssetIncRef(frag);
							sr.m_ph = bgfx::createProgram(vert, frag);

							materialAssetIncRef(handle);
						}
						break;
					}
				}
			}

			delete[] entries;
			bx::close(&reader);
			return true;
		}

		MENGINE_API_FUNC(bool unloadAssetPack(const bx::FilePath& _filePath))
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
			AssetPackEntry* entries = new AssetPackEntry[header.numEntries];
			bx::read(&reader, entries, sizeof(AssetPackEntry) * header.numEntries, &err);

			//
			for (U32 i = 0; i < header.numEntries; i++)
			{
				switch (entries[i].type)
				{
					case AssetType::Geometry:
					{
						U16 idx = m_geometryAssetHashMap.find(entries[i].hash);
						if (kInvalidHandle != idx)
						{
							destroyGeometry({ idx });
						}
						break;
					}
					case AssetType::Shader:
					{
						U16 idx = m_shaderAssetHashMap.find(entries[i].hash);
						if (kInvalidHandle != idx)
						{
							destroyShader({ idx });
						}
						break;
					}
					case AssetType::Texture:
					{
						U16 idx = m_textureAssetHashMap.find(entries[i].hash);
						if (kInvalidHandle != idx)
						{
							destroyTexture({ idx });
						}
						break;
					}
					case AssetType::Material:
					{
						U16 idx = m_materialAssetHashMap.find(entries[i].hash);
						if (kInvalidHandle != idx)
						{
							destroyMaterial({ idx });
						}
						break;
					}
				}
			}

			delete[] entries;
			bx::close(&reader);
			return true;
		}
		// 

		void geometryAssetIncRef(GeometryAssetHandle _handle)
		{
			GeometryRef& sr = m_geometryAssets[_handle.idx];
			++sr.m_refCount;
		}

		void geometryAssetDecRef(GeometryAssetHandle _handle)
		{
			GeometryRef& sr = m_geometryAssets[_handle.idx];
			U16 refs = --sr.m_refCount;

			if (0 == refs)
			{
				bool ok = m_freeGeometryAssets.queue(_handle); BX_UNUSED(ok);
				BX_ASSERT(ok, "Geometry Asset handle %d is already destroyed!", _handle.idx);

				bgfx::destroy(sr.m_vbh);
				bgfx::destroy(sr.m_ibh);

				sr.m_layout = bgfx::VertexLayout();

				m_geometryAssetHashMap.removeByHandle(_handle.idx);
			}
		}

		MENGINE_API_FUNC(GeometryAssetHandle createGeometry(const void* _vertices, U32 _verticesSize, const void* _indices, U32 _indicesSize, bgfx::VertexLayout _layout, const bx::FilePath& _virtualPath))
		{
			U32 hash = bx::hash<bx::HashMurmur2A>(_virtualPath.getCPtr());

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
			U16 refs = --sr.m_refCount;
			if (0 == refs)
			{
				bool ok = m_freeShaderAssets.queue(_handle); BX_UNUSED(ok);
				BX_ASSERT(ok, "Shader Asset handle %d is already destroyed!", _handle.idx);

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
				sr.m_codeData = bgfx::makeRef(_mem->data, _mem->size); // @todo ??? why not m_codeData = _mem?
				
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

			//std::vector<Uniform> uniforms;
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

					U8 num = 1;
					bx::StringView array = bx::strSubstr(parse, 0, 1);
					if (0 == bx::strCmp(array, "[", 1))
					{
						parse = bx::strLTrimSpace(bx::StringView(parse.getPtr() + 1, parse.getTerm()));

						U32 tmp;
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

					for (U32 ii = 0; ii < bgfx::UniformType::Count * 2; ++ii)
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

		void textureAssetIncRef(TextureAssetHandle _handle)
		{
			TextureRef& sr = m_textureAssets[_handle.idx];
			++sr.m_refCount;
		}

		void textureAssetDecRef(TextureAssetHandle _handle)
		{
			TextureRef& sr = m_textureAssets[_handle.idx];
			U16 refs = --sr.m_refCount;
			if (0 == refs)
			{
				bool ok = m_freeTextureAssets.queue(_handle); BX_UNUSED(ok);
				BX_ASSERT(ok, "Texture Asset handle %d is already destroyed!", _handle.idx);

				bgfx::destroy(sr.m_th);

				m_textureAssetHashMap.removeByHandle(_handle.idx);
			}
		}

		MENGINE_API_FUNC(TextureAssetHandle createTexture(void* _data, U32 _size, U16 _width, U16 _height, bool _hasMips, 
			bgfx::TextureFormat::Enum _format, U64 _flags, const bx::FilePath& _virtualPath))
		{
			U32 hash = bx::hash<bx::HashMurmur2A>(_virtualPath.getCPtr());

			U16 idx = m_textureAssetHashMap.find(hash);
			if (kInvalidHandle != idx)
			{
				TextureAssetHandle handle = { idx };
				textureAssetIncRef(handle);
				return handle;
			}

			TextureAssetHandle handle = { m_textureAssetHandle.alloc() };
			if (isValid(handle))
			{
				bool ok = m_textureAssetHashMap.insert(hash, handle.idx);
				BX_ASSERT(ok, "Texture asset already exists!"); BX_UNUSED(ok);

				TextureRef& sr = m_textureAssets[handle.idx];
				sr.m_refCount = 1;
				sr.width = _width;
				sr.height = _height;
				sr.hasMips = _hasMips;
				sr.format = _format;
				sr.flags = _flags;
				sr.mem = bgfx::makeRef(_data, _size);
				sr.m_th = bgfx::createTexture2D(sr.width, sr.height, sr.hasMips, 1, sr.format, sr.flags, sr.mem);

				return handle;
			}

			BX_TRACE("Failed to create asset handle.");
			return MENGINE_INVALID_HANDLE;
		}

		MENGINE_API_FUNC(TextureAssetHandle loadTexture(const bx::FilePath& _filePath))
		{
			U32 hash = bx::hash<bx::HashMurmur2A>(_filePath.getCPtr());

			U16 idx = m_textureAssetHashMap.find(hash);
			if (kInvalidHandle != idx)
			{
				TextureAssetHandle handle = { idx };
				textureAssetIncRef(handle);
				return handle;
			}

			bx::Error err;
			bx::FileReader reader;
			if (!bx::open(&reader, _filePath, &err))
			{
				BX_TRACE("Failed to open asset at path %s.", _filePath.getCPtr());
				return MENGINE_INVALID_HANDLE;
			}

			TextureAssetHandle handle = { m_textureAssetHandle.alloc() };
			if (isValid(handle))
			{
				bool ok = m_textureAssetHashMap.insert(hash, handle.idx);
				BX_ASSERT(ok, "Texture asset already exists!"); BX_UNUSED(ok);

				TextureRef& sr = m_textureAssets[handle.idx];
				sr.read(&reader, &err);
				sr.m_th = bgfx::createTexture2D(sr.width, sr.height, sr.hasMips, 1, sr.format, sr.flags, sr.mem);

				textureAssetIncRef(handle);

				bx::close(&reader);
				return handle;
			}

			bx::close(&reader);
			BX_TRACE("Failed to load asset handle.");
			return MENGINE_INVALID_HANDLE;
		}

		MENGINE_API_FUNC(void destroyTexture(TextureAssetHandle _handle))
		{
			MENGINE_MUTEX_SCOPE(m_resourceApiLock);

			if (!isValid(_handle) && !m_textureAssetHandle.isValid(_handle.idx))
			{
				BX_WARN(false, "Passing invalid texture asset handle to mengine::destroyTexture.");
				return;
			}

			textureAssetDecRef(_handle);
		}

		void materialAssetIncRef(MaterialAssetHandle _handle)
		{
			MaterialRef& sr = m_materialAssets[_handle.idx];
			++sr.m_refCount;

			shaderAssetIncRef({ m_shaderAssetHashMap.find(sr.m_vertHash) });
			shaderAssetIncRef({ m_shaderAssetHashMap.find(sr.m_fragHash) });
		}

		void materialAssetDecRef(MaterialAssetHandle _handle)
		{
			MaterialRef& sr = m_materialAssets[_handle.idx];
			U16 refs = --sr.m_refCount;

			shaderAssetDecRef({ m_shaderAssetHashMap.find(sr.m_vertHash) });
			shaderAssetDecRef({ m_shaderAssetHashMap.find(sr.m_fragHash) });

			if (0 == refs)
			{
				bool ok = m_freeMaterialAssets.queue(_handle); BX_UNUSED(ok);
				BX_ASSERT(ok, "Material Asset handle %d is already destroyed!", _handle.idx);

				bgfx::destroy(sr.m_ph);

				destroyShader({ m_shaderAssetHashMap.find(sr.m_vertHash) });
				destroyShader({ m_shaderAssetHashMap.find(sr.m_fragHash) });

				m_materialAssetHashMap.removeByHandle(_handle.idx);
			}
		}
		// @todo use shader virtual paths instead of assethandles?
		MENGINE_API_FUNC(MaterialAssetHandle createMaterial(ShaderAssetHandle _vert, ShaderAssetHandle _frag, const bx::FilePath& _virtualPath))
		{
			U32 hash = bx::hash<bx::HashMurmur2A>(_virtualPath.getCPtr());

			U16 idx = m_materialAssetHashMap.find(hash);
			if (kInvalidHandle != idx)
			{
				MaterialAssetHandle handle = { idx };
				materialAssetIncRef(handle);
				return handle;
			}

			MaterialAssetHandle handle = { m_materialAssetHandle.alloc() };
			if (isValid(handle))
			{
				bool ok = m_materialAssetHashMap.insert(hash, handle.idx);
				BX_ASSERT(ok, "Material asset already exists!"); BX_UNUSED(ok);

				MaterialRef& sr = m_materialAssets[handle.idx];
				sr.m_refCount = 1;

				// Create
				sr.m_vertHash = m_shaderAssetHashMap.findByHandle(_vert.idx); // @todo findbyhandle is slow and doesnt scale. 
				sr.m_fragHash = m_shaderAssetHashMap.findByHandle(_frag.idx); // Alternative soloution using paths as refs for user and just call loadShader()

				sr.m_ph = bgfx::createProgram(_vert, _frag);

				return handle;
			}

			BX_TRACE("Failed to create asset handle.");
			return MENGINE_INVALID_HANDLE;
		}

		MENGINE_API_FUNC(MaterialAssetHandle loadMaterial(const bx::FilePath& _filePath))
		{
			U32 hash = bx::hash<bx::HashMurmur2A>(_filePath.getCPtr());

			U16 idx = m_materialAssetHashMap.find(hash);
			if (kInvalidHandle != idx)
			{
				MaterialAssetHandle handle = { idx };
				materialAssetIncRef(handle);
				return handle;
			}

			bx::Error err;
			bx::FileReader reader;
			if (!bx::open(&reader, _filePath, &err))
			{
				BX_TRACE("Failed to open asset at path %s.", _filePath.getCPtr());
				return MENGINE_INVALID_HANDLE;
			}

			MaterialAssetHandle handle = { m_materialAssetHandle.alloc() };
			if (isValid(handle))
			{
				bool ok = m_materialAssetHashMap.insert(hash, handle.idx);
				BX_ASSERT(ok, "Material asset already exists!"); BX_UNUSED(ok);

				MaterialRef& sr = m_materialAssets[handle.idx];
				sr.read(&reader, &err);

				// Load
				ShaderAssetHandle vert = { m_shaderAssetHashMap.find(sr.m_vertHash) };
				ShaderAssetHandle frag = { m_shaderAssetHashMap.find(sr.m_fragHash) };

				sr.m_ph = bgfx::createProgram(vert, frag);

				materialAssetIncRef(handle);

				bx::close(&reader);
				return handle;
			}

			bx::close(&reader);
			BX_TRACE("Failed to load asset handle.");
			return MENGINE_INVALID_HANDLE;
		}

		MENGINE_API_FUNC(void destroyMaterial(MaterialAssetHandle _handle))
		{
			MENGINE_MUTEX_SCOPE(m_resourceApiLock);

			if (!isValid(_handle) && !m_materialAssetHandle.isValid(_handle.idx))
			{
				BX_WARN(false, "Passing invalid material asset handle to mengine::destroyMaterial.");
				return;
			}

			materialAssetDecRef(_handle);
		}

		MENGINE_API_FUNC(void addMaterialTextures(MaterialAssetHandle _material, const char* _uniform, TextureAssetHandle _texture))
		{
			
		}

		MENGINE_API_FUNC(void addMaterialUniform(MaterialAssetHandle _material, const char* _uniform, void* _data, U32 _size))
		{

		}

		MENGINE_API_FUNC(const mrender::MouseState* getMouseState())
		{
			return &m_mouseState;
		}

		MENGINE_API_FUNC(const Stats* getStats())
		{
			Stats& stats = m_stats;

			stats.numEntities = m_entityHandle.getNumHandles();
			stats.numComponents = m_componentHandle.getNumHandles();
			stats.numGeometryAssets = m_geometryAssetHandle.getNumHandles();
			stats.numShaderAssets = m_shaderAssetHandle.getNumHandles();
			stats.numTextureAssets = m_textureAssetHandle.getNumHandles();
			stats.numMaterialAssets = m_materialAssetHandle.getNumHandles();

			for (U16 i = 0; i < stats.numEntities; i++)
			{
				stats.entitiesRef[i] = m_entities[i].m_refCount;
			}
			for (U16 i = 0; i < stats.numComponents; i++)
			{
				stats.componentsRef[i] = m_components[i].m_refCount;
			}
			for (U16 i = 0; i < stats.numGeometryAssets; i++)
			{
				stats.geometryRef[i] = m_geometryAssets[i].m_refCount;
			}
			for (U16 i = 0; i < stats.numShaderAssets; i++)
			{
				stats.shaderRef[i] = m_shaderAssets[i].m_refCount;
			}
			for (U16 i = 0; i < stats.numTextureAssets; i++)
			{
				stats.textureRef[i] = m_textureAssets[i].m_refCount;
			}
			for (U16 i = 0; i < stats.numMaterialAssets; i++)
			{
				stats.materialRef[i] = m_materialAssets[i].m_refCount;
			}

			return &stats;
		}

		Stats m_stats;

		bx::HandleAllocT<MENGINE_CONFIG_MAX_COMPONENTS> m_componentHandle;
		bx::HandleHashMapT<MENGINE_CONFIG_MAX_COMPONENTS_PER_TYPE> m_componentHashMap[32];
		ComponentRef m_components[MENGINE_CONFIG_MAX_COMPONENTS];

		bx::HandleAllocT<MENGINE_CONFIG_MAX_ENTITIES> m_entityHandle;
		bx::HandleHashMapT<MENGINE_CONFIG_MAX_COMPONENTS_PER_TYPE> m_entityHashMap[32];
		EntityRef m_entities[MENGINE_CONFIG_MAX_ENTITIES];

		bx::HandleAllocT<MENGINE_CONFIG_MAX_GEOMETRY_ASSETS> m_geometryAssetHandle;
		bx::HandleHashMapT<MENGINE_CONFIG_MAX_GEOMETRY_ASSETS> m_geometryAssetHashMap;
		GeometryRef m_geometryAssets[MENGINE_CONFIG_MAX_GEOMETRY_ASSETS];

		bx::HandleAllocT<MENGINE_CONFIG_MAX_SHADER_ASSETS> m_shaderAssetHandle;
		bx::HandleHashMapT<MENGINE_CONFIG_MAX_SHADER_ASSETS> m_shaderAssetHashMap;
		ShaderRef m_shaderAssets[MENGINE_CONFIG_MAX_SHADER_ASSETS];

		bx::HandleAllocT<MENGINE_CONFIG_MAX_TEXTURE_ASSETS> m_textureAssetHandle;
		bx::HandleHashMapT<MENGINE_CONFIG_MAX_TEXTURE_ASSETS> m_textureAssetHashMap;
		TextureRef m_textureAssets[MENGINE_CONFIG_MAX_TEXTURE_ASSETS];

		bx::HandleAllocT<MENGINE_CONFIG_MAX_MATERIAL_ASSETS> m_materialAssetHandle;
		bx::HandleHashMapT<MENGINE_CONFIG_MAX_MATERIAL_ASSETS> m_materialAssetHashMap;
		MaterialRef m_materialAssets[MENGINE_CONFIG_MAX_MATERIAL_ASSETS];

		template<typename Ty, U32 Max>
		struct FreeHandle
		{
			FreeHandle()
				: m_num(0)
			{
			}

			bool isQueued(Ty _handle)
			{
				for (U32 ii = 0, num = m_num; ii < num; ++ii)
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

			Ty get(U16 _idx) const
			{
				return m_queue[_idx];
			}

			U16 getNumQueued() const
			{
				return m_num;
			}

			Ty m_queue[Max];
			U16 m_num;
		};

		FreeHandle<EntityHandle, MENGINE_CONFIG_MAX_ENTITIES>  m_freeEntities;
		FreeHandle<ComponentHandle, MENGINE_CONFIG_MAX_COMPONENTS> m_freeComponents;
		FreeHandle<GeometryAssetHandle, MENGINE_CONFIG_MAX_GEOMETRY_ASSETS>  m_freeGeometryAssets;
		FreeHandle<ShaderAssetHandle, MENGINE_CONFIG_MAX_SHADER_ASSETS> m_freeShaderAssets;
		FreeHandle<TextureAssetHandle, MENGINE_CONFIG_MAX_TEXTURE_ASSETS> m_freeTextureAssets;
		FreeHandle<MaterialAssetHandle, MENGINE_CONFIG_MAX_TEXTURE_ASSETS> m_freeMaterialAssets;
		FreeHandle<MeshAssetHandle, MENGINE_CONFIG_MAX_TEXTURE_ASSETS> m_freeMeshAssets;

		mrender::MouseState m_mouseState;
	};

} // namespace mengine

#endif // MENGINE_P_H_HEADER_GUARD