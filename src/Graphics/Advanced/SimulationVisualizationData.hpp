#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <cstdint>

namespace SZM::Graphics {

/**
 * @struct SimulationFrameData
 * @brief Single frame of simulation visualization data
 */
struct SimulationFrameData {
    uint32_t frameNumber;
    float timestamp;
    std::vector<glm::vec3> componentPositions;
    std::vector<float> stressValues;
    std::vector<float> temperatureValues;
    std::vector<glm::vec3> deformations;
    std::vector<bool> failureStates;
};

/**
 * @struct SimulationPlaybackState
 * @brief Playback state for simulation visualization
 */
struct SimulationPlaybackState {
    bool isPlaying = false;
    bool isLooping = false;
    float playbackSpeed = 1.0f;
    uint32_t currentFrame = 0;
    uint32_t totalFrames = 0;
    float currentTime = 0.0f;
    float totalTime = 0.0f;
};

/**
 * @class SimulationVisualizationData
 * @brief Manages simulation frame data for visualization
 */
class SimulationVisualizationData {
public:
    static SimulationVisualizationData& GetInstance();

    /**
     * @brief Add frame data
     */
    void AddFrame(const SimulationFrameData& frame);

    /**
     * @brief Get frame by index
     */
    const SimulationFrameData* GetFrame(uint32_t frameIndex) const;

    /**
     * @brief Get current frame
     */
    const SimulationFrameData* GetCurrentFrame() const;

    /**
     * @brief Set current frame
     */
    void SetCurrentFrame(uint32_t frameIndex);

    /**
     * @brief Get frame count
     */
    uint32_t GetFrameCount() const { return m_Frames.size(); }

    /**
     * @brief Clear all frames
     */
    void ClearFrames();

    /**
     * @brief Get playback state
     */
    const SimulationPlaybackState& GetPlaybackState() const { return m_PlaybackState; }

    /**
     * @brief Set playback state
     */
    void SetPlaybackState(const SimulationPlaybackState& state) { m_PlaybackState = state; }

    /**
     * @brief Start playback
     */
    void Play();

    /**
     * @brief Pause playback
     */
    void Pause();

    /**
     * @brief Stop playback
     */
    void Stop();

    /**
     * @brief Advance to next frame
     */
    void NextFrame();

    /**
     * @brief Go to previous frame
     */
    void PreviousFrame();

    /**
     * @brief Update playback (call each frame)
     */
    void Update(float deltaTime);

private:
    SimulationVisualizationData() = default;
    ~SimulationVisualizationData() = default;

    SimulationVisualizationData(const SimulationVisualizationData&) = delete;
    SimulationVisualizationData& operator=(const SimulationVisualizationData&) = delete;

    std::vector<SimulationFrameData> m_Frames;
    SimulationPlaybackState m_PlaybackState;
    uint32_t m_CurrentFrameIndex = 0;
};

} // namespace SZM::Graphics
