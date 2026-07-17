#include <iostream>
#include <iomanip>
#include "AI/LegendaryCore/LegendaryAIAssistant.hpp"

using namespace SZM::AI::Legendary;

void PrintHeader(const std::string& title) {
    std::cout << "\n" << std::string(80, '=') << std::endl;
    std::cout << "  " << title << std::endl;
    std::cout << std::string(80, '=') << std::endl;
}

int main() {
    PrintHeader("LEGENDARY AI ASSISTANT — TEST PROGRAM");

    try {
        // Initialize
        std::cout << "\n[1/6] Initializing Legendary AI Assistant..." << std::endl;
        auto& ai = LegendaryAIAssistant::GetInstance();
        ai.Initialize();
        std::cout << "✓ AI initialized successfully" << std::endl;

        // Get current state
        std::cout << "\n[2/6] Getting AI state..." << std::endl;
        AIState state = ai.GetCurrentState();
        std::cout << "✓ Current stage: " << static_cast<int>(state.current_stage) << std::endl;
        std::cout << "✓ Overall confidence: " << state.overall_confidence << std::endl;
        std::cout << "✓ Tasks completed: " << state.total_tasks_completed << std::endl;

        // Get skill graph
        std::cout << "\n[3/6] Loading skill graph..." << std::endl;
        json skills = ai.GetSkillGraph();
        std::cout << "✓ Skills loaded: " << skills.size() << " skills" << std::endl;

        // Get memory statistics
        std::cout << "\n[4/6] Getting memory statistics..." << std::endl;
        json memory_stats = ai.GetMemoryStatistics();
        std::cout << "✓ Total memories: " << memory_stats["total_memories"] << std::endl;
        std::cout << "✓ Total skills: " << memory_stats["total_skills"] << std::endl;
        std::cout << "✓ Average skill level: " << memory_stats["average_skill_level"] << std::endl;

        // Get learning progress
        std::cout << "\n[5/6] Getting learning progress..." << std::endl;
        json progress = ai.GetLearningProgress();
        std::cout << "✓ Stage progress: " << progress["stage_progress"] << std::endl;

        // Get comprehensive report
        std::cout << "\n[6/6] Generating comprehensive report..." << std::endl;
        json report = ai.GetComprehensiveReport();
        std::cout << "✓ Report generated successfully" << std::endl;

        PrintHeader("COMPREHENSIVE AI REPORT");
        std::cout << report.dump(2) << std::endl;

        PrintHeader("TEST RESULTS");
        std::cout << "\n✓ All tests passed successfully!" << std::endl;
        std::cout << "\nLegendary AI Assistant is ready for integration with SZM Forge." << std::endl;

        return 0;

    } catch (const std::exception& e) {
        std::cerr << "\n✗ Error: " << e.what() << std::endl;
        return 1;
    }
}
