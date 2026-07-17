namespace SZM::Physics {

// Helper to accumulate integrals over a tetrahedron formed by the origin and a triangle
static void computeTetrahedronIntegrals(
    const Vector3& p1, const Vector3& p2, const Vector3& p3,
    float& volumeOut, Vector3& cgOut, 
    float& ixx, float& iyy, float& izz,
    float& ixy, float& iyz, float& izx) 
{
    // Signed volume of the tetrahedron: 1/6 * (p1 dot (p2 cross p3))
    float det = p1.dot(p2.cross(p3));
    float vol = det / 6.0f;
    volumeOut = vol;

    // Center of mass of the tetrahedron (origin is 0,0,0 so it's just average of 4 points)
    cgOut = (p1 + p2 + p3) / 4.0f;

    // Inertia sub-terms (Mirtich/Eberly polynomial accumulation)
    // Formula simplifies the integration of x^2, y^2, z^2 over the simplex
    auto termSq = [](float x1, float x2, float x3) {
        return x1*x1 + x1*x2 + x2*x2 + x1*x3 + x2*x3 + x3*x3;
    };
    auto termProd = [](float x1, float x2, float x3, float y1, float y2, float y3) {
        return 2.0f*x1*y1 + x2*y1 + x3*y1 + x1*y2 + 2.0f*x2*y2 + x3*y2 + x1*y3 + x2*y3 + 2.0f*x3*y3;
    };

    // Sub-integrals for moments
    float intX2 = (det / 60.0f) * termSq(p1.x, p2.x, p3.x);
    float intY2 = (det / 60.0f) * termSq(p1.y, p2.y, p3.y);
    float intZ2 = (det / 60.0f) * termSq(p1.z, p2.z, p3.z);
    
    float intXY = (det / 120.0f) * termProd(p1.x, p2.x, p3.x, p1.y, p2.y, p3.y);
    float intYZ = (det / 120.0f) * termProd(p1.y, p2.y, p3.y, p1.z, p2.z, p3.z);
    float intZX = (det / 120.0f) * termProd(p1.z, p2.z, p3.z, p1.x, p2.x, p3.x);

    // Apply to inertia tensor components
    ixx = intY2 + intZ2;
    iyy = intX2 + intZ2;
    izz = intX2 + intY2;
    ixy = -intXY;
    iyz = -intYZ;
    izx = -intZX;
}

std::shared_ptr<RigidBody> RigidBody::createFromMesh(const std::vector<Triangle>& mesh, float density, KinematicState state) {
    auto body = std::make_shared<RigidBody>(state);

    float totalVolume = 0.0f;
    Vector3 totalCG = {0,0,0};
    
    float Ixx = 0, Iyy = 0, Izz = 0;
    float Ixy = 0, Iyz = 0, Izx = 0;

    // Accumulate integrals across all triangles
    for (const auto& tri : mesh) {
        float vol;
        Vector3 cg;
        float ixx, iyy, izz, ixy, iyz, izx;
        
        computeTetrahedronIntegrals(tri.v0, tri.v1, tri.v2, vol, cg, ixx, iyy, izz, ixy, iyz, izx);

        totalVolume += vol;
        totalCG = totalCG + (cg * vol);

        Ixx += ixx; Iyy += iyy; Izz += izz;
        Ixy += ixy; Iyz += iyz; Izx += izx;
    }

    // Failure Condition Handling
    if (totalVolume <= 1e-6f) {
        std::cerr << "SZM Physics Warning: Geometry has zero or negative volume. Defaulting to Static infinite mass." << std::endl;
        body->state = KinematicState::Static;
        body->mass = 0.0f;
        body->invMass = 0.0f;
        body->inertiaTensorLocal = {0};
        body->invInertiaTensorLocal = {0};
        return body;
    }

    // Resolve final CoM
    body->centerOfMassLocal = totalCG / totalVolume;
    
    // Apply Material Density to get actual Mass
    body->mass = totalVolume * density;
    
    // If Kinematic or Static, inverse mass and inverse inertia are zeroed out (infinite resistance)
    if (state == KinematicState::Static || state == KinematicState::Kinematic) {
        body->invMass = 0.0f;
        body->inertiaTensorLocal = {0};
        body->invInertiaTensorLocal = {0};
        return body;
    }

    body->invMass = 1.0f / body->mass;

    // Apply Material Density to Inertia Tensor
    Ixx *= density; Iyy *= density; Izz *= density;
    Ixy *= density; Iyz *= density; Izx *= density;

    // Shift Inertia Tensor from Origin to Center of Mass (Parallel Axis Theorem)
    float m = body->mass;
    float cx = body->centerOfMassLocal.x;
    float cy = body->centerOfMassLocal.y;
    float cz = body->centerOfMassLocal.z;

    Ixx -= m * (cy*cy + cz*cz);
    Iyy -= m * (cx*cx + cz*cz);
    Izz -= m * (cx*cx + cy*cy);
    Ixy -= m * (-cx*cy);
    Iyz -= m * (-cy*cz);
    Izx -= m * (-cx*cz);

    // Assemble the Symmetric Matrix
    body->inertiaTensorLocal.m[0][0] = Ixx; body->inertiaTensorLocal.m[0][1] = Ixy; body->inertiaTensorLocal.m[0][2] = Izx;
    body->inertiaTensorLocal.m[1][0] = Ixy; body->inertiaTensorLocal.m[1][1] = Iyy; body->inertiaTensorLocal.m[1][2] = Iyz;
    body->inertiaTensorLocal.m[2][0] = Izx; body->inertiaTensorLocal.m[2][1] = Iyz; body->inertiaTensorLocal.m[2][2] = Izz;

    body->invInertiaTensorLocal = body->inertiaTensorLocal.inverse();

    return body;
}

void RigidBody::updateWorldInertia(const Matrix3x3& rotationMatrix) {
    if (state == KinematicState::Static || state == KinematicState::Kinematic) return;

    // I_world^-1 = R * I_local^-1 * R^T
    Matrix3x3 R_T = rotationMatrix.transpose();
    Matrix3x3 temp = rotationMatrix * invInertiaTensorLocal;
    invInertiaTensorWorld = temp * R_T;
}

} // namespace SZM::Physics
