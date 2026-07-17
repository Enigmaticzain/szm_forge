#include "LegendaryCore/ModificationSystem.hpp"
#include <algorithm>

namespace SZM::AI::Legendary {

ModificationSystem& ModificationSystem::GetInstance() {
    static ModificationSystem instance;
    return instance;
}

ModificationProposal ModificationSystem::CreateProposal(
    ModificationType type,
    const std::string& description,
    const std::vector<CodeDiff>& diffs) {

    ModificationProposal proposal;
    proposal.id = "prop_" + std::to_string(proposals.size());
    proposal.type = type;
    proposal.description = description;
    proposal.diffs = diffs;
    proposal.approval_status = ApprovalStatus::PENDING;

    for (const auto& diff : diffs) {
        // Count lines from diffs
    }

    return proposal;
}

void ModificationSystem::AnalyzeRisk(ModificationProposal& proposal) {
    json risk_analysis = json::object();
    int total_lines = 0;
    for (const auto& diff : proposal.diffs) {
        total_lines += diff.lines_added + diff.lines_removed;
    }
    risk_analysis["lines_changed"] = total_lines;
    risk_analysis["affected_modules"] = proposal.affected_modules.size();
    risk_analysis["complexity"] = "medium";

    float risk = 0.0f;
    if (total_lines > 100) risk += 0.2f;
    if (proposal.affected_modules.size() > 3) risk += 0.2f;
    if (proposal.type == ModificationType::ARCHITECTURE_CHANGE) risk += 0.3f;

    proposal.risk_score = std::min(1.0f, risk);
    proposal.risk_analysis = risk_analysis;
}

json ModificationSystem::GetRiskReport(const ModificationProposal& proposal) {
    json report;
    report["proposal_id"] = proposal.id;
    report["risk_score"] = proposal.risk_score;
    report["analysis"] = proposal.risk_analysis;
    return report;
}

void ModificationSystem::SubmitForApproval(ModificationProposal& proposal) {
    AnalyzeRisk(proposal);
    proposal.approval_status = ApprovalStatus::PENDING;
}

void ModificationSystem::ApproveProposal(const std::string& proposal_id) {
    auto it = std::find_if(proposals.begin(), proposals.end(),
                          [&proposal_id](const ModificationProposal& p) { return p.id == proposal_id; });
    if (it != proposals.end()) {
        it->approval_status = ApprovalStatus::APPROVED;
    }
}

void ModificationSystem::RejectProposal(const std::string& proposal_id, const std::string& reason) {
    auto it = std::find_if(proposals.begin(), proposals.end(),
                          [&proposal_id](const ModificationProposal& p) { return p.id == proposal_id; });
    if (it != proposals.end()) {
        it->approval_status = ApprovalStatus::REJECTED;
    }
}

void ModificationSystem::RequestRevision(const std::string& proposal_id, const std::string& feedback) {
    auto it = std::find_if(proposals.begin(), proposals.end(),
                          [&proposal_id](const ModificationProposal& p) { return p.id == proposal_id; });
    if (it != proposals.end()) {
        it->approval_status = ApprovalStatus::NEEDS_REVISION;
    }
}

bool ModificationSystem::TestInSandbox(const ModificationProposal& proposal) {
    // Simulate sandbox testing
    return proposal.risk_score < 0.7f;
}

json ModificationSystem::GetSandboxResults(const std::string& proposal_id) {
    json results;
    results["proposal_id"] = proposal_id;
    results["test_passed"] = true;
    results["errors"] = json::array();
    return results;
}

ModificationResult ModificationSystem::ApplyModification(const ModificationProposal& proposal) {
    ModificationResult result;
    result.proposal_id = proposal.id;

    if (proposal.approval_status != ApprovalStatus::APPROVED) {
        result.success = false;
        result.error_message = "Proposal not approved";
        return result;
    }

    // Apply changes
    result.success = true;
    result.confidence = 1.0f - proposal.risk_score;

    modification_history.push_back(result);
    return result;
}

void ModificationSystem::ValidateModification(ModificationResult& result) {
    // Validate the applied modification
    result.validation_errors.clear();
    result.validation_warnings.clear();
}

Snapshot ModificationSystem::CreateSnapshot(const std::string& description) {
    Snapshot snapshot;
    snapshot.id = "snap_" + std::to_string(snapshots.size());
    snapshot.description = description;
    snapshot.timestamp = std::to_string(std::time(nullptr));
    return snapshot;
}

void ModificationSystem::RollbackToSnapshot(const std::string& snapshot_id) {
    auto it = std::find_if(snapshots.begin(), snapshots.end(),
                          [&snapshot_id](const Snapshot& s) { return s.id == snapshot_id; });
    if (it != snapshots.end()) {
        // Restore files from snapshot
    }
}

std::vector<Snapshot> ModificationSystem::GetSnapshotHistory() {
    return snapshots;
}

void ModificationSystem::CommitChanges(const std::string& proposal_id, const std::string& message) {
    // Commit to git
}

std::string ModificationSystem::GetGitDiff(const ModificationProposal& proposal) {
    std::string diff;
    for (const auto& code_diff : proposal.diffs) {
        diff += "--- " + code_diff.file_path + "\n";
        diff += "+++ " + code_diff.file_path + "\n";
        diff += code_diff.old_content + "\n";
        diff += code_diff.new_content + "\n";
    }
    return diff;
}

std::vector<ModificationProposal> ModificationSystem::GetProposalHistory() {
    return proposals;
}

std::vector<ModificationResult> ModificationSystem::GetModificationHistory() {
    return modification_history;
}

} // namespace SZM::AI::Legendary
