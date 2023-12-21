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
			bx::write(_writer, &layout, (I32)layoutSize, _err);
		};

		void read(bx::ReaderSeekerI* _reader, bx::Error* _err) override
		{
			U32 vertexDataSize;
			bx::read(_reader, &vertexDataSize, sizeof(vertexDataSize), _err);
			vertexData = bgfx::alloc(vertexDataSize);
			bx::read(_reader, vertexData->data, vertexDataSize, _err);

			U32 indexDataSize;
			bx::read(_reader, &indexDataSize, sizeof(indexDataSize), _err);
			indexData = bgfx::alloc(indexDataSize);
			bx::read(_reader, indexData->data, indexDataSize, _err);

			U32 layoutSize;
			bx::read(_reader, &layoutSize, sizeof(layoutSize), _err);
			bx::read(_reader, &layout, (I32)layoutSize, _err);
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
			codeData = bgfx::alloc(size);
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
			mem = bgfx::alloc(size);
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
			// Account for null-terminators in both strings
			return (bx::kMaxFilePath + 1) * 2;
		}

		void write(bx::WriterI* _writer, bx::Error* _err) override
		{
			// Include null terminator in the size calculation
			U32 vertPathSize = static_cast<U32>(bx::strLen(vertPath)) + 1;
			U32 fragPathSize = static_cast<U32>(bx::strLen(fragPath)) + 1;

			bx::write(_writer, &vertPathSize, sizeof(vertPathSize), _err);
			bx::write(_writer, vertPath, vertPathSize, _err);

			bx::write(_writer, &fragPathSize, sizeof(fragPathSize), _err);
			bx::write(_writer, fragPath, fragPathSize, _err);
		}

		void read(bx::ReaderSeekerI* _reader, bx::Error* _err) override
		{
			U32 vertPathSize, fragPathSize;

			bx::read(_reader, &vertPathSize, sizeof(vertPathSize), _err);
			bx::read(_reader, vertPath, vertPathSize, _err);

			bx::read(_reader, &fragPathSize, sizeof(fragPathSize), _err);
			bx::read(_reader, fragPath, fragPathSize, _err);

			// Ensure null-termination
			vertPath[bx::kMaxFilePath] = '\0';
			fragPath[bx::kMaxFilePath] = '\0';
		}

		char vertPath[bx::kMaxFilePath + 1];
		char fragPath[bx::kMaxFilePath + 1];
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
		U16 m_refCount;
	};

	struct ShaderRef
	{
		bgfx::ShaderHandle m_sh;
		U16 m_refCount;
	};

	struct TextureRef
	{
		bgfx::TextureHandle m_th;
		U16 m_refCount;
	};

	struct MaterialRef
	{
		bgfx::ProgramHandle m_ph;
		ShaderHandle m_vsh;
		ShaderHandle m_fsh;
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

		// @todo Asset packing needs refactoring, currently loads entire pack into memory.
		// Implement load on demand. Also dont like the use of 'new' use alloc instead.
		MENGINE_API_FUNC(bool packAssets(const bx::FilePath& _filePath))
		{
			// Clear directory
			bx::removeAll(_filePath);
			bx::makeAll(_filePath.getPath());

			// Get File Writer
			bx::FileWriter writer;

			// Open file.
			if (!bx::open(&writer, _filePath, bx::ErrorAssert{}))
			{
				BX_TRACE("Failed to write assetpack at path %s.", _filePath.getCPtr());
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

			return true;
		}

		MENGINE_API_FUNC(bool loadAssetPack(const bx::FilePath& _filePath))
		{
			// Get File Reader.
			U32 hash = bx::hash<bx::HashMurmur2A>(_filePath.getCPtr());
			U16 fileReaderHandle = m_pakReaderHashMap.find(hash);
			if (fileReaderHandle != kInvalidHandle)
			{
				BX_TRACE("Already loaded this assetpack.")
				return false;
			}
			fileReaderHandle = m_pakReaderHandle.alloc();
			m_pakReaderHashMap.insert(hash, fileReaderHandle);
			
			bx::FileReader* pr = &m_pakReaders[fileReaderHandle];

			// Open file (This file will stay open until unloadAssetPack() is called).
			if (!bx::open(pr, _filePath, bx::ErrorAssert{}))
			{
				BX_TRACE("Failed to open assetpack at path %s.", _filePath.getCPtr());
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
				BX_ASSERT(ok, "Asset pack already loaded.")
				bx::read(pr, &m_pakEntries[entryHandle], sizeof(PakEntryRef), bx::ErrorAssert{});
			}
			
			return true;
		}

		MENGINE_API_FUNC(bool unloadAssetPack(const bx::FilePath& _filePath))
		{
			// Get File Reader.
			U32 hash = bx::hash<bx::HashMurmur2A>(_filePath.getCPtr());
			U16 fileReaderHandle = m_pakReaderHashMap.find(hash);
			bx::FileReader& pr = m_pakReaders[fileReaderHandle];

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

				// Find already created entry handle using hash
				U16 entryHandle = m_pakEntryHashMap.find(hash);

				// Remove entry from map
				m_pakEntryHashMap.removeByHandle(entryHandle);
				m_pakEntryHandle.free(entryHandle);

				// Jump over the entry data as we dont need that when unloading.
				bx::seek(&pr, sizeof(PakEntryRef), bx::Whence::Current);
			}

			// Finally close the asset pack file since its no longer in use.
			bx::close(&pr);

			// Remove reader from map
			m_pakReaderHashMap.removeByHandle(fileReaderHandle);
			m_pakReaderHandle.free(fileReaderHandle);

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

				//bx::free(mrender::getAllocator(), sr.resource);
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

		void geometryAssetIncRef(GeometryHandle _handle)
		{
			GeometryRef& gr = m_geometryAssets[_handle.idx];
			++gr.m_refCount;
		}

		void geometryAssetDecRef(GeometryHandle _handle)
		{
			GeometryRef& gr = m_geometryAssets[_handle.idx];
			U16 refs = --gr.m_refCount;

			if (0 == refs)
			{
				bool ok = m_freeGeometryAssets.queue(_handle); BX_UNUSED(ok);
				BX_ASSERT(ok, "Geometry Asset handle %d is already destroyed!", _handle.idx);

				bgfx::destroy(gr.m_vbh);
				bgfx::destroy(gr.m_ibh);

				m_geometryAssetHashMap.removeByHandle(_handle.idx);
			}
		}

		bool geometryAssetFindOrCreate(U32 _hash, GeometryHandle& _handle)
		{
			U16 idx = m_geometryAssetHashMap.find(_hash);
			if (kInvalidHandle != idx)
			{
				GeometryHandle handle = { idx };
				geometryAssetIncRef(handle);
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
			if (geometryAssetFindOrCreate(hash, handle))
			{
				return handle;
			}

			bool ok = m_geometryAssetHashMap.insert(hash, handle.idx);
			BX_ASSERT(ok, "Geometry asset already exists!"); BX_UNUSED(ok);

			GeometryResource* geomResource = (GeometryResource*)resource.resource;
			if (NULL == geomResource)
			{
				BX_TRACE("Resource handle is not a geometry resource.")
				return MENGINE_INVALID_HANDLE;
			}

			GeometryRef& gr = m_geometryAssets[handle.idx];
			gr.m_refCount = 1;
			gr.m_vbh = bgfx::createVertexBuffer(geomResource->vertexData, geomResource->layout);
			gr.m_ibh = bgfx::createIndexBuffer(geomResource->indexData);
			return handle;
		}

		MENGINE_API_FUNC(ResourceHandle createGeometryResource(const GeometryCreate& _data, const bx::FilePath& _vfp))
		{
			ResourceHandle handle = createResource(_vfp);

		    ResourceRef& rr = m_resources[handle.idx];
			rr.resource = new GeometryResource(); 

			((GeometryResource*)rr.resource)->vertexData = bgfx::makeRef(_data.vertices, _data.verticesSize);
			((GeometryResource*)rr.resource)->indexData = bgfx::makeRef(_data.indices, _data.indicesSize);
			((GeometryResource*)rr.resource)->layout = _data.layout;

			return handle;
		}

		MENGINE_API_FUNC(ResourceHandle loadGeometryResource(const bx::FilePath& _filePath))
		{
			ResourceHandle handle = createResource(_filePath);

			U16 entryHandle = m_pakEntryHashMap.find(bx::hash<bx::HashMurmur2A>(_filePath.getCPtr()));
			PakEntryRef& per = m_pakEntries[entryHandle];
			bx::FileReader* reader = &m_pakReaders[m_pakReaderHashMap.find(per.pakHash)];
			// Seek to the offset of the entry using the entry file pointer.
			bx::seek(reader, per.offset, bx::Whence::Begin);

			// Read resource data at offset position.
			ResourceRef& rr = m_resources[handle.idx];
			rr.resource = new GeometryResource();
			rr.resource->read(reader, bx::ErrorAssert{});

			// Return now loaded resource.
			return handle;
		}

		MENGINE_API_FUNC(void destroyGeometry(GeometryHandle _handle))
		{
			MENGINE_MUTEX_SCOPE(m_resourceApiLock);

			if (!isValid(_handle) && !m_geometryHandle.isValid(_handle.idx))
			{
				BX_WARN(false, "Passing invalid geometry asset handle to mengine::destroyGeometry.");
				return;
			}

			geometryAssetDecRef(_handle);
		}

		void shaderAssetIncRef(ShaderHandle _handle)
		{
			ShaderRef& sr = m_shaderAssets[_handle.idx];
			++sr.m_refCount;
		}

		void shaderAssetDecRef(ShaderHandle _handle)
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

		bool shaderAssetFindOrCreate(U32 _hash, ShaderHandle& _handle)
		{
			U16 idx = m_shaderAssetHashMap.find(_hash);
			if (kInvalidHandle != idx)
			{
				ShaderHandle handle = { idx };
				shaderAssetIncRef(handle);
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
			if (shaderAssetFindOrCreate(hash, handle))
			{
				return handle;
			}

			bool ok = m_shaderAssetHashMap.insert(hash, handle.idx);
			BX_ASSERT(ok, "Shader asset already exists!"); BX_UNUSED(ok);

			ShaderResource* shadResource = (ShaderResource*)resource.resource;
			if (NULL == shadResource)
			{
				BX_TRACE("Resource handle is not a shader resource.")
				return MENGINE_INVALID_HANDLE;
			}

			ShaderRef& sr = m_shaderAssets[handle.idx];
			sr.m_refCount = 1;
			sr.m_sh = bgfx::createShader(shadResource->codeData);

			return handle;
		}

		MENGINE_API_FUNC(ResourceHandle createShaderResource(const ShaderCreate& _data, const bx::FilePath& _vfp))
		{
			ResourceHandle handle = createResource(_vfp);

			ResourceRef& rr = m_resources[handle.idx];
			rr.resource = new ShaderResource();

			((ShaderResource*)rr.resource)->codeData = _data.mem;

			return handle;
		}

		MENGINE_API_FUNC(ResourceHandle loadShaderResource(const bx::FilePath& _filePath))
		{
			ResourceHandle handle = createResource(_filePath);

			U16 entryHandle = m_pakEntryHashMap.find(bx::hash<bx::HashMurmur2A>(_filePath.getCPtr()));
			PakEntryRef& per = m_pakEntries[entryHandle];
			bx::FileReader* reader = &m_pakReaders[m_pakReaderHashMap.find(per.pakHash)];
			// Seek to the offset of the entry using the entry file pointer.
			bx::seek(reader, per.offset, bx::Whence::Begin);

			// Read resource data at offset position.
			ResourceRef& rr = m_resources[handle.idx];
			rr.resource = new ShaderResource();
			rr.resource->read(reader, bx::ErrorAssert{});

			// Return now loaded resource.
			return handle;
		}


		MENGINE_API_FUNC(void destroyShader(ShaderHandle _handle))
		{
			MENGINE_MUTEX_SCOPE(m_resourceApiLock);

			if (!isValid(_handle) && !m_shaderHandle.isValid(_handle.idx))
			{
				BX_WARN(false, "Passing invalid shader asset handle to mengine::destroyShader.");
				return;
			}

			shaderAssetDecRef(_handle);
		}

		void textureAssetIncRef(TextureHandle _handle)
		{
			TextureRef& sr = m_textureAssets[_handle.idx];
			++sr.m_refCount;
		}

		void textureAssetDecRef(TextureHandle _handle)
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

		bool textureAssetFindOrCreate(U32 _hash, TextureHandle& _handle)
		{
			U16 idx = m_textureAssetHashMap.find(_hash);
			if (kInvalidHandle != idx)
			{
				TextureHandle handle = { idx };
				textureAssetIncRef(handle);
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
			if (textureAssetFindOrCreate(hash, handle))
			{
				return handle;
			}
			
			bool ok = m_textureAssetHashMap.insert(hash, handle.idx);
			BX_ASSERT(ok, "Texture asset already exists!"); BX_UNUSED(ok);

			TextureResource* texResource = (TextureResource*)resource.resource;
			if (NULL == texResource)
			{
				BX_TRACE("Resource handle is not a texture resource.")
					return MENGINE_INVALID_HANDLE;
			}

			TextureRef& sr = m_textureAssets[handle.idx];
			sr.m_refCount = 1;
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
			((TextureResource*)rr.resource)->mem = bgfx::makeRef(_data.mem, _data.memSize);

			return handle;
		}

		MENGINE_API_FUNC(ResourceHandle loadTextureResource(const bx::FilePath& _filePath))
		{
			ResourceHandle handle = createResource(_filePath);

			U16 entryHandle = m_pakEntryHashMap.find(bx::hash<bx::HashMurmur2A>(_filePath.getCPtr()));
			PakEntryRef& per = m_pakEntries[entryHandle];
			bx::FileReader* reader = &m_pakReaders[m_pakReaderHashMap.find(per.pakHash)];
			// Seek to the offset of the entry using the entry file pointer.
			bx::seek(reader, per.offset, bx::Whence::Begin);

			// Read resource data at offset position.
			ResourceRef& rr = m_resources[handle.idx];
			rr.resource = new TextureResource();
			rr.resource->read(reader, bx::ErrorAssert{});

			// Return now loaded resource.
			return handle;
		}

		MENGINE_API_FUNC(void destroyTexture(TextureHandle _handle))
		{
			MENGINE_MUTEX_SCOPE(m_resourceApiLock);

			if (!isValid(_handle) && !m_textureHandle.isValid(_handle.idx))
			{
				BX_WARN(false, "Passing invalid texture asset handle to mengine::destroyTexture.");
				return;
			}

			textureAssetDecRef(_handle);
		}

		void materialAssetIncRef(MaterialHandle _handle)
		{
			MaterialRef& sr = m_materialAssets[_handle.idx];
			++sr.m_refCount;
		}

		void materialAssetDecRef(MaterialHandle _handle)
		{
			MaterialRef& sr = m_materialAssets[_handle.idx];
			U16 refs = --sr.m_refCount;

			if (0 == refs)
			{
				bool ok = m_freeMaterialAssets.queue(_handle); BX_UNUSED(ok);
				BX_ASSERT(ok, "Material Asset handle %d is already destroyed!", _handle.idx);

				bgfx::destroy(sr.m_ph);
				destroyShader(sr.m_vsh);
				destroyShader(sr.m_fsh);

				m_materialAssetHashMap.removeByHandle(_handle.idx);
			}
		}

		bool materialAssetFindOrCreate(U32 _hash, MaterialHandle& _handle)
		{
			U16 idx = m_materialAssetHashMap.find(_hash);
			if (kInvalidHandle != idx)
			{
				MaterialHandle handle = { idx };
				materialAssetIncRef(handle);
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
			if (materialAssetFindOrCreate(hash, handle))
			{
				return handle;
			}

			bool ok = m_materialAssetHashMap.insert(hash, handle.idx);
			BX_ASSERT(ok, "Material asset already exists!"); BX_UNUSED(ok);

			MaterialResource* matResource = (MaterialResource*)resource.resource;
			if (NULL == matResource)
			{
				BX_TRACE("Resource handle is not a material resource.")
					return MENGINE_INVALID_HANDLE;
			}

			MaterialRef& sr = m_materialAssets[handle.idx];
			sr.m_refCount = 1;
			
			sr.m_vsh = createShader(loadShader(matResource->vertPath));
			sr.m_fsh = createShader(loadShader(matResource->fragPath));
			sr.m_ph = bgfx::createProgram(sr.m_vsh, sr.m_fsh);

			return handle;
		}

		MENGINE_API_FUNC(ResourceHandle createMaterialResource(const MaterialCreate& _data, const bx::FilePath& _vfp))
		{
			ResourceHandle handle = createResource(_vfp);

			ResourceRef& rr = m_resources[handle.idx];
			rr.resource = new MaterialResource();

			bx::strCopy(((MaterialResource*)rr.resource)->vertPath, bx::kMaxFilePath, _data.vertShaderPath.getCPtr());
			bx::strCopy(((MaterialResource*)rr.resource)->fragPath, bx::kMaxFilePath, _data.fragShaderPath.getCPtr());

			return handle;
		}

		MENGINE_API_FUNC(ResourceHandle loadMaterialResource(const bx::FilePath& _filePath))
		{
			ResourceHandle handle = createResource(_filePath);

			U16 entryHandle = m_pakEntryHashMap.find(bx::hash<bx::HashMurmur2A>(_filePath.getCPtr()));
			PakEntryRef& per = m_pakEntries[entryHandle];
			bx::FileReader* reader = &m_pakReaders[m_pakReaderHashMap.find(per.pakHash)];
			// Seek to the offset of the entry using the entry file pointer.
			bx::seek(reader, per.offset, bx::Whence::Begin);

			// Read resource data at offset position.
			ResourceRef& rr = m_resources[handle.idx];
			rr.resource = new MaterialResource();
			rr.resource->read(reader, bx::ErrorAssert{});

			// Return now loaded resource.
			return handle;
		}

		MENGINE_API_FUNC(void destroyMaterial(MaterialHandle _handle))
		{
			MENGINE_MUTEX_SCOPE(m_resourceApiLock);

			if (!isValid(_handle) && !m_materialHandle.isValid(_handle.idx))
			{
				BX_WARN(false, "Passing invalid material asset handle to mengine::destroyMaterial.");
				return;
			}

			materialAssetDecRef(_handle);
		}

		MENGINE_API_FUNC(void setMaterialTextures(MaterialHandle _material, const char* _uniform, TextureHandle _texture))
		{
			
		}

		MENGINE_API_FUNC(void setMaterialUniform(MaterialHandle _handle, bgfx::UniformType::Enum _type, const char* _name, void* _value, U16 _num))
		{
			
		}

		MENGINE_API_FUNC(const mrender::MouseState* getMouseState())
		{
			return &m_mouseState;
		}

		MENGINE_API_FUNC(const Stats* getStats())
		{
			Stats& stats = m_stats;

			stats.numPaks = m_pakReaderHashMap.getNumElements();
			stats.numEntries = m_pakEntryHashMap.getNumElements();

			stats.numEntities = m_entityHandle.getNumHandles();
			stats.numComponents = m_componentHandle.getNumHandles();
			stats.numResources = m_resourceHandle.getNumHandles();
			stats.numGeometryAssets = m_geometryHandle.getNumHandles();
			stats.numShaderAssets = m_shaderHandle.getNumHandles();
			stats.numTextureAssets = m_textureHandle.getNumHandles();
			stats.numMaterialAssets = m_materialHandle.getNumHandles();

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
		
		bx::HandleAllocT<MENGINE_CONFIG_MAX_ASSET_PACKS> m_pakEntryHandle;
		bx::HandleHashMapT<MENGINE_CONFIG_MAX_ASSET_PACKS> m_pakEntryHashMap;
		PakEntryRef m_pakEntries[MENGINE_CONFIG_MAX_ASSET_PACKS];

		bx::HandleAllocT<MENGINE_CONFIG_MAX_ASSET_PACKS> m_pakReaderHandle;
		bx::HandleHashMapT<MENGINE_CONFIG_MAX_ASSET_PACKS> m_pakReaderHashMap;
		bx::FileReader m_pakReaders[MENGINE_CONFIG_MAX_ASSET_PACKS];

		bx::HandleAllocT<MENGINE_CONFIG_MAX_RESOURCES> m_resourceHandle;
		bx::HandleHashMapT<MENGINE_CONFIG_MAX_RESOURCES> m_resourceHashMap;
		ResourceRef m_resources[MENGINE_CONFIG_MAX_RESOURCES];

		bx::HandleAllocT<MENGINE_CONFIG_MAX_COMPONENTS> m_componentHandle;
		bx::HandleHashMapT<MENGINE_CONFIG_MAX_COMPONENTS_PER_TYPE> m_componentHashMap[32];
		ComponentRef m_components[MENGINE_CONFIG_MAX_COMPONENTS];

		bx::HandleAllocT<MENGINE_CONFIG_MAX_ENTITIES> m_entityHandle;
		bx::HandleHashMapT<MENGINE_CONFIG_MAX_COMPONENTS_PER_TYPE> m_entityHashMap[32];
		EntityRef m_entities[MENGINE_CONFIG_MAX_ENTITIES];

		bx::HandleAllocT<MENGINE_CONFIG_MAX_GEOMETRY_ASSETS> m_geometryHandle;
		bx::HandleHashMapT<MENGINE_CONFIG_MAX_GEOMETRY_ASSETS> m_geometryAssetHashMap;
		GeometryRef m_geometryAssets[MENGINE_CONFIG_MAX_GEOMETRY_ASSETS];

		bx::HandleAllocT<MENGINE_CONFIG_MAX_SHADER_ASSETS> m_shaderHandle;
		bx::HandleHashMapT<MENGINE_CONFIG_MAX_SHADER_ASSETS> m_shaderAssetHashMap;
		ShaderRef m_shaderAssets[MENGINE_CONFIG_MAX_SHADER_ASSETS];

		bx::HandleAllocT<MENGINE_CONFIG_MAX_TEXTURE_ASSETS> m_textureHandle;
		bx::HandleHashMapT<MENGINE_CONFIG_MAX_TEXTURE_ASSETS> m_textureAssetHashMap;
		TextureRef m_textureAssets[MENGINE_CONFIG_MAX_TEXTURE_ASSETS];

		bx::HandleAllocT<MENGINE_CONFIG_MAX_MATERIAL_ASSETS> m_materialHandle;
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

		FreeHandle<ResourceHandle, MENGINE_CONFIG_MAX_RESOURCES>  m_freeResources;
		FreeHandle<EntityHandle, MENGINE_CONFIG_MAX_ENTITIES>  m_freeEntities;
		FreeHandle<ComponentHandle, MENGINE_CONFIG_MAX_COMPONENTS> m_freeComponents;
		FreeHandle<GeometryHandle, MENGINE_CONFIG_MAX_GEOMETRY_ASSETS>  m_freeGeometryAssets;
		FreeHandle<ShaderHandle, MENGINE_CONFIG_MAX_SHADER_ASSETS> m_freeShaderAssets;
		FreeHandle<TextureHandle, MENGINE_CONFIG_MAX_TEXTURE_ASSETS> m_freeTextureAssets;
		FreeHandle<MaterialHandle, MENGINE_CONFIG_MAX_TEXTURE_ASSETS> m_freeMaterialAssets;
		FreeHandle<MeshHandle, MENGINE_CONFIG_MAX_TEXTURE_ASSETS> m_freeMeshAssets;

		mrender::MouseState m_mouseState;
	};

} // namespace mengine

#endif // MENGINE_P_H_HEADER_GUARD