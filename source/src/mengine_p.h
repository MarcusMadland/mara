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

namespace mengine 
{
	struct GeometryResource : ResourceI
	{
		U32 getSize() override
		{
			return sizeof(U32) + vertexData->size + sizeof(U32) + indexData->size + sizeof(U32) + (I32)sizeof(layout);
		};

		void write(bx::WriterI* _writer, bx::Error* _err) override
		{
			U32 layoutSize = sizeof(layout);

			bx::write(_writer, &vertexData->size, sizeof(U32), _err);
			bx::write(_writer, vertexData->data, (I32)vertexData->size, _err);
			bx::write(_writer, &indexData->size, sizeof(U32), _err);
			bx::write(_writer, indexData->data, (I32)indexData->size, _err);
			bx::write(_writer, &layoutSize, sizeof(U32), _err);
			bx::write(_writer, &layout, layoutSize, _err);
		};

		void read(bx::ReaderSeekerI* _reader, bx::Error* _err) override
		{
			U32 vertexDataSize;
			bx::read(_reader, &vertexDataSize, sizeof(vertexDataSize), _err);
			vertexData = bgfx::alloc(vertexDataSize);//bgfx::makeRef(bx::alloc(mrender::getAllocator(), vertexDataSize), vertexDataSize);
			bx::read(_reader, vertexData->data, vertexDataSize, _err);

			U32 indexDataSize;
			bx::read(_reader, &indexDataSize, sizeof(indexDataSize), _err);
			indexData = bgfx::alloc(indexDataSize);//bgfx::makeRef(bx::alloc(mrender::getAllocator(), indexDataSize), indexDataSize);
			bx::read(_reader, indexData->data, indexDataSize, _err);

			U32 layoutSize;
			bx::read(_reader, &layoutSize, sizeof(layoutSize), _err);
			bx::read(_reader, &layout, layoutSize, _err);
		};

		const bgfx::Memory* vertexData;
		const bgfx::Memory* indexData;
		bgfx::VertexLayout layout;
	};

	struct ShaderResource : ResourceI
	{
		U32 getSize() override
		{
			return sizeof(codeData->size) + codeData->size;
		};

		void write(bx::WriterI* _writer, bx::Error* _err) override
		{
			bx::write(_writer, &codeData->size, sizeof(U32), _err);
			bx::write(_writer, codeData->data, codeData->size, _err);
		};

		void read(bx::ReaderSeekerI* _reader, bx::Error* _err) override
		{
			U32 size;
			bx::read(_reader, &size, sizeof(size), _err);
			codeData = bgfx::alloc(size);//bgfx::makeRef(bx::alloc(mrender::getAllocator(), size), size);
			bx::read(_reader, codeData->data, size, _err);
		};

		const bgfx::Memory* codeData;
	};

	struct TextureResource : ResourceI
	{
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
			mem = bgfx::alloc(size);//bgfx::makeRef(bx::alloc(mrender::getAllocator(), size), size);
			bx::read(_reader, mem->data, size, _err);
		};

		U16 width;
		U16 height;
		bool hasMips;
		bgfx::TextureFormat::Enum format;
		U64 flags;
		const bgfx::Memory* mem;
	};

	struct MaterialResource : ResourceI
	{
		U32 getSize() override
		{
			U32 size = 0;
			
			size += (U32)sizeof(U32) + (U32)bx::strLen(vertPath) + 1;
			size += (U32)sizeof(U32) + (U32)bx::strLen(fragPath) + 1;

			size += (U32)sizeof(U32);

			for (U32 i = 0; i < parameters.parameterHashMap.getNumElements(); i++)
			{
				U32 hash = parameters.parameterHashMap.findByHandle(i);
				const MaterialParameters::UniformData& uniformData = parameters.parameters[i];

				size += sizeof(U32); 
				size += sizeof(uniformData.type);
				size += sizeof(uniformData.num);
				size += sizeof(U32) + uniformData.data->size; 
			}

			return size;
		}

		void write(bx::WriterI* _writer, bx::Error* _err) override
		{
			U32 vertPathSize = static_cast<U32>(bx::strLen(vertPath)) + 1;
			U32 fragPathSize = static_cast<U32>(bx::strLen(fragPath)) + 1;

			bx::write(_writer, &vertPathSize, sizeof(vertPathSize), _err);
			bx::write(_writer, vertPath, vertPathSize, _err);

			bx::write(_writer, &fragPathSize, sizeof(fragPathSize), _err);
			bx::write(_writer, fragPath, fragPathSize, _err);

			////
			U32 numParameter = (U32)parameters.parameterHashMap.getNumElements();
			bx::write(_writer, &numParameter, sizeof(numParameter), _err);

			for (U32 i = 0; i < numParameter; i++)
			{
				U32 hash = parameters.parameterHashMap.findByHandle(i);
				bx::write(_writer, &hash, sizeof(hash), _err);

				const MaterialParameters::UniformData& uniformData = parameters.parameters[i];
				bx::write(_writer, &uniformData.type, sizeof(uniformData.type), _err);
				bx::write(_writer, &uniformData.num, sizeof(uniformData.num), _err);
				bx::write(_writer, &uniformData.data->size, sizeof(uniformData.data->size), _err);
				bx::write(_writer, uniformData.data->data, uniformData.data->size, _err);
			}
		}

		void read(bx::ReaderSeekerI* _reader, bx::Error* _err) override
		{
			U32 vertPathSize, fragPathSize;

			bx::read(_reader, &vertPathSize, sizeof(vertPathSize), _err);
			bx::read(_reader, vertPath, vertPathSize, _err);

			bx::read(_reader, &fragPathSize, sizeof(fragPathSize), _err);
			bx::read(_reader, fragPath, fragPathSize, _err);

			vertPath[bx::kMaxFilePath] = '\0';
			fragPath[bx::kMaxFilePath] = '\0';

			////
			U32 numParameter;
			bx::read(_reader, &numParameter, sizeof(numParameter), _err);

			// Iterate through the entries and read each one
			for (U32 i = 0; i < numParameter; ++i)
			{
				// Read the hash
				U32 hash;
				bx::read(_reader, &hash, sizeof(hash), _err);

				// Read the UniformData structure
				MaterialParameters::UniformData uniformData;
				bx::read(_reader, &uniformData.type, sizeof(uniformData.type), _err);
				bx::read(_reader, &uniformData.num, sizeof(uniformData.num), _err);
				U32 size;
				bx::read(_reader, &size, sizeof(size), _err);
				uniformData.data = bgfx::alloc(size);//bgfx::makeRef(bx::alloc(mrender::getAllocator(), size), size);
				bx::read(_reader, uniformData.data->data, size, _err);

				// Insert the entry into the map
				parameters.parameterHashMap.insert(hash, i);
				parameters.parameters[i] = uniformData;
			}
		}

		char vertPath[bx::kMaxFilePath + 1];
		char fragPath[bx::kMaxFilePath + 1];
		MaterialParameters parameters;
	};

	struct MeshResource : ResourceI
	{
		U32 getSize() override
		{
			U32 size = 0;
			size += (U32)sizeof(U32) + (U32)bx::strLen(materialPath) + 1;

			size += (U32)sizeof(U32);
			for (U32 i = 0; i < numGeometries; i++)
			{
				size += (U32)sizeof(U32) + (U32)bx::strLen(geometryPaths[i]) + 1;
			}

			return size;
		}

		void write(bx::WriterI* _writer, bx::Error* _err) override
		{
			U32 materialPathSize = static_cast<U32>(bx::strLen(materialPath)) + 1;
			bx::write(_writer, &materialPathSize, sizeof(materialPathSize), _err);
			bx::write(_writer, materialPath, materialPathSize, _err);

			bx::write(_writer, &numGeometries, sizeof(numGeometries), _err);
			for (U32 i = 0; i < numGeometries; i++)
			{
				U32 geometryPathSize = static_cast<U32>(bx::strLen(geometryPaths[i])) + 1;
				bx::write(_writer, &geometryPathSize, sizeof(geometryPathSize), _err);
				bx::write(_writer, geometryPaths[i], geometryPathSize, _err);
			}
		}

		void read(bx::ReaderSeekerI* _reader, bx::Error* _err) override
		{
			U32 materialPathSize;
			bx::read(_reader, &materialPathSize, sizeof(materialPathSize), _err);
			bx::read(_reader, materialPath, materialPathSize, _err);
			materialPath[bx::kMaxFilePath] = '\0';

			bx::read(_reader, &numGeometries, sizeof(numGeometries), _err);
			for (U32 i = 0; i < numGeometries; i++)
			{
				U32 geometryPathSize;
				bx::read(_reader, &geometryPathSize, sizeof(geometryPathSize), _err);
				bx::read(_reader, geometryPaths[i], geometryPathSize, _err);
				geometryPaths[i][bx::kMaxFilePath] = '\0';
			}
		}

		char materialPath[bx::kMaxFilePath + 1];

		U16 numGeometries;
		char geometryPaths[MENGINE_CONFIG_MAX_GEOMETRIES_PER_MESH][bx::kMaxFilePath + 1];

		F32 m_transforms[MENGINE_CONFIG_MAX_GEOMETRIES_PER_MESH][16];
	};

	struct PakEntryRef
	{
		U32 pakHash;
		I64 offset;
	};

	struct ResourceRef
	{
		ResourceI* resource;
		bx::FilePath vfp;
		U16 m_refCount;
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
		ComponentRef() // @todo I don't like to use constructors like this, the code depends on it. Fix this
			: m_data(NULL)
			, m_refCount(0)
		{}

		ComponentI* m_data;
		U16 m_refCount;
	};

	struct GeometryRef
	{
		bgfx::VertexBufferHandle m_vbh;
		bgfx::IndexBufferHandle m_ibh;

		U32 m_hash;
		U16 m_refCount;
	};

	struct ShaderRef
	{
		bgfx::ShaderHandle m_sh;

		U32 m_hash;
		U16 m_refCount;
	};

	struct TextureRef
	{
		bgfx::TextureHandle m_th;

		U32 m_hash;
		U16 m_refCount;
	};

	struct MaterialRef
	{
		bgfx::ProgramHandle m_ph;
		ShaderHandle m_vsh;
		ShaderHandle m_fsh;
		U16 m_numTextures;
		TextureHandle m_textures[MENGINE_CONFIG_MAX_UNIFORMS_PER_SHADER];
		
		U32 m_hash;
		U16 m_refCount;
	};

	struct MeshRef
	{
		MaterialHandle m_material;
		U16 m_numGeometries;
		GeometryHandle m_geometries[MENGINE_CONFIG_MAX_GEOMETRIES_PER_MESH];

		U32 m_hash;
		U16 m_refCount;
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

		MENGINE_API_FUNC(bool createPak(const bx::FilePath& _filePath))
		{
			// LAYOUT:                  // Example:
			// 
			// numEntries (U32);        // 3
			// 
			// hash (U32);              // 2039230
			// entry (PakEntryRef);     // PakEntryRef()
			// 
			// hash (U32);              // 2039230
			// entry (PakEntryRef);     // PakEntryRef()
			// 
			// hash (U32);              // 2039230
			// entry (PakEntryRef);		// PakEntryRef()
			// 
			// data (we dont know, but we dont care, should always be inherited from ResourceI anyway);
			// 

			// Clear directory
			bx::removeAll(_filePath);
			bx::makeAll(_filePath.getPath());

			// Get File Writer
			bx::FileWriter writer;

			// Open file.
			if (!bx::open(&writer, _filePath, bx::ErrorAssert{}))
			{
				BX_TRACE("Failed to write pack at path %s.", _filePath.getCPtr());
				return false;
			}

			// Write Entries
			U32 numEntries = m_resourceHashMap.getNumElements();
			bx::write(&writer, &numEntries, sizeof(U32), bx::ErrorAssert{});
			//			  numEntries             hashes                      entries
			U32 offset = sizeof(U32) + (numEntries * sizeof(U32)) + (numEntries * sizeof(PakEntryRef));
			for (U32 i = 0; i < numEntries; i++)
			{
				// Write entry hash
				ResourceRef& resource = m_resources[i];
				U32 entryHash = bx::hash<bx::HashMurmur2A>(resource.vfp.getCPtr());
				bx::write(&writer, &entryHash, sizeof(U32), bx::ErrorAssert{});

				// Create entry 
				PakEntryRef pak;
				U32 pakHash = bx::hash<bx::HashMurmur2A>(_filePath.getCPtr());
				pak.pakHash = pakHash;
				pak.offset = offset;
				offset += resource.resource->getSize();

				// Write entry 
				bx::write(&writer, &pak, sizeof(PakEntryRef), bx::ErrorAssert{});
			}

			// Write Data
			for (U32 i = 0; i < numEntries; i++)
			{
				// Write resource data
				ResourceRef& resource = m_resources[i];
				resource.resource->write(&writer, bx::ErrorAssert{});
			}

			return true;
		}

		MENGINE_API_FUNC(bool loadPak(const bx::FilePath& _filePath))
		{
			// Get File Reader.
			U32 hash = bx::hash<bx::HashMurmur2A>(_filePath.getCPtr());
			U16 fileReaderHandle = m_pakHashMap.find(hash);
			if (fileReaderHandle != kInvalidHandle)
			{
				BX_TRACE("Already loaded this pack.")
				return false;
			}
			fileReaderHandle = m_pakHandle.alloc();
			m_pakHashMap.insert(hash, fileReaderHandle);
			
			bx::FileReader* pr = &m_paks[fileReaderHandle];

			// Open file (This file will stay open until unloadPack() is called).
			if (!bx::open(pr, _filePath, bx::ErrorAssert{}))
			{
				BX_TRACE("Failed to open pack at path %s.", _filePath.getCPtr());
				return false;
			}

			// Read Entries
			U32 numEntries;
			bx::read(pr, &numEntries, sizeof(U32), bx::ErrorAssert{});
			for (U32 i = 0; i < numEntries; i++)
			{
				// Read entry hash
				U32 hash;
				bx::read(pr, &hash, sizeof(U32), bx::ErrorAssert{});

				// Read and create entry handle
				U16 entryHandle = m_pakEntryHandle.alloc();
				bool ok = m_pakEntryHashMap.insert(hash, entryHandle);
				BX_ASSERT(ok, " pack already loaded.")
				bx::read(pr, &m_pakEntries[entryHandle], sizeof(PakEntryRef), bx::ErrorAssert{});
			}
			
			return true;
		}

		MENGINE_API_FUNC(bool unloadPak(const bx::FilePath& _filePath))
		{
			// Get File Reader.
			U32 hash = bx::hash<bx::HashMurmur2A>(_filePath.getCPtr());
			U16 fileReaderHandle = m_pakHashMap.find(hash);
			bx::FileReader& pr = m_paks[fileReaderHandle];

			// Make sure we read from beginning since resources could've already been loaded 
			// and we would be in a different position.
			bx::seek(&pr, 0, bx::Whence::Begin);

			// Read Entries
			U32 numEntries;
			bx::read(&pr, &numEntries, sizeof(U32), bx::ErrorAssert{});
			for (U32 i = 0; i < numEntries; i++)
			{
				// Read entry hash
				U32 hash;
				bx::read(&pr, &hash, sizeof(U32), bx::ErrorAssert{});

				// Find resource at hash if we have it loaded 
				U16 handle = m_resourceHashMap.find(hash);
				if (kInvalidHandle != handle)
				{
					destroyResource({ handle });
				}

				// Find already created entry handle using hash
				U16 entryHandle = m_pakEntryHashMap.find(hash);

				// Remove entry from map
				m_pakEntryHashMap.removeByHandle(entryHandle);
				m_pakEntryHandle.free(entryHandle);

				// Jump over the entry data as we dont need that when unloading.
				bx::seek(&pr, sizeof(PakEntryRef), bx::Whence::Current);
			}

			// Finally close the  pack file since its no longer in use.
			bx::close(&pr);

			// Remove reader from map
			m_pakHashMap.removeByHandle(fileReaderHandle);
			m_pakHandle.free(fileReaderHandle);

			return true;
		}

		void resourceIncRef(ResourceHandle _handle)
		{
			ResourceRef& sr = m_resources[_handle.idx];
			++sr.m_refCount;
		}

		void resourceDecRef(ResourceHandle _handle)
		{
			ResourceRef& sr = m_resources[_handle.idx];
			U16 refs = --sr.m_refCount;

			if (0 == refs)
			{
				bool ok = m_freeResources.queue(_handle); BX_UNUSED(ok);
				BX_ASSERT(ok, "Resource handle %d is already destroyed!", _handle.idx);

				delete sr.resource;

				m_resourceHashMap.removeByHandle(_handle.idx);
			}
		}

		bool resourceFindOrCreate(U32 _hash, ResourceHandle& _handle)
		{
			U16 idx = m_resourceHashMap.find(_hash);
			if (kInvalidHandle != idx)
			{
				ResourceHandle handle = { idx };
				resourceIncRef(handle);
				_handle = handle;
				return true;
			}
			else
			{
				_handle = { m_resourceHandle.alloc() };
				return false;
			}
		}

		MENGINE_API_FUNC(ResourceHandle createResource(const bx::FilePath& _vfp))
		{
			U32 hash = bx::hash<bx::HashMurmur2A>(_vfp.getCPtr());

			ResourceHandle handle;
			if (resourceFindOrCreate(hash, handle))
			{
				return handle;
			}

			bool ok = m_resourceHashMap.insert(hash, handle.idx);
			BX_ASSERT(ok, "Resource already exists!"); BX_UNUSED(ok);

			ResourceRef& rr = m_resources[handle.idx];
			rr.m_refCount = 1;
			rr.vfp = _vfp;
			return handle;
		}

		MENGINE_API_FUNC(void destroyResource(ResourceHandle _handle))
		{
			MENGINE_MUTEX_SCOPE(m_resourceApiLock);

			if (!isValid(_handle) && !m_resourceHandle.isValid(_handle.idx))
			{
				BX_WARN(false, "Passing invalid resource handle to mengine::destroyResource.");
				return;
			}

			resourceDecRef(_handle);
		}

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

		void geometryIncRef(GeometryHandle _handle)
		{
			GeometryRef& gr = m_geometries[_handle.idx];
			++gr.m_refCount;
		}

		void geometryDecRef(GeometryHandle _handle)
		{
			GeometryRef& gr = m_geometries[_handle.idx];
			U16 refs = --gr.m_refCount;

			if (0 == refs)
			{
				bool ok = m_freeGeometries.queue(_handle); BX_UNUSED(ok);
				BX_ASSERT(ok, "Geometry  handle %d is already destroyed!", _handle.idx);

				bgfx::destroy(gr.m_vbh);
				bgfx::destroy(gr.m_ibh);

				m_geometryHashMap.removeByHandle(_handle.idx);
			}
		}

		bool geometryFindOrCreate(U32 _hash, GeometryHandle& _handle)
		{
			U16 idx = m_geometryHashMap.find(_hash);
			if (kInvalidHandle != idx)
			{
				GeometryHandle handle = { idx };
				geometryIncRef(handle);
				_handle = handle;
				return true;
			}
			else
			{
				_handle = { m_geometryHandle.alloc() };
				return false;
			}
		}

		MENGINE_API_FUNC(GeometryHandle createGeometry(ResourceHandle _resource))
		{
			ResourceRef& resource = m_resources[_resource.idx];
			U32 hash = bx::hash<bx::HashMurmur2A>(resource.vfp.getCPtr());

			GeometryHandle handle;
			if (geometryFindOrCreate(hash, handle))
			{
				return handle;
			}

			bool ok = m_geometryHashMap.insert(hash, handle.idx);
			BX_ASSERT(ok, "Geometry  already exists!"); BX_UNUSED(ok);

			GeometryResource* geomResource = (GeometryResource*)resource.resource;
			if (NULL == geomResource)
			{
				BX_TRACE("Resource handle is not a geometry resource.")
				return MENGINE_INVALID_HANDLE;
			}

			GeometryRef& gr = m_geometries[handle.idx];
			gr.m_refCount = 1;
			gr.m_hash = hash;

			gr.m_vbh = bgfx::createVertexBuffer(geomResource->vertexData, geomResource->layout);
			gr.m_ibh = bgfx::createIndexBuffer(geomResource->indexData);

			return handle;
		}

		MENGINE_API_FUNC(ResourceHandle createGeometryResource(const GeometryCreate& _data, const bx::FilePath& _vfp))
		{
			ResourceHandle handle = createResource(_vfp);

		    ResourceRef& rr = m_resources[handle.idx];
			rr.resource = new GeometryResource(); 

			((GeometryResource*)rr.resource)->vertexData = bgfx::copy(_data.vertices, _data.verticesSize);
			((GeometryResource*)rr.resource)->indexData = bgfx::copy(_data.indices, _data.indicesSize);
			((GeometryResource*)rr.resource)->layout = _data.layout;

			return handle;
		}

		MENGINE_API_FUNC(ResourceHandle loadGeometryResource(const bx::FilePath& _filePath))
		{
			U32 hash = bx::hash<bx::HashMurmur2A>(_filePath.getCPtr());

			ResourceHandle handle;
			if (resourceFindOrCreate(hash, handle))
			{
				return handle;
			}

			// Check if resource is inside a loaded  pack
			U16 entryHandle = m_pakEntryHashMap.find(bx::hash<bx::HashMurmur2A>(_filePath.getCPtr()));
			if (kInvalidHandle != entryHandle)
			{
				// Create resource
				bool ok = m_resourceHashMap.insert(hash, handle.idx);
				BX_ASSERT(ok, "Resource already exists!"); BX_UNUSED(ok);

				// Get pak file reader
				PakEntryRef& per = m_pakEntries[entryHandle];
				bx::FileReader* reader = &m_paks[m_pakHashMap.find(per.pakHash)];

				// Seek to the offset of the entry using the entry file pointer.
				bx::seek(reader, per.offset, bx::Whence::Begin);

				// Read resource data at offset position.
				ResourceRef& rr = m_resources[handle.idx];
				rr.m_refCount = 1;
				rr.vfp = _filePath;
				rr.resource = new GeometryResource();
				rr.resource->read(reader, bx::ErrorAssert{});

				// Return now loaded resource.
				return handle;
			}

			BX_TRACE("@todo Add support for single file loading here...")
			return handle;
		}

		MENGINE_API_FUNC(void destroyGeometry(GeometryHandle _handle))
		{
			MENGINE_MUTEX_SCOPE(m_resourceApiLock);

			if (!isValid(_handle) && !m_geometryHandle.isValid(_handle.idx))
			{
				BX_WARN(false, "Passing invalid geometry handle to mengine::destroyGeometry.");
				return;
			}

			geometryDecRef(_handle);

			GeometryRef& gr = m_geometries[_handle.idx];
			U16 resourceHandle = m_resourceHashMap.find(gr.m_hash);
			resourceDecRef({ resourceHandle });
		}

		void shaderIncRef(ShaderHandle _handle)
		{
			ShaderRef& sr = m_shaders[_handle.idx];
			++sr.m_refCount;
		}

		void shaderDecRef(ShaderHandle _handle)
		{
			ShaderRef& sr = m_shaders[_handle.idx];
			U16 refs = --sr.m_refCount;

			if (0 == refs)
			{
				bool ok = m_freeShaders.queue(_handle); BX_UNUSED(ok);
				BX_ASSERT(ok, "Shader  handle %d is already destroyed!", _handle.idx);

				bgfx::destroy(sr.m_sh);

				m_shaderHashMap.removeByHandle(_handle.idx);
			}
		}

		bool shaderFindOrCreate(U32 _hash, ShaderHandle& _handle)
		{
			U16 idx = m_shaderHashMap.find(_hash);
			if (kInvalidHandle != idx)
			{
				ShaderHandle handle = { idx };
				shaderIncRef(handle);
				_handle = handle;
				return true;
			}
			else
			{
				_handle = { m_shaderHandle.alloc() };
				return false;
			}
		}

		MENGINE_API_FUNC(ShaderHandle createShader(ResourceHandle _resource))
		{
			ResourceRef& resource = m_resources[_resource.idx];
			U32 hash = bx::hash<bx::HashMurmur2A>(resource.vfp.getCPtr());

			ShaderHandle handle;
			if (shaderFindOrCreate(hash, handle))
			{
				return handle;
			}

			bool ok = m_shaderHashMap.insert(hash, handle.idx);
			BX_ASSERT(ok, "Shader  already exists!"); BX_UNUSED(ok);

			ShaderResource* shadResource = (ShaderResource*)resource.resource;
			if (NULL == shadResource)
			{
				BX_TRACE("Resource handle is not a shader resource.")
				return MENGINE_INVALID_HANDLE;
			}

			ShaderRef& sr = m_shaders[handle.idx];
			sr.m_refCount = 1;
			sr.m_hash = hash;

			sr.m_sh = bgfx::createShader(shadResource->codeData);

			return handle;
		}

		MENGINE_API_FUNC(ResourceHandle createShaderResource(const ShaderCreate& _data, const bx::FilePath& _vfp))
		{
			ResourceHandle handle = createResource(_vfp);

			ResourceRef& rr = m_resources[handle.idx];
			rr.resource = new ShaderResource();

			((ShaderResource*)rr.resource)->codeData = bgfx::copy(_data.mem->data, _data.mem->size);

			return handle;
		}

		MENGINE_API_FUNC(ResourceHandle loadShaderResource(const bx::FilePath& _filePath))
		{
			U32 hash = bx::hash<bx::HashMurmur2A>(_filePath.getCPtr());

			ResourceHandle handle;
			if (resourceFindOrCreate(hash, handle))
			{
				return handle;
			}

			// Check if resource is inside a loaded  pack
			U16 entryHandle = m_pakEntryHashMap.find(bx::hash<bx::HashMurmur2A>(_filePath.getCPtr()));
			if (kInvalidHandle != entryHandle)
			{
				// Create resource
				bool ok = m_resourceHashMap.insert(hash, handle.idx);
				BX_ASSERT(ok, "Resource already exists!"); BX_UNUSED(ok);

				// Get pak file reader
				PakEntryRef& per = m_pakEntries[entryHandle];
				bx::FileReader* reader = &m_paks[m_pakHashMap.find(per.pakHash)];

				// Seek to the offset of the entry using the entry file pointer.
				bx::seek(reader, per.offset, bx::Whence::Begin);

				// Read resource data at offset position.
				ResourceRef& rr = m_resources[handle.idx];
				rr.m_refCount = 1;
				rr.vfp = _filePath;
				rr.resource = new ShaderResource();
				rr.resource->read(reader, bx::ErrorAssert{});

				// Return now loaded resource.
				return handle;
			}

			BX_TRACE("@todo Add support for single file loading here...")
			return handle;
		}

		MENGINE_API_FUNC(void destroyShader(ShaderHandle _handle))
		{
			MENGINE_MUTEX_SCOPE(m_resourceApiLock);

			if (!isValid(_handle) && !m_shaderHandle.isValid(_handle.idx))
			{
				BX_WARN(false, "Passing invalid shader  handle to mengine::destroyShader.");
				return;
			}

			shaderDecRef(_handle);

			ShaderRef& sr = m_shaders[_handle.idx];
			U16 resourceHandle = m_resourceHashMap.find(sr.m_hash);
			resourceDecRef({ resourceHandle });
		}

		void textureIncRef(TextureHandle _handle)
		{
			TextureRef& sr = m_textures[_handle.idx];
			++sr.m_refCount;
		}

		void textureDecRef(TextureHandle _handle)
		{
			TextureRef& sr = m_textures[_handle.idx];
			U16 refs = --sr.m_refCount;

			if (0 == refs)
			{
				bool ok = m_freeTextures.queue(_handle); BX_UNUSED(ok);
				BX_ASSERT(ok, "Texture  handle %d is already destroyed!", _handle.idx);

				bgfx::destroy(sr.m_th);

				m_textureHashMap.removeByHandle(_handle.idx);
			}
		}

		bool textureFindOrCreate(U32 _hash, TextureHandle& _handle)
		{
			U16 idx = m_textureHashMap.find(_hash);
			if (kInvalidHandle != idx)
			{
				TextureHandle handle = { idx };
				textureIncRef(handle);
				_handle = handle;
				return true;
			}
			else
			{
				_handle = { m_textureHandle.alloc() };
				return false;
			}
		}

		MENGINE_API_FUNC(TextureHandle createTexture(ResourceHandle _resource))
		{
			ResourceRef& resource = m_resources[_resource.idx];
			U32 hash = bx::hash<bx::HashMurmur2A>(resource.vfp.getCPtr());

			TextureHandle handle;
			if (textureFindOrCreate(hash, handle))
			{
				return handle;
			}
			
			bool ok = m_textureHashMap.insert(hash, handle.idx);
			BX_ASSERT(ok, "Texture  already exists!"); BX_UNUSED(ok);

			TextureResource* texResource = (TextureResource*)resource.resource;
			if (NULL == texResource)
			{
				BX_TRACE("Resource handle is not a texture resource.")
					return MENGINE_INVALID_HANDLE;
			}

			TextureRef& sr = m_textures[handle.idx];
			sr.m_refCount = 1;
			sr.m_hash = hash;

			sr.m_th = bgfx::createTexture2D(
				texResource->width,
				texResource->height,
				texResource->hasMips,
				1, 
				texResource->format,
				texResource->flags,
				texResource->mem
			);

			return handle;
		}

		MENGINE_API_FUNC(ResourceHandle createTextureResource(const TextureCreate& _data, const bx::FilePath& _vfp))
		{
			ResourceHandle handle = createResource(_vfp);

			ResourceRef& rr = m_resources[handle.idx];
			rr.resource = new TextureResource();

			((TextureResource*)rr.resource)->width = _data.width;
			((TextureResource*)rr.resource)->height = _data.height;
			((TextureResource*)rr.resource)->hasMips = _data.hasMips;
			((TextureResource*)rr.resource)->format = _data.format;
			((TextureResource*)rr.resource)->flags = _data.flags;
			((TextureResource*)rr.resource)->mem = bgfx::copy(_data.mem, _data.memSize);

			return handle;
		}

		MENGINE_API_FUNC(ResourceHandle loadTextureResource(const bx::FilePath& _filePath))
		{
			U32 hash = bx::hash<bx::HashMurmur2A>(_filePath.getCPtr());

			ResourceHandle handle;
			if (resourceFindOrCreate(hash, handle))
			{
				return handle;
			}

			// Check if resource is inside a loaded  pack
			U16 entryHandle = m_pakEntryHashMap.find(bx::hash<bx::HashMurmur2A>(_filePath.getCPtr()));
			if (kInvalidHandle != entryHandle)
			{
				// Create resource
				bool ok = m_resourceHashMap.insert(hash, handle.idx);
				BX_ASSERT(ok, "Resource already exists!"); BX_UNUSED(ok);

				// Get pak file reader
				PakEntryRef& per = m_pakEntries[entryHandle];
				bx::FileReader* reader = &m_paks[m_pakHashMap.find(per.pakHash)];
				
				// Seek to the offset of the entry using the entry file pointer.
				bx::seek(reader, per.offset, bx::Whence::Begin);

				// Read resource data at offset position.
				ResourceRef& rr = m_resources[handle.idx];
				rr.m_refCount = 1;
				rr.vfp = _filePath;
				rr.resource = new TextureResource();
				rr.resource->read(reader, bx::ErrorAssert{});

				// Return now loaded resource.
				return handle;
			}
			
			BX_TRACE("@todo Add support for single file loading here...")
			return handle;
		}

		MENGINE_API_FUNC(void destroyTexture(TextureHandle _handle))
		{
			MENGINE_MUTEX_SCOPE(m_resourceApiLock);

			if (!isValid(_handle) && !m_textureHandle.isValid(_handle.idx))
			{
				BX_WARN(false, "Passing invalid texture  handle to mengine::destroyTexture.");
				return;
			}

			textureDecRef(_handle);

			TextureRef& sr = m_textures[_handle.idx];
			U16 resourceHandle = m_resourceHashMap.find(sr.m_hash);
			resourceDecRef({ resourceHandle });
		}

		void materialIncRef(MaterialHandle _handle)
		{
			MaterialRef& sr = m_materials[_handle.idx];
			++sr.m_refCount;
		}

		void materialDecRef(MaterialHandle _handle)
		{
			MaterialRef& mr = m_materials[_handle.idx];
			U16 refs = --mr.m_refCount;

			if (0 == refs)
			{
				bool ok = m_freeMaterials.queue(_handle); BX_UNUSED(ok);
				BX_ASSERT(ok, "Material  handle %d is already destroyed!", _handle.idx);

				U16 resourceHandle = m_resourceHashMap.find(mr.m_hash);

				MaterialResource* matResource = (MaterialResource*)m_resources[resourceHandle].resource;

				bgfx::destroy(mr.m_ph);
				destroyShader(mr.m_vsh);
				destroyShader(mr.m_fsh);
				for (U32 i = 0; i < matResource->parameters.parameterHashMap.getNumElements(); i++)
				{
					MaterialParameters::UniformData& data = matResource->parameters.parameters[i];
					if (data.type == bgfx::UniformType::Sampler)
					{
						destroyTexture(mr.m_textures[i]);
					}
				}

				m_materialHashMap.removeByHandle(_handle.idx);
			}
		}

		bool materialFindOrCreate(U32 _hash, MaterialHandle& _handle)
		{
			U16 idx = m_materialHashMap.find(_hash);
			if (kInvalidHandle != idx)
			{
				MaterialHandle handle = { idx };
				materialIncRef(handle);
				_handle = handle;
				return true;
			}
			else
			{
				_handle = { m_materialHandle.alloc() };
				return false;
			}
		}

		MENGINE_API_FUNC(MaterialHandle createMaterial(ResourceHandle _resource))
		{
			ResourceRef& resource = m_resources[_resource.idx];
			U32 hash = bx::hash<bx::HashMurmur2A>(resource.vfp.getCPtr());

			MaterialHandle handle;
			if (materialFindOrCreate(hash, handle))
			{
				return handle;
			}

			bool ok = m_materialHashMap.insert(hash, handle.idx);
			BX_ASSERT(ok, "Material  already exists!"); BX_UNUSED(ok);

			MaterialResource* matResource = (MaterialResource*)resource.resource;
			if (NULL == matResource)
			{
				BX_TRACE("Resource handle is not a material resource.")
				return MENGINE_INVALID_HANDLE;
			}

			MaterialRef& sr = m_materials[handle.idx];
			sr.m_refCount = 1;
			sr.m_hash = hash;
			
			sr.m_vsh = createShader(loadShader(matResource->vertPath));
			sr.m_fsh = createShader(loadShader(matResource->fragPath));
			sr.m_ph = bgfx::createProgram(sr.m_vsh, sr.m_fsh);
			for (U32 i = 0; i < matResource->parameters.parameterHashMap.getNumElements(); i++)
			{
				MaterialParameters::UniformData& data = matResource->parameters.parameters[i];
				if (data.type == bgfx::UniformType::Sampler)
				{
					const char* vfp = (const char*)data.data->data;
					sr.m_textures[i] = createTexture(loadTexture(vfp));
				}
			}

			return handle;
		}

		MENGINE_API_FUNC(ResourceHandle createMaterialResource(const MaterialCreate& _data, const bx::FilePath& _vfp))
		{
			ResourceHandle handle = createResource(_vfp);

			ResourceRef& rr = m_resources[handle.idx];
			rr.resource = new MaterialResource();

			bx::strCopy(((MaterialResource*)rr.resource)->vertPath, bx::kMaxFilePath, _data.vertShaderPath.getCPtr());
			bx::strCopy(((MaterialResource*)rr.resource)->fragPath, bx::kMaxFilePath, _data.fragShaderPath.getCPtr());
			((MaterialResource*)rr.resource)->parameters = _data.parameters;
			return handle;
		}

		MENGINE_API_FUNC(ResourceHandle loadMaterialResource(const bx::FilePath& _filePath))
		{
			U32 hash = bx::hash<bx::HashMurmur2A>(_filePath.getCPtr());

			ResourceHandle handle;
			if (resourceFindOrCreate(hash, handle))
			{
				return handle;
			}

			// Check if resource is inside a loaded  pack
			U16 entryHandle = m_pakEntryHashMap.find(bx::hash<bx::HashMurmur2A>(_filePath.getCPtr()));
			if (kInvalidHandle != entryHandle)
			{
				// Create resource
				bool ok = m_resourceHashMap.insert(hash, handle.idx);
				BX_ASSERT(ok, "Resource already exists!"); BX_UNUSED(ok);

				// Get pak file reader
				PakEntryRef& per = m_pakEntries[entryHandle];
				bx::FileReader* reader = &m_paks[m_pakHashMap.find(per.pakHash)];

				// Seek to the offset of the entry using the entry file pointer.
				bx::seek(reader, per.offset, bx::Whence::Begin);

				// Read resource data at offset position.
				ResourceRef& rr = m_resources[handle.idx];
				rr.m_refCount = 1;
				rr.vfp = _filePath;
				rr.resource = new MaterialResource();
				rr.resource->read(reader, bx::ErrorAssert{});

				// Return now loaded resource.
				return handle;
			}

			BX_TRACE("@todo Add support for single file loading here...")
			return handle;
		}

		MENGINE_API_FUNC(void destroyMaterial(MaterialHandle _handle))
		{
			MENGINE_MUTEX_SCOPE(m_resourceApiLock);

			if (!isValid(_handle) && !m_materialHandle.isValid(_handle.idx))
			{
				BX_WARN(false, "Passing invalid material  handle to mengine::destroyMaterial.");
				return;
			}

			materialDecRef(_handle);

			// Resources
			MaterialRef& mr = m_materials[_handle.idx];
			U16 resourceHandle = m_resourceHashMap.find(mr.m_hash);
			destroyResource({ resourceHandle });

			//ShaderRef& vsr = m_shaders[mr.m_vsh.idx];
			//destroyResource({ m_resourceHashMap.find(vsr.m_hash) });

			//ShaderRef& fsr = m_shaders[mr.m_fsh.idx];
			//destroyResource({ m_resourceHashMap.find(fsr.m_hash) });
		}

		void meshIncRef(MeshHandle _handle)
		{
			MeshRef& sr = m_meshes[_handle.idx];
			++sr.m_refCount;
		}

		void meshDecRef(MeshHandle _handle)
		{
			MeshRef& mr = m_meshes[_handle.idx];
			U16 refs = --mr.m_refCount;

			if (0 == refs)
			{
				bool ok = m_freeMeshes.queue(_handle); BX_UNUSED(ok);
				BX_ASSERT(ok, "Mesh  handle %d is already destroyed!", _handle.idx);

				destroyMaterial(mr.m_material);
				for (U32 i = 0; i < mr.m_numGeometries; i++)
				{
					destroyGeometry(mr.m_geometries[i]);
				}
				mr.m_numGeometries = 0;

				m_meshHashMap.removeByHandle(_handle.idx);
			}
		}

		bool meshFindOrCreate(U32 _hash, MeshHandle& _handle)
		{
			U16 idx = m_meshHashMap.find(_hash);
			if (kInvalidHandle != idx)
			{
				MeshHandle handle = { idx };
				meshIncRef(handle);
				_handle = handle;
				return true;
			}
			else
			{
				_handle = { m_meshHandle.alloc() };
				return false;
			}
		}

		MENGINE_API_FUNC(MeshHandle createMesh(ResourceHandle _resource))
		{
			ResourceRef& resource = m_resources[_resource.idx];
			U32 hash = bx::hash<bx::HashMurmur2A>(resource.vfp.getCPtr());

			MeshHandle handle;
			if (meshFindOrCreate(hash, handle))
			{
				return handle;
			}

			bool ok = m_meshHashMap.insert(hash, handle.idx);
			BX_ASSERT(ok, "Mesh already exists!"); BX_UNUSED(ok);

			MeshResource* meshResource = (MeshResource*)resource.resource;
			if (NULL == meshResource)
			{
				BX_TRACE("Resource handle is not a mesh resource.")
				return MENGINE_INVALID_HANDLE;
			}

			MeshRef& sr = m_meshes[handle.idx];
			sr.m_refCount = 1;
			sr.m_hash = hash;

			sr.m_material = createMaterial(loadMaterial(meshResource->materialPath));
			sr.m_numGeometries = meshResource->numGeometries;
			for (U32 i = 0; i < meshResource->numGeometries; i++)
			{
				sr.m_geometries[i] = createGeometry(loadGeometry(meshResource->geometryPaths[i]));
			}

			return handle;
		}

		MENGINE_API_FUNC(ResourceHandle createMeshResource(const MeshCreate& _data, const bx::FilePath& _vfp))
		{
			ResourceHandle handle = createResource(_vfp);

			ResourceRef& rr = m_resources[handle.idx];
			rr.resource = new MeshResource();

			bx::strCopy(((MeshResource*)rr.resource)->materialPath, bx::kMaxFilePath, _data.materialPath.getCPtr());
			for (U32 i = 0; i < _data.numGeometries; i++)
			{
				bx::strCopy(((MeshResource*)rr.resource)->geometryPaths[i], bx::kMaxFilePath, _data.geometryPaths[i].getCPtr());
				for (U32 j = 0; j < 16; j++)
				{
					((MeshResource*)rr.resource)->m_transforms[i][j] = _data.m_transforms[i][j];
				}
			}
			((MeshResource*)rr.resource)->numGeometries = _data.numGeometries;


			return handle;
		}

		MENGINE_API_FUNC(ResourceHandle loadMeshResource(const bx::FilePath& _filePath))
		{
			U32 hash = bx::hash<bx::HashMurmur2A>(_filePath.getCPtr());

			ResourceHandle handle;
			if (resourceFindOrCreate(hash, handle))
			{
				return handle;
			}

			// Check if resource is inside a loaded  pack
			U16 entryHandle = m_pakEntryHashMap.find(bx::hash<bx::HashMurmur2A>(_filePath.getCPtr()));
			if (kInvalidHandle != entryHandle)
			{
				// Create resource
				bool ok = m_resourceHashMap.insert(hash, handle.idx);
				BX_ASSERT(ok, "Resource already exists!"); BX_UNUSED(ok);

				// Get pak file reader
				PakEntryRef& per = m_pakEntries[entryHandle];
				bx::FileReader* reader = &m_paks[m_pakHashMap.find(per.pakHash)];

				// Seek to the offset of the entry using the entry file pointer.
				bx::seek(reader, per.offset, bx::Whence::Begin);

				// Read resource data at offset position.
				ResourceRef& rr = m_resources[handle.idx];
				rr.m_refCount = 1;
				rr.vfp = _filePath;
				rr.resource = new MeshResource();
				rr.resource->read(reader, bx::ErrorAssert{});

				// Return now loaded resource.
				return handle;
			}

			BX_TRACE("@todo Add support for single file loading here...")
			return handle;
		}

		MENGINE_API_FUNC(void destroyMesh(MeshHandle _handle))
		{
			MENGINE_MUTEX_SCOPE(m_resourceApiLock);

			if (!isValid(_handle) && !m_meshHandle.isValid(_handle.idx))
			{
				BX_WARN(false, "Passing invalid mesh  handle to mengine::destroyMesh.");
				return;
			}

			meshDecRef(_handle);

			// Resources
			MeshRef& mr = m_meshes[_handle.idx];
			U16 resourceHandle = m_resourceHashMap.find(mr.m_hash);
			destroyResource({ resourceHandle });
		}

		MENGINE_API_FUNC(const mrender::MouseState* getMouseState())
		{
			return &m_mouseState;
		}

		MENGINE_API_FUNC(const Stats* getStats())
		{
			Stats& stats = m_stats;

			stats.numPaks = m_pakHashMap.getNumElements();
			stats.numPakEntries = m_pakEntryHashMap.getNumElements();

			stats.numEntities = m_entityHandle.getNumHandles();
			stats.numComponents = m_componentHandle.getNumHandles();
			stats.numResources = m_resourceHandle.getNumHandles();
			stats.numGeometries = m_geometryHandle.getNumHandles();
			stats.numShaders = m_shaderHandle.getNumHandles();
			stats.numTextures = m_textureHandle.getNumHandles();
			stats.numMaterials = m_materialHandle.getNumHandles();
			stats.numMeshes = m_meshHandle.getNumHandles();

			for (U16 i = 0; i < stats.numResources; i++)
			{
				stats.resourcesRef[i] = m_resources[i].m_refCount;
			}
			for (U16 i = 0; i < stats.numEntities; i++)
			{
				stats.entitiesRef[i] = m_entities[i].m_refCount;
			}
			for (U16 i = 0; i < stats.numComponents; i++)
			{
				stats.componentsRef[i] = m_components[i].m_refCount;
			}
			for (U16 i = 0; i < stats.numGeometries; i++)
			{
				stats.geometryRef[i] = m_geometries[i].m_refCount;
			}
			for (U16 i = 0; i < stats.numShaders; i++)
			{
				stats.shaderRef[i] = m_shaders[i].m_refCount;
			}
			for (U16 i = 0; i < stats.numTextures; i++)
			{
				stats.textureRef[i] = m_textures[i].m_refCount;
			}
			for (U16 i = 0; i < stats.numMaterials; i++)
			{
				stats.materialRef[i] = m_materials[i].m_refCount;
			}
			for (U16 i = 0; i < stats.numMeshes; i++)
			{
				stats.meshRef[i] = m_meshes[i].m_refCount;
			}

			return &stats;
		}

		Stats m_stats;
		
		bx::HandleAllocT<MENGINE_CONFIG_MAX_PAKS> m_pakHandle;
		bx::HandleHashMapT<MENGINE_CONFIG_MAX_PAKS> m_pakHashMap;
		bx::FileReader m_paks[MENGINE_CONFIG_MAX_PAKS];

		bx::HandleAllocT<MENGINE_CONFIG_MAX_PAK_ENTRIES> m_pakEntryHandle;
		bx::HandleHashMapT<MENGINE_CONFIG_MAX_PAK_ENTRIES> m_pakEntryHashMap;
		PakEntryRef m_pakEntries[MENGINE_CONFIG_MAX_PAK_ENTRIES];

		bx::HandleAllocT<MENGINE_CONFIG_MAX_RESOURCES> m_resourceHandle;
		bx::HandleHashMapT<MENGINE_CONFIG_MAX_RESOURCES> m_resourceHashMap;
		ResourceRef m_resources[MENGINE_CONFIG_MAX_RESOURCES];

		bx::HandleAllocT<MENGINE_CONFIG_MAX_COMPONENTS> m_componentHandle;
		bx::HandleHashMapT<MENGINE_CONFIG_MAX_COMPONENTS_PER_TYPE> m_componentHashMap[32];
		ComponentRef m_components[MENGINE_CONFIG_MAX_COMPONENTS];

		bx::HandleAllocT<MENGINE_CONFIG_MAX_ENTITIES> m_entityHandle;
		bx::HandleHashMapT<MENGINE_CONFIG_MAX_COMPONENTS_PER_TYPE> m_entityHashMap[32];
		EntityRef m_entities[MENGINE_CONFIG_MAX_ENTITIES];

		bx::HandleAllocT<MENGINE_CONFIG_MAX_GEOMETRIES> m_geometryHandle;
		bx::HandleHashMapT<MENGINE_CONFIG_MAX_GEOMETRIES> m_geometryHashMap;
		GeometryRef m_geometries[MENGINE_CONFIG_MAX_GEOMETRIES];

		bx::HandleAllocT<MENGINE_CONFIG_MAX_SHADERS> m_shaderHandle;
		bx::HandleHashMapT<MENGINE_CONFIG_MAX_SHADERS> m_shaderHashMap;
		ShaderRef m_shaders[MENGINE_CONFIG_MAX_SHADERS];

		bx::HandleAllocT<MENGINE_CONFIG_MAX_TEXTURES> m_textureHandle;
		bx::HandleHashMapT<MENGINE_CONFIG_MAX_TEXTURES> m_textureHashMap;
		TextureRef m_textures[MENGINE_CONFIG_MAX_TEXTURES];

		bx::HandleAllocT<MENGINE_CONFIG_MAX_MATERIALS> m_materialHandle;
		bx::HandleHashMapT<MENGINE_CONFIG_MAX_MATERIALS> m_materialHashMap;
		MaterialRef m_materials[MENGINE_CONFIG_MAX_MATERIALS];

		bx::HandleAllocT<MENGINE_CONFIG_MAX_MESHES> m_meshHandle;
		bx::HandleHashMapT<MENGINE_CONFIG_MAX_MESHES> m_meshHashMap;
		MeshRef m_meshes[MENGINE_CONFIG_MAX_MESHES];

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

		FreeHandle<ResourceHandle, MENGINE_CONFIG_MAX_RESOURCES>  m_freeResources;
		FreeHandle<EntityHandle, MENGINE_CONFIG_MAX_ENTITIES>  m_freeEntities;
		FreeHandle<ComponentHandle, MENGINE_CONFIG_MAX_COMPONENTS> m_freeComponents;
		FreeHandle<GeometryHandle, MENGINE_CONFIG_MAX_GEOMETRIES>  m_freeGeometries;
		FreeHandle<ShaderHandle, MENGINE_CONFIG_MAX_SHADERS> m_freeShaders;
		FreeHandle<TextureHandle, MENGINE_CONFIG_MAX_TEXTURES> m_freeTextures;
		FreeHandle<MaterialHandle, MENGINE_CONFIG_MAX_MATERIALS> m_freeMaterials;
		FreeHandle<MeshHandle, MENGINE_CONFIG_MAX_MESHES> m_freeMeshes;

		mrender::MouseState m_mouseState;
	};

} // namespace mengine

#endif // MENGINE_P_H_HEADER_GUARD