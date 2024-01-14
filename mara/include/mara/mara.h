/*
 * Copyright 2023 Marcus Madland. All rights reserved.
 * License: https://github.com/MarcusMadland/mara/blob/main/LICENSE
 */

#ifndef MARA_H_HEADER_GUARD
#define MARA_H_HEADER_GUARD

#include <graphics/graphics.h>
#include <graphics/entry.h>

#include <base/types.h>
#include <base/readerwriter.h>
#include <base/handlealloc.h>

#include "defines.h"

#include "../../src/config.h" // @todo

///
#define MARA_HANDLE(_name) \
	struct _name { U16 idx; }; \
	inline bool isValid(_name _handle) { return mara::kInvalidHandle != _handle.idx; }

#define MARA_INVALID_HANDLE { mara::kInvalidHandle }

#define MARA_DEFINE_COMPONENT(name) static const U32 name = (1u << __COUNTER__);

/// MARA
namespace mara 
{
	static const U16 kInvalidHandle = UINT16_MAX;

	MARA_HANDLE(ResourceHandle)
	MARA_HANDLE(EntityHandle)
	MARA_HANDLE(ComponentHandle)
	MARA_HANDLE(GeometryHandle)
	MARA_HANDLE(ShaderHandle)
	MARA_HANDLE(TextureHandle)
	MARA_HANDLE(MaterialHandle)
	MARA_HANDLE(MeshHandle)
	MARA_HANDLE(PrefabHandle)

	/// Component interface to implement destructor for it's data.
	///
	struct BASE_NO_VTABLE ComponentI
	{
		///
		virtual ~ComponentI() = 0;
	};

	inline ComponentI::~ComponentI()
	{
	}

	///
	struct BASE_NO_VTABLE ResourceI
	{
		virtual U32 getSize() = 0;

		virtual void write(base::WriterI* _writer, base::Error* _err) = 0;
		virtual void read(base::ReaderSeekerI* _reader, base::Error* _err) = 0;
	};

	/// Queried entities data.
	///
	struct EntityQuery
	{
		U32 m_count;				   //!< Number of queried entities.
		EntityHandle m_entities[MARA_CONFIG_MAX_ENTITIES_TO_QUERY]; //!< List of queried entities.
		// @todo Should be allocated on heap, making it dynamic
	};

	///
	struct MaterialParameters
	{
		MaterialParameters& begin();

		MaterialParameters& addVec4(const char* _name, F32* _value, U16 _num = 1);
		MaterialParameters& addMat3(const char* _name, F32 value[9], U16 _num = 1);
		MaterialParameters& addMat4(const char* _name, F32 value[16], U16 _num = 1);
		MaterialParameters& addTexture(const char* _name, mara::ResourceHandle _handle, U16 _stage = 0);

		void end();

		struct UniformData
		{
			graphics::UniformType::Enum type;
			const graphics::Memory* data;
			U16 num;
		};
		base::HandleHashMapT<MARA_CONFIG_MAX_UNIFORMS_PER_SHADER> parameterHashMap;
		UniformData parameters[MARA_CONFIG_MAX_UNIFORMS_PER_SHADER];
	};

	///
	struct ResourceInfo
	{
		base::FilePath vfp;
		U16 refCount;
	};

	///
	struct GeometryCreate
	{
		const void* vertices;
		U32 verticesSize;
		const U16* indices;
		U32 indicesSize;
		graphics::VertexLayout layout;
	};

	struct ShaderCreate
	{
		const graphics::Memory* mem;
	};

	struct TextureCreate
	{
		U16 width;
		U16 height;
		bool hasMips;
		graphics::TextureFormat::Enum format;
		U64 flags;
		const void* mem;
		U32 memSize;
	};

	struct MaterialCreate
	{
		base::FilePath vertShaderPath;
		base::FilePath fragShaderPath;
		MaterialParameters parameters;
	};

	struct MeshCreate
	{
		base::FilePath materialPath;
		base::FilePath geometryPath;
		F32 m_transform[16];
	};

	struct PrefabCreate
	{
		U16 m_numMeshes;
		base::FilePath meshPaths[MARA_CONFIG_MAX_MESHES_PER_PREFAB];
	};

	/// Initialization parameters used by `mara::init`.
	///
	struct Init
	{
		Init();

		/// Select rendering backend. When set to RendererType::Count
		/// a default rendering backend will be selected appropriate to the platform.
		/// See: `graphics::RendererType`
		graphics::RendererType::Enum graphicsApi; //!< Backend graphics api entities.
		
		/// Vendor PCI ID. If set to `GRAPHICS_PCI_ID_NONE`, discrete and integrated
		/// GPUs will be prioritised.
		///   - `GRAPHICS_PCI_ID_NONE` - Auto-select adapter.
		///   - `GRAPHICS_PCI_ID_SOFTWARE_RASTERIZER` - Software rasterizer.
		///   - `GRAPHICS_PCI_ID_AMD` - AMD adapter.
		///   - `GRAPHICS_PCI_ID_APPLE` - Apple adapter.
		///   - `GRAPHICS_PCI_ID_INTEL` - Intel adapter.
		///   - `GRAPHICS_PCI_ID_NVIDIA` - NVIDIA adapter.
		///   - `GRAPHICS_PCI_ID_MICROSOFT` - Microsoft adapter.
		U16 vendorId;

		/// Backbuffer resolution and reset parameters. See: `graphics::Resolution`.
		graphics::Resolution resolution;
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
		U16 prefabRef[100];		//!< Number of references of prefab.


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
		U16 numPrefabs;			//!< Number of loaded prefabs.
	};

	/// Initialize the mara.
	///
	/// @param[in] _init Initialization parameters. See: `mara::Init` for more info.
	///
	/// @returns `true` if initialization was successful.
	///
	bool init(const Init& _init = {});

	/// Shutdown mara.
	///
	void shutdown();

	/// Update the mara.
	///
	/// @param[in] _debug Available flags:
	///   - `GRAPHICS_DEBUG_IFH` - Infinitely fast hardware. When this flag is set
	///     all rendering calls will be skipped. This is useful when profiling
	///     to quickly assess potential bottlenecks between CPU and GPU.
	///   - `GRAPHICS_DEBUG_PROFILER` - Enable profiler.
	///   - `GRAPHICS_DEBUG_STATS` - Display internal statistics.
	///   - `GRAPHICS_DEBUG_TEXT` - Display debug text.
	///   - `GRAPHICS_DEBUG_WIREFRAME` - Wireframe rendering. All rendering
	///     primitives will be rendered as lines.
	/// @param[in] _reset See: `GRAPHICS_RESET_*` for more info.
	///   - `GRAPHICS_RESET_NONE` - No reset flags.
	///   - `GRAPHICS_RESET_FULLSCREEN` - Not supported yet.
	///   - `GRAPHICS_RESET_MSAA_X[2/4/8/16]` - Enable 2, 4, 8 or 16 x MSAA.
	///   - `GRAPHICS_RESET_VSYNC` - Enable V-Sync.
	///   - `GRAPHICS_RESET_MAXANISOTROPY` - Turn on/off max anisotropy.
	///   - `GRAPHICS_RESET_CAPTURE` - Begin screen capture.
	///   - `GRAPHICS_RESET_FLUSH_AFTER_RENDER` - Flush rendering after submitting to GPU.
	///   - `GRAPHICS_RESET_FLIP_AFTER_RENDER` - This flag  specifies where flip
	///     occurs. Default behavior is that flip occurs before rendering new
	///     frame. This flag only has effect when `GRAPHICS_CONFIG_MULTITHREADED=0`.
	///   - `GRAPHICS_RESET_SRGB_BACKBUFFER` - Enable sRGB back-buffer.
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
	bool createPak(const base::FilePath& _filePath);

	//
	bool loadPak(const base::FilePath& _filePath);

	//
	bool unloadPak(const base::FilePath& _filePath);

	// 
	U32 getResourceInfo(ResourceInfo* _outInfoList, bool _sort = false);

	//
	GeometryHandle createGeometry(ResourceHandle _resource);

	//
	ResourceHandle loadGeometry(const base::FilePath& _filePath);

	//
	ResourceHandle createResource(const GeometryCreate& _data, const base::FilePath& _vfp);

	//
	void destroy(GeometryHandle _handle);

	//
	ShaderHandle createShader(ResourceHandle _resource);

	//
	ResourceHandle loadShader(const base::FilePath& _filePath);

	//
	ResourceHandle createResource(const ShaderCreate& _data, const base::FilePath& _vfp);

	//
	void destroy(ShaderHandle _handle);

	//
	TextureHandle createTexture(ResourceHandle _resource);

	//
	ResourceHandle loadTexture(const base::FilePath& _filePath);

	//
	ResourceHandle createResource(const TextureCreate& _data, const base::FilePath& _vfp);

	//
	void destroy(TextureHandle _handle);

	//
	MaterialHandle createMaterial(ResourceHandle _resource);

	//
	ResourceHandle loadMaterial(const base::FilePath& _filePath);

	//
	ResourceHandle createResource(const MaterialCreate& _data, const base::FilePath& _vfp);

	//
	void destroy(MaterialHandle _handle);

	//
	MeshHandle createMesh(ResourceHandle _resource);

	//
	ResourceHandle loadMesh(const base::FilePath& _filePath);

	//
	ResourceHandle createResource(const MeshCreate& _data, const base::FilePath& _vfp);

	//
	void destroy(MeshHandle _handle);

	//
	void getMeshTransform(F32* _result, MeshHandle _handle);

	//
	PrefabHandle createPrefab(ResourceHandle _resource);

	//
	ResourceHandle loadPrefab(const base::FilePath& _filePath);

	//
	ResourceHandle createResource(const PrefabCreate& _data, const base::FilePath& _vfp);

	//
	void destroy(PrefabHandle _handle);

	//
	U16 getNumMeshes(PrefabHandle _handle);

	//
	MeshHandle* getMeshes(PrefabHandle _handle);

	/// Returns mouse state for input.
	///
	const entry::MouseState* getMouseState();

	/// Returns delta time in seconds.
	///
	const F32 getDeltaTime();

	/// Returns performance counters.
	///
	const Stats* getStats();

} // namespace mara

namespace graphics {

	ProgramHandle createProgram(mara::ShaderHandle _vsah, mara::ShaderHandle _fsah, bool _destroyShaders = false);

	void setGeometry(mara::GeometryHandle _handle);

	void setTexture(U8 _stage, mara::TextureHandle _texture, UniformHandle _uniform);

	void submit(ViewId _view, mara::MaterialHandle _material);

	void submit(ViewId _view, mara::MeshHandle _handle);
}

#endif // MARA_H_HEADER_GUARD