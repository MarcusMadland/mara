#pragma once

#include <string>

#include "mrender/bgfx.h"
#include "mapp/bx.h"
#include <bimg/bimg.h>
#include "mapp/readerwriter.h"
#include <mapp/timer.h>
#include <mapp/math.h>
#include <mapp/bounds.h>
#include <mapp/pixelformat.h>
#include <mapp/string.h>
#include <mapp/allocator.h>

#include <vector>
#include <mapp/file.h>

namespace mengine {

	///
	struct Args
	{
		Args(int _argc, const char* const* _argv);

		bgfx::RendererType::Enum m_type;
		uint16_t m_pciId;
	};

	struct AssetI
	{
		virtual ~AssetI() {}

		virtual bool serialize(const bx::FilePath& _filePath) = 0;
		virtual bool deserialize(const bx::FilePath& _filePath) = 0;
	};

	struct ShaderAsset : public AssetI
	{
		const bgfx::Memory* m_memory;

		ShaderAsset()
			: m_memory(NULL)
		{}

		~ShaderAsset()
		{}

		bool serialize(const bx::FilePath& _filePath) override
		{
			// @todo
			return false;
		};

		bool deserialize(const bx::FilePath& _filePath) override
		{
			bx::Error err;

			bx::FileReader reader;
			if (!bx::open(&reader, _filePath, &err))
			{
				return false;
			}

			//
			const I32 fileSize = bx::getSize(&reader);
			m_memory = bgfx::alloc(fileSize);
			bx::read(&reader, m_memory->data, fileSize, &err);
			//

			bx::close(&reader);
			return true;
		};
	};

	struct GeometryAsset : public AssetI
	{
		const bgfx::Memory* m_vertexMemory;
		const bgfx::Memory* m_indexMemory;

		GeometryAsset()
			: m_vertexMemory(NULL), m_indexMemory(NULL)
		{
		}

		~GeometryAsset() override
		{}

		bool serialize(const bx::FilePath& _filePath) override
		{
			bx::FileWriter writer;
			bx::makeAll(_filePath.getPath(), bx::ErrorAssert{});
			if (!bx::open(&writer, _filePath, bx::ErrorAssert{}))
			{
				return false;
			}

			// Vertex 
			bx::write(&writer, &m_vertexMemory->size, sizeof(m_vertexMemory->size), bx::ErrorAssert{});
			bx::write(&writer, m_vertexMemory->data, (I32)m_vertexMemory->size, bx::ErrorAssert{});

			// Indices
			bx::write(&writer, &m_indexMemory->size, sizeof(m_indexMemory->size), bx::ErrorAssert{});
			bx::write(&writer, m_indexMemory->data, (I32)m_indexMemory->size, bx::ErrorAssert{});

			bx::close(&writer);
			return true;
		}

		bool deserialize(const bx::FilePath& _filePath) override
		{
			bx::FileReader reader;
			if (!bx::open(&reader, _filePath, bx::ErrorAssert{}))
			{
				return false;
			}

			// Vertex positions
			U32 vertexMemorySize;
			bx::read(&reader, &vertexMemorySize, sizeof(vertexMemorySize), bx::ErrorAssert{});
			m_vertexMemory = bgfx::alloc(vertexMemorySize);
			bx::read(&reader, m_vertexMemory->data, vertexMemorySize, bx::ErrorAssert{});

			// Indices
			U32 indexMemorySize;
			bx::read(&reader, &indexMemorySize, sizeof(indexMemorySize), bx::ErrorAssert{});
			m_indexMemory = bgfx::alloc(indexMemorySize);
			bx::read(&reader, m_indexMemory->data, indexMemorySize, bx::ErrorAssert{});

			bx::close(&reader);
			return true;
		}
	};

	struct TextureAsset : public AssetI
	{
		const bgfx::Memory* m_memory;

		TextureAsset()
			: m_memory(NULL)
		{}

		~TextureAsset() {}

		bool serialize(const bx::FilePath& _filePath) override
		{
			bx::FileWriter writer;
			bx::makeAll(_filePath.getPath(), bx::ErrorAssert{});
			if (!bx::open(&writer, _filePath, bx::ErrorAssert{}))
			{
				return false;
			}

			bx::write(&writer, m_memory->data, m_memory->size, bx::ErrorAssert{});

			bx::close(&writer);
			return true;
		};

		bool deserialize(const bx::FilePath& _filePath) override
		{
			bx::FileReader reader;
			if (!bx::open(&reader, _filePath, bx::ErrorAssert{}))
			{
				return false;
			}

			//
			const I32 fileSize = bx::getSize(&reader);
			m_memory = bgfx::alloc(fileSize);
			bx::read(&reader, m_memory->data, fileSize,bx::ErrorAssert{});
			//

			bx::close(&reader);
			return true;
		};
	};

	struct MeshVertex // @todo make this game based? and make tools depend on game instead of engine
	{
		F32 positions[3];
		F32 normals[3];
		F32 tangents[3];
		F32 uv[2];

		bool operator==(const MeshVertex& other) const
		{
			return memcmp(this, &other, sizeof(mengine::MeshVertex)) == 0;
		}
	};

} // namespace mengine