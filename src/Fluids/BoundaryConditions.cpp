#include "BoundaryConditions.hpp"
#include <cmath>
#include <algorithm>
#include <fstream>
#include <json/json.h>

namespace SZM::Fluids {

BoundaryConditionManager::BoundaryConditionManager()
    : domain{{-1, -1, -1}, {1, 1, 1}, 0.1f, 20, 20, 20} {
    containerGeometry.type = GeometryType::Box;
    containerGeometry.position = {0, 0, 0};
    containerGeometry.dimensions = {2, 2, 2};
    containerGeometry.name = "Default Box";
}

BoundaryConditionManager::BoundaryConditionManager(const ContainerGeometry& geometry)
    : containerGeometry(geometry) {
    initializeRectangularDomain();
}

void BoundaryConditionManager::setContainerGeometry(const ContainerGeometry& geometry) {
    containerGeometry = geometry;
    initializeRectangularDomain();
    createWallSegments();
}

void BoundaryConditionManager::setFluidDomain(const FluidDomain& newDomain) {
    domain = newDomain;
    updateGridResolution();
}

void BoundaryConditionManager::addBoundarySegment(const BoundarySegment& segment) {
    boundarySegments.push_back(segment);
}

void BoundaryConditionManager::removeBoundarySegment(size_t index) {
    if (index < boundarySegments.size()) {
        boundarySegments.erase(boundarySegments.begin() + index);
    }
}

const BoundarySegment& BoundaryConditionManager::getBoundarySegment(size_t index) const {
    return boundarySegments[index];
}

BoundarySegment& BoundaryConditionManager::getBoundarySegment(size_t index) {
    return boundarySegments[index];
}

void BoundaryConditionManager::createRectangularContainer(
    float length, float width, float height,
    float inletVelocity, float outletPressure) {
    
    clearAllBoundaries();
    
    // Set geometry
    containerGeometry.type = GeometryType::Box;
    containerGeometry.dimensions = {length, width, height};
    containerGeometry.position = {0, height/2, 0};
    
    // Create domain
    domain.minBound = {-length/2, 0, -width/2};
    domain.maxBound = {length/2, height, width/2};
    updateGridResolution();
    
    // Inlet (front face, bottom half)
    BoundarySegment inlet;
    inlet.type = BoundaryType::Inlet;
    inlet.position = {-length/2, height/4, 0};
    inlet.normal = {1, 0, 0};
    inlet.area = width * height / 2;
    inlet.value = inletVelocity;
    inlet.direction = {1, 0, 0};
    inlet.isActive = true;
    addBoundarySegment(inlet);
    
    // Outlet (back face, top half)
    BoundarySegment outlet;
    outlet.type = BoundaryType::Outlet;
    outlet.position = {length/2, 3*height/4, 0};
    outlet.normal = {-1, 0, 0};
    outlet.area = width * height / 2;
    outlet.value = outletPressure;
    outlet.direction = {-1, 0, 0};
    outlet.isActive = true;
    addBoundarySegment(outlet);
    
    // Walls
    createWallSegments();
}

void BoundaryConditionManager::createCircularPipeflow(
    float diameter, float length,
    float inletVelocity, float inletPressure) {
    
    clearAllBoundaries();
    
    // Set geometry
    containerGeometry.type = GeometryType::Cylinder;
    containerGeometry.radius = diameter / 2;
    containerGeometry.height = length;
    
    // Create domain
    float radius = diameter / 2;
    domain.minBound = {-radius, -radius, -length/2};
    domain.maxBound = {radius, radius, length/2};
    updateGridResolution();
    
    // Inlet (circular inlet)
    BoundarySegment inlet;
    inlet.type = BoundaryType::Inlet;
    inlet.position = {0, 0, -length/2};
    inlet.normal = {0, 0, 1};
    inlet.area = 3.14159f * radius * radius;
    inlet.value = inletVelocity;
    inlet.direction = {0, 0, 1};
    inlet.isActive = true;
    addBoundarySegment(inlet);
    
    // Outlet (pressure boundary)
    BoundarySegment outlet;
    outlet.type = BoundaryType::Pressure;
    outlet.position = {0, 0, length/2};
    outlet.normal = {0, 0, -1};
    outlet.area = inlet.area;
    outlet.value = inletPressure;
    outlet.isActive = true;
    addBoundarySegment(outlet);
    
    // Cylindrical wall
    BoundarySegment wall;
    wall.type = BoundaryType::Wall;
    wall.position = {0, 0, 0};
    wall.normal = {1, 0, 0};
    wall.area = 2 * 3.14159f * radius * length;
    wall.isActive = true;
    addBoundarySegment(wall);
}

void BoundaryConditionManager::createTankWithInletOutlet(
    float tankLength, float tankWidth, float tankHeight,
    float inletArea, float outletArea,
    glm::vec3 inletPosition, glm::vec3 outletPosition) {
    
    clearAllBoundaries();
    
    containerGeometry.type = GeometryType::Box;
    containerGeometry.dimensions = {tankLength, tankHeight, tankWidth};
    
    domain.minBound = {-tankLength/2, 0, -tankWidth/2};
    domain.maxBound = {tankLength/2, tankHeight, tankWidth/2};
    updateGridResolution();
    
    // Inlet
    BoundarySegment inlet;
    inlet.type = BoundaryType::Inlet;
    inlet.position = inletPosition;
    inlet.area = inletArea;
    inlet.value = 0.5f; // Default velocity
    inlet.isActive = true;
    addBoundarySegment(inlet);
    
    // Outlet
    BoundarySegment outlet;
    outlet.type = BoundaryType::Outlet;
    outlet.position = outletPosition;
    outlet.area = outletArea;
    outlet.value = 101325.0f; // Atmospheric pressure
    outlet.isActive = true;
    addBoundarySegment(outlet);
    
    createWallSegments();
}

void BoundaryConditionManager::createChannelFlow(
    float channelLength, float channelWidth, float channelHeight,
    float flowVelocity) {
    
    clearAllBoundaries();
    
    containerGeometry.type = GeometryType::Box;
    containerGeometry.dimensions = {channelLength, channelHeight, channelWidth};
    
    domain.minBound = {0, 0, 0};
    domain.maxBound = {channelLength, channelHeight, channelWidth};
    updateGridResolution();
    
    // Inlet - uniform velocity
    BoundarySegment inlet;
    inlet.type = BoundaryType::Velocity;
    inlet.position = {0, channelHeight/2, channelWidth/2};
    inlet.area = channelHeight * channelWidth;
    inlet.value = flowVelocity;
    inlet.direction = {1, 0, 0};
    inlet.isActive = true;
    addBoundarySegment(inlet);
    
    // Outlet - pressure
    BoundarySegment outlet;
    outlet.type = BoundaryType::Pressure;
    outlet.position = {channelLength, channelHeight/2, channelWidth/2};
    outlet.value = 101325.0f;
    outlet.isActive = true;
    addBoundarySegment(outlet);
    
    // Top and bottom walls
    BoundarySegment topWall;
    topWall.type = BoundaryType::Wall;
    topWall.position = {channelLength/2, channelHeight, channelWidth/2};
    topWall.area = channelLength * channelWidth;
    topWall.isActive = true;
    addBoundarySegment(topWall);
    
    BoundarySegment bottomWall;
    bottomWall.type = BoundaryType::Wall;
    bottomWall.position = {channelLength/2, 0, channelWidth/2};
    bottomWall.area = channelLength * channelWidth;
    bottomWall.isActive = true;
    addBoundarySegment(bottomWall);
}

float BoundaryConditionManager::getTotalInletArea() const {
    float total = 0;
    for (const auto& seg : boundarySegments) {
        if (seg.type == BoundaryType::Inlet || seg.type == BoundaryType::Velocity) {
            total += seg.area;
        }
    }
    return total;
}

float BoundaryConditionManager::getTotalOutletArea() const {
    float total = 0;
    for (const auto& seg : boundarySegments) {
        if (seg.type == BoundaryType::Outlet || seg.type == BoundaryType::Pressure) {
            total += seg.area;
        }
    }
    return total;
}

float BoundaryConditionManager::getTotalWallArea() const {
    float total = 0;
    for (const auto& seg : boundarySegments) {
        if (seg.type == BoundaryType::Wall || seg.type == BoundaryType::FreeSlip) {
            total += seg.area;
        }
    }
    return total;
}

glm::vec3 BoundaryConditionManager::getTotalInletVelocity() const {
    glm::vec3 totalVel(0);
    for (const auto& seg : boundarySegments) {
        if (seg.type == BoundaryType::Inlet || seg.type == BoundaryType::Velocity) {
            totalVel += seg.direction * seg.value * seg.area;
        }
    }
    return totalVel;
}

float BoundaryConditionManager::getAverageInletPressure() const {
    float totalPressure = 0;
    int count = 0;
    for (const auto& seg : boundarySegments) {
        if (seg.type == BoundaryType::Inlet) {
            totalPressure += seg.value;
            count++;
        }
    }
    return count > 0 ? totalPressure / count : 0;
}

float BoundaryConditionManager::getAverageOutletPressure() const {
    float totalPressure = 0;
    int count = 0;
    for (const auto& seg : boundarySegments) {
        if (seg.type == BoundaryType::Outlet || seg.type == BoundaryType::Pressure) {
            totalPressure += seg.value;
            count++;
        }
    }
    return count > 0 ? totalPressure / count : 0;
}

bool BoundaryConditionManager::isPointInDomain(const glm::vec3& point) const {
    return point.x >= domain.minBound.x && point.x <= domain.maxBound.x &&
           point.y >= domain.minBound.y && point.y <= domain.maxBound.y &&
           point.z >= domain.minBound.z && point.z <= domain.maxBound.z;
}

bool BoundaryConditionManager::isPointNearBoundary(const glm::vec3& point, float tolerance) const {
    for (const auto& seg : boundarySegments) {
        float dist = glm::distance(point, seg.position);
        if (dist < tolerance) return true;
    }
    return false;
}

int BoundaryConditionManager::getNearestBoundarySegment(const glm::vec3& point) const {
    float minDist = std::numeric_limits<float>::max();
    int nearestIdx = -1;
    
    for (size_t i = 0; i < boundarySegments.size(); ++i) {
        float dist = glm::distance(point, boundarySegments[i].position);
        if (dist < minDist) {
            minDist = dist;
            nearestIdx = i;
        }
    }
    return nearestIdx;
}

float BoundaryConditionManager::getDistanceToBoundary(const glm::vec3& point) const {
    float minDist = std::numeric_limits<float>::max();
    for (const auto& seg : boundarySegments) {
        float dist = glm::distance(point, seg.position);
        minDist = std::min(minDist, dist);
    }
    return minDist;
}

float BoundaryConditionManager::getContainerVolume() const {
    return domain.getVolume();
}

float BoundaryConditionManager::getContainerSurfaceArea() const {
    glm::vec3 size = domain.getSize();
    return 2 * (size.x * size.y + size.y * size.z + size.z * size.x);
}

float BoundaryConditionManager::calculateMassFlowRateIn(float fluidDensity) const {
    float volumeFlowRate = 0;
    for (const auto& seg : boundarySegments) {
        if (seg.type == BoundaryType::Inlet) {
            volumeFlowRate += seg.area * seg.value;
        }
    }
    return fluidDensity * volumeFlowRate;
}

float BoundaryConditionManager::calculateMassFlowRateOut(float fluidDensity) const {
    float volumeFlowRate = 0;
    for (const auto& seg : boundarySegments) {
        if (seg.type == BoundaryType::Outlet) {
            volumeFlowRate += seg.area * seg.value;
        }
    }
    return fluidDensity * volumeFlowRate;
}

bool BoundaryConditionManager::isFlowBalanced(float fluidDensity, float tolerance) const {
    float massIn = calculateMassFlowRateIn(fluidDensity);
    float massOut = calculateMassFlowRateOut(fluidDensity);
    return std::abs(massIn - massOut) / std::max(massIn, massOut) < tolerance;
}

void BoundaryConditionManager::clearAllBoundaries() {
    boundarySegments.clear();
}

void BoundaryConditionManager::resetToDefault() {
    clearAllBoundaries();
    containerGeometry.type = GeometryType::Box;
    containerGeometry.dimensions = {2, 2, 2};
    domain = {{-1, -1, -1}, {1, 1, 1}, 0.1f, 20, 20, 20};
}

std::vector<size_t> BoundaryConditionManager::getBoundariesOfType(BoundaryType type) const {
    std::vector<size_t> indices;
    for (size_t i = 0; i < boundarySegments.size(); ++i) {
        if (boundarySegments[i].type == type) {
            indices.push_back(i);
        }
    }
    return indices;
}

BoundaryConditionManager::BoundaryVisualization BoundaryConditionManager::getVisualizationMesh() const {
    BoundaryVisualization vis;
    // Placeholder for visualization mesh generation
    return vis;
}

bool BoundaryConditionManager::exportToFile(const std::string& filename) const {
    // Placeholder for JSON export
    return true;
}

bool BoundaryConditionManager::importFromFile(const std::string& filename) {
    // Placeholder for JSON import
    return true;
}

void BoundaryConditionManager::initializeRectangularDomain() {
    if (containerGeometry.type == GeometryType::Box) {
        glm::vec3 dims = containerGeometry.dimensions;
        glm::vec3 pos = containerGeometry.position;
        
        domain.minBound = pos - dims * 0.5f;
        domain.maxBound = pos + dims * 0.5f;
    }
}

void BoundaryConditionManager::createWallSegments() {
    glm::vec3 size = domain.getSize();
    glm::vec3 center = domain.getCenter();
    
    // Create wall segments for each face
    // Bottom
    BoundarySegment bottom;
    bottom.type = BoundaryType::Wall;
    bottom.position = center + glm::vec3(0, -size.y/2, 0);
    bottom.normal = {0, 1, 0};
    bottom.area = size.x * size.z;
    bottom.isActive = true;
    addBoundarySegment(bottom);
    
    // Top
    BoundarySegment top;
    top.type = BoundaryType::Wall;
    top.position = center + glm::vec3(0, size.y/2, 0);
    top.normal = {0, -1, 0};
    top.area = size.x * size.z;
    top.isActive = true;
    addBoundarySegment(top);
}

void BoundaryConditionManager::updateGridResolution() {
    glm::vec3 size = domain.getSize();
    domain.xCells = std::max(1, (int)(size.x / domain.gridResolution));
    domain.yCells = std::max(1, (int)(size.y / domain.gridResolution));
    domain.zCells = std::max(1, (int)(size.z / domain.gridResolution));
}

// VesselLibrary implementations
VesselPreset VesselLibrary::getSwimmingPool() {
    VesselPreset pool;
    pool.name = "Swimming Pool";
    pool.geometry.type = GeometryType::Box;
    pool.geometry.dimensions = {50, 2, 25};  // Olympic pool dimensions
    pool.recommendedGridSize = 0.5f;
    return pool;
}

VesselPreset VesselLibrary::getInfinityPool() {
    VesselPreset pool;
    pool.name = "Infinity Pool";
    pool.geometry.type = GeometryType::Box;
    pool.geometry.dimensions = {100, 1.5f, 30};
    pool.recommendedGridSize = 0.25f;
    return pool;
}

VesselPreset VesselLibrary::getLabBeaker() {
    VesselPreset beaker;
    beaker.name = "Lab Beaker";
    beaker.geometry.type = GeometryType::Cylinder;
    beaker.geometry.radius = 0.05f;
    beaker.geometry.height = 0.1f;
    beaker.recommendedGridSize = 0.005f;
    return beaker;
}

VesselPreset VesselLibrary::getOilTank() {
    VesselPreset tank;
    tank.name = "Oil Tank";
    tank.geometry.type = GeometryType::Cylinder;
    tank.geometry.radius = 5;
    tank.geometry.height = 10;
    tank.recommendedGridSize = 0.2f;
    return tank;
}

VesselPreset VesselLibrary::getPipeline() {
    VesselPreset pipe;
    pipe.name = "Pipeline";
    pipe.geometry.type = GeometryType::Cylinder;
    pipe.geometry.radius = 0.3f;
    pipe.geometry.height = 1000;
    pipe.recommendedGridSize = 0.05f;
    return pipe;
}

VesselPreset VesselLibrary::getReactorVessel() {
    VesselPreset reactor;
    reactor.name = "Reactor Vessel";
    reactor.geometry.type = GeometryType::Cylinder;
    reactor.geometry.radius = 2;
    reactor.geometry.height = 5;
    reactor.recommendedGridSize = 0.1f;
    return reactor;
}

VesselPreset VesselLibrary::getChannelFlow() {
    VesselPreset channel;
    channel.name = "Channel Flow";
    channel.geometry.type = GeometryType::Box;
    channel.geometry.dimensions = {100, 1, 1};
    channel.recommendedGridSize = 0.1f;
    return channel;
}

} // namespace SZM::Fluids
