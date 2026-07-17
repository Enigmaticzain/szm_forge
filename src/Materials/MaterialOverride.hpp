struct MaterialOverride {
    bool isActive = false;
    float rgba[4] = {1.0f, 1.0f, 1.0f, 1.0f};
    float roughness = 0.5f;
    float metallic = 0.0f;
};

// Represents the heavy, in-memory CAD Part (loaded only once)