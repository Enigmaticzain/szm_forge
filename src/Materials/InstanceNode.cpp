class InstanceNode : public SZM::Entity {
private:
    UUID basePartUUID;
    MaterialOverride material;

public:
    InstanceNode(std::string nodeName, UUID partRef, Transform initialTransform)
        : SZM::Entity(std::move(nodeName)), basePartUUID(partRef) {
        // Assume Entity has a setLocalTransform method added to Module 020.1
        // this->setLocalTransform(initialTransform); 
    }

    UUID getBasePartUUID() const { return basePartUUID; }
    MaterialOverride getMaterial() const { return material; }
    void setMaterialOverride(const MaterialOverride& mat) { material = mat; }
};

// Bill of Materials Entry