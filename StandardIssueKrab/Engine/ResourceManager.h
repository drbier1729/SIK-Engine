#pragma once

#include "MemoryResources.h"
#include "FrameTimer.h"
#include "Material.h"

//Forward declarations
class Texture;
class FontTextures;
class Mesh;
class Model;

// Wrapper around rapidjson::Document that also includes
// the filepath the document corresponds to on disk so it can be
// easily reloaded or saved.
struct JSON {
	rapidjson::Document doc;
	std::filesystem::path path;
};


class ResourceManager
{
private:
	// We store OpenGL shader program ids together with their shader ids so 
	// we can delete them properly. This only works because each program is 
	// uniquely associated with two shaders with matching names... not a very
	// flexible system, but oh well.
	struct ShaderProgram {
		GLuint id = 0;
		GLuint vert = 0;
		GLuint frag = 0;
		GLuint comp = 0;
	};

	// Job that can be placed on a queue indicating the path to the data that
	// needs to be loaded, along with the type of data we're looking at.
	struct LoadJob {
		enum class Type {
			NONE,
			Shader,
			Texture,
			Mesh,
			JSON
		};

		Type				  type = Type::NONE;
		StringID			  key  = StringID{0};
		std::filesystem::path path{};
	};

	// Job that can be placed on a queue which owns data that was previously
	// loaded from a LoadJob, along with additional arguments needed to 
	// construct an object of the indicated type.
	struct ProcessJob {
		enum class Type {
			NONE,
			Shader,
			Texture,
			Mesh,
			JSON
		};

		struct ShaderData {
			UniquePtr<char[]> vert_str;
			UniquePtr<char[]> frag_str;
			UniquePtr<char[]> geom_str;
		};

		struct TextureData {
			UniquePtr<unsigned char[]> image;
			Int32 width, height, channel_count;
		};

		struct MeshData {
			Vector<Float32> verts;
			Vector<Uint32> idxs;
			Vector<Float32> tangents;
		};

		using DataHolder = std::variant<ShaderData, TextureData, MeshData, JSON>;

		Type       type = Type::NONE;
		StringID   key  = StringID{ 0 };
		DataHolder data{};
	};

private:
	// Async Support
	static constexpr SizeT					NUM_WORKER_THREADS = 3;
	Array<std::jthread, NUM_WORKER_THREADS> thread_pool;
	std::atomic<bool>						threads_continue;
	std::atomic<int>						threads_working;
	
	std::queue<ProcessJob>					processing_queue;
	std::mutex								processing_mtx;	

	std::queue<LoadJob>						loading_queue;
	std::mutex								loading_mtx;


	// In-Memory Resource Storage (single-threaded only)
	PoolMemoryResource						mem_pool;
	PolymorphicAllocator					alloc;

	// In-Memory Data Tables
	UnorderedMap<StringID, Texture>			textures;
	UnorderedMap<StringID, Mesh>			meshes;
	UnorderedMap<StringID, JSON>			json_docs;
	UnorderedMap<StringID, ShaderProgram>   shader_programs;
	UnorderedMap<StringID, Model>			models;
	UnorderedMap<StringID, FontTextures>	m_font_map;
	UnorderedMap<StringID, Material>		m_material_map;

	// Default values, if not provided by class
	ShaderProgram							default_shader;
	Material								default_material;

	// Prefixes for resource and asset filepaths
	std::filesystem::path					resources_path;
	std::filesystem::path					assets_path;

public:
	ResourceManager();
	~ResourceManager() noexcept;
	ResourceManager(ResourceManager const&)            = delete;
	ResourceManager(ResourceManager &&)				   = delete;
	ResourceManager& operator=(ResourceManager const&) = delete;
	ResourceManager& operator=(ResourceManager &&)     = delete;

	// Must be called at EngineInit after OpenGL context is created
	void	  InitDefaultAssets();

	// Called in the destructor. Must be called before OpenGL context
	// is destroyed.
	void	  FreeDefaultAssets();

	// Called each frame in order to process assets loaded asynchronously.
	// Returns number of assets processed. 
	Uint32	  ProcessAssetsFor(std::convertible_to<FrameTimer::duration_type> auto duration);

	// Processes a single loaded asset. Blocks the main thread until finished.
	// Returns true if an asset was processed.
	Bool	  ProcessAsset();

	// Blocks the main thread until finished. Returns: number of processed assets.
	Uint32	  ProcessAllAssets();

	/*
	* Loads a Font Texture. Saves it in the Font texture map
	* Returns: FontTextures* pointer to the created FontTextures object
	*/
	FontTextures* LoadFontTexture(const char* ttf_name, Uint8 font_pixel_height);

	/*
	* Get a FontTextures from the FontTextures map
	* Returns: FontTextures* pointer to the FontTextures object. nullptr if it doesn't exit
	*/
	FontTextures* GetFontTexture(const char* ttf_name, Uint8 font_pixel_height);

	// Async Methods
	void      BatchLoadAssets(std::filesystem::path const& manifest_filepath);  // for single-threaded version: set RM_TEST_ASYNC to 0 in .cpp

	/*
	 * Return value :
	 * IF( resource is loaded ):
	 *		ptr to the resource
	 * ELSE: ptr to a default resource which will be replaced by the requested
	 *		resource, once loading has completed
	*/
	GLuint*   AsyncLoadShaderProgram(const char* prg_name); // filename NO extension
	GLuint*	  AsyncLoadShaderProgram(const char* prg_name, bool use_geo); // filename NO extension
	Texture*  AsyncLoadTexture(const char* texture_name);   // filename with extension
	Mesh*     AsyncLoadMesh(const char* mesh_name);		    // filename with extension
	JSON*     AsyncLoadJSON(const char* json_name);         // filename with extension


	// Single-threaded Methods
	GLuint*   LoadShaderProgram(const char* prg_name);	// filename NO extension
	GLuint*	  LoadComputeShader(const char* shader_name);
	GLuint*   LoadShaderProgram(const char*, bool use_geo); // filename NO extension
	Texture*  LoadTexture(const char* texture_name);	// filename with extension
	Mesh*     LoadMesh(const char* mesh_name);			// filename with extension
	JSON*     LoadJSON(const char* json_name);			// filename with extension
	Material* LoadMaterial(const char* material_name);  // [INCOMPLETE]
	Model*	  LoadModel(const char* model_name);		// TODO: combine with Async loading
	SDL_Surface* LoadImageAsSurface(const char* filepath);


	void	  UnloadShaderProgram(StringID prg_name_id);
	void	  UnloadComputeShader(StringID prg_name_id);
	void	  UnloadTexture(StringID texture_name_id);
	void	  UnloadMesh(StringID mesh_name_id);
	void	  UnloadJSON(StringID json_name_id);
	void	  UnloadModel(StringID model_name_id);
	
	/* 
	 * Return value :
	 * IF( resource is loaded ): 
	 *		ptr to the resource
	 * ELSE IF( resource is currently loading ): 
			ptr to the a default resource which will be replaced by the 
			requested resource, once loading has completed
	 * ELSE: nullptr 
	*/
	GLuint*	  GetShaderProgram(StringID prg_name_id);
	GLuint*	  GetShaderProgram(const char* prg_name);
	Texture*  GetTexture(StringID texture_name_id);
	Texture*  GetTexture(const char* texture_name);
	Mesh*	  GetMesh(StringID mesh_name_id);
	Mesh*	  GetMesh(const char* mesh_name);
	JSON*	  GetJSON(StringID json_name_sid);
	JSON*	  GetJSON(const char* json_name);
	Model*	  GetModel(StringID model_name_id);
	Model*	  GetModel(const char* model_name);
	Material* GetMaterial(StringID material_name_sid);
	Material* GetMaterial(const char* material_name);


	// Special-purpose methods
	inline
	GLuint    GetDefaultShaderProgram();

	void	  ReloadAllJSON();
	void      ReloadJSON(const char* filename);
	Bool	  WriteJSONToDisk(const char* json_name);
	
private:
	// Jobs which read from file asynchronously and fill
	// one of the async_buffers with read in data
	void LoadShaderDataJob(std::filesystem::path path, StringID key);
	void LoadTextureDataJob(std::filesystem::path path, StringID key);
	void LoadMeshDataJob(std::filesystem::path path, StringID key);
	void LoadJSONDataJob(std::filesystem::path path, StringID key);
	void LoadingThreadWork();

	// Jobs which will be enqueued asynchronously but handled
	// synchronously by the main thread. These do final processing
	// on loaded data, including make OpenGL calls.
	void ProcessShaderDataJob(ProcessJob& job);
	void ProcessTextureDataJob(ProcessJob& job);
	void ProcessMeshDataJob(ProcessJob& job);
	void ProcessJSONDataJob(ProcessJob& job);

	// Some helpers for loading and initializing the default shader
	// program.
	void InitOwnedDefaults();
	void FreeOwnedDefaults();
};

//Declared as an extern variable so it can be accessed throughout the project
extern ResourceManager* p_resource_manager;




// Inline Definitions
Uint32 ResourceManager::ProcessAssetsFor(std::convertible_to<FrameTimer::duration_type> auto duration) {

	Uint32 completion_count = 0;
	auto begin = FrameTimer::Now();
	
	while ( (FrameTimer::Now() - begin < duration) && ProcessAsset()) {
			++completion_count;
	}

	return completion_count;
}

inline GLuint ResourceManager::GetDefaultShaderProgram() {
	return default_shader.id;
}