#include "stdafx.h"
#include "ResourceManager.h"

#include "Texture.h"
#include "FontTextures.h"
#include "Mesh.h"
#include "AudioManager.h"
#include "AnimationData.h"
#include "VQS.h"
#include "Bone.h"
#include "SkinnedMesh.h"
#include "Model.h"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

#include "stb_image.h"


#define CHECKERROR {GLenum err = glGetError(); if (err != GL_NO_ERROR) { SIK_ERROR("OpenGL error in ResourceManager.cpp:\"{}\": \n", reinterpret_cast<const char*>(glewGetErrorString(err))); SIK_ASSERT(false, "");} }

// For testing the difference between async batch-loading and single-threaded.
// Compile with this set to 1 for asynchronous batch-loading, or 0 for single-threaded
#define RM_TEST_ASYNC 1


static StringID GetFontID(const char* font_name, Uint8 font_pixel_height) {
	String font_str(font_name);
	font_str += std::to_string(font_pixel_height);
	return ToStringID(font_str.c_str());
}

////////////////////////////////////////////////////////////////////////////////
// Types, Aliases, and Static Values
////////////////////////////////////////////////////////////////////////////////

namespace fs = std::filesystem;


////////////////////////////////////////////////////////////////////////////////
// Static Helpers
////////////////////////////////////////////////////////////////////////////////

// buffer should be sized to count_bytes + 1 or greater
static UniquePtr<char[]> BinaryReadFile(fs::path const& filepath) {

	// Make sure filepath is valid and refers to a regular file, not a directory
	if (not fs::exists(filepath) || not fs::is_regular_file(filepath)) {
		SIK_ERROR("Filepath does not exist, or did not refer to a regular file \"{}\"", filepath.string().c_str());
		return nullptr;
	}

	// Compute the file size
	auto err = std::error_code{};
	SizeT count_bytes = fs::file_size(filepath, err);
	if (count_bytes == static_cast<uintmax_t>(-1)) { 
		SIK_ERROR("File size was not able to be computed. Error: \"{}\"", err.message().c_str());
		return nullptr; 
	}

	auto content = std::make_unique<char[]>(count_bytes + 1);

	// Create file stream to read the file
	std::ifstream file_stream;
	file_stream.open(filepath, std::ios_base::binary);

	// Read the entire file into the buffer
	file_stream.seekg(0, std::ios_base::beg);
	file_stream.read(content.get(), count_bytes);
	file_stream.close();

	// Finish with null, just in case
	content[count_bytes] = char(0);

	return content;
}

// checks if file exists
static bool IsValidFile(fs::path const& filepath) {

	// Make sure filepath is valid and refers to a regular file, not a directory
	return fs::exists(filepath) && fs::is_regular_file(filepath);
}

// Compiles a single shader
static GLuint CompileShader(GLint gl_shader_macro, char* shader_str) {
	CHECKERROR
	GLuint shader_id = glCreateShader(gl_shader_macro);
	glShaderSource(shader_id, 1, &shader_str, NULL);
	glCompileShader(shader_id);
	CHECKERROR

	GLint success;
	char info_log[512];

	glGetShaderiv(shader_id, GL_COMPILE_STATUS, &success);
	if (not success)
	{
		glGetShaderInfoLog(shader_id, 512, NULL, info_log);
		SIK_ERROR("Shader {} failed to compile. Error: \"{}\"", shader_id, info_log);
		return 0;
	};
	CHECKERROR

	return shader_id;
}

// Convert JSON file to rapidjson::Document using a provided buffer
static void JSONToDocument(rapidjson::Document& out_doc, 
	std::filesystem::path const& filepath, char* buffer, SizeT size) {

	// Open our file. Need to use C-style here because RapidJSON needs a FILE*.
	FILE* fp = std::fopen(filepath.string().c_str(), "rb");
	if (fp == NULL) {
		SIK_ERROR("File failed to open: {}", filepath.string().c_str());
		return;
	}

	// Use RapidJSON to read and parse the file as a rapidjson::Document
	rapidjson::FileReadStream input_stream{ fp, buffer, size };
	out_doc.ParseStream(input_stream);

	std::fclose(fp);
}

////////////////////////////////////////////////////////////////////////////////
// ResourceManager Methods
////////////////////////////////////////////////////////////////////////////////

ResourceManager::ResourceManager() 
	: thread_pool{}, threads_continue{ true },
	processing_queue{},
	processing_mtx{},
	loading_queue{},
	loading_mtx{},
	mem_pool{ std::pmr::get_default_resource() },
	alloc{ &mem_pool },
	shader_programs{ alloc },
	textures{ alloc },
	meshes{ alloc },
	json_docs{ alloc },
	default_shader{},
	default_material{},
	resources_path{ fs::current_path() / ".." / "Resources" },
	assets_path{ fs::current_path() / ".." / "StandardIssueKrab" / "Engine" / "Assets"}
{ 
	for (SizeT i = 0; i < NUM_WORKER_THREADS; ++i) {
		thread_pool[i] = std::jthread(&ResourceManager::LoadingThreadWork, this);
	}
}

ResourceManager::~ResourceManager() noexcept {
	threads_continue.store(false);

	for (SizeT i = 0; i < NUM_WORKER_THREADS; ++i) {
		thread_pool[i].join();
	}

	for (auto&& [key, sp] : shader_programs) {
		glDeleteProgram(sp.id);
		glDeleteShader(sp.frag);
		glDeleteShader(sp.vert);
	}
	FreeDefaultAssets();
	CHECKERROR
}

void ResourceManager::InitDefaultAssets() {
	InitOwnedDefaults();
	Mesh::InitDefaults();
	Texture::InitDefault();
}

void ResourceManager::FreeDefaultAssets() {
	Texture::FreeDefault();
	Mesh::FreeDefaults();
	FreeOwnedDefaults();
}

Bool ResourceManager::ProcessAsset() {

	ProcessJob current{};
	{
		std::lock_guard lk{ processing_mtx };
		if (not processing_queue.empty()) {
			current = std::move(processing_queue.front());
			processing_queue.pop();
		}
	}

	switch (current.type) {
	break; case ProcessJob::Type::Shader: {
		ProcessShaderDataJob(current);
		return true;
	}

	break; case ProcessJob::Type::Texture: {
		ProcessTextureDataJob(current);
		return true;
	}

	break; case ProcessJob::Type::Mesh: {
		ProcessMeshDataJob(current);
		return true;
	}

	break; case ProcessJob::Type::JSON: {
		ProcessJSONDataJob(current);
		return true;
	}

	break; default: { // Type::NONE
		return false;
	}
	}
}

Uint32 ResourceManager::ProcessAllAssets() {
	
	while (threads_working != 0) {
		auto start = FrameTimer::Now();
		while (FrameTimer::Now() - start < 1ms) {} // wait 1ms
	}

	Uint32 asset_count = 0;
	while (ProcessAsset()) {
		++asset_count;
	}

	return asset_count;
}

#if RM_TEST_ASYNC == 1
void ResourceManager::BatchLoadAssets(fs::path const& manifest_filename) {
	static char buf[1024];
	static const fs::path manifest_path = assets_path / "Manifest";
	static const fs::path audio_path = resources_path / "Sounds";
	static const fs::path tex_path = resources_path / "Textures";
	static const fs::path mesh_path = resources_path / "Models";
	static const fs::path shader_path = assets_path / "Shaders";
	static const fs::path json_path = assets_path / "JSON";

	SIK_TIMER("Asset batch load and process with async.");

	fs::path const filepath = manifest_path / manifest_filename;
	if (not fs::exists(filepath) || not fs::is_regular_file(filepath)) {
		SIK_ERROR("Filepath does not exist, or did not refer to a regular file. File was: \"{}\"", filepath.string().c_str());
		SIK_ASSERT(false, "Invalid file");
		return;
	}
	if (filepath.extension() != ".json") {
		SIK_ERROR("File must be a JSON file. File was: \"{}\"", filepath.string().c_str());
		SIK_ASSERT(false, "Invalid file");
		return;
	}

	rapidjson::Document manifest{};
	JSONToDocument(manifest, filepath, buf, sizeof(buf));

	Uint32 asset_count = 0;

	auto it = manifest.FindMember("Textures");
	if (it != manifest.MemberEnd()) {
		SIK_ASSERT(it->value.IsArray(), "Textures must be listed in a array of filenames with extensions.");
		for (auto&& filename : it->value.GetArray()) {
			AsyncLoadTexture(filename.GetString());
			++asset_count;
		}
	}

	it = manifest.FindMember("Models");
	if (it != manifest.MemberEnd()) {
		SIK_ASSERT(it->value.IsArray(), "Models must be listed in a array of filenames with extensions.");
		for (auto&& filename : it->value.GetArray()) {
			AsyncLoadMesh(filename.GetString());
			++asset_count;
		}
	}

	it = manifest.FindMember("ShaderPrograms");
	if (it != manifest.MemberEnd()) {
		SIK_ASSERT(it->value.IsArray(), "ShaderPrograms must be listed in a array of filenames WITHOUT extensions.");
		for (auto&& filename : it->value.GetArray()) {
			AsyncLoadShaderProgram(filename.GetString());
			++asset_count;
		}
	}

	it = manifest.FindMember("JSON");
	if (it != manifest.MemberEnd()) {
		SIK_ASSERT(it->value.IsArray(), "JSONs must be listed in a array of filenames with extensions.");
		for (auto&& filename : it->value.GetArray()) {
			AsyncLoadJSON(filename.GetString());
			++asset_count;
		}
	}

	// Load audio files single-threaded
	it = manifest.FindMember("Audio");
	if (it != manifest.MemberEnd()) {
		SIK_ASSERT(it->value.IsObject(), "Audio files must be listed as a JSON object.");

		for (auto&& [name, val] : it->value.GetObj()) {
			fs::path const audio_file = audio_path / name.GetString();
			Bool const     is_stream = val["is_stream"].GetBool();
			StringID const tag = ToStringID(val["tag"].GetString());

			p_audio_manager->LoadSound(audio_file.string().c_str(), is_stream, tag);
		}
	}

	SIK_INFO("{} assets (not including audio files) loading asynchronously from manifest: \"{}\".", asset_count, manifest_filename.string().c_str());
}

#else

void ResourceManager::BatchLoadAssets(fs::path const& manifest_filename) {
	static char buf[1024];
	static const fs::path manifest_path = assets_path / "Manifest";
	static const fs::path audio_path = resources_path / "Sounds";
	static const fs::path tex_path = resources_path / "Textures";
	static const fs::path mesh_path = resources_path / "Models";
	static const fs::path shader_path = assets_path / "Shaders";
	static const fs::path json_path = assets_path / "JSON";
	

	SIK_TIMER("Asset batch load without async.");

	Uint32 asset_count = 0;

	fs::path const filepath = manifest_path / manifest_filename;
	if (not fs::exists(filepath) || not fs::is_regular_file(filepath)) {
		SIK_ERROR("Filepath does not exist, or did not refer to a regular file. File was: \"{}\"", filepath.string().c_str());
		SIK_ASSERT(false, "Invalid file");
		return;
	}
	if (filepath.extension() != ".json") {
		SIK_ERROR("File must be a JSON file. File was: \"{}\"", filepath.string().c_str());
		SIK_ASSERT(false, "Invalid file");
		return;
	}

	rapidjson::Document manifest{};
	JSONToDocument(manifest, filepath, buf, sizeof(buf));

	auto it = manifest.FindMember("Textures");
	if (it != manifest.MemberEnd()) {
		SIK_ASSERT(it->value.IsArray(), "Textures must be listed in a array of filenames with extensions.");
		for (auto&& filename : it->value.GetArray()) {
			LoadTexture(filename.GetString());
			asset_count++;
		}
	}

	it = manifest.FindMember("Models");
	if (it != manifest.MemberEnd()) {
		SIK_ASSERT(it->value.IsArray(), "Models must be listed in a array of filenames with extensions.");
		for (auto&& filename : it->value.GetArray()) {
			LoadMesh(filename.GetString());
			asset_count++;
		}
	}

	it = manifest.FindMember("ShaderPrograms");
	if (it != manifest.MemberEnd()) {
		SIK_ASSERT(it->value.IsArray(), "ShaderPrograms must be listed in a array of filenames WITHOUT extensions.");
		for (auto&& filename : it->value.GetArray()) {
			LoadShaderProgram(filename.GetString());
			asset_count++;
		}
	}
	
	it = manifest.FindMember("JSON");
	if (it != manifest.MemberEnd()) {
		SIK_ASSERT(it->value.IsArray(), "JSONs must be listed in a array of filenames with extensions.");
		for (auto&& filename : it->value.GetArray()) {
			LoadJSON(filename.GetString());
			asset_count++;
		}
	}

	// Load audio files single-threaded
	it = manifest.FindMember("Audio");
	if (it != manifest.MemberEnd()) {
		SIK_ASSERT(it->value.IsObject(), "Audio files must be listed as a JSON object.");
		
		for (auto&& [name, val] : it->value.GetObj()) {
			fs::path const audio_file = audio_path / name.GetString();
			Bool const     is_stream  = val["is_stream"].GetBool();
			StringID const tag        = ToStringID(val["tag"].GetString());

			p_audio_manager->LoadSound(audio_file.string().c_str(), is_stream, tag);
			asset_count++;
		}
	}

	SIK_INFO("{} assets batch loaded from file \"{}\".", asset_count, manifest_filename.string().c_str());
}

#endif


// -----------------------------------------------------------------------------
// Shader Programs
// -----------------------------------------------------------------------------

GLuint* ResourceManager::LoadShaderProgram(const char* shader_name) {
	static const fs::path shader_path = assets_path / "Shaders";

	if (GLuint* sp = GetShaderProgram(shader_name); sp != nullptr) {
		return sp;
	}

	// Absolute paths of shader files. These are built off of the name of the
	// shader program so each shader program can only be associated with one
	// vertex shader and one fragment shader.
	const fs::path vert_path = (shader_path / shader_name).replace_extension(".vert");
	const fs::path frag_path = (shader_path / shader_name).replace_extension(".frag");

	// Read and compile shaders
	UniquePtr<char[]> vert_str = BinaryReadFile(vert_path);
	UniquePtr<char[]> frag_str = BinaryReadFile(frag_path);
	if (vert_str == nullptr || frag_str == nullptr) { 
		SIK_ERROR("Shaders failed to load. Returning default shader program.");
		return &default_shader.id; 
	}

	GLuint vertex_shader   = CompileShader(GL_VERTEX_SHADER, vert_str.get());
	GLuint fragment_shader = CompileShader(GL_FRAGMENT_SHADER, frag_str.get());

	// Create the shader program
	GLuint shader_program_id = glCreateProgram();
	glAttachShader(shader_program_id, vertex_shader);
	glAttachShader(shader_program_id, fragment_shader);
	glLinkProgram(shader_program_id);

	//Check if program was linked correctly
	GLint programLinked = GL_FALSE;
	glGetProgramiv(shader_program_id, GL_LINK_STATUS, &programLinked);
	if (programLinked != GL_TRUE) {
		SIK_ERROR("ShaderProgram linking failed. ID: {}. Name: \"{}\"", shader_program_id, shader_name);
		SIK_ASSERT(false, "ShaderProgram linking failed.");
	}
	
	// OpenGL error check
	CHECKERROR
	
	auto && [it, b] = shader_programs.insert({ ToStringID(shader_name), ShaderProgram{.id = shader_program_id, .vert = vertex_shader, .frag = fragment_shader } });
	return &(it->second.id);
}

GLuint* ResourceManager::LoadComputeShader(const char* shader_name) {
	static const fs::path shader_path = assets_path / "Shaders";

	if (GLuint* sp = GetShaderProgram(shader_name); sp != nullptr) {
		return sp;
	}

	// Absolute paths of shader files. These are built off of the name of the
	// shader program so each shader program can only be associated with one
	// vertex shader and one fragment shader.
	const fs::path path = (shader_path / shader_name).replace_extension(".comp");

	// Read and compile shader
	UniquePtr<char[]> comp_str = BinaryReadFile(path);
	SIK_ASSERT(comp_str != nullptr, 
		"Compute Shader failed to load.");

	GLuint compute_shader = CompileShader(GL_COMPUTE_SHADER, comp_str.get());

	// Create the shader program
	GLuint shader_program_id = glCreateProgram();
	glAttachShader(shader_program_id, compute_shader);
	glLinkProgram(shader_program_id);

	//Check if program was linked correctly
	GLint programLinked = GL_FALSE;
	glGetProgramiv(shader_program_id, GL_LINK_STATUS, &programLinked);
	if (programLinked != GL_TRUE) {
		SIK_ERROR("ShaderProgram linking failed. ID: {}. Name: \"{}\"", shader_program_id, shader_name);
		SIK_ASSERT(false, "ShaderProgram linking failed.");
	}

	// OpenGL error check
	CHECKERROR

		auto&& [it, b] = shader_programs.insert({ ToStringID(shader_name), ShaderProgram{.id = shader_program_id, .comp = compute_shader} });
	return &(it->second.id);
}

void ResourceManager::UnloadShaderProgram(StringID prg_name_id) {
	auto it = shader_programs.find(prg_name_id);
	if (it == shader_programs.end()) { 
		SIK_WARN("Shader could not be found, unload operation was no op. StringID: {}", static_cast<Uint64>(prg_name_id));
		return; 
	}

	glDeleteProgram(it->second.id);
	if (it->second.comp == 0) {
		glDeleteShader(it->second.frag);
		glDeleteShader(it->second.vert);
	}
	else {
		glDeleteShader(it->second.comp);
	}
	
	shader_programs.erase(it);
}

/*
* Loads shader program ID associated with given name
* Param: const char* name, file path to shader
* Returns GLuint: non-zero if loaded successfully, 0 if not found
*/
GLuint* ResourceManager::LoadShaderProgram(const char* shader_name, bool use_geo) {
	static const fs::path shader_path = assets_path / "Shaders";

	if (GLuint* sp = GetShaderProgram(shader_name); sp != nullptr) {
		return sp;
	}

	// Absolute paths of shader files. These are built off of the name of the
	// shader program so each shader program can only be associated with one
	// vertex shader and one fragment shader.
	const fs::path vert_path = (shader_path / shader_name).replace_extension(".vert");
	const fs::path frag_path = (shader_path / shader_name).replace_extension(".frag");
	const fs::path geom_path = (shader_path / shader_name).replace_extension(".geom"); 
	


	// Read and compile shaders
	UniquePtr<char[]> vert_str = BinaryReadFile(vert_path);
	UniquePtr<char[]> frag_str = BinaryReadFile(frag_path);
	if (vert_str == nullptr || frag_str == nullptr) {
		SIK_ERROR("Shaders failed to load. Returning default shader program.");
		return &default_shader.id;
	}
	UniquePtr<char[]> geom_str = nullptr; 
	if (use_geo)
	{
		geom_str = BinaryReadFile(geom_path);
	}

	GLuint vertex_shader = CompileShader(GL_VERTEX_SHADER, vert_str.get());
	GLuint fragment_shader = CompileShader(GL_FRAGMENT_SHADER, frag_str.get());
	GLuint geometry_shader = 0;
	if (use_geo)
	{
		geometry_shader = CompileShader(GL_GEOMETRY_SHADER, geom_str.get());
	}

	// Create the shader program
	GLuint shader_program_id = glCreateProgram();
	glAttachShader(shader_program_id, vertex_shader);
	glAttachShader(shader_program_id, fragment_shader);
	if (use_geo)
	{
		glAttachShader(shader_program_id, geometry_shader);
	}
	glLinkProgram(shader_program_id);

	//Check if program was linked correctly
	GLint programLinked = GL_FALSE;
	glGetProgramiv(shader_program_id, GL_LINK_STATUS, &programLinked);
	if (programLinked != GL_TRUE) {
		SIK_ERROR("ShaderProgram linking failed. ID: {}. Name: \"{}\"", shader_program_id, shader_name);
		SIK_ASSERT(false, "ShaderProgram linking failed.");
	}

	// OpenGL error check
	CHECKERROR

	auto&& [it, b] = shader_programs.insert({ ToStringID(shader_name), ShaderProgram{.id = shader_program_id, .vert = vertex_shader, .frag = fragment_shader } });
	return &(it->second.id);
}

/*
* Gets shader program ID associated with given name
* Param: const char* name, file path to shader
* Returns GLuint: non-zero if found, 0 if not found
*/
GLuint* ResourceManager::GetShaderProgram(StringID prg_name_id) {
	auto it = shader_programs.find(prg_name_id);
	if (it != shader_programs.end()) {
		return &(it->second.id);
	}
	return nullptr;
}

GLuint* ResourceManager::GetShaderProgram(const char* prg_name) {
	return GetShaderProgram(ToStringID(prg_name));
}

GLuint* ResourceManager::AsyncLoadShaderProgram(const char* prg_name) {
	static const fs::path shader_path = assets_path / "Shaders";
	
	StringID sid = ToStringID(prg_name);
	{
		auto it = shader_programs.find(sid);
		if (it != shader_programs.end()) {
			return &(it->second.id);
		}
	}

	// Return the address of where the new ShaderProgram
	// will be located, once it is constructed on another thread.
	auto&& [it, b] = shader_programs.emplace(sid, default_shader);
	GLuint* ret = &(it->second.id);

	// Post the load job
	std::lock_guard lk{ loading_mtx };
	loading_queue.push(LoadJob{
		.type = LoadJob::Type::Shader,
		.key = sid,
		.path = (shader_path / prg_name)});

	return ret;
}

// -----------------------------------------------------------------------------
// Models
// -----------------------------------------------------------------------------

Model* ResourceManager::LoadModel(const char* model_name) {
	static const fs::path models_path = resources_path / "Models";

	Model* model = GetModel(model_name);
	if (model != nullptr) {
		return model;
	}

	const fs::path path = models_path / model_name;

	auto [it, b] = models.insert({ ToStringID(model_name), Model{ model_name } });

	return &(it->second);
}

void ResourceManager::UnloadModel(StringID sid) {
	models.erase(sid);
}

Model* ResourceManager::GetModel(StringID sid) {
	auto it = models.find(sid);
	if (it != models.end()) {
		return &(it->second);
	}
	return nullptr;
}

Model* ResourceManager::GetModel(const char* name) {
	return GetModel(ToStringID(name));
}

// -----------------------------------------------------------------------------
// Textures
// -----------------------------------------------------------------------------


SDL_Surface* ResourceManager::LoadImageAsSurface(const char* filename)
{
	static const fs::path textures_path = resources_path / "Textures" / filename;

	// Read data
	Int32 width, height, bytesPerPixel;
	UniquePtr<unsigned char[]> data{ stbi_load(textures_path.string().c_str(), &width, &height, &bytesPerPixel, 0)};
	if (!data) {
		SIK_ERROR("Texture filepath: {}. Error: {}", textures_path.string().c_str(), stbi_failure_reason());
	}

	// Calculate pitch
	int pitch;
	pitch = width * bytesPerPixel;
	pitch = (pitch + 3) & ~3;

	// Setup relevance bitmask
	Int32 Rmask, Gmask, Bmask, Amask;
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
	Rmask = 0x000000FF;
	Gmask = 0x0000FF00;
	Bmask = 0x00FF0000;
	Amask = (bytesPerPixel == 4) ? 0xFF000000 : 0;
#else
	int s = (bytesPerPixel == 4) ? 0 : 8;
	Rmask = 0xFF000000 >> s;
	Gmask = 0x00FF0000 >> s;
	Bmask = 0x0000FF00 >> s;
	Amask = 0x000000FF >> s;
#endif
	SDL_Surface* surface = SDL_CreateRGBSurfaceFrom(data.get(), width, height, bytesPerPixel * 8, pitch, Rmask, Gmask, Bmask, Amask);
	if (!surface) {
		return nullptr;
	}
	return surface;
}


Texture* ResourceManager::LoadTexture(const char* texture_name) {
	static const fs::path textures_path = resources_path / "Textures";

	Texture* tex = GetTexture(texture_name);
	if (tex != nullptr) {
		return tex;
	}

	const fs::path path = textures_path / texture_name;

	Int32 width, height, channel_count;
	UniquePtr<unsigned char[]> content{ stbi_load(path.string().c_str(), &width, &height, &channel_count, 0) };
	if (content == nullptr) {
		SIK_ERROR("Texture failed to load using stb_image. Reason: \"{}\"", stbi_failure_reason());
		return nullptr;
	}

	auto [it, b] = textures.insert({ ToStringID(texture_name), Texture{ width, height, channel_count, content.get() }});
	it->second.GenerateDefaultMipmap();

	return &(it->second);
}

Texture* ResourceManager::AsyncLoadTexture(const char* texture_name) {
	static const fs::path texture_path = resources_path / "Textures";

	StringID sid = ToStringID(texture_name);
	{
		auto it = textures.find(sid);
		if (it != textures.end()) {
			return &(it->second);
		}
	}

	// Return address of where the new Mesh
	// will be located, once it is constructed on another thread.
	auto&& [it, b] = textures.emplace(sid, Texture{});
	Texture* ret = &(it->second);
	
	// Post the load job
	std::lock_guard lk{ loading_mtx };
	loading_queue.push(LoadJob{
		.type = LoadJob::Type::Texture,
		.key = sid,
		.path = (texture_path / texture_name) });

	return ret;
}

void ResourceManager::UnloadTexture(StringID sid) {
	textures.erase(sid);
}

Texture* ResourceManager::GetTexture(StringID sid) {
	auto it = textures.find(sid);
	if (it != textures.end()) {
		return &(it->second);
	}
	return nullptr;
}

/*
* Loads a Font Texture. Saves it in the Font texture map
* Returns: FontTextures* pointer to the created FontTextures object
*/
FontTextures* ResourceManager::LoadFontTexture(const char* ttf_name, Uint8 font_pixel_height) {
	FontTextures* return_font = GetFontTexture(ttf_name, font_pixel_height);
	if (return_font != nullptr)
		return return_font;

	fs::path font_path = resources_path / "Fonts" / ttf_name;

	auto [it, b] = m_font_map.insert(
		{ GetFontID(ttf_name, font_pixel_height), 
		  FontTextures(font_path.string().c_str(), font_pixel_height, alloc)}
	);

	return  &(it->second);;
}

/*
* Get a FontTextures from the FontTextures map
* Returns: FontTextures* pointer to the FontTextures object. nullptr if it doesn't exit
*/
FontTextures* ResourceManager::GetFontTexture(const char* ttf_name, Uint8 font_pixel_height) {
	auto font_iter = m_font_map.find(GetFontID(ttf_name, font_pixel_height));
	if (font_iter != m_font_map.end())
		return &font_iter->second;
	return nullptr;
}

Texture* ResourceManager::GetTexture(const char* name) {
	return GetTexture(ToStringID(name));
}

// -----------------------------------------------------------------------------
// Meshes
// -----------------------------------------------------------------------------

Mesh* ResourceManager::LoadMesh(const char* mesh_name) {
	static const fs::path mesh_path = resources_path / "Models";
	
	{
		Mesh* mesh = GetMesh(mesh_name);
		if (mesh != nullptr) { return mesh; }
	}

	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(
							(mesh_path / mesh_name).string().c_str(),	// absolute filepath
							aiProcess_Triangulate | aiProcess_FlipUVs); // flags

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
		SIK_ERROR("ASSIMP ERROR {}", importer.GetErrorString());
		return nullptr;
	}

	if (scene->mNumMeshes < 1) {
		SIK_ERROR("No meshes found in file \"{}\"", (mesh_path / mesh_name).string().c_str());
		return nullptr;
	}

	aiMesh* mesh = scene->mMeshes[0];

	// Approximate size for data
	Vector<Float32> vert_data(mesh->mNumVertices * 8);
	Vector<Uint32> idx_data(mesh->mNumFaces * mesh->mFaces[0].mNumIndices);
	Vector<Float32> tan_data;
	// Vertices
	for (Uint32 j = 0; j < mesh->mNumVertices; j++) {

		// Positions
		vert_data.push_back(mesh->mVertices[j].x);
		vert_data.push_back(mesh->mVertices[j].y);
		vert_data.push_back(mesh->mVertices[j].z);

		// Normals
		vert_data.push_back(mesh->mNormals[j].x);
		vert_data.push_back(mesh->mNormals[j].y);
		vert_data.push_back(mesh->mNormals[j].z);

		// UVs
		if (mesh->mTextureCoords[0]) {
			vert_data.push_back(mesh->mTextureCoords[0][j].x);
			vert_data.push_back(mesh->mTextureCoords[0][j].y);
		}
		else {
			vert_data.push_back(0);
			vert_data.push_back(0);
		}

		if (mesh->mTangents != nullptr) {
			tan_data.push_back(mesh->mTangents[j].x);
			tan_data.push_back(mesh->mTangents[j].y);
			tan_data.push_back(mesh->mTangents[j].z);
		}
	}

	// Indices
	for (Uint32 j = 0; j < mesh->mNumFaces; j++) {
			
		aiFace face = mesh->mFaces[j];
		for (Uint32 k = 0; k < face.mNumIndices; k++) {
			idx_data.push_back(face.mIndices[k]);
		}
	}
	// TODO : Load material index per mesh


	// TODO : load multiple meshes from a single file. for now, just return the first mesh from the file.
	auto [it, b] = meshes.insert({ ToStringID(mesh_name), Mesh{vert_data.data(), vert_data.size(), idx_data.data(), idx_data.size(), tan_data.data(), tan_data.size()}});
	return &(it->second);

}

Mesh* ResourceManager::AsyncLoadMesh(const char* mesh_name) {
	static const fs::path texture_path = resources_path / "Models";

	StringID sid = ToStringID(mesh_name);
	{
		auto it = meshes.find(sid);
		if (it != meshes.end()) {
			return &(it->second);
		}
	}

	// Return the address of where the new Mesh
	// will be located, once it is constructed on another thread.
	auto&& [it, b] = meshes.emplace(sid, Mesh{});
	Mesh* ret = &(it->second);

	// Post the load job
	std::lock_guard lk{ loading_mtx };
	loading_queue.push(LoadJob{
		.type = LoadJob::Type::Mesh,
		.key = sid,
		.path = (texture_path / mesh_name) });

	return ret;
}

void ResourceManager::UnloadMesh(StringID sid) {
	meshes.erase(sid);
}

Mesh* ResourceManager::GetMesh(StringID sid) {
	switch (sid) {
		break; case "Plane"_sid: { 
			return Mesh::PlanePtr(); 
		}

		break; case "Cube"_sid: { 
			return Mesh::CubePtr();
		}

		break; case "Sphere"_sid: { 
			return Mesh::SpherePtr();
		}

		break; case "Cone"_sid: { 
			return Mesh::ConePtr();
		}

		break; case "Cylinder"_sid: {
			return Mesh::CylinderPtr();
		}

		break; default: {
			auto it = meshes.find(sid);
			if (it != meshes.end()) {
				return &(it->second);
			}
		}
	}

	return nullptr;
}

Mesh* ResourceManager::GetMesh(const char* name) {
	return GetMesh(ToStringID(name));
}


// -----------------------------------------------------------------------------
// JSON Documents
// -----------------------------------------------------------------------------

JSON* ResourceManager::LoadJSON(const char* json_name) {
	static const fs::path json_path = assets_path / "JSON";
	static char buf[1024];

	JSON* json = GetJSON(json_name);
	if (json != nullptr) {
		return json;
	}
	
	// Insert a new JSON object into the map
	auto [it, b] = json_docs.insert({ ToStringID(json_name), JSON{ rapidjson::Document{}, json_path / json_name } });

	// Parse the JSON file into the document we just created
	JSONToDocument(it->second.doc, it->second.path, buf, 1024);
	SIK_ASSERT(it->second.doc.IsObject(), "rapidjson document not an object.");

	return &(it->second);
}

JSON* ResourceManager::AsyncLoadJSON(const char* json_name) {
	static const fs::path texture_path = assets_path / "JSON";

	StringID sid = ToStringID(json_name);
	{
		auto it = json_docs.find(sid);
		if (it != json_docs.end()) {
			return &(it->second);
		}
	}

	// Return the address of where the new JSON
	// will be located, once it is constructed on another thread.
	auto&& [it, b] = json_docs.emplace(sid, JSON{});
	JSON* ret = &(it->second);

	std::lock_guard lk{ loading_mtx };
	loading_queue.push(LoadJob{
		.type = LoadJob::Type::JSON,
		.key = sid,
		.path = (texture_path / json_name) });

	return ret;
}

void ResourceManager::UnloadJSON(StringID sid) {
	json_docs.erase(sid);
}

JSON* ResourceManager::GetJSON(StringID sid) {
	auto it = json_docs.find(sid);
	if (it != json_docs.end()) {
		return &(it->second);
	}
	return nullptr;
}

JSON* ResourceManager::GetJSON(const char* name) {
	return GetJSON(ToStringID(name));
}

void ResourceManager::ReloadAllJSON() {
	static char buf[1024];

	for (auto&& [sid, json] : json_docs) {
		json.doc.RemoveAllMembers();
		JSONToDocument(json.doc, json.path, buf, 1024);
	}
}

void ResourceManager::ReloadJSON(const char* filename) {
	static char buf[1024];

	for (auto&& [sid, json] : json_docs) {
		if (std::strcmp(json.path.string().c_str(), filename) != 0) continue;
		json.doc.RemoveAllMembers();
		JSONToDocument(json.doc, json.path, buf, 1024);
	}
}

// TODO : move this elsewhere
// TODO : this can take a StringID
Bool ResourceManager::WriteJSONToDisk(const char* json_name) {

	static char buf[1024];

	JSON* json = GetJSON(json_name);
	if (json == nullptr) {
		SIK_ERROR("JSON file not found: \"{}\"", json_name);
		return false;
	}

	FILE* fp = std::fopen(json->path.string().c_str(), "w");
	if (fp == nullptr) {
		SIK_ERROR("File failed to open: \"{}\"", json->path.string().c_str());
		return false;
	}

	rapidjson::FileWriteStream stream{ fp, buf, 1024 };
	rapidjson::PrettyWriter<rapidjson::FileWriteStream> writer{ stream };

	Bool result = json->doc.Accept(writer);

	std::fclose(fp);
	return result;
}



//------------------------------------------------------------------------------
// Materials
//------------------------------------------------------------------------------

Material* ResourceManager::LoadMaterial(const char* material_name) {
 
	static const fs::path material_path = assets_path / "Materials";

	Material* material = GetMaterial(material_name);
	if (material != nullptr) { 
		return material; 
	}

	const fs::path path = material_path / material_name;

	JSON* mat_json = LoadJSON(path.string().c_str());
	if (mat_json == nullptr)
	{
		return &default_material;
	}

	Material mat;

	rapidjson::Document& doc = mat_json->doc;
	rapidjson::Value::ConstMemberIterator doc_itr;

	doc_itr = doc.FindMember("diffuse");
	if (doc_itr != doc.MemberEnd()) {
		auto const& source = doc_itr->value.GetArray();
		mat.diffuse.r = source[0].GetFloat();
		mat.diffuse.g = source[1].GetFloat();
		mat.diffuse.b = source[2].GetFloat();
	}

	doc_itr = doc.FindMember("specular");
	if (doc_itr != doc.MemberEnd()) {
		auto const& source = doc_itr->value.GetArray();
		mat.specular.r = source[0].GetFloat();
		mat.specular.g = source[1].GetFloat();
		mat.specular.b = source[2].GetFloat();
	}

	doc_itr = doc.FindMember("emission");
	if (doc_itr != doc.MemberEnd()) {
		auto const& source = doc_itr->value.GetArray();
		mat.emission.r = source[0].GetFloat();
		mat.emission.g = source[1].GetFloat();
		mat.emission.b = source[2].GetFloat();
	}

	doc_itr = doc.FindMember("glossiness");
	if (doc_itr != doc.MemberEnd()) {
		mat.glossiness = doc_itr->value.GetFloat();
	}

	doc_itr = doc.FindMember("base_color_texture");
	if (doc_itr != doc.MemberEnd()) {
		String name = doc_itr->value.GetString();
		if (!name.empty()) {
			mat.base_color = AsyncLoadTexture(name.c_str());
		}
	}

	doc_itr = doc.FindMember("normal_map");
	if (doc_itr != doc.MemberEnd()) {
		String name = doc_itr->value.GetString();
		if (!name.empty()) {
			mat.normal_map = AsyncLoadTexture(name.c_str());
		}
	}

	doc_itr = doc.FindMember("shader");
	if (doc_itr != doc.MemberEnd()) {
		mat.shader = AsyncLoadShaderProgram(doc_itr->value.GetString());
	}

	auto [it, b] = m_material_map.insert({ ToStringID(material_name), mat });

	return &(it->second);

}

Material* ResourceManager::GetMaterial(StringID sid) {
	
	auto it = m_material_map.find(sid);
	if (it != m_material_map.end()) {
		return &(it->second);
	}
	
	return nullptr;
}

Material* ResourceManager::GetMaterial(const char* name) {
	return GetMaterial(ToStringID(name));
}

//------------------------------------------------------------------------------
// Async Helpers
//------------------------------------------------------------------------------

void ResourceManager::LoadingThreadWork() {

	while (threads_continue) {
		
		LoadJob current{};
		{
			std::lock_guard lk{ loading_mtx };
			if (not loading_queue.empty()) { 
				current = loading_queue.front();
				loading_queue.pop();
			}
		}

		switch (current.type) {

		break; case LoadJob::Type::Shader: {
			threads_working++;
			LoadShaderDataJob(current.path, current.key);
			threads_working--;
		}

		break; case LoadJob::Type::Texture: {
			threads_working++;
			LoadTextureDataJob(current.path, current.key);
			threads_working--;
		}

		break; case LoadJob::Type::Mesh: {
			threads_working++;
			LoadMeshDataJob(current.path, current.key);
			threads_working--;
		}

		break; case LoadJob::Type::JSON: {
			threads_working++;
			LoadJSONDataJob(current.path, current.key);
			threads_working--;
		}

		break; default: { // Type::NONE
			std::this_thread::sleep_for(8ms);
		}
		}
	}
}

void ResourceManager::LoadShaderDataJob(fs::path path, StringID key) {
	
	// Get the correct files based on the shader program path (provided
	// with no extension)
	const fs::path vert_path = path.replace_extension(".vert");
	const fs::path frag_path = path.replace_extension(".frag");
	const fs::path geom_path = path.replace_extension(".geom");

	// Read the two shader files
	auto vert = BinaryReadFile(vert_path);
	if (vert == nullptr) {
		SIK_ERROR("Vertex shader read failed for \"{}\"", vert_path.string().c_str());
		return;
	}
	auto frag = BinaryReadFile(frag_path);
	if (frag == nullptr) { 
		SIK_ERROR("Fragment shader read failed for \"{}\"", frag_path.string().c_str());
		return; 
	}
	UniquePtr<char[]> geom = nullptr;
	if (IsValidFile(geom_path))
	{
		geom = BinaryReadFile(geom_path);
	}

	// Package up the data and post a ProcessJob
	std::lock_guard lk{ processing_mtx };
	processing_queue.push(ProcessJob{
		.type = ProcessJob::Type::Shader,
		.key  = key,
		.data = ProcessJob::ShaderData{ 
			.vert_str = std::move(vert), 
			.frag_str = std::move(frag),
			.geom_str = std::move(geom)
		}
	});
}

void ResourceManager::LoadTextureDataJob(fs::path path, StringID key) {
	
	Int32 width = 0, height = 0, channel_count = 0;

	UniquePtr<unsigned char[]> content{ stbi_load(path.string().c_str(), &width, &height, &channel_count, 0) };
	if (content == nullptr) { 
		SIK_ERROR("Texture failed to load using stb_image. Reason: \"{}\". File: {}", stbi_failure_reason(), path.string().c_str());
		return; 
	}
	
	// Package up the data and post a ProcessJob
	std::lock_guard lk{ processing_mtx };
	processing_queue.push(ProcessJob{
		.type = ProcessJob::Type::Texture,
		.key  = key,
		.data = ProcessJob::TextureData{
			.image         = std::move(content),
			.width         = width,
			.height        = height,
			.channel_count = channel_count }});
}

void ResourceManager::LoadMeshDataJob(fs::path path, StringID key) {
	
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(path.string().c_str(),	aiProcess_Triangulate | aiProcess_FlipUVs);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode || scene->mNumMeshes < 1) {
		SIK_ERROR("Mesh load failed for \"{}\". Assimp error: \"{}\".", path.string().c_str(), importer.GetErrorString());
		return;
	}

	// Only allowed one mesh per file
	aiMesh* mesh = scene->mMeshes[0];
	Vector<Float32> vert_data;
	Vector<Uint32> idx_data;
	Vector<Float32> tangent_data;

	// Reserve approximately the amount of space we need
	vert_data.reserve(mesh->mNumVertices * 8);
	idx_data.reserve(mesh->mNumFaces * mesh->mFaces[0].mNumIndices);

	// Vertices
	for (Uint32 j = 0; j < mesh->mNumVertices; j++) {

		// Positions
		vert_data.push_back(mesh->mVertices[j].x);
		vert_data.push_back(mesh->mVertices[j].y);
		vert_data.push_back(mesh->mVertices[j].z);

		// Normals
		vert_data.push_back(mesh->mNormals[j].x);
		vert_data.push_back(mesh->mNormals[j].y);
		vert_data.push_back(mesh->mNormals[j].z);

		// UVs
		if (mesh->mTextureCoords[0]) {
			vert_data.push_back(mesh->mTextureCoords[0][j].x);
			vert_data.push_back(mesh->mTextureCoords[0][j].y);
		}
		else {
			vert_data.push_back(0);
			vert_data.push_back(0);
		}

		if (mesh->mTangents != nullptr) {
			tangent_data.push_back(mesh->mTangents[j].x);
			tangent_data.push_back(mesh->mTangents[j].y);
			tangent_data.push_back(mesh->mTangents[j].z);
		}
	}

	// Indices
	for (Uint32 j = 0; j < mesh->mNumFaces; j++) {

		aiFace face = mesh->mFaces[j];
		for (Uint32 k = 0; k < face.mNumIndices; k++) {
			idx_data.push_back(face.mIndices[k]);
		}
	}
	// TODO : Load material index per mesh


	// Package up the data and post a ProcessJob
	std::lock_guard lk{ processing_mtx };
	processing_queue.push(ProcessJob{
		.type = ProcessJob::Type::Mesh,
		.key  = key,
		.data = ProcessJob::MeshData{
			.verts = std::move(vert_data),
			.idxs  = std::move(idx_data),
			.tangents = std::move(tangent_data)}});
}

void ResourceManager::LoadJSONDataJob(fs::path path, StringID key) {

	thread_local char parsing_buffer[1024];

	// Parse the file as a rapidjson::Document
	rapidjson::Document doc{};
	JSONToDocument(doc, path, parsing_buffer, 1024);

	// Package up the data and post a ProcessJob
	std::lock_guard lk{ processing_mtx };
	processing_queue.push(ProcessJob{
		.type = ProcessJob::Type::JSON,
		.key  = key,
		.data = JSON{
			.doc  = std::move(doc),
			.path = std::move(path)}});
}

void ResourceManager::ProcessShaderDataJob(ProcessJob& job) {
	
	auto&& [vert_data, frag_data, geom_data] = std::get<ProcessJob::ShaderData>(job.data);

	GLuint vertex_shader   = CompileShader(GL_VERTEX_SHADER, vert_data.get());
	GLuint fragment_shader = CompileShader(GL_FRAGMENT_SHADER, frag_data.get());
	
	bool has_geo = (geom_data != nullptr);
	GLuint geometry_shader = 0;
	if (has_geo)
	{
		geometry_shader = CompileShader(GL_GEOMETRY_SHADER, geom_data.get());
	}

	// Create the shader program
	GLuint shader_program_id = glCreateProgram();
	glAttachShader(shader_program_id, vertex_shader);
	if (has_geo)
	{
		glAttachShader(shader_program_id, geometry_shader);
	}
	glAttachShader(shader_program_id, fragment_shader);
	glLinkProgram(shader_program_id);

	//Check if program was linked correctly
	GLint programLinked = GL_FALSE;
	glGetProgramiv(shader_program_id, GL_LINK_STATUS, &programLinked);
	if (programLinked != GL_TRUE) {
		SIK_ERROR("Program linking failed. ID: {}", shader_program_id);
		SIK_ASSERT(false, "Program linking failed.");
	}

	shader_programs.insert_or_assign(
			job.key,
			ShaderProgram{
				.id = shader_program_id,
				.vert = vertex_shader,
				.frag = fragment_shader });
}

void ResourceManager::ProcessTextureDataJob(ProcessJob& job) {
	
	auto&& [image, w, h, chs] = std::get<ProcessJob::TextureData>(job.data);
	
	auto [it, b] = textures.insert_or_assign(
		job.key, 
		Texture{ w, h, chs, image.get() });

	it->second.GenerateDefaultMipmap();
}

void ResourceManager::ProcessMeshDataJob(ProcessJob& job) {
	
	auto&& [verts, idxs, tangents] = std::get<ProcessJob::MeshData>(job.data);

	meshes.insert_or_assign(
			job.key,
			Mesh{verts.data(), verts.size(), 
				 idxs.data(), idxs.size(),
				 tangents.data(), tangents.size()});
}

void ResourceManager::ProcessJSONDataJob(ProcessJob& job) {

	json_docs.insert_or_assign(
			job.key,
			JSON{ std::move(std::get<JSON>(job.data)) });
}


// -----------------------------------------------------------------------------
// Other helpers
// -----------------------------------------------------------------------------

void ResourceManager::InitOwnedDefaults() {
	static char vert_str[] = "#version 330 \n layout(location = 0) in vec3 inVertex; layout(location = 1) in vec3 inNorm; layout(location = 2) in vec2 inUV; uniform mat4 model, proj, view; void main() { gl_Position = proj * view * model * vec4(inVertex, 1.0); }";
	static char frag_str[] = "#version 330 \n out vec4 FragColor; void main() { FragColor = vec4(0,0,1,1); }";

	if (default_shader.id == 0) {

		// Compile shaders
		GLuint vertex_shader = CompileShader(GL_VERTEX_SHADER, vert_str);
		GLuint fragment_shader = CompileShader(GL_FRAGMENT_SHADER, frag_str);

		// Create the shader program
		GLuint shader_program_id = glCreateProgram();
		glAttachShader(shader_program_id, vertex_shader);
		glAttachShader(shader_program_id, fragment_shader);
		glLinkProgram(shader_program_id);

		//Check if program was linked correctly
		GLint programLinked = GL_FALSE;
		glGetProgramiv(shader_program_id, GL_LINK_STATUS, &programLinked);
		if (programLinked != GL_TRUE) {
			SIK_ERROR("Program linking failed. ID: {}. Name: default", shader_program_id);
			SIK_ASSERT(false, "Program linking failed.");
		}

		default_shader = ShaderProgram{
			.id = shader_program_id,
			.vert = vertex_shader,
			.frag = fragment_shader };
	}

	if (default_material.shader == 0) {
		default_material.shader = &(default_shader.id);
	}
}

void ResourceManager::FreeOwnedDefaults() {
	std::destroy_at(&default_shader);
	std::destroy_at(&default_material);
}

#undef RM_TEST_ASYNC