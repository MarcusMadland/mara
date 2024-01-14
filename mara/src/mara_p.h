/*
 * Copyright 2023 Marcus Madland. All rights reserved.
 * License: https://github.com/MarcusMadland/mara/blob/main/LICENSE
 */

#ifndef MARA_P_H_HEADER_GUARD
#define MARA_P_H_HEADER_GUARD

#include <base/platform.h>

#ifndef BASE_CONFIG_DEBUG
#	error "BASE_CONFIG_DEBUG must be defined in build script!"
#endif // BASE_CONFIG_DEBUG

#define MARA_CONFIG_DEBUG BASE_CONFIG_DEBUG

#if BASE_CONFIG_DEBUG
//#	define BASE_TRACE  _MARA_TRACE
//#	define BASE_WARN   _MARA_WARN
//#	define BASE_ASSERT _MARA_ASSERT
#endif // BASE_CONFIG_DEBUG

#include <mara/mara.h>
#include "config.h"

 // Check handle, cannot be mara::kInvalidHandle and must be valid.
#define MARA_CHECK_HANDLE(_desc, _handleAlloc, _handle) \
	BASE_ASSERT(isValid(_handle)                          \
		&& _handleAlloc.isValid(_handle.idx)            \
		, "Invalid handle. %s handle: %d (max %d)"      \
		, _desc                                         \
		, _handle.idx                                   \
		, _handleAlloc.getMaxHandles()                  \
		)

// Check handle, it's ok to be mara::kInvalidHandle or must be valid.
#define MARA_CHECK_HANDLE_INVALID_OK(_desc, _handleAlloc, _handle) \
	BASE_ASSERT(!isValid(_handle)                                    \
		|| _handleAlloc.isValid(_handle.idx)                       \
		, "Invalid handle. %s handle: %d (max %d)"                 \
		, _desc                                                    \
		, _handle.idx                                              \
		, _handleAlloc.getMaxHandles()                             \
		)

#if MARA_CONFIG_MULTITHREADED
#	define MARA_MUTEX_SCOPE(_mutex) base::MutexScope BASE_CONCATENATE(mutexScope, __LINE__)(_mutex)
#else
#	define MARA_MUTEX_SCOPE(_mutex) BASE_NOOP()
#endif // MARA_CONFIG_MULTITHREADED

#if MARA_CONFIG_PROFILER
#	define MARA_PROFILER_SCOPE(_name, _abgr)            ProfilerScope BASE_CONCATENATE(profilerScope, __LINE__)(_name, _abgr, __FILE__, uint16_t(__LINE__) )
#	define MARA_PROFILER_BEGIN(_name, _abgr)            g_callback->profilerBegin(_name, _abgr, __FILE__, uint16_t(__LINE__) )
#	define MARA_PROFILER_BEGIN_LITERAL(_name, _abgr)    g_callback->profilerBeginLiteral(_name, _abgr, __FILE__, uint16_t(__LINE__) )
#	define MARA_PROFILER_END()                          g_callback->profilerEnd()
#	define MARA_PROFILER_SET_CURRENT_THREAD_NAME(_name) BASE_NOOP()
#else	   
#	define MARA_PROFILER_SCOPE(_name, _abgr)            BASE_NOOP()
#	define MARA_PROFILER_BEGIN(_name, _abgr)            BASE_NOOP()
#	define MARA_PROFILER_BEGIN_LITERAL(_name, _abgr)    BASE_NOOP()
#	define MARA_PROFILER_END()                          BASE_NOOP()
#	define MARA_PROFILER_SET_CURRENT_THREAD_NAME(_name) BASE_NOOP()
#endif // GRAPHICS_PROFILER_SCOPE

namespace mara
{
#if BASE_COMPILER_CLANG_ANALYZER
	void __attribute__((analyzer_noreturn)) fatal(const char* _filePath, uint16_t _line, Fatal::Enum _code, const char* _format, ...);
#else
	void fatal(const char* _filePath, uint16_t _line, Fatal::Enum _code, const char* _format, ...);
#endif // BASE_COMPILER_CLANG_ANALYZER

	void trace(const char* _filePath, uint16_t _line, const char* _format, ...);
}

#define _MARA_TRACE(_format, ...)                                                       \
	BASE_MACRO_BLOCK_BEGIN                                                                \
		mara::trace(__FILE__, U16(__LINE__), "MARA " _format "\n", ##__VA_ARGS__); \
	BASE_MACRO_BLOCK_END

#define _MARA_WARN(_condition, _format, ...)          \
	BASE_MACRO_BLOCK_BEGIN                              \
		if (!BASE_IGNORE_C4127(_condition) )            \
		{                                             \
			BASE_TRACE("WARN " _format, ##__VA_ARGS__); \
		}                                             \
	BASE_MACRO_BLOCK_END

#define _MARA_ASSERT(_condition, _format, ...)                                                          \
	BASE_MACRO_BLOCK_BEGIN                                                                                \
		if (!BASE_IGNORE_C4127(_condition) )                                                              \
		{                                                                                               \
			BASE_TRACE("ASSERT " _format, ##__VA_ARGS__);                                                 \
			mara::fatal(__FILE__, U16(__LINE__), mara::Fatal::DebugCheck, _format, ##__VA_ARGS__); \
		}                                                                                               \
	BASE_MACRO_BLOCK_END

#define MARA_FATAL(_condition, _err, _format, ...)                             \
	BASE_MACRO_BLOCK_BEGIN                                                       \
		if (!BASE_IGNORE_C4127(_condition) )                                     \
		{                                                                      \
			fatal(__FILE__, U16(__LINE__), _err, _format, ##__VA_ARGS__); \
		}                                                                      \
	BASE_MACRO_BLOCK_END

#define MARA_ERROR_CHECK(_condition, _err, _result, _msg, _format, ...) \
	if (!BASE_IGNORE_C4127(_condition) )                                  \
	{                                                                   \
		BASE_ERROR_SET(_err, _result, _msg);                              \
		BASE_TRACE("%S: 0x%08x '%S' - " _format                           \
			, &baseErrorScope.getName()                                   \
			, _err->get().code                                          \
			, &_err->getMessage()                                       \
			, ##__VA_ARGS__                                             \
			);                                                          \
		return;                                                         \
	}

#include <base/allocator.h>
#include <base/base.h>
#include <base/timer.h>
#include <base/math.h>
#include <base/bounds.h>
#include <base/pixelformat.h>
#include <base/string.h>
#include <base/allocator.h>
#include <base/readerwriter.h>
#include <base/file.h>
#include <base/handlealloc.h>
#include <base/hash.h>
#include <base/commandline.h>
#include <base/commandline.h>
#include <base/endian.h>
#include <base/math.h>
#include <base/string.h>

#include <graphics/platform.h>

namespace mara 
{
	extern CallbackI* g_callback;

	struct ProfilerScope
	{
		ProfilerScope(const char* _name, uint32_t _abgr, const char* _filePath, uint16_t _line)
		{
			g_callback->profilerBeginLiteral(_name, _abgr, _filePath, _line);
		}

		~ProfilerScope()
		{
			g_callback->profilerEnd();
		}
	};

	struct GeometryResource : ResourceI
	{
		U32 getSize() override
		{
			return sizeof(U32) + vertexData->size + sizeof(U32) + indexData->size + sizeof(U32) + (I32)sizeof(layout);
		};

		void write(base::WriterI* _writer, base::Error* _err) override
		{
			U32 layoutSize = sizeof(layout);

			base::write(_writer, &vertexData->size, sizeof(U32), _err);
			base::write(_writer, vertexData->data, (I32)vertexData->size, _err);
			base::write(_writer, &indexData->size, sizeof(U32), _err);
			base::write(_writer, indexData->data, (I32)indexData->size, _err);
			base::write(_writer, &layoutSize, sizeof(U32), _err);
			base::write(_writer, &layout, layoutSize, _err);
		};

		void read(base::ReaderSeekerI* _reader, base::Error* _err) override
		{
			U32 vertexDataSize;
			base::read(_reader, &vertexDataSize, sizeof(vertexDataSize), _err);
			vertexData = graphics::alloc(vertexDataSize);
			base::read(_reader, vertexData->data, vertexDataSize, _err);

			U32 indexDataSize;
			base::read(_reader, &indexDataSize, sizeof(indexDataSize), _err);
			indexData = graphics::alloc(indexDataSize);
			base::read(_reader, indexData->data, indexDataSize, _err);

			U32 layoutSize;
			base::read(_reader, &layoutSize, sizeof(layoutSize), _err);
			base::read(_reader, &layout, layoutSize, _err);
		};

		const graphics::Memory* vertexData;
		const graphics::Memory* indexData;
		graphics::VertexLayout layout;
	};

	struct ShaderResource : ResourceI
	{
		U32 getSize() override
		{
			return sizeof(codeData->size) + codeData->size;
		};

		void write(base::WriterI* _writer, base::Error* _err) override
		{
			base::write(_writer, &codeData->size, sizeof(U32), _err);
			base::write(_writer, codeData->data, codeData->size, _err);
		};

		void read(base::ReaderSeekerI* _reader, base::Error* _err) override
		{
			U32 size;
			base::read(_reader, &size, sizeof(size), _err);
			codeData = graphics::alloc(size);//graphics::makeRef(base::alloc(graphics::getAllocator(), size), size);
			base::read(_reader, codeData->data, size, _err);
		};

		const graphics::Memory* codeData;
	};

	struct TextureResource : ResourceI
	{
		U32 getSize() override
		{
			return sizeof(width) + sizeof(height) + sizeof(hasMips) + sizeof(format) + sizeof(flags)
				+ sizeof(mem->size) + mem->size;
		};

		void write(base::WriterI* _writer, base::Error* _err) override
		{
			base::write(_writer, &width, sizeof(width), _err);
			base::write(_writer, &height, sizeof(height), _err);
			base::write(_writer, &hasMips, sizeof(hasMips), _err);
			base::write(_writer, &format, sizeof(format), _err);
			base::write(_writer, &flags, sizeof(flags), _err);

			base::write(_writer, &mem->size, sizeof(mem->size), _err);
			base::write(_writer, mem->data, mem->size, _err);
		};

		void read(base::ReaderSeekerI* _reader, base::Error* _err) override
		{
			base::read(_reader, &width, sizeof(width), _err);
			base::read(_reader, &height, sizeof(height), _err);
			base::read(_reader, &hasMips, sizeof(hasMips), _err);
			base::read(_reader, &format, sizeof(format), _err);
			base::read(_reader, &flags, sizeof(flags), _err);

			U32 size;
			base::read(_reader, &size, sizeof(size), _err);
			mem = graphics::alloc(size);//graphics::makeRef(base::alloc(graphics::getAllocator(), size), size);
			base::read(_reader, mem->data, size, _err);
		};

		U16 width;
		U16 height;
		bool hasMips;
		graphics::TextureFormat::Enum format;
		U64 flags;
		const graphics::Memory* mem;
	};

	struct MaterialResource : ResourceI
	{
		U32 getSize() override
		{
			U32 size = 0;
			
			size += (U32)sizeof(U32) + (U32)base::strLen(vertPath) + 1;
			size += (U32)sizeof(U32) + (U32)base::strLen(fragPath) + 1;

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

		void write(base::WriterI* _writer, base::Error* _err) override
		{
			U32 vertPathSize = static_cast<U32>(base::strLen(vertPath)) + 1;
			U32 fragPathSize = static_cast<U32>(base::strLen(fragPath)) + 1;

			base::write(_writer, &vertPathSize, sizeof(vertPathSize), _err);
			base::write(_writer, vertPath, vertPathSize, _err);

			base::write(_writer, &fragPathSize, sizeof(fragPathSize), _err);
			base::write(_writer, fragPath, fragPathSize, _err);

			////
			U32 numParameter = (U32)parameters.parameterHashMap.getNumElements();
			base::write(_writer, &numParameter, sizeof(numParameter), _err);

			for (U32 i = 0; i < numParameter; i++)
			{
				U32 hash = parameters.parameterHashMap.findByHandle(i);
				base::write(_writer, &hash, sizeof(hash), _err);

				const MaterialParameters::UniformData& uniformData = parameters.parameters[i];
				base::write(_writer, &uniformData.type, sizeof(uniformData.type), _err);
				base::write(_writer, &uniformData.num, sizeof(uniformData.num), _err);
				base::write(_writer, &uniformData.data->size, sizeof(uniformData.data->size), _err);
				base::write(_writer, uniformData.data->data, uniformData.data->size, _err);
			}
		}

		void read(base::ReaderSeekerI* _reader, base::Error* _err) override
		{
			U32 vertPathSize; 
			base::read(_reader, &vertPathSize, sizeof(vertPathSize), _err);
			base::read(_reader, vertPath, vertPathSize, _err);
			vertPath[base::kMaxFilePath] = '\0';

			U32 fragPathSize;
			base::read(_reader, &fragPathSize, sizeof(fragPathSize), _err);
			base::read(_reader, fragPath, fragPathSize, _err);
			fragPath[base::kMaxFilePath] = '\0';

			////
			U32 numParameter;
			base::read(_reader, &numParameter, sizeof(numParameter), _err);

			// Iterate through the entries and read each one
			for (U32 i = 0; i < numParameter; ++i)
			{
				// Read the hash
				U32 hash;
				base::read(_reader, &hash, sizeof(hash), _err);

				// Read the UniformData structure
				MaterialParameters::UniformData uniformData;
				base::read(_reader, &uniformData.type, sizeof(uniformData.type), _err);
				base::read(_reader, &uniformData.num, sizeof(uniformData.num), _err);
				U32 size;
				base::read(_reader, &size, sizeof(size), _err);
				uniformData.data = graphics::alloc(size);//graphics::makeRef(base::alloc(graphics::getAllocator(), size), size);
				base::read(_reader, uniformData.data->data, size, _err);

				// Insert the entry into the map
				parameters.parameterHashMap.insert(hash, i);
				parameters.parameters[i] = uniformData;
			}
		}

		char vertPath[base::kMaxFilePath + 1];
		char fragPath[base::kMaxFilePath + 1];
		MaterialParameters parameters;
	};

	struct MeshResource : ResourceI
	{
		U32 getSize() override
		{
			U32 size = 0;
			size += (U32)sizeof(U32) + (U32)base::strLen(materialPath) + 1;
			size += (U32)sizeof(U32) + (U32)base::strLen(geometryPath) + 1;
			size += (U32)sizeof(F32) * 16;

			return size;
		}

		void write(base::WriterI* _writer, base::Error* _err) override
		{
			U32 materialPathSize = static_cast<U32>(base::strLen(materialPath)) + 1;
			base::write(_writer, &materialPathSize, sizeof(materialPathSize), _err);
			base::write(_writer, materialPath, materialPathSize, _err);
			
			U32 geometryPathSize = static_cast<U32>(base::strLen(geometryPath)) + 1;
			base::write(_writer, &geometryPathSize, sizeof(geometryPathSize), _err);
			base::write(_writer, geometryPath, geometryPathSize, _err);

			for (U32 i = 0; i < 16; i++)
			{
				base::write(_writer, &m_transform[i], sizeof(F32), _err);
			}
		}

		void read(base::ReaderSeekerI* _reader, base::Error* _err) override
		{
			U32 materialPathSize;
			base::read(_reader, &materialPathSize, sizeof(materialPathSize), _err);
			base::read(_reader, materialPath, materialPathSize, _err);
			materialPath[base::kMaxFilePath] = '\0';

			U32 geometryPathSize;
			base::read(_reader, &geometryPathSize, sizeof(geometryPathSize), _err);
			base::read(_reader, geometryPath, geometryPathSize, _err);
			geometryPath[base::kMaxFilePath] = '\0';

			for (U32 i = 0; i < 16; i++)
			{
				base::read(_reader, &m_transform[i], sizeof(F32), _err);
			}
		}

		char materialPath[base::kMaxFilePath + 1];
		char geometryPath[base::kMaxFilePath + 1];
		F32 m_transform[16];
	};

	struct PrefabResource : ResourceI
	{
		U32 getSize() override
		{
			U32 size = 0;

			size += sizeof(m_numMeshes);

			for (U16 i = 0; i < m_numMeshes; i++)
			{
				size += (U32)sizeof(U32) + (U32)base::strLen(meshPaths[i]) + 1;
			}

			return size;
		}

		void write(base::WriterI* _writer, base::Error* _err) override
		{
			base::write(_writer, &m_numMeshes, sizeof(m_numMeshes), _err);

			for (U16 i = 0; i < m_numMeshes; i++)
			{
				U32 materialPathSize = static_cast<U32>(base::strLen(meshPaths[i])) + 1;
				base::write(_writer, &materialPathSize, sizeof(materialPathSize), _err);
				base::write(_writer, meshPaths[i], materialPathSize, _err);
			}
		}

		void read(base::ReaderSeekerI* _reader, base::Error* _err) override
		{
			base::read(_reader, &m_numMeshes, sizeof(m_numMeshes), _err);

			for (U16 i = 0; i < m_numMeshes; i++)
			{
				U32 materialPathSize;
				base::read(_reader, &materialPathSize, sizeof(materialPathSize), _err);
				base::read(_reader, meshPaths[i], materialPathSize, _err);
				meshPaths[i][base::kMaxFilePath] = '\0';
			}
		}

		U16 m_numMeshes;
		char meshPaths[MARA_CONFIG_MAX_MESHES_PER_PREFAB][base::kMaxFilePath + 1];
	};

	struct PakEntryRef
	{
		U32 pakHash;
		I64 offset;
	};

	struct ResourceRef
	{
		ResourceI* resource;
		base::FilePath vfp;
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
		graphics::VertexBufferHandle m_vbh;
		graphics::IndexBufferHandle m_ibh;

		U32 m_hash;
		U16 m_refCount;
	};

	struct ShaderRef
	{
		graphics::ShaderHandle m_sh;

		U32 m_hash;
		U16 m_refCount;
	};

	struct TextureRef
	{
		graphics::TextureHandle m_th;

		U32 m_hash;
		U16 m_refCount;
	};

	struct MaterialRef
	{
		graphics::ProgramHandle m_ph;
		ShaderHandle m_vsh;
		ShaderHandle m_fsh;
		U16 m_numTextures;
		TextureHandle m_textures[MARA_CONFIG_MAX_UNIFORMS_PER_SHADER];
		
		U32 m_hash;
		U16 m_refCount;
	};

	struct MeshRef
	{
		MaterialHandle m_material;
		F32 m_transform[16];
		GeometryHandle m_geometry;

		U32 m_hash;
		U16 m_refCount;
	};

	struct PrefabRef
	{
		U16 m_numMeshes;
		MeshHandle m_meshes[MARA_CONFIG_MAX_MESHES_PER_PREFAB];

		U32 m_hash;
		U16 m_refCount;
	};

#if MARA_CONFIG_DEBUG
#	define MARA_API_FUNC(_func) BASE_NO_INLINE _func
#else
#	define MARA_API_FUNC(_func) _func
#endif // GRAPHICS_CONFIG_DEBUG

	struct Context
	{
		static constexpr U32 kAlignment = 64;

		Context()
			: m_stats(mara::Stats())
			, m_time(0)
			, m_deltaTime(0.0f)
		{
		}

		~Context()
		{
		}

		bool init(const Init& _init);
		void shutdown();
		bool update(U32 _debug, U32 _reset);

		MARA_API_FUNC(bool createPak(const base::FilePath& _filePath))
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
			base::removeAll(_filePath);
			base::makeAll(_filePath.getPath());

			// Get File Writer
			base::FileWriter writer;

			// Open file.
			if (!base::open(&writer, _filePath, base::ErrorAssert{}))
			{
				BASE_TRACE("Failed to write pack at path %s.", _filePath.getCPtr());
				return false;
			}

			// Write Entries
			U32 numEntries = m_resourceHashMap.getNumElements();
			base::write(&writer, &numEntries, sizeof(U32), base::ErrorAssert{});
			//			  numEntries             hashes                      entries
			U32 offset = sizeof(U32) + (numEntries * sizeof(U32)) + (numEntries * sizeof(PakEntryRef));
			for (U32 i = 0; i < numEntries; i++)
			{
				// Write entry hash
				ResourceRef& resource = m_resources[i];
				U32 entryHash = base::hash<base::HashMurmur2A>(resource.vfp.getCPtr());
				base::write(&writer, &entryHash, sizeof(U32), base::ErrorAssert{});

				// Create entry 
				PakEntryRef pak;
				U32 pakHash = base::hash<base::HashMurmur2A>(_filePath.getCPtr());
				pak.pakHash = pakHash;
				pak.offset = offset;
				offset += resource.resource->getSize();

				// Write entry 
				base::write(&writer, &pak, sizeof(PakEntryRef), base::ErrorAssert{});
			}

			// Write Data
			for (U32 i = 0; i < numEntries; i++)
			{
				// Write resource data
				ResourceRef& resource = m_resources[i];
				resource.resource->write(&writer, base::ErrorAssert{});
			}

			return true;
		}

		MARA_API_FUNC(bool loadPak(const base::FilePath& _filePath))
		{
			// Get File Reader.
			U32 hash = base::hash<base::HashMurmur2A>(_filePath.getCPtr());
			U16 fileReaderHandle = m_pakHashMap.find(hash);
			if (fileReaderHandle != kInvalidHandle)
			{
				BASE_TRACE("Already loaded this pack.")
				return false;
			}
			fileReaderHandle = m_pakHandle.alloc();
			m_pakHashMap.insert(hash, fileReaderHandle);
			
			base::FileReader* pr = &m_paks[fileReaderHandle];

			// Open file (This file will stay open until unloadPack() is called).
			if (!base::open(pr, _filePath, base::ErrorAssert{}))
			{
				BASE_TRACE("Failed to open pack at path %s.", _filePath.getCPtr());
				return false;
			}

			// Read Entries
			U32 numEntries;
			base::read(pr, &numEntries, sizeof(U32), base::ErrorAssert{});
			for (U32 i = 0; i < numEntries; i++)
			{
				// Read entry hash
				U32 hash;
				base::read(pr, &hash, sizeof(U32), base::ErrorAssert{});

				// Read and create entry handle
				U16 entryHandle = m_pakEntryHandle.alloc();
				bool ok = m_pakEntryHashMap.insert(hash, entryHandle);
				BASE_ASSERT(ok, " pack already loaded.")
				base::read(pr, &m_pakEntries[entryHandle], sizeof(PakEntryRef), base::ErrorAssert{});
			}
			
			return true;
		}

		MARA_API_FUNC(bool unloadPak(const base::FilePath& _filePath))
		{
			// Get File Reader.
			U32 hash = base::hash<base::HashMurmur2A>(_filePath.getCPtr());
			U16 fileReaderHandle = m_pakHashMap.find(hash);
			base::FileReader& pr = m_paks[fileReaderHandle];

			// Make sure we read from beginning since resources could've already been loaded 
			// and we would be in a different position.
			base::seek(&pr, 0, base::Whence::Begin);

			// Read Entries
			U32 numEntries;
			base::read(&pr, &numEntries, sizeof(U32), base::ErrorAssert{});
			for (U32 i = 0; i < numEntries; i++)
			{
				// Read entry hash
				U32 hash;
				base::read(&pr, &hash, sizeof(U32), base::ErrorAssert{});

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
				base::seek(&pr, sizeof(PakEntryRef), base::Whence::Current);
			}

			// Finally close the  pack file since its no longer in use.
			base::close(&pr);

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
				bool ok = m_freeResources.queue(_handle); BASE_UNUSED(ok);
				BASE_ASSERT(ok, "Resource handle %d is already destroyed!", _handle.idx);

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

		MARA_API_FUNC(ResourceHandle createResource(const base::FilePath& _vfp))
		{
			U32 hash = base::hash<base::HashMurmur2A>(_vfp.getCPtr());

			ResourceHandle handle;
			if (resourceFindOrCreate(hash, handle))
			{
				return handle;
			}

			bool ok = m_resourceHashMap.insert(hash, handle.idx);
			BASE_ASSERT(ok, "Resource already exists!"); BASE_UNUSED(ok);

			ResourceRef& rr = m_resources[handle.idx];
			rr.m_refCount = 1;
			rr.vfp = _vfp;
			return handle;
		}

		MARA_API_FUNC(void destroyResource(ResourceHandle _handle))
		{
			MARA_MUTEX_SCOPE(m_resourceApiLock);

			if (!isValid(_handle) && !m_resourceHandle.isValid(_handle.idx))
			{
				BASE_WARN(false, "Passing invalid resource handle to mara::destroyResource.");
				return;
			}

			resourceDecRef(_handle);
		}

		// @todo Make heap allocated array
		MARA_API_FUNC(U32 getResourceInfo(ResourceInfo* _outInfoList, bool _sort))
		{
			U32 count = 0;

			// Give information about each loaded resource
			for (U32 i = 0; i < MARA_CONFIG_MAX_RESOURCES; i++)
			{
				if (m_resources[i].vfp.isEmpty())
				{
					break;
				}

				_outInfoList[i].vfp = m_resources[i].vfp;
				_outInfoList[i].refCount = m_resources[i].m_refCount;
				count++;
			}

			// Simple bouble sort
			if (_sort && count > 1)
			{
				for (U32 i = 0; i < count - 1; i++)
				{
					for (U32 j = 0; j < count - i - 1; j++)
					{
						if (base::strCmp(_outInfoList[j].vfp.getCPtr(), _outInfoList[j + 1].vfp.getCPtr()) > 0)
						{
							ResourceInfo temp = _outInfoList[j];
							_outInfoList[j] = _outInfoList[j + 1];
							_outInfoList[j + 1] = temp;
						}
					}
				}
			}

			return count;
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
				bool ok = m_freeComponents.queue(_handle); BASE_UNUSED(ok);
				BASE_ASSERT(ok, "Component handle %d is already destroyed!", _handle.idx);

				// @todo We are using delete because we are using virtual destructors. Can we do this manually?
				delete sr.m_data; 
				sr.m_data = NULL;
			}
		}

		MARA_API_FUNC(ComponentHandle createComponent(ComponentI* _data))
		{
			ComponentHandle handle = { m_componentHandle.alloc() };

			if (isValid(handle))
			{
				ComponentRef& sr = m_components[handle.idx];
				sr.m_refCount = 1;

				sr.m_data = _data;
				
				return handle;
			}

			BASE_TRACE("Failed to create component handle.");
			return MARA_INVALID_HANDLE;
		}

		MARA_API_FUNC(void destroyComponent(ComponentHandle _handle))
		{
			MARA_MUTEX_SCOPE(m_resourceApiLock);

			if (!isValid(_handle) && !m_componentHandle.isValid(_handle.idx))
			{
				BASE_WARN(false, "Passing invalid component handle to mara::destroyComponent.");
				return;
			}

			componentDecRef(_handle);
		}

		MARA_API_FUNC(void addComponent(EntityHandle _entity, U32 _type, ComponentHandle _component))
		{
			bool ok = m_componentHashMap[_type].insert(_entity.idx, _component.idx);
			BASE_ASSERT(ok, "Entities cannot have duplicated components!", _entity.idx);

			m_entityHashMap[_type].insert(m_entityHashMap[_type].getNumElements(), _entity.idx);

			EntityRef& sr = m_entities[_entity.idx];
			sr.m_mask |= _type;
		}

		bool hasComponent(EntityHandle _handle, U32 _type)
		{
			const U16 idx = m_componentHashMap[_type].find(_handle.idx);
			return idx != kInvalidHandle;
		}

		MARA_API_FUNC(void* getComponentData(EntityHandle _handle, U32 _type))
		{
			const U16 idx = m_componentHashMap[_type].find(_handle.idx);
			if (idx != kInvalidHandle)
			{
				void* data = m_components[idx].m_data;
				if (NULL != data)
				{
					return data;
				}

				BASE_ASSERT(true, "Component data is NULL but entity contains component type")
				return NULL;
			}

			BASE_ASSERT(true, "Component handle is invalid")
			return NULL;
		}

		MARA_API_FUNC(EntityQuery* queryEntities(U32 _types))
		{
			// @todo What if- instead of allocating an entity query here, we have a global entity query we change instead. 
			// Then we can allocate and deallocate that ourself without leaving that respnsibility to the user.
			EntityQuery* query = (EntityQuery*)base::alloc(entry::getAllocator(), sizeof(EntityQuery));
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
				bool ok = m_freeEntities.queue(_handle); BASE_UNUSED(ok);
				BASE_ASSERT(ok, "Entity handle %d is already destroyed!", _handle.idx);

				sr.m_mask = 0;
			}
		}

		MARA_API_FUNC(EntityHandle createEntity())
		{
			EntityHandle handle = { m_entityHandle.alloc() };

			if (isValid(handle))
			{
				EntityRef& sr = m_entities[handle.idx];
				sr.m_refCount = 1;

				return handle;
			}

			BASE_TRACE("Failed to create entity handle.");
			return MARA_INVALID_HANDLE;
		}

		MARA_API_FUNC(void destroyEntity(EntityHandle _handle))
		{
			MARA_MUTEX_SCOPE(m_resourceApiLock);

			if (!isValid(_handle))
			{
				BASE_WARN(false, "Passing invalid entity handle to mara::destroyEntity.");
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
				bool ok = m_freeGeometries.queue(_handle); BASE_UNUSED(ok);
				BASE_ASSERT(ok, "Geometry  handle %d is already destroyed!", _handle.idx);

				graphics::destroy(gr.m_vbh);
				graphics::destroy(gr.m_ibh);

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

		MARA_API_FUNC(GeometryHandle createGeometry(ResourceHandle _resource))
		{
			ResourceRef& resource = m_resources[_resource.idx];
			U32 hash = base::hash<base::HashMurmur2A>(resource.vfp.getCPtr());

			GeometryHandle handle;
			if (geometryFindOrCreate(hash, handle))
			{
				return handle;
			}

			bool ok = m_geometryHashMap.insert(hash, handle.idx);
			BASE_ASSERT(ok, "Geometry  already exists!"); BASE_UNUSED(ok);

			GeometryResource* geomResource = (GeometryResource*)resource.resource;
			if (NULL == geomResource)
			{
				BASE_TRACE("Resource handle is not a geometry resource.")
				return MARA_INVALID_HANDLE;
			}

			GeometryRef& gr = m_geometries[handle.idx];
			gr.m_refCount = 1;
			gr.m_hash = hash;

			gr.m_vbh = graphics::createVertexBuffer(geomResource->vertexData, geomResource->layout);
			gr.m_ibh = graphics::createIndexBuffer(geomResource->indexData);

			return handle;
		}

		MARA_API_FUNC(ResourceHandle createGeometryResource(const GeometryCreate& _data, const base::FilePath& _vfp))
		{
			ResourceHandle handle = createResource(_vfp);

		    ResourceRef& rr = m_resources[handle.idx];
			if (rr.m_refCount > 1)
			{
				return handle;
			}

			rr.resource = new GeometryResource(); 

			((GeometryResource*)rr.resource)->vertexData = graphics::copy(_data.vertices, _data.verticesSize);
			((GeometryResource*)rr.resource)->indexData = graphics::copy(_data.indices, _data.indicesSize);
			((GeometryResource*)rr.resource)->layout = _data.layout;

			return handle;
		}

		MARA_API_FUNC(ResourceHandle loadGeometryResource(const base::FilePath& _filePath))
		{
			U32 hash = base::hash<base::HashMurmur2A>(_filePath.getCPtr());

			ResourceHandle handle;
			if (resourceFindOrCreate(hash, handle))
			{
				return handle;
			}

			// Check if resource is inside a loaded  pack
			U16 entryHandle = m_pakEntryHashMap.find(base::hash<base::HashMurmur2A>(_filePath.getCPtr()));
			if (kInvalidHandle != entryHandle)
			{
				// Create resource
				bool ok = m_resourceHashMap.insert(hash, handle.idx);
				BASE_ASSERT(ok, "Resource already exists!"); BASE_UNUSED(ok);

				// Get pak file reader
				PakEntryRef& per = m_pakEntries[entryHandle];
				base::FileReader* reader = &m_paks[m_pakHashMap.find(per.pakHash)];

				// Seek to the offset of the entry using the entry file pointer.
				base::seek(reader, per.offset, base::Whence::Begin);

				// Read resource data at offset position.
				ResourceRef& rr = m_resources[handle.idx];
				rr.m_refCount = 1;
				rr.vfp = _filePath;
				rr.resource = new GeometryResource();
				rr.resource->read(reader, base::ErrorAssert{});

				// Return now loaded resource.
				return handle;
			}

			BASE_TRACE("@todo Add support for single file loading here...")
			return handle;
		}

		MARA_API_FUNC(void destroyGeometry(GeometryHandle _handle))
		{
			MARA_MUTEX_SCOPE(m_resourceApiLock);

			if (!isValid(_handle) && !m_geometryHandle.isValid(_handle.idx))
			{
				BASE_WARN(false, "Passing invalid geometry handle to mara::destroyGeometry.");
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
				bool ok = m_freeShaders.queue(_handle); BASE_UNUSED(ok);
				BASE_ASSERT(ok, "Shader  handle %d is already destroyed!", _handle.idx);

				graphics::destroy(sr.m_sh);

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

		MARA_API_FUNC(ShaderHandle createShader(ResourceHandle _resource))
		{
			ResourceRef& resource = m_resources[_resource.idx];
			U32 hash = base::hash<base::HashMurmur2A>(resource.vfp.getCPtr());

			ShaderHandle handle;
			if (shaderFindOrCreate(hash, handle))
			{
				return handle;
			}

			bool ok = m_shaderHashMap.insert(hash, handle.idx);
			BASE_ASSERT(ok, "Shader  already exists!"); BASE_UNUSED(ok);

			ShaderResource* shadResource = (ShaderResource*)resource.resource;
			if (NULL == shadResource)
			{
				BASE_TRACE("Resource handle is not a shader resource.")
				return MARA_INVALID_HANDLE;
			}

			ShaderRef& sr = m_shaders[handle.idx];
			sr.m_refCount = 1;
			sr.m_hash = hash;

			sr.m_sh = graphics::createShader(shadResource->codeData);

			return handle;
		}

		MARA_API_FUNC(ResourceHandle createShaderResource(const ShaderCreate& _data, const base::FilePath& _vfp))
		{
			ResourceHandle handle = createResource(_vfp);

			ResourceRef& rr = m_resources[handle.idx];
			if (rr.m_refCount > 1)
			{
				return handle;
			}

			rr.resource = new ShaderResource();

			((ShaderResource*)rr.resource)->codeData = graphics::copy(_data.mem->data, _data.mem->size);

			return handle;
		}

		MARA_API_FUNC(ResourceHandle loadShaderResource(const base::FilePath& _filePath))
		{
			U32 hash = base::hash<base::HashMurmur2A>(_filePath.getCPtr());

			ResourceHandle handle;
			if (resourceFindOrCreate(hash, handle))
			{
				return handle;
			}

			// Check if resource is inside a loaded  pack
			U16 entryHandle = m_pakEntryHashMap.find(base::hash<base::HashMurmur2A>(_filePath.getCPtr()));
			if (kInvalidHandle != entryHandle)
			{
				// Create resource
				bool ok = m_resourceHashMap.insert(hash, handle.idx);
				BASE_ASSERT(ok, "Resource already exists!"); BASE_UNUSED(ok);

				// Get pak file reader
				PakEntryRef& per = m_pakEntries[entryHandle];
				base::FileReader* reader = &m_paks[m_pakHashMap.find(per.pakHash)];

				// Seek to the offset of the entry using the entry file pointer.
				base::seek(reader, per.offset, base::Whence::Begin);

				// Read resource data at offset position.
				ResourceRef& rr = m_resources[handle.idx];
				rr.m_refCount = 1;
				rr.vfp = _filePath;
				rr.resource = new ShaderResource();
				rr.resource->read(reader, base::ErrorAssert{});

				// Return now loaded resource.
				return handle;
			}

			BASE_TRACE("@todo Add support for single file loading here...")
			return handle;
		}

		MARA_API_FUNC(void destroyShader(ShaderHandle _handle))
		{
			MARA_MUTEX_SCOPE(m_resourceApiLock);

			if (!isValid(_handle) && !m_shaderHandle.isValid(_handle.idx))
			{
				BASE_WARN(false, "Passing invalid shader  handle to mara::destroyShader.");
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
				bool ok = m_freeTextures.queue(_handle); BASE_UNUSED(ok);
				BASE_ASSERT(ok, "Texture  handle %d is already destroyed!", _handle.idx);

				graphics::destroy(sr.m_th);

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

		MARA_API_FUNC(TextureHandle createTexture(ResourceHandle _resource))
		{
			ResourceRef& resource = m_resources[_resource.idx];
			U32 hash = base::hash<base::HashMurmur2A>(resource.vfp.getCPtr());

			TextureHandle handle;
			if (textureFindOrCreate(hash, handle))
			{
				return handle;
			}
			
			bool ok = m_textureHashMap.insert(hash, handle.idx);
			BASE_ASSERT(ok, "Texture  already exists!"); BASE_UNUSED(ok);

			TextureResource* texResource = (TextureResource*)resource.resource;
			if (NULL == texResource)
			{
				BASE_TRACE("Resource handle is not a texture resource.")
					return MARA_INVALID_HANDLE;
			}

			TextureRef& sr = m_textures[handle.idx];
			sr.m_refCount = 1;
			sr.m_hash = hash;

			sr.m_th = graphics::createTexture2D(
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

		MARA_API_FUNC(ResourceHandle createTextureResource(const TextureCreate& _data, const base::FilePath& _vfp))
		{
			ResourceHandle handle = createResource(_vfp);

			ResourceRef& rr = m_resources[handle.idx];
			if (rr.m_refCount > 1)
			{
				return handle;
			}

			rr.resource = new TextureResource();

			((TextureResource*)rr.resource)->width = _data.width;
			((TextureResource*)rr.resource)->height = _data.height;
			((TextureResource*)rr.resource)->hasMips = _data.hasMips;
			((TextureResource*)rr.resource)->format = _data.format;
			((TextureResource*)rr.resource)->flags = _data.flags;
			((TextureResource*)rr.resource)->mem = graphics::copy(_data.mem, _data.memSize);

			return handle;
		}

		MARA_API_FUNC(ResourceHandle loadTextureResource(const base::FilePath& _filePath))
		{
			U32 hash = base::hash<base::HashMurmur2A>(_filePath.getCPtr());

			ResourceHandle handle;
			if (resourceFindOrCreate(hash, handle))
			{
				return handle;
			}

			// Check if resource is inside a loaded  pack
			U16 entryHandle = m_pakEntryHashMap.find(base::hash<base::HashMurmur2A>(_filePath.getCPtr()));
			if (kInvalidHandle != entryHandle)
			{
				// Create resource
				bool ok = m_resourceHashMap.insert(hash, handle.idx);
				BASE_ASSERT(ok, "Resource already exists!"); BASE_UNUSED(ok);

				// Get pak file reader
				PakEntryRef& per = m_pakEntries[entryHandle];
				base::FileReader* reader = &m_paks[m_pakHashMap.find(per.pakHash)];
				
				// Seek to the offset of the entry using the entry file pointer.
				base::seek(reader, per.offset, base::Whence::Begin);

				// Read resource data at offset position.
				ResourceRef& rr = m_resources[handle.idx];
				rr.m_refCount = 1;
				rr.vfp = _filePath;
				rr.resource = new TextureResource();
				rr.resource->read(reader, base::ErrorAssert{});

				// Return now loaded resource.
				return handle;
			}
			
			BASE_TRACE("@todo Add support for single file loading here...")
			return handle;
		}

		MARA_API_FUNC(void destroyTexture(TextureHandle _handle))
		{
			MARA_MUTEX_SCOPE(m_resourceApiLock);

			if (!isValid(_handle) && !m_textureHandle.isValid(_handle.idx))
			{
				BASE_WARN(false, "Passing invalid texture  handle to mara::destroyTexture.");
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
				bool ok = m_freeMaterials.queue(_handle); BASE_UNUSED(ok);
				BASE_ASSERT(ok, "Material  handle %d is already destroyed!", _handle.idx);

				U16 resourceHandle = m_resourceHashMap.find(mr.m_hash);

				MaterialResource* matResource = (MaterialResource*)m_resources[resourceHandle].resource;

				graphics::destroy(mr.m_ph);
				destroyShader(mr.m_vsh);
				destroyShader(mr.m_fsh);
				for (U32 i = 0; i < matResource->parameters.parameterHashMap.getNumElements(); i++)
				{
					MaterialParameters::UniformData& data = matResource->parameters.parameters[i];
					if (data.type == graphics::UniformType::Sampler)
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

		MARA_API_FUNC(MaterialHandle createMaterial(ResourceHandle _resource))
		{
			ResourceRef& resource = m_resources[_resource.idx];
			U32 hash = base::hash<base::HashMurmur2A>(resource.vfp.getCPtr());

			MaterialHandle handle;
			if (materialFindOrCreate(hash, handle))
			{
				return handle;
			}

			bool ok = m_materialHashMap.insert(hash, handle.idx);
			BASE_ASSERT(ok, "Material  already exists!"); BASE_UNUSED(ok);

			MaterialResource* matResource = (MaterialResource*)resource.resource;
			if (NULL == matResource)
			{
				BASE_TRACE("Resource handle is not a material resource.")
				return MARA_INVALID_HANDLE;
			}

			MaterialRef& sr = m_materials[handle.idx];
			sr.m_refCount = 1;
			sr.m_hash = hash;
			
			sr.m_vsh = createShader(loadShader(matResource->vertPath));
			sr.m_fsh = createShader(loadShader(matResource->fragPath));
			sr.m_ph = graphics::createProgram(sr.m_vsh, sr.m_fsh);
			for (U32 i = 0; i < matResource->parameters.parameterHashMap.getNumElements(); i++)
			{
				MaterialParameters::UniformData& data = matResource->parameters.parameters[i];
				if (data.type == graphics::UniformType::Sampler)
				{
					const char* vfp = (const char*)data.data->data;
					sr.m_textures[i] = createTexture(loadTexture(vfp));
				}
			}

			return handle;
		}

		MARA_API_FUNC(ResourceHandle createMaterialResource(const MaterialCreate& _data, const base::FilePath& _vfp))
		{
			ResourceHandle handle = createResource(_vfp);

			ResourceRef& rr = m_resources[handle.idx];
			if (rr.m_refCount > 1)
			{
				return handle;
			}

			rr.resource = new MaterialResource();

			base::strCopy(((MaterialResource*)rr.resource)->vertPath, base::kMaxFilePath, _data.vertShaderPath.getCPtr());
			base::strCopy(((MaterialResource*)rr.resource)->fragPath, base::kMaxFilePath, _data.fragShaderPath.getCPtr());
			((MaterialResource*)rr.resource)->parameters = _data.parameters;
			return handle;
		}

		MARA_API_FUNC(ResourceHandle loadMaterialResource(const base::FilePath& _filePath))
		{
			U32 hash = base::hash<base::HashMurmur2A>(_filePath.getCPtr());

			ResourceHandle handle;
			if (resourceFindOrCreate(hash, handle))
			{
				return handle;
			}

			// Check if resource is inside a loaded  pack
			U16 entryHandle = m_pakEntryHashMap.find(base::hash<base::HashMurmur2A>(_filePath.getCPtr()));
			if (kInvalidHandle != entryHandle)
			{
				// Create resource
				bool ok = m_resourceHashMap.insert(hash, handle.idx);
				BASE_ASSERT(ok, "Resource already exists!"); BASE_UNUSED(ok);

				// Get pak file reader
				PakEntryRef& per = m_pakEntries[entryHandle];
				base::FileReader* reader = &m_paks[m_pakHashMap.find(per.pakHash)];

				// Seek to the offset of the entry using the entry file pointer.
				base::seek(reader, per.offset, base::Whence::Begin);

				// Read resource data at offset position.
				ResourceRef& rr = m_resources[handle.idx];
				rr.m_refCount = 1;
				rr.vfp = _filePath;
				rr.resource = new MaterialResource();
				rr.resource->read(reader, base::ErrorAssert{});

				// Return now loaded resource.
				return handle;
			}

			BASE_TRACE("@todo Add support for single file loading here...")
			return handle;
		}

		MARA_API_FUNC(void destroyMaterial(MaterialHandle _handle))
		{
			MARA_MUTEX_SCOPE(m_resourceApiLock);

			if (!isValid(_handle) && !m_materialHandle.isValid(_handle.idx))
			{
				BASE_WARN(false, "Passing invalid material  handle to mara::destroyMaterial.");
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
				bool ok = m_freeMeshes.queue(_handle); BASE_UNUSED(ok);
				BASE_ASSERT(ok, "Mesh  handle %d is already destroyed!", _handle.idx);

				destroyMaterial(mr.m_material);
				destroyGeometry(mr.m_geometry);

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

		MARA_API_FUNC(MeshHandle createMesh(ResourceHandle _resource))
		{
			ResourceRef& resource = m_resources[_resource.idx];
			U32 hash = base::hash<base::HashMurmur2A>(resource.vfp.getCPtr());

			MeshHandle handle;
			if (meshFindOrCreate(hash, handle))
			{
				return handle;
			}

			bool ok = m_meshHashMap.insert(hash, handle.idx);
			BASE_ASSERT(ok, "Mesh already exists!"); BASE_UNUSED(ok);

			MeshResource* meshResource = (MeshResource*)resource.resource;
			if (NULL == meshResource)
			{
				BASE_TRACE("Resource handle is not a mesh resource.")
				return MARA_INVALID_HANDLE;
			}

			MeshRef& sr = m_meshes[handle.idx];
			sr.m_refCount = 1;
			sr.m_hash = hash;

			sr.m_material = createMaterial(loadMaterial(meshResource->materialPath));
			sr.m_geometry = createGeometry(loadGeometry(meshResource->geometryPath));

			sr.m_transform[0] = meshResource->m_transform[0];
			sr.m_transform[1] = meshResource->m_transform[1];
			sr.m_transform[2] = meshResource->m_transform[2];
			sr.m_transform[3] = meshResource->m_transform[3];
			sr.m_transform[4] = meshResource->m_transform[4];
			sr.m_transform[5] = meshResource->m_transform[5];
			sr.m_transform[6] = meshResource->m_transform[6];
			sr.m_transform[7] = meshResource->m_transform[7];
			sr.m_transform[8] = meshResource->m_transform[8];
			sr.m_transform[9] = meshResource->m_transform[9];
			sr.m_transform[10] = meshResource->m_transform[10];
			sr.m_transform[11] = meshResource->m_transform[11];
			sr.m_transform[12] = meshResource->m_transform[12];
			sr.m_transform[13] = meshResource->m_transform[13];
			sr.m_transform[14] = meshResource->m_transform[14];
			sr.m_transform[15] = meshResource->m_transform[15];

			return handle;
		}

		MARA_API_FUNC(ResourceHandle createMeshResource(const MeshCreate& _data, const base::FilePath& _vfp))
		{
			ResourceHandle handle = createResource(_vfp);

			ResourceRef& rr = m_resources[handle.idx];
			if (rr.m_refCount > 1)
			{
				return handle;
			}

			rr.resource = new MeshResource();

			base::strCopy(((MeshResource*)rr.resource)->materialPath, base::kMaxFilePath, _data.materialPath.getCPtr());
			base::strCopy(((MeshResource*)rr.resource)->geometryPath, base::kMaxFilePath, _data.geometryPath.getCPtr());

			for (U32 i = 0; i < 16; i++)
			{
				((MeshResource*)rr.resource)->m_transform[i] = _data.m_transform[i];
			}

			return handle;
		}

		MARA_API_FUNC(ResourceHandle loadMeshResource(const base::FilePath& _filePath))
		{
			U32 hash = base::hash<base::HashMurmur2A>(_filePath.getCPtr());

			ResourceHandle handle;
			if (resourceFindOrCreate(hash, handle))
			{
				return handle;
			}

			// Check if resource is inside a loaded  pack
			U16 entryHandle = m_pakEntryHashMap.find(base::hash<base::HashMurmur2A>(_filePath.getCPtr()));
			if (kInvalidHandle != entryHandle)
			{
				// Create resource
				bool ok = m_resourceHashMap.insert(hash, handle.idx);
				BASE_ASSERT(ok, "Resource already exists!"); BASE_UNUSED(ok);

				// Get pak file reader
				PakEntryRef& per = m_pakEntries[entryHandle];
				base::FileReader* reader = &m_paks[m_pakHashMap.find(per.pakHash)];

				// Seek to the offset of the entry using the entry file pointer.
				base::seek(reader, per.offset, base::Whence::Begin);

				// Read resource data at offset position.
				ResourceRef& rr = m_resources[handle.idx];
				rr.m_refCount = 1;
				rr.vfp = _filePath;
				rr.resource = new MeshResource();
				rr.resource->read(reader, base::ErrorAssert{});

				// Return now loaded resource.
				return handle;
			}

			BASE_TRACE("@todo Add support for single file loading here...")
			return handle;
		}

		MARA_API_FUNC(void destroyMesh(MeshHandle _handle))
		{
			MARA_MUTEX_SCOPE(m_resourceApiLock);

			if (!isValid(_handle) && !m_meshHandle.isValid(_handle.idx))
			{
				BASE_WARN(false, "Passing invalid mesh  handle to mara::destroyMesh.");
				return;
			}

			meshDecRef(_handle);

			// Resources
			MeshRef& mr = m_meshes[_handle.idx];
			U16 resourceHandle = m_resourceHashMap.find(mr.m_hash);
			destroyResource({ resourceHandle });
		}

		MARA_API_FUNC(void getMeshTransform(F32* _result, MeshHandle _handle))
		{
			MeshRef& mr = m_meshes[_handle.idx];
			_result[0] = mr.m_transform[0];
			_result[1] = mr.m_transform[1];
			_result[2] = mr.m_transform[2];
			_result[3] = mr.m_transform[3];
			_result[4] = mr.m_transform[4];
			_result[5] = mr.m_transform[5];
			_result[6] = mr.m_transform[6];
			_result[7] = mr.m_transform[7];
			_result[8] = mr.m_transform[8];
			_result[9] = mr.m_transform[9];
			_result[10] = mr.m_transform[10];
			_result[11] = mr.m_transform[11];
			_result[12] = mr.m_transform[12];
			_result[13] = mr.m_transform[13];
			_result[14] = mr.m_transform[14];
			_result[15] = mr.m_transform[15];
		}

		void prefabIncRef(PrefabHandle _handle)
		{
			PrefabRef& pr = m_prefabs[_handle.idx];
			++pr.m_refCount;
		}

		void prefabDecRef(PrefabHandle _handle)
		{
			PrefabRef& pr = m_prefabs[_handle.idx];
			U16 refs = --pr.m_refCount;

			if (0 == refs)
			{
				bool ok = m_freePrefabs.queue(_handle); BASE_UNUSED(ok);
				BASE_ASSERT(ok, "Prefab handle %d is already destroyed!", _handle.idx);

				for (U16 i = 0; i < pr.m_numMeshes; i++)
				{
					destroyMesh(pr.m_meshes[i]);
				}

				m_prefabHashMap.removeByHandle(_handle.idx);
			}
		}

		bool prefabFindOrCreate(U32 _hash, PrefabHandle& _handle)
		{
			U16 idx = m_prefabHashMap.find(_hash);
			if (kInvalidHandle != idx)
			{
				PrefabHandle handle = { idx };
				prefabIncRef(handle);
				_handle = handle;
				return true;
			}
			else
			{
				_handle = { m_prefabHandle.alloc() };
				return false;
			}
		}

		MARA_API_FUNC(PrefabHandle createPrefab(ResourceHandle _resource))
		{
			ResourceRef& resource = m_resources[_resource.idx];
			U32 hash = base::hash<base::HashMurmur2A>(resource.vfp.getCPtr());

			PrefabHandle handle;
			if (prefabFindOrCreate(hash, handle))
			{
				return handle;
			}

			bool ok = m_prefabHashMap.insert(hash, handle.idx);
			BASE_ASSERT(ok, "Prefab already exists!"); BASE_UNUSED(ok);

			PrefabResource* prefabResource = (PrefabResource*)resource.resource;
			if (NULL == prefabResource)
			{
				BASE_TRACE("Resource handle is not a mesh resource.")
				return MARA_INVALID_HANDLE;
			}

			PrefabRef& sr = m_prefabs[handle.idx];
			sr.m_refCount = 1;
			sr.m_hash = hash;

			sr.m_numMeshes = prefabResource->m_numMeshes;
			for (U16 i = 0; i < sr.m_numMeshes; i++)
			{
				sr.m_meshes[i] = createMesh(loadMesh(prefabResource->meshPaths[i]));
			}

			return handle;
		}

		MARA_API_FUNC(ResourceHandle createPrefabResource(const PrefabCreate& _data, const base::FilePath& _vfp))
		{
			ResourceHandle handle = createResource(_vfp);

			ResourceRef& rr = m_resources[handle.idx];
			if (rr.m_refCount > 1)
			{
				return handle;
			}

			rr.resource = new PrefabResource();

			for (U16 i = 0; i < _data.m_numMeshes; i++)
			{
				base::strCopy(((PrefabResource*)rr.resource)->meshPaths[i], base::kMaxFilePath, _data.meshPaths[i].getCPtr());
			}
			((PrefabResource*)rr.resource)->m_numMeshes = _data.m_numMeshes;

			return handle;
		}

		MARA_API_FUNC(ResourceHandle loadPrefabResource(const base::FilePath& _filePath))
		{
			U32 hash = base::hash<base::HashMurmur2A>(_filePath.getCPtr());

			ResourceHandle handle;
			if (resourceFindOrCreate(hash, handle))
			{
				return handle;
			}

			// Check if resource is inside a loaded  pack
			U16 entryHandle = m_pakEntryHashMap.find(base::hash<base::HashMurmur2A>(_filePath.getCPtr()));
			if (kInvalidHandle != entryHandle)
			{
				// Create resource
				bool ok = m_resourceHashMap.insert(hash, handle.idx);
				BASE_ASSERT(ok, "Resource already exists!"); BASE_UNUSED(ok);

				// Get pak file reader
				PakEntryRef& per = m_pakEntries[entryHandle];
				base::FileReader* reader = &m_paks[m_pakHashMap.find(per.pakHash)];

				// Seek to the offset of the entry using the entry file pointer.
				base::seek(reader, per.offset, base::Whence::Begin);

				// Read resource data at offset position.
				ResourceRef& rr = m_resources[handle.idx];
				rr.m_refCount = 1;
				rr.vfp = _filePath;
				rr.resource = new PrefabResource();
				rr.resource->read(reader, base::ErrorAssert{});

				// Return now loaded resource.
				return handle;
			}

			BASE_TRACE("@todo Add support for single file loading here...")
			return handle;
		}

		MARA_API_FUNC(void destroyPrefab(PrefabHandle _handle))
		{
			MARA_MUTEX_SCOPE(m_resourceApiLock);

			if (!isValid(_handle) && !m_prefabHandle.isValid(_handle.idx))
			{
				BASE_WARN(false, "Passing invalid prefab handle to mara::destroyPrefab.");
				return;
			}

			prefabDecRef(_handle);

			// Resources
			PrefabRef& mr = m_prefabs[_handle.idx];
			U16 resourceHandle = m_resourceHashMap.find(mr.m_hash);
			destroyResource({ resourceHandle });
		}

		MARA_API_FUNC(U16 getNumMeshes(PrefabHandle _handle))
		{
			PrefabRef& pr = m_prefabs[_handle.idx];
			return pr.m_numMeshes;
		}

		MARA_API_FUNC(MeshHandle* getMeshes(PrefabHandle _handle))
		{
			PrefabRef& pr = m_prefabs[_handle.idx];
			return pr.m_meshes;
		}

		MARA_API_FUNC(const entry::MouseState* getMouseState())
		{
			return &m_mouseState;
		}

		MARA_API_FUNC(const F32 getDeltaTime())
		{
			return m_deltaTime;
		}

		MARA_API_FUNC(const Stats* getStats())
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
			stats.numPrefabs = m_prefabHandle.getNumHandles();

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
			for (U16 i = 0; i < stats.numMeshes; i++)
			{
				//stats.prefabRef[i] = m_prefabs[i].m_refCount; // @todo Why does this fail on sponza?
			}

			return &stats;
		}

		I64 m_time;
		F32 m_deltaTime;
		Stats m_stats;
		
		base::HandleAllocT<MARA_CONFIG_MAX_PAKS> m_pakHandle;
		base::HandleHashMapT<MARA_CONFIG_MAX_PAKS> m_pakHashMap;
		base::FileReader m_paks[MARA_CONFIG_MAX_PAKS];

		base::HandleAllocT<MARA_CONFIG_MAX_PAK_ENTRIES> m_pakEntryHandle;
		base::HandleHashMapT<MARA_CONFIG_MAX_PAK_ENTRIES> m_pakEntryHashMap;
		PakEntryRef m_pakEntries[MARA_CONFIG_MAX_PAK_ENTRIES];

		base::HandleAllocT<MARA_CONFIG_MAX_RESOURCES> m_resourceHandle;
		base::HandleHashMapT<MARA_CONFIG_MAX_RESOURCES> m_resourceHashMap;
		ResourceRef m_resources[MARA_CONFIG_MAX_RESOURCES];

		base::HandleAllocT<MARA_CONFIG_MAX_COMPONENTS> m_componentHandle;
		base::HandleHashMapT<MARA_CONFIG_MAX_COMPONENTS_PER_TYPE> m_componentHashMap[32];
		ComponentRef m_components[MARA_CONFIG_MAX_COMPONENTS];

		base::HandleAllocT<MARA_CONFIG_MAX_ENTITIES> m_entityHandle;
		base::HandleHashMapT<MARA_CONFIG_MAX_COMPONENTS_PER_TYPE> m_entityHashMap[32];
		EntityRef m_entities[MARA_CONFIG_MAX_ENTITIES];

		base::HandleAllocT<MARA_CONFIG_MAX_GEOMETRIES> m_geometryHandle;
		base::HandleHashMapT<MARA_CONFIG_MAX_GEOMETRIES> m_geometryHashMap;
		GeometryRef m_geometries[MARA_CONFIG_MAX_GEOMETRIES];

		base::HandleAllocT<MARA_CONFIG_MAX_SHADERS> m_shaderHandle;
		base::HandleHashMapT<MARA_CONFIG_MAX_SHADERS> m_shaderHashMap;
		ShaderRef m_shaders[MARA_CONFIG_MAX_SHADERS];

		base::HandleAllocT<MARA_CONFIG_MAX_TEXTURES> m_textureHandle;
		base::HandleHashMapT<MARA_CONFIG_MAX_TEXTURES> m_textureHashMap;
		TextureRef m_textures[MARA_CONFIG_MAX_TEXTURES];

		base::HandleAllocT<MARA_CONFIG_MAX_MATERIALS> m_materialHandle;
		base::HandleHashMapT<MARA_CONFIG_MAX_MATERIALS> m_materialHashMap;
		MaterialRef m_materials[MARA_CONFIG_MAX_MATERIALS];

		base::HandleAllocT<MARA_CONFIG_MAX_MESHES> m_meshHandle;
		base::HandleHashMapT<MARA_CONFIG_MAX_MESHES> m_meshHashMap;
		MeshRef m_meshes[MARA_CONFIG_MAX_MESHES];

		base::HandleAllocT<MARA_CONFIG_MAX_PREFABS> m_prefabHandle;
		base::HandleHashMapT<MARA_CONFIG_MAX_PREFABS> m_prefabHashMap;
		PrefabRef m_prefabs[MARA_CONFIG_MAX_PREFABS];

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
				if (BASE_ENABLED(MARA_CONFIG_DEBUG))
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

		FreeHandle<ResourceHandle, MARA_CONFIG_MAX_RESOURCES>  m_freeResources;
		FreeHandle<EntityHandle, MARA_CONFIG_MAX_ENTITIES>  m_freeEntities;
		FreeHandle<ComponentHandle, MARA_CONFIG_MAX_COMPONENTS> m_freeComponents;
		FreeHandle<GeometryHandle, MARA_CONFIG_MAX_GEOMETRIES>  m_freeGeometries;
		FreeHandle<ShaderHandle, MARA_CONFIG_MAX_SHADERS> m_freeShaders;
		FreeHandle<TextureHandle, MARA_CONFIG_MAX_TEXTURES> m_freeTextures;
		FreeHandle<MaterialHandle, MARA_CONFIG_MAX_MATERIALS> m_freeMaterials;
		FreeHandle<MeshHandle, MARA_CONFIG_MAX_MESHES> m_freeMeshes;
		FreeHandle<PrefabHandle, MARA_CONFIG_MAX_PREFABS> m_freePrefabs;

		entry::MouseState m_mouseState;
	};

#undef MARA_API_FUNC

} // namespace mara

#endif // MARA_P_H_HEADER_GUARD