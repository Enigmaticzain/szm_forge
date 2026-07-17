struct BasePart {
    UUID partID;
    std::string partName;
    float mass = 0.0f;
    Vector3 localCG = {0.0f, 0.0f, 0.0f};
    
    // In a full implementation, this holds the actual Mesh/VBO/IBO pointers
    // std::shared_ptr<Geometry::SolidBody> geometry; 
};

// The lightweight Reference Node inserted into the Scene Graph