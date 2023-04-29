#pragma once

#define PROTOTYPE_LAUNCH(name) void name(EngineExport* p_engine_export_struct, int argc, char* args[])
typedef PROTOTYPE_LAUNCH(prototype_launch_fcn);

#define PROTOTYPE_UPDATE(name) void name(Float32 dt)
typedef PROTOTYPE_UPDATE(prototype_update_fcn);

#define PROTOTYPE_END(name) void name()
typedef PROTOTYPE_END(prototype_end_fcn);