#include "mengine/mengine.h"
#include "mrender/dialog.h"
#include "mapp/file.h"
#include "mapp/settings.h"
#include "imgui/imgui.h"

#include <fbxsdk.h>

#include "mapp/process.h"

namespace am
{
	struct AssetInfo;
}
namespace ImGui { bool ErrorText(const std::vector<am::AssetInfo>& _assets); }

namespace am
{

struct AssetInfo
{
	AssetInfo()
		: id(0)
		, type("geometry")
		, sourcePath("")
		, outputPath("")
		, localPath("")
		, exist(false)
	{}

	U16 id;
	const char* type;
	bx::FilePath sourcePath;
	bx::FilePath outputPath;
	bx::FilePath localPath;
	bool exist;
};

class AssetManager : public mrender::AppI
{
public:
	AssetManager(const char* _name, const char* _description)
		: mrender::AppI(_name, _description)
		, m_width(1280)
		, m_height(720)
		, m_debug(BGFX_DEBUG_NONE)
		, m_reset(BGFX_RESET_NONE)
		, m_canCompile(true)
		, m_selectedAssetID(-1)
	{}

	void init(I32 _argc, const char* const* _argv, U32 _width, U32 _height) override
	{
		// Members
		m_width = _width;
		m_height = _height;
		m_debug = BGFX_DEBUG_TEXT;
		m_reset = BGFX_RESET_VSYNC;

		// Create argument context
		mengine::Args args = mengine::Args(_argc, _argv);

		// Create renderer backend
		bgfx::Init init;
		init.type = args.m_type;
		init.vendorId = args.m_pciId;
		init.resolution.width = _width;
		init.resolution.height = _height;
		init.platformData.nwh = mrender::getNativeWindowHandle(mrender::kDefaultWindowHandle);
		init.platformData.ndt = mrender::getNativeDisplayHandle();
		bgfx::init(init);

		// Render commands
		bgfx::setViewClear(0
			, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH
			, 0x202020FF
			, 1.0f
			, 0
		);

		// Create settings path @todo Make it in a folder in documents/ 
		m_settingsPath = bx::FilePath(bx::Dir::Executable);
		m_settingsPath = m_settingsPath.getPath();
		m_settingsPath.join("asset_manager.ini");

		// Load Settings
		loadSettings();

		// Create imgui context
		mengine::imguiCreate();
	}

	int shutdown() override
	{
		mengine::imguiDestroy();
		bgfx::shutdown();

		return 0;
	}

	bool update() override
	{
		if (!mrender::processEvents(m_width, m_height, m_debug, m_reset, &m_mouseState))
		{
			mengine::imguiBeginFrame(m_mouseState.m_mx
			                         , m_mouseState.m_my
			                         , (m_mouseState.m_buttons[mrender::MouseButton::Left] ? IMGUI_MBUT_LEFT : 0)
			                         | (m_mouseState.m_buttons[mrender::MouseButton::Right] ? IMGUI_MBUT_RIGHT : 0)
			                         | (m_mouseState.m_buttons[mrender::MouseButton::Middle] ? IMGUI_MBUT_MIDDLE : 0)
			                         , m_mouseState.m_mz
			                         , U16(m_width)
			                         , U16(m_height)
			);
			imguiRender();
			mengine::imguiEndFrame();

			bgfx::setViewRect(0, 0, 0, U16(m_width), U16(m_height));
			bgfx::touch(0);
			bgfx::frame();

			return true;
		}

		return false;
	}

private:
	void imguiRender()
	{
		ImGui::SetNextWindowPos({ 0.0f, 0.0f });
		ImGui::SetNextWindowSize({ (float)m_width, (float)m_height });
		if (ImGui::Begin("Assets", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoMove))
		{
			if (ImGui::Button("Save"))
			{
				saveSettings();
			}
			ImGui::SameLine();
			if (ImGui::Button("Refresh"))
			{
				updateAssetInfoOutputPath();
				updateAssetInfoExist();
			}

			ImGui::Separator();

			ImGui::Text("Source: %s", m_sourcesPath.getCPtr());
			ImGui::SameLine();
			if (ImGui::Button("Browse##1"))
			{
				mrender::openDirectorySelectionDialog(m_sourcesPath, "Select source path");
				saveSettings();
			}

			ImGui::Text("Output: %s", m_outputPath.getCPtr());
			ImGui::SameLine();
			if (ImGui::Button("Browse##2"))
			{
				mrender::openDirectorySelectionDialog(m_outputPath, "Select output path");
				saveSettings();
			}

			ImGui::Separator();

			m_canCompile = ImGui::ErrorText(m_assets);

			if (ImGui::BeginTable("##unique_id", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_ScrollY, { (float)m_width, ((float)m_height / 2.0f) }))
			{
				ImGui::TableSetupColumn("ID", ImGuiTableColumnFlags_WidthFixed, 200.0f); 
				ImGui::TableSetupColumn("Source", ImGuiTableColumnFlags_WidthFixed); 
				ImGui::TableSetupColumn("Runtime", ImGuiTableColumnFlags_WidthFixed);       
				ImGui::TableHeadersRow();

				for (U16 i = 0; i < m_assets.size(); i++)
				{
					ImGui::TableNextRow();

					ImGui::TableSetColumnIndex(0);
					char buffer[32];
					snprintf(buffer, sizeof(buffer), "%u", m_assets[i].id);
					if (ImGui::Selectable(buffer, false, ImGuiSelectableFlags_SpanAllColumns))
					{
						m_selectedAssetID = m_assets[i].id;
					}
					/* @todo does not work with drag and drop
					if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
					{
						ImGui::Image(0, { 100,100 });
					}*/
					if (ImGui::BeginDragDropSource())
					{
						ImGui::SetDragDropPayload("AM_OUTPUT_PATH", m_assets[i].outputPath.getCPtr(), strlen(m_assets[i].outputPath.getCPtr()) * sizeof(char));
						bx::debugPrintf("%s with sizeof %u\n", m_assets[i].outputPath.getCPtr(), strlen(m_assets[i].outputPath.getCPtr()) * sizeof(char));
						mrender::dragPath(m_assets[i].outputPath);
						ImGui::EndDragDropSource();
					}

					ImGui::TableSetColumnIndex(1);
					ImGui::Text(m_assets[i].sourcePath.getCPtr());
					
					
					ImGui::TableSetColumnIndex(2);
					const ImColor textColor = m_assets[i].exist ? ImColor(1.0f, 1.0f, 1.0f, 1.0f) : ImColor(1.0f, 0.0f, 0.0f, 1.0f);
					ImGui::TextColored(textColor, m_assets[i].outputPath.getCPtr());
				}
				ImGui::EndTable();
			}

			if (ImGui::Button("Import Asset"))
			{
				const char* filter = "Supported Files (*.fbx, *.sc)|*.fbx;*.sc;";

				bx::FilePath fullPath = m_sourcesPath;
				mrender::openFileSelectionDialog(fullPath, mrender::FileSelectionDialogType::Open, "Import Asset", filter);

				if (!fullPath.isEmpty() && strcmp(fullPath.getExt().getPtr(), "") != 0)
				{
					importAssetInfo(fullPath);
				}
			}
			ImGui::SameLine();

			if (ImGui::Button("Compile Missing"))
			{
				compile(false);
			}

			ImGui::SameLine();

			if (ImGui::Button("Recompile All"))
			{
				compile(true);
			}

			ImGui::Separator();

			if (m_selectedAssetID != 65535)
			{
				ImGui::Text("Selected Details");
				ImGui::Separator();
				ImGui::Text("ID: %u", m_selectedAssetID);
				ImGui::Text("Name: %s", m_assets[m_selectedAssetID].sourcePath.getBaseName().getPtr());
				{
					const char* items[] = { "geometry", "skeleton", "animation", "vertex", "fragment" };

					if (ImGui::BeginCombo("Asset Type", m_assets[m_selectedAssetID].type))
					{
						for (int n = 0; n < IM_ARRAYSIZE(items); n++)
						{
							bool is_selected = (m_assets[m_selectedAssetID].type == items[n]);
							if (ImGui::Selectable(items[n], is_selected))
							{
								m_assets[m_selectedAssetID].type = items[n];
								if (is_selected)
								{
									ImGui::SetItemDefaultFocus();
								}

								updateAssetInfoOutputPath();
								updateAssetInfoExist();
							}
						}
						ImGui::EndCombo();
					}
				}
				ImGui::Text("Source: %s", m_assets[m_selectedAssetID].sourcePath.getCPtr());
				ImGui::Text("Output: %s", m_assets[m_selectedAssetID].outputPath.getCPtr());

				if (ImGui::Button("Delete"))
				{
					m_assets.erase(m_assets.begin() + m_selectedAssetID);
					for (U16 i = 0; i < m_assets.size(); i++)
					{
						m_assets[i].id = i;
					}
					saveSettings();
					m_selectedAssetID = -1;
				}
			}
		}
		ImGui::End();
	}

	void loadSettings()
	{
		// Create settings
		bx::Settings settings = bx::Settings(mrender::getAllocator());
		bx::FileReader reader;
		if (bx::open(&reader, m_settingsPath))
		{
			bx::read(&reader, settings, bx::ErrorAssert{});
			bx::close(&reader);

			m_sourcesPath = settings.get("sources_path");
			m_outputPath = settings.get("runtime_path");
			{
				char* endPtr;
				U32 numResources = strtoul(settings.get("resource_count").getPtr(), &endPtr, 10);

				for (U32 i = 0; i < numResources; i++)
				{
					char buffer[32];
					snprintf(buffer, sizeof(buffer), "path_%u", i);

					bx::FilePath path = settings.get(buffer);
					importAssetInfo(path);
				}
			}
		}
	}

	void saveSettings()
	{
		bx::Settings settings(mrender::getAllocator());

		settings.set("sources_path", m_sourcesPath);
		settings.set("runtime_path", m_outputPath);

		{
			char buffer[32];
			snprintf(buffer, sizeof(buffer), "%u", (U32)m_assets.size());
			settings.set("resource_count", buffer);
		}

		for (U32 i = 0; i < m_assets.size(); ++i)
		{
			char buffer[32];
			snprintf(buffer, sizeof(buffer), "path_%u", i);
			settings.set(buffer, m_assets[i].sourcePath);
		}

		bx::FileWriter writer;
		if (bx::open(&writer, m_settingsPath));
		{
			bx::write(&writer, settings, bx::ErrorAssert{});
			bx::close(&writer);
		}
	}

	void importAssetInfo(const bx::FilePath& _path)
	{
		bx::StringView relativePathStr = m_sourcesPath;
		bx::FilePath relativePath = bx::strSubstr(_path, relativePathStr.getLength());

		AssetInfo assetInfo;
		assetInfo.id = m_assets.size();
		assetInfo.localPath = relativePath.getPath();
		assetInfo.sourcePath = _path;

		if (strcmp(_path.getExt().getPtr(),".fbx") == 0)
		{
			assetInfo.type = "geometry";
		}
		else if (strcmp(_path.getExt().getPtr(), ".sc") == 0)
		{
			assetInfo.type = "vertex";
		}

		m_assets.push_back(assetInfo);

		updateAssetInfoOutputPath();
		updateAssetInfoExist();
		saveSettings();
	}

	void updateAssetInfoExist()
	{
		for (U32 i = 0; i < m_assets.size(); i++)
		{
			bx::FilePath checkPath = m_assets[i].outputPath;

			bx::FileReader reader;
			if (bx::open(&reader, checkPath))
			{
				m_assets[i].exist = true;
				bx::close(&reader);
			}
			else
			{
				m_assets[i].exist = false;
			}
		}
		
	}

	void updateAssetInfoOutputPath()
	{
		for (U32 i = 0; i < m_assets.size(); i++)
		{
			bx::StringView relativePathStr = m_sourcesPath;
			bx::FilePath relativePath = bx::strSubstr(m_assets[i].sourcePath, relativePathStr.getLength());

			bx::FilePath outputPath = m_outputPath;
			outputPath.join(relativePath);

			bx::FilePath basePath = outputPath.getPath();
			bx::FilePath baseName = outputPath.getBaseName();
			char buffer[32];
			snprintf(buffer, sizeof(buffer), "%s_%s.bin", baseName.getCPtr(), m_assets[i].type);
			basePath.join(buffer);

			m_assets[i].outputPath = basePath;
		}
	}

	void compile(bool _recompileAll)
	{
		if (!m_canCompile)
		{
			return;
		}

		for (U16 i = 0; i < m_assets.size(); i++)
		{
			auto& asset = m_assets[i];
			if (!asset.exist || _recompileAll)
			{
				// FBX
				if (strcmp(asset.sourcePath.getExt().getPtr(), ".fbx") == 0)
				{
					compileAssetFromFBX(i);
				}

				// SC
				if (strcmp(asset.sourcePath.getExt().getPtr(), ".sc") == 0)
				{
					compileAssetFromSC(i);
				}
			}
		}
		updateAssetInfoExist();
	}

	void compileAssetFromFBX(U16 _id)
	{
		AssetInfo& assetInfo = m_assets[_id];
		if (assetInfo.type == "geometry")
		{
			mengine::GeometryAsset geometry;

			FbxManager* sdkManager = FbxManager::Create();

			FbxIOSettings* ios = FbxIOSettings::Create(sdkManager, IOSROOT);
			sdkManager->SetIOSettings(ios);

			FbxImporter* importer = FbxImporter::Create(sdkManager, "GeometryImporter");
			if (!importer->Initialize(assetInfo.sourcePath.getCPtr(), -1, sdkManager->GetIOSettings()))
			{
				bx::debugPrintf("FBX SDK failed to find fbx file.");
				return;
			}

			FbxScene* scene = FbxScene::Create(sdkManager, "ImportedScene");
			importer->Import(scene);
			importer->Destroy();

			FbxNode* rootNode = scene->GetRootNode();
			if (rootNode)
			{
				int meshId = 0;
				for (int i = 0; i < rootNode->GetChildCount(); i++)
				{
					const bool multiple = rootNode->GetChildCount() > 1;

					FbxNode* childNode = rootNode->GetChild(i);
					FbxMesh* mesh = childNode->GetMesh();

					if (mesh)
					{
						meshId++;
						const int numVertices = mesh->GetControlPointsCount();
						const int numIndices = mesh->GetPolygonVertexCount();

						struct vec3 { float x; float y; float z; };
						vec3* vertexPositions = new vec3[numVertices];
						uint16_t* indices = new uint16_t[numIndices];

						for (int j = 0; j < numVertices; j++)
						{
							FbxVector4 vertexPositionFBX = mesh->GetControlPointAt(j);

							vertexPositions[j].x = static_cast<float>(vertexPositionFBX[0]);
							vertexPositions[j].y = static_cast<float>(vertexPositionFBX[1]);
							vertexPositions[j].z = static_cast<float>(vertexPositionFBX[2]);
						}

						int vertexIndex = 0;
						for (int polyIndex = 0; polyIndex < mesh->GetPolygonCount(); polyIndex++)
						{
							int numPolyVertices = mesh->GetPolygonSize(polyIndex);
							for (int polyVertIndex = 0; polyVertIndex < numPolyVertices; polyVertIndex++)
							{
								indices[vertexIndex] = static_cast<uint16_t>(mesh->GetPolygonVertex(polyIndex, polyVertIndex));
								vertexIndex++;
							}
						}

						geometry = mengine::GeometryAsset(
							bgfx::makeRef(vertexPositions, sizeof(vec3) * numVertices),
							bgfx::makeRef(indices, sizeof(uint16_t) * numIndices));

						if (multiple)
						{
							bx::FilePath outPathWithIndex = assetInfo.outputPath.getPath();
							outPathWithIndex.join(assetInfo.outputPath.getBaseName());
							char buffer[100];
							if (meshId <= 1)
							{
								snprintf(buffer, sizeof(buffer), "%s.bin", outPathWithIndex.getCPtr());
							}
							else
							{
								snprintf(buffer, sizeof(buffer), "%s%u.bin", outPathWithIndex.getCPtr(), meshId);
							}

							if (!geometry.serialize(buffer))
							{
								bx::debugPrintf("Failed to serialize asset in %s\n", buffer);
							}
						}
						else
						{
							if (!geometry.serialize(assetInfo.outputPath))
							{
								bx::debugPrintf("Failed to serialize asset\n");
							}
						}

						delete[] vertexPositions;
						delete[] indices;
					}
				}
			}
			sdkManager->Destroy();
		}
	}

	void compileAssetFromSC(U16 _id)
	{
		// bx::open add args etc
		bx::ProcessReader process;
		char buffer[300];
		if (m_assets[_id].type == "vertex")
		{
			snprintf(buffer, sizeof(buffer), "-f %s -o %s --platform windows --profile 130 --type vertex --verbose -i ./", m_assets[_id].sourcePath.getCPtr(), m_assets[_id].outputPath.getCPtr());
		}
		if (m_assets[_id].type == "fragment")
		{
			snprintf(buffer, sizeof(buffer), "-f %s -o %s --platform windows --profile 130 --type fragment --verbose -i ./", m_assets[_id].sourcePath.getCPtr(), m_assets[_id].outputPath.getCPtr());
		}
		if (process.open("shaderc.exe", buffer, bx::ErrorAssert{}))
		{
		}
		else
		{
			bx::debugPrintf("Failed to compile using shaderc");
		}
		process.close();
	}

private:
	U32 m_width;
	U32 m_height;
	U32 m_debug;
	U32 m_reset;
	mrender::MouseState m_mouseState;

	bool m_canCompile;
	U16 m_selectedAssetID;

	std::vector<AssetInfo> m_assets;
	bx::FilePath m_sourcesPath;
	bx::FilePath m_outputPath;
	bx::FilePath m_settingsPath;
};

} // namespace am

namespace ImGui {

bool ErrorText(const std::vector<am::AssetInfo>& _assets)
{
	bool errorDuplicate = false;
	for (size_t i = 0; i < _assets.size(); ++i) {
		for (size_t j = i + 1; j < _assets.size(); ++j) {
			if (strcmp(_assets[i].outputPath.getCPtr(), _assets[j].outputPath.getCPtr()) == 0) {
				errorDuplicate = true;
			}
		}
	}
	if (errorDuplicate)
	{
		::ImGui::TextColored({ 1.0f, 0.0f, 0.0f, 1.0f }, "ERROR: There are assets with duplicate output paths. ");
	}

	return !errorDuplicate;
}

}

ENTRY_IMPLEMENT_MAIN(
	am::AssetManager
	, "Asset Manager"
	, "An example of a asset manager"
);
