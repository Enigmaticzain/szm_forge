#include "SimulationVisualizationData.hpp"
#include <algorithm>

namespace SZM::Graphics {

SimulationVisualizationData& SimulationVisualizationData::GetInstance() {
    static SimulationVisualizationData instance;
    return instance;
}

void SimulationVisualizationData::AddFrame(const SimulationFrameData& frame) {
    m_Frames.push_back(frame);
    m_PlaybackState.totalFrames = m_Frames.size();
}

const SimulationFrameData* SimulationVisualizationData::GetFrame(uint32_t frameIndex) const {
    if (frameIndex >= m_Frames.size()) {
        return nullptr;
    }
    return &m_Frames[frameIndex];
}

const SimulationFrameData* SimulationVisualizationData::GetCurrentFrame() const {
    return GetFrame(m_CurrentFrameIndex);
}

void SimulationVisualizationData::SetCurrentFrame(uint32_t frameIndex) {
    if (frameIndex < m_Frames.size()) {
        m_CurrentFrameIndex = frameIndex;
        m_PlaybackState.currentFrame = frameIndex;
        if (!m_Frames.empty()) {
            m_PlaybackState.currentTime = m_Frames[frameIndex].timestamp;
        }
    }
}

void SimulationVisualizationData::ClearFrames() {
    m_Frames.clear();
    m_CurrentFrameIndex = 0;
    m_PlaybackState.totalFrames = 0;
    m_PlaybackState.currentFrame = 0;
    m_PlaybackState.currentTime = 0.0f;
}

void SimulationVisualizationData::Play() {
    m_PlaybackState.isPlaying = true;
}

void SimulationVisualizationData::Pause() {
    m_PlaybackState.isPlaying = false;
}

void SimulationVisualizationData::Stop() {
    m_PlaybackState.isPlaying = false;
    m_CurrentFrameIndex = 0;
    m_PlaybackState.currentFrame = 0;
    m_PlaybackState.currentTime = 0.0f;
}

void SimulationVisualizationData::NextFrame() {
    if (m_CurrentFrameIndex < m_Frames.size() - 1) {
        SetCurrentFrame(m_CurrentFrameIndex + 1);
    } else if (m_PlaybackState.isLooping) {
        SetCurrentFrame(0);
    }
}

void SimulationVisualizationData::PreviousFrame() {
    if (m_CurrentFrameIndex > 0) {
        SetCurrentFrame(m_CurrentFrameIndex - 1);
    }
}

void SimulationVisualizationData::Update(float deltaTime) {
    if (!m_PlaybackState.isPlaying || m_Frames.empty()) {
        return;
    }

    m_PlaybackState.currentTime += deltaTime * m_PlaybackState.playbackSpeed;

    for (uint32_t i = 0; i < m_Frames.size(); ++i) {
        if (m_Frames[i].timestamp >= m_PlaybackState.currentTime) {
            SetCurrentFrame(i);
            return;
        }
    }

    if (m_PlaybackState.isLooping) {
        SetCurrentFrame(0);
        m_PlaybackState.currentTime = 0.0f;
    } else {
        m_PlaybackState.isPlaying = false;
        SetCurrentFrame(m_Frames.size() - 1);
    }
}

} // namespace SZM::Graphics
