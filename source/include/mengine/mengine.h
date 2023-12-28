/*
 * Copyright 2023 Marcus Madland. All rights reserved.
 * License: https://github.com/MarcusMadland/mengine/blob/main/LICENSE
 */

#ifndef MENGINE_H_HEADER_GUARD
#define MENGINE_H_HEADER_GUARD

#include <mrender/bgfx.h>
#include <mrender/entry.h>
#include <mapp/types.h>
#include <mapp/readerwriter.h>
#include <mapp/handlealloc.h>

#include "defines.h"

#include "../../src/config.h" // @todo

///
#define MENGINE_HANDLE(_name) \
	struct _name { U16 idx; }; \
	inline bool isValid(_name _handle) { return mengine::kInvalidHandle != _handle.idx; }

#define MENGINE_INVALID_HANDLE { mengine::kInvalidHandle }

#define MENGINE_DEFINE_COMPONENT(name) static const U32 name = (1u << __COUNTER__);

/// MENGINE
namespace mengine 
{
	static const U16 kInvalidHandle = UINT16_MAX;

	MENGINE_HANDLE(ResourceHandle)
	MENGINE_HANDLE(EntityHandle)
	MENGINE_HANDLE(ComponentHandle)
	MENGINE_HANDLE(GeometryHandle)
	MENGINE_HANDLE(ShaderHandle)
	MENGINE_HANDLE(TextureHandle)
	MENGINE_HANDLE(MaterialHandle)
	MENGINE_HANDLE(MeshHandle)

	/// Component interface to implement destructor for it's data.
	///
	struct BX_NO_VTABLE ComponentI
	{
		///
		virtual ~ComponentI() = 0;
	};

	inline ComponentI::~ComponentI()
	{
	}

	///
	struct BX_NO_VTABLE ResourceI
	{
		// @todo Virtual destructor?
		virtual U32 getSize() = 0;

		virtual void write(bx::WriterI* _writer, bx::Error* _err) = 0;
		virtual void read(bx::ReaderSeekerI* _reader, bx::Error* _err) = 0;
	};

	/// Queried entities data.
	///
	struct EntityQuery
	{
		U32 m_count;				   //!< Number of queried entities.
		EntityHandle m_entities[1000]; //!< List of queried entities.
		// @todo Should be allocated on heap, making it dynamic
	};

	///
	struct MaterialParameters
	{
		MaterialParameters& begin();

		MaterialParameters& addVec4(const char* _name, float value[4], U16 _num = 1);
		MaterialParameters& addMat3(const char* _name, float value[9], U16 _num = 1);
		MaterialParameters& addMat4(const char* _name, float value[16], U16 _num = 1);
		MaterialParameters& addTexture(const char* _name, mengine::ResourceHandle _handle, U16 _stage = 0);

		void end();

		struct UniformData
		{
			bgfx::UniformType::Enum type;
			const bgfx::Memory* data;
			U16 num;
		};
		bx::HandleHashMapT<MENGINE_CONFIG_MAX_UNIFORMS_PER_SHADER> parameterHashMap;
		UniformData parameters[MENGINE_CONFIG_MAX_UNIFORMS_PER_SHADER];
	};

	///
	struct GeometryCreate
	{
		const void* vertices;
		U32 verticesSize;
		const U16* indices;
		U32 indicesSize;
		bgfx::VertexLayout layout;
	};

	struct ShaderCreate
	{
		const bgfx::Memory* mem;
	};

	struct TextureCreate
	{
		U16 width;
		U16 height;
		bool hasMips;
		bgfx::TextureFormat::Enum format;
		U64 flags;
		const void* mem;
		U32 memSize;
	};

	struct MaterialCreate
	{
		bx::FilePath vertShaderPath;
		bx::FilePath fragShaderPath;
		MaterialParameters parameters;
	};

	struct MeshCreate
	{
		bx::FilePath materialPath;

		U16 numGeometries;
		bx::FilePath geometryPaths[MENGINE_CONFIG_MAX_GEOMETRIES_PER_MESH];

		F32 m_transforms[MENGINE_CONFIG_MAX_GEOMETRIES_PER_MESH][16];
	};

	/// Initialization parameters used by `mengine::init`.
	///
	struct Init
	{
		Init();

		/// Select rendering backend. When set to RendererType::Count
		/// a default rendering backend will be selected appropriate to the platform.
		/// See: `bgfx::RendererType`
		bgfx::RendererType::Enum graphicsApi; //!< Backend graphics api entities.
		
		/// Vendor PCI ID. If set to `BGFX_PCI_ID_NONE`, discrete and integrated
		/// GPUs will be prioritised.
		///   - `BGFX_PCI_ID_NONE` - Auto-select adapter.
		///   - `BGFX_PCI_ID_SOFTWARE_RASTERIZER` - Software rasterizer.
		///   - `BGFX_PCI_ID_AMD` - AMD adapter.
		///   - `BGFX_PCI_ID_APPLE` - Apple adapter.
		///   - `BGFX_PCI_ID_INTEL` - Intel adapter.
		///   - `BGFX_PCI_ID_NVIDIA` - NVIDIA adapter.
		///   - `BGFX_PCI_ID_MICROSOFT` - Microsoft adapter.
		U16 vendorId;

		/// Backbuffer resolution and reset parameters. See: `bgfx::Resolution`.
		bgfx::Resolution resolution;
	};

	/// Engine statistics data.
	///
	struct Stats
	{
		// @todo Remove ref stats, or make it lots better.
		U16 resourcesRef[100];	//!< Number of references of resources.
		U16 entitiesRef[100];	//!< Number of references of entities.
		U16 componentsRef[100]; //!< Number of references of components.
		U16 geometryRef[100];	//!< Number of references of geometries.
		U16 shaderRef[100];		//!< Number of references of shaders.
		U16 textureRef[100];	//!< Number of references of textures.
		U16 materialRef[100];	//!< Number of references of materials.
		U16 meshRef[100];		//!< Number of references of mesh.

		U16 numPaks;			//!< Number of loaded of paks.
		U16 numPakEntries;		//!< Number of loaded of pak entries.
		U16 numResources;		//!< Number of loaded resources.
		U16 numEntities;		//!< Number of loaded entities.
		U16 numComponents;		//!< Number of loaded components.
		U16 numGeometries;		//!< Number of loaded geometries.
		U16 numShaders;			//!< Number of loaded shaders.
		U16 numTextures;		//!< Number of loaded textures.
		U16 numMaterials;		//!< Number of loaded materials.
		U16 numMeshes;			//!< Number of loaded meshes.
	};

	/// Initialize the mengine.
	///
	/// @param[in] _init Initialization parameters. See: `mengine::Init` for more info.
	///
	/// @returns `true` if initialization was successful.
	///
	bool init(const Init& _init = {});

	/// Shutdown mengine.
	///
	void shutdown();

	/// Update the mengine.
	///
	/// @param[in] _debug Available flags:
	///   - `BGFX_DEBUG_IFH` - Infinitely fast hardware. When this flag is set
	///     all rendering calls will be skipped. This is useful when profiling
	///     to quickly assess potential bottlenecks between CPU and GPU.
	///   - `BGFX_DEBUG_PROFILER` - Enable profiler.
	///   - `BGFX_DEBUG_STATS` - Display internal statistics.
	///   - `BGFX_DEBUG_TEXT` - Display debug text.
	///   - `BGFX_DEBUG_WIREFRAME` - Wireframe rendering. All rendering
	///     primitives will be rendered as lines.
	/// @param[in] _reset See: `BGFX_RESET_*` for more info.
	///   - `BGFX_RESET_NONE` - No reset flags.
	///   - `BGFX_RESET_FULLSCREEN` - Not supported yet.
	///   - `BGFX_RESET_MSAA_X[2/4/8/16]` - Enable 2, 4, 8 or 16 x MSAA.
	///   - `BGFX_RESET_VSYNC` - Enable V-Sync.
	///   - `BGFX_RESET_MAXANISOTROPY` - Turn on/off max anisotropy.
	///   - `BGFX_RESET_CAPTURE` - Begin screen capture.
	///   - `BGFX_RESET_FLUSH_AFTER_RENDER` - Flush rendering after submitting to GPU.
	///   - `BGFX_RESET_FLIP_AFTER_RENDER` - This flag  specifies where flip
	///     occurs. Default behavior is that flip occurs before rendering new
	///     frame. This flag only has effect when `BGFX_CONFIG_MULTITHREADED=0`.
	///   - `BGFX_RESET_SRGB_BACKBUFFER` - Enable sRGB back-buffer.
	///
	///  @returns `true` as long as engine is running.
	/// 
	bool update(U32 _debug, U32 _reset);

	//
	void destroy(ResourceHandle _handle);

	//
	ComponentHandle createComponent(ComponentI* _data);

	//
	void destroy(ComponentHandle _handle);

	//
	void addComponent(EntityHandle _entity, U32 _type, ComponentHandle _component);

	//
	void* getComponentData(EntityHandle _entity, U32 _type);

	//
	EntityQuery* queryEntities(U32 _types);

	//
	EntityHandle createEntity();

	//
	void destroy(EntityHandle _handle);

	//
	bool createPak(const bx::FilePath& _filePath);

	//
	bool loadPak(const bx::FilePath& _filePath);

	//
	bool unloadPak(const bx::FilePath& _filePath);

	//
	GeometryHandle createGeometry(ResourceHandle _resource);

	//
	ResourceHandle loadGeometry(const bx::FilePath& _filePath);

	//
	ResourceHandle createResource(const GeometryCreate& _data, const bx::FilePath& _vfp);

	//
	void destroy(GeometryHandle _handle);

	//
	ShaderHandle createShader(ResourceHandle _resource);

	//
	ResourceHandle loadShader(const bx::FilePath& _filePath);

	//
	ResourceHandle createResource(const ShaderCreate& _data, const bx::FilePath& _vfp);

	//
	void destroy(ShaderHandle _handle);

	//
	TextureHandle createTexture(ResourceHandle _resource);

	//
	ResourceHandle loadTexture(const bx::FilePath& _filePath);

	//
	ResourceHandle createResource(const TextureCreate& _data, const bx::FilePath& _vfp);

	//
	void destroy(TextureHandle _handle);

	//
	MaterialHandle createMaterial(ResourceHandle _resource);

	//
	ResourceHandle loadMaterial(const bx::FilePath& _filePath);

	//
	ResourceHandle createResource(const MaterialCreate& _data, const bx::FilePath& _vfp);

	//
	void destroy(MaterialHandle _handle);

	//
	MeshHandle createMesh(ResourceHandle _resource);

	//
	ResourceHandle loadMesh(const bx::FilePath& _filePath);

	//
	ResourceHandle createResource(const MeshCreate& _data, const bx::FilePath& _vfp);

	//
	void destroy(MeshHandle _handle);

	/// Returns mouse state for input.
	///
	const mrender::MouseState* getMouseState();

	/// Returns performance counters.
	///
	const Stats* getStats();

} // namespace mengine

namespace bgfx {

	ProgramHandle createProgram(mengine::ShaderHandle _vsah, mengine::ShaderHandle _fsah, bool _destroyShaders = false);

	void setGeometry(mengine::GeometryHandle _handle);

	void setTexture(U16 _stage, mengine::TextureHandle _texture, UniformHandle _uniform);

	void submit(ViewId _view, mengine::MaterialHandle _material);

	void submit(ViewId _view, mengine::MeshHandle _handle);
}

#endif // MENGINE_H_HEADER_GUARD