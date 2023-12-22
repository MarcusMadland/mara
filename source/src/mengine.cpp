#include "mengine/mengine.h"
#include "mengine_p.h"

#include <mapp/commandline.h>
#include <mapp/allocator.h>
#include <mapp/endian.h>
#include <mapp/math.h>
#include <mapp/readerwriter.h>
#include <mapp/string.h>

namespace mengine {

	static Context* s_ctx = NULL;

	MaterialParameters& MaterialParameters::begin()
	{
		// I will do this later
		return *this;
	}

	MaterialParameters& MaterialParameters::addVec4(const char* _name, F32 value[4], U16 _num)
	{
		U32 hash = bx::hash<bx::HashMurmur2A>(_name);

		uniformData[hash].type = bgfx::UniformType::Vec4;
		uniformData[hash].data = bgfx::makeRef(&value[0], sizeof(F32) * 4);
		uniformData[hash].num = _num;

		return *this;
	}

	MaterialParameters& MaterialParameters::addMat3(const char* _name, F32 value[9], U16 _num)
	{
		U32 hash = bx::hash<bx::HashMurmur2A>(_name);

		uniformData[hash].type = bgfx::UniformType::Mat3;
		uniformData[hash].data = bgfx::makeRef(&value[0], sizeof(F32) * 9);
		uniformData[hash].num = _num;

		return *this;
	}

	MaterialParameters& MaterialParameters::addMat4(const char* _name, F32 value[16], U16 _num)
	{
		U32 hash = bx::hash<bx::HashMurmur2A>(_name);

		uniformData[hash].type = bgfx::UniformType::Mat4;
		uniformData[hash].data = bgfx::makeRef(&value[0], sizeof(F32) * 16);
		uniformData[hash].num = _num;

		return *this;
	}

	MaterialParameters& MaterialParameters::addTexture(const char* _name, mengine::ResourceHandle _handle, U16 _stage)
	{
		U32 hash = bx::hash<bx::HashMurmur2A>(_name);
		const char* vfp = s_ctx->m_resources[_handle.idx].vfp.getCPtr();

		size_t vfpLength = bx::strLen(vfp);
		char* vfpWithNullTerminator = new char[vfpLength + 1];
		bx::strCopy(vfpWithNullTerminator, vfpLength + 1, vfp);
		vfpWithNullTerminator[vfpLength] = '\0';

		uniformData[hash].type = bgfx::UniformType::Sampler;
		uniformData[hash].data = bgfx::copy(vfpWithNullTerminator, vfpLength + 1);
		uniformData[hash].num = _stage;

		delete[] vfpWithNullTerminator;

		return *this;
	}

	void MaterialParameters::end()
	{
		// I will do this later
	}

	bool Context::init(const Init& _init)
	{
		bgfx::Init bgfxInit;
		bgfxInit.type = _init.graphicsApi;
		bgfxInit.vendorId = _init.vendorId;
		bgfxInit.resolution = _init.resolution;
		bgfxInit.platformData.nwh = mrender::getNativeWindowHandle(mrender::kDefaultWindowHandle);
		bgfxInit.platformData.ndt = mrender::getNativeDisplayHandle();
		bgfxInit.allocator = mrender::getAllocator();
		if (bgfx::init(bgfxInit))
		{
			bgfx::setViewRect(0, 0, 0, U16(_init.resolution.width), U16(_init.resolution.height));
			bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x000000FF, 1.0f, 0);
			bgfx::touch(0);

			return true;
		}

		return false;
	}

	void Context::shutdown()
	{
		bgfx::shutdown();
	}

	bool Context::update(U32 _debug, U32 _reset)
	{
		bgfx::setDebug(_debug);

		U32 width, height;
		if (!mrender::processEvents(width, height, _debug, _reset, &m_mouseState))
		{
			bgfx::setViewRect(0, 0, 0, U16(width), U16(height));

			for (U16 ii = 0, num = m_freeEntities.getNumQueued(); ii < num; ++ii)
			{
				m_entityHandle.free(m_freeEntities.get(ii).idx);
			}

			for (U16 ii = 0, num = m_freeComponents.getNumQueued(); ii < num; ++ii)
			{
				m_componentHandle.free(m_freeComponents.get(ii).idx);
			}

			for (U16 ii = 0, num = m_freeGeometryAssets.getNumQueued(); ii < num; ++ii)
			{
				m_geometryHandle.free(m_freeGeometryAssets.get(ii).idx);
			}

			for (U16 ii = 0, num = m_freeShaderAssets.getNumQueued(); ii < num; ++ii)
			{
				m_shaderHandle.free(m_freeShaderAssets.get(ii).idx);
			}

			for (U16 ii = 0, num = m_freeTextureAssets.getNumQueued(); ii < num; ++ii)
			{
				m_textureHandle.free(m_freeTextureAssets.get(ii).idx);
			}

			for (U16 ii = 0, num = m_freeMaterialAssets.getNumQueued(); ii < num; ++ii)
			{
				m_materialHandle.free(m_freeMaterialAssets.get(ii).idx);
			}

			m_freeEntities.reset();
			m_freeComponents.reset();
			m_freeGeometryAssets.reset();
			m_freeShaderAssets.reset();
			m_freeTextureAssets.reset();
			m_freeMaterialAssets.reset();

			return true;
		}

		return false;
	}

	Init::Init()
		: graphicsApi(bgfx::RendererType::Count)
		, vendorId(BGFX_PCI_ID_NONE)
	{

	}

	bool init(const Init& _init)
	{
		if (NULL != s_ctx)
		{
			BX_TRACE("mengine is already initialized.");
			return false;
		}
		Init init = _init;

		BX_TRACE("Init...")

		// mengine 1.104.7082
		//      ^ ^^^ ^^^^
		//      | |   +--- Commit number  (https://github.com/marcusmadland/mengine / git rev-list --count HEAD)
		//      | +------- API version    (from https://github.com/marcusmadland/mengine/blob/master/scripts/mengine.idl#L4)
		//      +--------- Major revision (always 1)
		BX_TRACE("Version 1.%d.%d (commit: " MENGINE_REV_SHA1 ")", MENGINE_API_VERSION, MENGINE_REV_NUMBER)

		s_ctx = BX_NEW(mrender::getAllocator(), Context);
		if (s_ctx->init(init))
		{
			BX_TRACE("Init complete.");
			return true;
		}

		BX_TRACE("Init failed.");
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

		BX_TRACE("Calling update before initializing.");
		return false;
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

	bool packAssets(const bx::FilePath& _filePath)
	{
		return s_ctx->packAssets(_filePath);
	}

	bool loadAssetPack(const bx::FilePath& _filePath)
	{
		return s_ctx->loadAssetPack(_filePath);
	}

	bool unloadAssetPack(const bx::FilePath& _filePath)
	{
		return s_ctx->unloadAssetPack(_filePath);
	}

	GeometryHandle createGeometry(ResourceHandle _resource)
	{
		if (isValid(_resource))
		{
			return s_ctx->createGeometry(_resource);
		}

		BX_TRACE("Data is null.");
		return MENGINE_INVALID_HANDLE;
	}

	ResourceHandle loadGeometry(const bx::FilePath& _filePath)
	{
		return s_ctx->loadGeometryResource(_filePath);
	}

	ResourceHandle createResource(const GeometryCreate& _data, const bx::FilePath& _vfp)
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

		BX_TRACE("Data is null.");
		return MENGINE_INVALID_HANDLE;
	}

	ResourceHandle loadShader(const bx::FilePath& _filePath)
	{
		return s_ctx->loadShaderResource(_filePath);
	}

	ResourceHandle createResource(const ShaderCreate& _data, const bx::FilePath& _vfp)
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

		BX_TRACE("Data is null.");
		return MENGINE_INVALID_HANDLE;
	}

	ResourceHandle loadTexture(const bx::FilePath& _filePath)
	{
		return s_ctx->loadTextureResource(_filePath);
	}

	ResourceHandle createResource(const TextureCreate& _data, const bx::FilePath& _vfp)
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

	ResourceHandle loadMaterial(const bx::FilePath& _filePath)
	{
		return s_ctx->loadMaterialResource(_filePath);
	}

	ResourceHandle createResource(const MaterialCreate& _data, const bx::FilePath& _vfp)
	{
		return s_ctx->createMaterialResource(_data, _vfp);
	}

	void destroy(MaterialHandle _handle)
	{
		s_ctx->destroyMaterial(_handle);
	}

	void setMaterialUniform(MaterialHandle _handle, bgfx::UniformType::Enum _type, const char* _name, void* _value, U16 _num)
	{
		s_ctx->setMaterialUniform(_handle, _type, _name, _value, _num);
	}

	const mrender::MouseState* getMouseState()
	{
		return s_ctx->getMouseState();
	}

	const Stats* getStats()
	{
		return s_ctx->getStats();
	}

} // namespace mengine

namespace bgfx {

	ProgramHandle createProgram(mengine::ShaderHandle _vsah, mengine::ShaderHandle _fsah, bool _destroyShaders)
	{
		if (!isValid(_vsah) || !isValid(_fsah))
		{
			BX_TRACE("Asset handle is invalid.");
			return BGFX_INVALID_HANDLE;
		}

		mengine::ShaderRef& vsr = mengine::s_ctx->m_shaderAssets[_vsah.idx];
		mengine::ShaderRef& fsr = mengine::s_ctx->m_shaderAssets[_fsah.idx];
		return bgfx::createProgram(vsr.m_sh, fsr.m_sh, _destroyShaders);
	}

	void setGeometry(mengine::GeometryHandle _handle)
	{
		if (!isValid(_handle))
		{
			BX_TRACE("Asset handle is invalid.");
		}

		mengine::GeometryRef& sr = mengine::s_ctx->m_geometryAssets[_handle.idx];
		bgfx::setVertexBuffer(0, sr.m_vbh);
		bgfx::setIndexBuffer(sr.m_ibh);
	}

	void setTexture(U16 _stage, mengine::TextureHandle _texture, UniformHandle _uniform)
	{
		if (!isValid(_texture))
		{
			BX_TRACE("Asset handle is invalid.");
		}

		mengine::TextureRef& sr = mengine::s_ctx->m_textureAssets[_texture.idx];
		bgfx::setTexture(_stage, _uniform, sr.m_th);
	}

	void submit(ViewId _view, mengine::MaterialHandle _material)
	{
		mengine::MaterialRef& mr = mengine::s_ctx->m_materialAssets[_material.idx];

		// @todo Set uniforms
		mengine::ShaderRef& sr = mengine::s_ctx->m_shaderAssets[mr.m_fsh.idx];
		
		bgfx::UniformHandle uniforms[MENGINE_CONFIG_MAX_UNIFORMS_PER_SHADER];
		U16 numUniforms = bgfx::getShaderUniforms(sr.m_sh, uniforms, MENGINE_CONFIG_MAX_UNIFORMS_PER_SHADER);
		for (U16 i = 0; i < numUniforms; i++)
		{
			bgfx::UniformInfo info;
			bgfx::getUniformInfo(uniforms[i], info);

			U16 handle = mengine::s_ctx->m_resourceHashMap.find(mr.m_hash);
			if (kInvalidHandle != handle)
			{
				mengine::MaterialResource* resource = (mengine::MaterialResource*)mengine::s_ctx->m_resources[handle].resource;
					
				U32 uniformHash = bx::hash<bx::HashMurmur2A>(info.name);
				mengine::MaterialParameters::UniformData& data = resource->parameters.uniformData[uniformHash];
					
				if (info.type != bgfx::UniformType::Sampler)
				{
					bgfx::setUniform(uniforms[i], data.data->data, data.num);
				}
				else
				{
					// @todo MEMORY LEAK
					mengine::ResourceHandle resource = mengine::loadTexture((const char*)data.data->data); 
					bgfx::setTexture(data.num, createTexture(resource), uniforms[i]);
				}
			}
			
		}

		bgfx::submit(_view, mr.m_ph);
	}
}


