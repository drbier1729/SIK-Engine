#pragma once

#define GAME_LAUNCH(name) void name(EngineExport* p_engine_export_struct, int argc, char* args[])
typedef GAME_LAUNCH(game_launch_fcn);

#define GAME_UPDATE(name) void name(Float32 dt)
typedef GAME_UPDATE(game_update_fcn);

#define GAME_END(name) void name()
typedef GAME_END(game_end_fcn);