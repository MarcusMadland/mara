#pragma once

#include "defines.h"

#include <mrender/bgfx.h>
#include <mrender/entry.h>
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

#define MENGINE_MAKE_COMPONENT_TYPE(name) static const U32 name = U32(__COUNTER__);

namespace mengine {

	static const U16 kInvalidHandle = UINT16_MAX;

	MENGINE_HANDLE(EntityHandle)
	MENGINE_HANDLE(ComponentHandle)

	MENGINE_HANDLE(GeometryAssetHandle)
	MENGINE_HANDLE(ShaderAssetHandle)

	struct ShaderType
	{
		enum Enum
		{
			Vertex,
			Fragment,
			Compute,
		};
	};

	//
	typedef void (*SystemFn)(EntityHandle _handle);

	struct Init
	{
		Init();

		/// Select rendering backend. When set to RendererType::Count
		/// a default rendering backend will be selected appropriate to the platform.
		/// See: `bgfx::RendererType`
		bgfx::RendererType::Enum graphicsApi;

		/// Vendor PCI ID. If set to `BGFX_PCI_ID_NONE`, discrete and integrated
		/// GPUs will be prioritised.
		///   - `BGFX_PCI_ID_NONE` - Auto-select adapter.
		///   - `BGFX_PCI_ID_SOFTWARE_RASTERIZER` - Software rasterizer.
		///   - `BGFX_PCI_ID_AMD` - AMD adapter.
		///   - `BGFX_PCI_ID_APPLE` - Apple adapter.
		///   - `BGFX_PCI_ID_INTEL` - Intel adapter.
		///   - `BGFX_PCI_ID_NVIDIA` - NVIDIA adapter.
		///   - `BGFX_PCI_ID_MICROSOFT` - Microsoft adapter.
		uint16_t vendorId;

		/// Backbuffer resolution and reset parameters. See: `bgfx::Resolution`.
		bgfx::Resolution resolution;

		/// Custom allocator. When a custom allocator is not
		/// specified, mengine uses the default allocator. mengine assumes
		/// custom allocator is thread safe.
		bx::AllocatorI* allocator;
	};

	struct Stats
	{
		U16 numEntities;       //!< Number of loaded entities.
		U16 numComponents;	   //!< Number of loaded components.
		U16 numGeometryAssets; //!< Number of loaded geometry assets.
		U16 numShaderAssets;   //!< Number of loaded shader assets.
	};

	

	//
	bool init(const Init& _init = {});

	// 
	void shutdown();

	//
	bool update();

	//
	ComponentHandle createComponent(void* _data, U32 _size);

	//
	void destroy(ComponentHandle _handle);

	//
	void addComponent(EntityHandle _entity, U32 _type, ComponentHandle _component);

	//
	void* getComponentData(EntityHandle _entity, U32 _type);

	//
	void forEachComponent(U32 _types, SystemFn _systemFn);

	//
	EntityHandle createEntity();

	//
	void destroy(EntityHandle _handle);

	//
	bool packAssets(const bx::FilePath& _filePath);

	//
	bool loadAssetPack(const bx::FilePath& _filePath);

	//
	GeometryAssetHandle createGeometry(const void* _vertices, U32 _verticesSize, const void* _indices, U32 _indicesSize, bgfx::VertexLayout _layout, const bx::FilePath _virtualPath);

	//
	GeometryAssetHandle loadGeometry(const bx::FilePath _filePath);

	//
	void destroy(GeometryAssetHandle _handle);

	//
	ShaderAssetHandle createShader(const bgfx::Memory* _mem, const bx::FilePath _virtualPath);

	//
	ShaderAssetHandle loadShader(const bx::FilePath _filePath);

	//
	void destroy(ShaderAssetHandle _handle);

	//
	const bgfx::Memory* compileShader(const char* _shaderCode, ShaderType::Enum _type);

	//
	const Stats* getStats();

	//
	bx::AllocatorI* getAllocator();

} // namespace mengine

namespace bgfx {

	ProgramHandle createProgram(mengine::ShaderAssetHandle _vsah, mengine::ShaderAssetHandle _fsah);

	void setGeometry(mengine::GeometryAssetHandle _handle);
}
