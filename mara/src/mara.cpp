#include "mara/mara.h"
#include "mara_p.h"

#include <base/commandline.h>
#include <base/allocator.h>
#include <base/endian.h>
#include <base/math.h>
#include <base/readerwriter.h>
#include <base/string.h>

#include <graphics/platform.h>

namespace mara {

	static Context* s_ctx = NULL;

	MaterialParameters& MaterialParameters::begin()
	{
		// I will do this later
		return *this;
	}

	MaterialParameters& MaterialParameters::addVec4(const char* _name, F32* _value, U16 _num)
	{
		U32 hash = base::hash<base::HashMurmur2A>(_name);
		U32 index = parameterHashMap.getNumElements();

		parameterHashMap.insert(hash, parameterHashMap.getNumElements());
		parameters[index].type = graphics::UniformType::Vec4;
		parameters[index].data = graphics::copy(_value, sizeof(F32) * 4);
		parameters[index].num = _num;

		return *this;
	}

	MaterialParameters& MaterialParameters::addMat3(const char* _name, F32 value[9], U16 _num)
	{
		U32 hash = base::hash<base::HashMurmur2A>(_name);
		U32 index = parameterHashMap.getNumElements();

		parameterHashMap.insert(hash, parameterHashMap.getNumElements());
		parameters[index].type = graphics::UniformType::Mat3;
		parameters[index].data = graphics::copy(value, sizeof(F32) * 9);
		parameters[index].num = _num;

		return *this;
	}

	MaterialParameters& MaterialParameters::addMat4(const char* _name, F32 value[16], U16 _num)
	{
		U32 hash = base::hash<base::HashMurmur2A>(_name);
		U32 index = parameterHashMap.getNumElements();

		parameterHashMap.insert(hash, parameterHashMap.getNumElements());
		parameters[index].type = graphics::UniformType::Mat4;
		parameters[index].data = graphics::copy(value, sizeof(F32) * 16);
		parameters[index].num = _num;

		return *this;
	}

	MaterialParameters& MaterialParameters::addTexture(const char* _name, mara::ResourceHandle _handle, U16 _stage)
	{
		U32 hash = base::hash<base::HashMurmur2A>(_name);
		U32 index = parameterHashMap.getNumElements();

		const char* vfp = s_ctx->m_resources[_handle.idx].vfp.getCPtr();
		size_t vfpLength = base::strLen(vfp);
		char* vfpWithNullTerminator = new char[vfpLength + 1];
		base::strCopy(vfpWithNullTerminator, vfpLength + 1, vfp);
		vfpWithNullTerminator[vfpLength] = '\0';

		parameterHashMap.insert(hash, parameterHashMap.getNumElements());
		parameters[index].type = graphics::UniformType::Sampler;
		parameters[index].data = graphics::copy(vfpWithNullTerminator, vfpLength + 1);
		parameters[index].num = _stage;

		delete[] vfpWithNullTerminator;

		return *this;
	}

	void MaterialParameters::end()
	{
		// I will do this later
	}

	bool Context::init(const Init& _init)
	{
		// @todo We call graphics::renderFrame before graphics::init to signal to bgfx not to create a render thread.
		// Most graphics APIs must be used on the same thread that created the window.
		// graphics::renderFrame();

		graphics::Init graphicsInit;
		graphicsInit.type = _init.graphicsApi;
		graphicsInit.vendorId = _init.vendorId;
		graphicsInit.resolution = _init.resolution;
		graphicsInit.platformData.nwh = entry::getNativeWindowHandle(entry::kDefaultWindowHandle);
		graphicsInit.platformData.ndt = entry::getNativeDisplayHandle();
		graphicsInit.allocator = entry::getAllocator();
		if (graphics::init(graphicsInit))
		{
			graphics::setViewRect(0, 0, 0, U16(_init.resolution.width), U16(_init.resolution.height));
			graphics::setViewClear(0, GRAPHICS_CLEAR_COLOR | GRAPHICS_CLEAR_DEPTH, 0x000000FF, 1.0f, 0);
			graphics::touch(0);

			return true;
		}

		return false;
	}

	void Context::shutdown()
	{
		graphics::shutdown();
	}

	bool Context::update(U32 _debug, U32 _reset)
	{
		graphics::setDebug(_debug);

		U32 width, height;
		if (!entry::processEvents(width, height, _debug, _reset, &m_mouseState))
		{
			graphics::setViewRect(0, 0, 0, U16(width), U16(height));

			for (U16 ii = 0, num = m_freeResources.getNumQueued(); ii < num; ++ii)
			{
				m_resourceHandle.free(m_freeResources.get(ii).idx);
			}

			for (U16 ii = 0, num = m_freeEntities.getNumQueued(); ii < num; ++ii)
			{
				m_entityHandle.free(m_freeEntities.get(ii).idx);
			}

			for (U16 ii = 0, num = m_freeComponents.getNumQueued(); ii < num; ++ii)
			{
				m_componentHandle.free(m_freeComponents.get(ii).idx);
			}

			for (U16 ii = 0, num = m_freeGeometries.getNumQueued(); ii < num; ++ii)
			{
				m_geometryHandle.free(m_freeGeometries.get(ii).idx);
			}

			for (U16 ii = 0, num = m_freeShaders.getNumQueued(); ii < num; ++ii)
			{
				m_shaderHandle.free(m_freeShaders.get(ii).idx);
			}

			for (U16 ii = 0, num = m_freeTextures.getNumQueued(); ii < num; ++ii)
			{
				m_textureHandle.free(m_freeTextures.get(ii).idx);
			}

			for (U16 ii = 0, num = m_freeMaterials.getNumQueued(); ii < num; ++ii)
			{
				m_materialHandle.free(m_freeMaterials.get(ii).idx);
			}

			for (U16 ii = 0, num = m_freeMeshes.getNumQueued(); ii < num; ++ii)
			{
				m_meshHandle.free(m_freeMeshes.get(ii).idx);
			}

			m_freeResources.reset();
			m_freeEntities.reset();
			m_freeComponents.reset();
			m_freeGeometries.reset();
			m_freeShaders.reset();
			m_freeTextures.reset();
			m_freeMaterials.reset();
			m_freeMeshes.reset();

			return true;
		}

		return false;
	}

	Init::Init()
		: graphicsApi(graphics::RendererType::Count)
		, vendorId(GRAPHICS_PCI_ID_NONE)
	{

	}

	bool init(const Init& _init)
	{
		if (NULL != s_ctx)
		{
			BASE_TRACE("mara is already initialized.");
			return false;
		}
		Init init = _init;

		BASE_TRACE("Init...")

		// mara 1.104.7082
		//      ^ ^^^ ^^^^
		//      | |   +--- Commit number  (https://github.com/marcusmadland/mara / git rev-list --count HEAD)
		//      | +------- API version    (from https://github.com/marcusmadland/mara/blob/master/scripts/mara.idl#L4)
		//      +--------- Major revision (always 1)
		BASE_TRACE("Version 1.%d.%d (commit: " MARA_REV_SHA1 ")", MARA_API_VERSION, MARA_REV_NUMBER)

		s_ctx = BASE_NEW(entry::getAllocator(), Context);
		if (s_ctx->init(init))
		{
			BASE_TRACE("Init complete.");
			return true;
		}

		BASE_TRACE("Init failed.");
		return false;
	}

	void shutdown()
	{
		if (NULL != s_ctx)
		{
			s_ctx->shutdown();
		}
	}

	bool update(U32 _debug, U32 _reset)
	{
		if (NULL != s_ctx)
		{
			return s_ctx->update(_debug, _reset);
		}

		BASE_TRACE("Calling update before initializing.");
		return false;
	}

	void destroy(ResourceHandle _handle)
	{
		s_ctx->destroyResource(_handle);
	}

	ComponentHandle createComponent(ComponentI* _data)
	{
		return s_ctx->createComponent(_data);
	}

	void destroy(ComponentHandle _handle)
	{
		s_ctx->destroyComponent(_handle);
	}

	void addComponent(EntityHandle _entity, U32 _type, ComponentHandle _component)
	{
		s_ctx->addComponent(_entity, _type, _component);
	}

	void* getComponentData(EntityHandle _entity, U32 _type)
	{
		return s_ctx->getComponentData(_entity, _type);
	}

	EntityQuery* queryEntities(U32 _types)
	{
		return s_ctx->queryEntities(_types);
	}

	EntityHandle createEntity()
	{
		return s_ctx->createEntity();
	}

	void destroy(EntityHandle _handle)
	{
		s_ctx->destroyEntity(_handle);
	}

	bool createPak(const base::FilePath& _filePath)
	{
		return s_ctx->createPak(_filePath);
	}

	bool loadPak(const base::FilePath& _filePath)
	{
		return s_ctx->loadPak(_filePath);
	}

	bool unloadPak(const base::FilePath& _filePath)
	{
		return s_ctx->unloadPak(_filePath);
	}

	GeometryHandle createGeometry(ResourceHandle _resource)
	{
		if (isValid(_resource))
		{
			return s_ctx->createGeometry(_resource);
		}

		BASE_TRACE("Data is null.");
		return MARA_INVALID_HANDLE;
	}

	ResourceHandle loadGeometry(const base::FilePath& _filePath)
	{
		return s_ctx->loadGeometryResource(_filePath);
	}

	ResourceHandle createResource(const GeometryCreate& _data, const base::FilePath& _vfp)
	{
		return s_ctx->createGeometryResource(_data, _vfp);
	}

	void destroy(GeometryHandle _handle)
	{
		s_ctx->destroyGeometry(_handle);
	}

	ShaderHandle createShader(ResourceHandle _resource)
	{
		if (isValid(_resource))
		{
			return s_ctx->createShader(_resource);
		}

		BASE_TRACE("Data is null.");
		return MARA_INVALID_HANDLE;
	}

	ResourceHandle loadShader(const base::FilePath& _filePath)
	{
		return s_ctx->loadShaderResource(_filePath);
	}

	ResourceHandle createResource(const ShaderCreate& _data, const base::FilePath& _vfp)
	{
		return s_ctx->createShaderResource(_data, _vfp);
	}

	void destroy(ShaderHandle _handle)
	{
		s_ctx->destroyShader(_handle);
	}

	TextureHandle createTexture(ResourceHandle _resource)
	{
		if (isValid(_resource))
		{
			return s_ctx->createTexture(_resource);
		}

		BASE_TRACE("Data is null.");
		return MARA_INVALID_HANDLE;
	}

	ResourceHandle loadTexture(const base::FilePath& _filePath)
	{
		return s_ctx->loadTextureResource(_filePath);
	}

	ResourceHandle createResource(const TextureCreate& _data, const base::FilePath& _vfp)
	{
		return s_ctx->createTextureResource(_data, _vfp);
	}

	void destroy(TextureHandle _handle)
	{
		s_ctx->destroyTexture(_handle);
	}

	MaterialHandle createMaterial(ResourceHandle _resource)
	{
		return s_ctx->createMaterial(_resource);
	}

	ResourceHandle loadMaterial(const base::FilePath& _filePath)
	{
		return s_ctx->loadMaterialResource(_filePath);
	}

	ResourceHandle createResource(const MaterialCreate& _data, const base::FilePath& _vfp)
	{
		return s_ctx->createMaterialResource(_data, _vfp);
	}

	void destroy(MaterialHandle _handle)
	{
		s_ctx->destroyMaterial(_handle);
	}

	//
	MeshHandle createMesh(ResourceHandle _resource)
	{
		return s_ctx->createMesh(_resource);
	}

	//
	ResourceHandle loadMesh(const base::FilePath& _filePath)
	{ 
		return s_ctx->loadMeshResource(_filePath);
	}

	//
	ResourceHandle createResource(const MeshCreate& _data, const base::FilePath& _vfp)
	{
		return s_ctx->createMeshResource(_data, _vfp);
	}

	void destroy(MeshHandle _handle)
	{
		s_ctx->destroyMesh(_handle);
	}

	void getMeshTransform(F32* _result, MeshHandle _handle)
	{
		s_ctx->getMeshTransform(_result, _handle);
	}

	const entry::MouseState* getMouseState()
	{
		return s_ctx->getMouseState();
	}

	const Stats* getStats()
	{
		return s_ctx->getStats();
	}

} // namespace mara

namespace graphics {

	ProgramHandle createProgram(mara::ShaderHandle _vsah, mara::ShaderHandle _fsah, bool _destroyShaders)
	{
		if (!isValid(_vsah) || !isValid(_fsah))
		{
			BASE_TRACE("Shader handle is invalid.");
			return GRAPHICS_INVALID_HANDLE;
		}

		mara::ShaderRef& vsr = mara::s_ctx->m_shaders[_vsah.idx];
		mara::ShaderRef& fsr = mara::s_ctx->m_shaders[_fsah.idx];
		return graphics::createProgram(vsr.m_sh, fsr.m_sh, _destroyShaders);
	}

	void setGeometry(mara::GeometryHandle _handle)
	{
		if (!isValid(_handle))
		{
			BASE_TRACE("Geometry handle is invalid.");
		}

		mara::GeometryRef& sr = mara::s_ctx->m_geometries[_handle.idx];
		graphics::setVertexBuffer(0, sr.m_vbh);
		graphics::setIndexBuffer(sr.m_ibh);
	}

	void setTexture(U16 _stage, mara::TextureHandle _texture, UniformHandle _uniform)
	{
		if (!isValid(_texture))
		{
			BASE_TRACE("Texture handle is invalid.");
		}

		mara::TextureRef& sr = mara::s_ctx->m_textures[_texture.idx];
		graphics::setTexture(_stage, _uniform, sr.m_th);
	}

	void submit(ViewId _view, mara::MaterialHandle _material)
	{
		mara::MaterialRef& mr = mara::s_ctx->m_materials[_material.idx];
		mara::ShaderRef& sr = mara::s_ctx->m_shaders[mr.m_fsh.idx];
		
		graphics::UniformHandle uniforms[MARA_CONFIG_MAX_UNIFORMS_PER_SHADER];
		U16 numUniforms = graphics::getShaderUniforms(sr.m_sh, uniforms, MARA_CONFIG_MAX_UNIFORMS_PER_SHADER);
		for (U16 i = 0; i < numUniforms; i++)
		{
			graphics::UniformInfo info;
			graphics::getUniformInfo(uniforms[i], info);

			U16 handle = mara::s_ctx->m_resourceHashMap.find(mr.m_hash);
			if (kInvalidHandle != handle)
			{
				mara::MaterialResource* resource = (mara::MaterialResource*)mara::s_ctx->m_resources[handle].resource;
				U32 uniformHash = base::hash<base::HashMurmur2A>(info.name);

				U32 index = resource->parameters.parameterHashMap.find(uniformHash);
				if (index == base::kInvalidHandle)
				{
					switch (info.type)
					{
						case UniformType::Vec4:
						{
							F32 vec4[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
							graphics::setUniform(uniforms[i], vec4, 1);
							break;
						}

						case UniformType::Mat3:
						{
							F32 mat3[9];
							base::mtxIdentity(mat3);
							graphics::setUniform(uniforms[i], mat3, 1);
							break;
						}

						case UniformType::Mat4:
						{
							F32 mat4[16];
							base::mtxIdentity(mat4);
							graphics::setUniform(uniforms[i], mat4, 1);
							break;
						}
					}

					continue;
				}

				mara::MaterialParameters::UniformData& data = resource->parameters.parameters[index];

				if (info.type == graphics::UniformType::Sampler)
				{
					graphics::setTexture(data.num, mr.m_textures[index], uniforms[i]);
				}
				else
				{
					graphics::setUniform(uniforms[i], data.data->data, data.num);

					F32* floatData = reinterpret_cast<F32*>(data.data->data);
					F32 x = floatData[0];
					F32 y = floatData[1];
					F32 z = floatData[2];
				}
			}
			
		}

		graphics::submit(_view, mr.m_ph);
	}

	void submit(ViewId _view, mara::MeshHandle _handle)
	{
		mara::MeshRef& mr = mara::s_ctx->m_meshes[_handle.idx];
		setGeometry(mr.m_geometry);

		submit(_view, mr.m_material);
	}
}


