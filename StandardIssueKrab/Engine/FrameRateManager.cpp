#include "stdafx.h"
#include "FrameRateManager.h"

FrameRateManager::FrameRateManager() : m_frame_counter(0), m_timer(0.0), m_framerate(0.0)
{    
}

FrameRateManager::~FrameRateManager()
{

}

void FrameRateManager::FrameStart(){}
void FrameRateManager::FrameEnd(Float64 elapsed)
{
    // elapsed => seconds
    m_timer += elapsed;
    ++m_frame_counter;
    if (m_frame_counter == 60) 
    {        
        m_framerate = 60.0/m_timer;
        
        // clear counter and accumulator
        m_frame_counter = 0;
        m_timer = 0.0;
    }
}

Float64 FrameRateManager::GetFPS() 
{ 
    return m_framerate;
}
