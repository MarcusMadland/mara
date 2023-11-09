#pragma once

#include "defines.h"

#include <mrender/bgfx.h>
#include <mapp/bx.h>
#include <mapp/readerwriter.h>
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

#define MENGINE_HANDLE(_name) \
	struct _name { U16 idx; }; \
	inline bool isValid(_name _handle) { return mengine::kInvalidHandle != _handle.idx; }

#define MENGINE_INVALID_HANDLE { mengine::kInvalidHandle }

namespace mengine {

	static const U16 kInvalidHandle = UINT16_MAX;

	MENGINE_HANDLE(AssetHandle)

	struct ShaderData
	{
		const bgfx::Memory* m_codeData;
	};

	struct GeometryData
	{
		const bgfx::Memory* m_vertexData;
		const bgfx::Memory* m_indexData;
		bgfx::VertexLayout m_layout;
	};

	struct TextureData
	{
		const bgfx::Memory* m_pixData;
		U16 m_width;
		U16 m_height;
		bool m_hasMips;
		U16 m_numLayers;
		bgfx::TextureFormat::Enum m_format;
		U64 m_flags;
	};

	struct MaterialData
	{
		U32* m_textures;
		U32 m_numTextures;
		// @todo uniform data
	};

	struct MeshData
	{
		U32 m_geometries;
		U32 m_material;
		float m_transform[16];
	};

	struct PrefabData
	{
		U32* m_meshes;
		U32 m_numMeshes;
	};

	struct Init
	{
		Init();

		/// Custom allocator. When a custom allocator is not
		/// specified, mengine uses the default allocator. mengine assumes
		/// custom allocator is thread safe.
		bx::AllocatorI* allocator;
	};

	struct Stats
	{
		U16 numAssets; //!< Number of loaded assets.
	};

	//
	bool init(const Init& _init = {});

	// 
	bool shutdown();
	
	//
	AssetHandle loadGeometry(const bx::FilePath _filePath);

	//
	bool saveGeometry(GeometryData* _geometryData, const bx::FilePath _filePath);

	//
	AssetHandle loadShader(const bx::FilePath _filePath);

	//
	bool saveShader(ShaderData* _shaderData, const bx::FilePath _filePath);

} // namespace mengine

namespace bgfx {

	ShaderHandle createShader(mengine::AssetHandle _handle);

	VertexBufferHandle createVertexBuffer(
		mengine::AssetHandle _handle
		, uint16_t _flags = BGFX_BUFFER_NONE
	);

	IndexBufferHandle createIndexBuffer(
		mengine::AssetHandle _handle
		, uint16_t _flags = BGFX_BUFFER_NONE
	);

	TextureHandle createTexture2D(
		mengine::AssetHandle _handle
		, uint64_t _flags = BGFX_TEXTURE_NONE | BGFX_SAMPLER_NONE
	);
}
