#pragma once

#include <string>
#include <vector>
#include <memory>
#include <nlohmann/json.hpp>

namespace SZM::AI::Legendary {

using json = nlohmann::json;

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// MODIFICATION TYPES
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

enum class ModificationType {
    CODE_GENERATION,
    CODE_REFACTORING,
    BUG_FIX,
    OPTIMIZATION,
    ARCHITECTURE_CHANGE,
    DEPENDENCY_UPDATE,
    CONFIGURATION_CHANGE
};

enum class ApprovalStatus {
    PENDING,
    APPROVED,
    REJECTED,
    NEEDS_REVISION
};

struct CodeDiff {
    std::string file_path;
    std::string old_content;
    std::string new_content;
    int lines_added = 0;
    int lines_removed = 0;
};

struct ModificationProposal {
    std::string id;
    ModificationType type;
    std::string description;
    std::string reasoning;
    std::vector<CodeDiff> diffs;
    std::vector<std::string> affected_modules;
    std::vector<std::string> required_skills;
    float risk_score = 0.0f;
    json risk_analysis;
    ApprovalStatus approval_status = ApprovalStatus::PENDING;
};

struct Snapshot {
    std::string id;
    std::string timestamp;
    std::string description;
    std::map<std::string, std::string> file_contents;
};

struct ModificationResult {
    std::string proposal_id;
    bool success = false;
    std::string error_message;
    std::vector<std::string> validation_errors;
    std::vector<std::string> validation_warnings;
    float confidence = 0.0f;
};

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// MODIFICATION SYSTEM
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

class ModificationSystem {
public:
    static ModificationSystem& GetInstance();

    // Proposal Generation
    ModificationProposal CreateProposal(
        ModificationType type,
        const std::string& description,
        const std::vector<CodeDiff>& diffs
    );

    // Risk Analysis
    void AnalyzeRisk(ModificationProposal& proposal);
    json GetRiskReport(const ModificationProposal& proposal);

    // Approval Workflow
    void SubmitForApproval(ModificationProposal& proposal);
    void ApproveProposal(const std::string& proposal_id);
    void RejectProposal(const std::string& proposal_id, const std::string& reason);
    void RequestRevision(const std::string& proposal_id, const std::string& feedback);

    // Sandbox Testing
    bool TestInSandbox(const ModificationProposal& proposal);
    json GetSandboxResults(const std::string& proposal_id);

    // Application
    ModificationResult ApplyModification(const ModificationProposal& proposal);
    void ValidateModification(ModificationResult& result);

    // Snapshots & Rollback
    Snapshot CreateSnapshot(const std::string& description);
    void RollbackToSnapshot(const std::string& snapshot_id);
    std::vector<Snapshot> GetSnapshotHistory();

    // Git Integration
    void CommitChanges(const std::string& proposal_id, const std::string& message);
    std::string GetGitDiff(const ModificationProposal& proposal);

    // History
    std::vector<ModificationProposal> GetProposalHistory();
    std::vector<ModificationResult> GetModificationHistory();

private:
    ModificationSystem() = default;

    std::vector<ModificationProposal> proposals;
    std::vector<ModificationResult> modification_history;
    std::vector<Snapshot> snapshots;
};

} // namespace SZM::AI::Legendary
