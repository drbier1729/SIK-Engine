#pragma once

#define TEST_LAUNCHER(name) void* name(unsigned int test_number, EngineExport* p_engine_export_struct)
typedef TEST_LAUNCHER(test_launch_fcn);

#define TEST_UPDATE(name) void* name(void* test_obj)
typedef TEST_UPDATE(test_update_fcn);

#define TEST_END(name) void name(void* test_obj)
typedef TEST_END(test_end_fcn);