#include "mengine/mengine.h"

#include <mapp/commandline.h>
#include <mapp/allocator.h>
#include <mapp/endian.h>
#include <mapp/math.h>
#include <mapp/readerwriter.h>
#include <mapp/string.h>

#include <vector>
#include <string>

namespace mengine {

Args::Args(int _argc, const char* const* _argv)
	: m_type(bgfx::RendererType::Count)
	, m_pciId(BGFX_PCI_ID_NONE)
{
	bx::CommandLine cmdLine(_argc, (const char**)_argv);

	if (cmdLine.hasArg("gl"))
	{
		m_type = bgfx::RendererType::OpenGL;
	}
	else if (cmdLine.hasArg("vk"))
	{
		m_type = bgfx::RendererType::Vulkan;
	}
	else if (cmdLine.hasArg("noop"))
	{
		m_type = bgfx::RendererType::Noop;
	}
	if (cmdLine.hasArg("d3d9"))
	{
		m_type = bgfx::RendererType::Direct3D9;
	}
	else if (cmdLine.hasArg("d3d11"))
	{
		m_type = bgfx::RendererType::Direct3D11;
	}
	else if (cmdLine.hasArg("d3d12"))
	{
		m_type = bgfx::RendererType::Direct3D12;
	}
	else if (BX_ENABLED(BX_PLATFORM_OSX))
	{
		if (cmdLine.hasArg("mtl"))
		{
			m_type = bgfx::RendererType::Metal;
		}
	}

	if (cmdLine.hasArg("amd"))
	{
		m_pciId = BGFX_PCI_ID_AMD;
	}
	else if (cmdLine.hasArg("nvidia"))
	{
		m_pciId = BGFX_PCI_ID_NVIDIA;
	}
	else if (cmdLine.hasArg("intel"))
	{
		m_pciId = BGFX_PCI_ID_INTEL;
	}
	else if (cmdLine.hasArg("sw"))
	{
		m_pciId = BGFX_PCI_ID_SOFTWARE_RASTERIZER;
	}
}

} // namespace mengine