#pragma once

#include <string>

#include "mrender/bgfx.h"
#include "mapp/bx.h"
#include "mrender/entry.h"
#include <bimg/bimg.h>
#include "mapp/readerwriter.h"
#include <mapp/timer.h>
#include <mapp/math.h>
#include <mapp/bounds.h>
#include <mapp/pixelformat.h>
#include <mapp/string.h>
#include <mapp/allocator.h>

#include <vector>

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
			return false;
		};

		bool deserialize(const bx::FilePath& _filePath) override
		{
			bx::Error err;

			bx::FileReaderI* reader = mrender::getFileReader();
			if (!reader->open(_filePath, &err))
			{
				return false;
			}

			//
			const I32 fileSize = bx::getSize(reader);
			m_memory = bgfx::alloc(fileSize);
			reader->read(m_memory->data, fileSize, &err);
			//

			reader->close();
			return true;
		};
	};

	struct GeometryAsset : public AssetI
	{
		const bgfx::Memory* m_vertexMemory;
		const bgfx::Memory* m_indexMemory;

		GeometryAsset()
			: m_vertexMemory(NULL), m_indexMemory(NULL)
		{}

		GeometryAsset(const bgfx::Memory* _vertexMemory, const bgfx::Memory* _indexMemory)
			: m_vertexMemory(_vertexMemory), m_indexMemory(_indexMemory)
		{}

		~GeometryAsset()
		{}

		bool serialize(const bx::FilePath& _filePath) override
		{
			bx::Error err;

			bx::FileWriterI* writer = mrender::getFileWriter();
			if (!writer->open(_filePath, &err))
			{
				return false;
			}

			writer->write(&m_vertexMemory->size, sizeof(m_vertexMemory->size), &err);
			writer->write(&m_indexMemory->size, sizeof(m_indexMemory->size), &err);

			writer->write(m_vertexMemory->data, m_vertexMemory->size, &err);
			writer->write(m_indexMemory->data, m_indexMemory->size, &err);

			writer->close();
			return true;
		};

		bool deserialize(const bx::FilePath& _filePath) override
		{
			bx::Error err;

			bx::FileReaderI* reader = mrender::getFileReader();
			if (!reader->open(_filePath, &err))
			{
				return false;
			}

			//
			uint32_t vertexMemorySize;
			reader->read(&vertexMemorySize, sizeof(vertexMemorySize), &err);
			
			uint32_t indexMemorySize;
			reader->read(&indexMemorySize, sizeof(indexMemorySize), &err);
		
			m_vertexMemory = bgfx::alloc(vertexMemorySize);
			reader->read(m_vertexMemory->data, vertexMemorySize, &err);

			m_indexMemory = bgfx::alloc(indexMemorySize);
			reader->read(m_indexMemory->data, indexMemorySize, &err);
			//

			reader->close();
			return true;
		};
	};

} // namespace mengine