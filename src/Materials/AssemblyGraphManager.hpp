class AssemblyGraphManager {
private:
    std::unordered_map<UUID, std::shared_ptr<BasePart>> partRegistry;
    std::shared_ptr<SZM::Entity> activeAssemblyRoot;

    void traverseForBOM(const std::shared_ptr<SZM::Entity>& node, std::unordered_map<UUID, BOMEntry>& bomMap) const;
    void traverseForMassCG(const std::shared_ptr<SZM::Entity>& node, float& outTotalMass, Vector3& outMassMoments) const;

public:
    AssemblyGraphManager(std::shared_ptr<SZM::Entity> root) : activeAssemblyRoot(root) {}

    // Registers a heavy part into memory once
    void registerBasePart(const std::shared_ptr<BasePart>& part);

    // Creates a lightweight instance in the assembly
    std::shared_ptr<InstanceNode> instantiatePart(
        std::shared_ptr<SZM::Entity> parentNode, 
        UUID partUUID, 
        const Transform& transform,
        const std::string& instanceName = "Instance"
    );

    // Generates the Bill of Materials
    std::vector<BOMEntry> generateBOM() const;

    // Calculates Assembly Mass and Center of Gravity
    void calculateMassProperties(float& outTotalMass, Vector3& outCG) const;

    // Extracts flat array of matrices for glDrawElementsInstanced / vkCmdDrawIndexed
    std::vector<Transform> getInstancedDrawData(UUID partUUID) const;
};

} // namespace SZM::Assembly
