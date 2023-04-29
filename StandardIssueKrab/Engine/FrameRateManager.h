#pragma once

class FrameRateManager
{
private:
    Uint8 m_frame_counter;
    Float64 m_timer;
    Float64 m_framerate;

public:
    FrameRateManager();
    ~FrameRateManager();
    void FrameStart();
    void FrameEnd(Float64 elapsed);
    Float64 GetFPS();
    
};

extern FrameRateManager* p_frame_rate_manager;