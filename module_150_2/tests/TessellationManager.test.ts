import { TessellationManager } from "../core/TessellationManager";
import { EventBusImpl } from "../systems/EventBusImpl";
import { RenderConfig } from "../types";

export function runTests() {
  const manager = new TessellationManager();
  const eventBus = new EventBusImpl();
  
  const config: RenderConfig = {
    maxMeshes: 10,
    maxVerticesPerMesh: 100000
  };

  const initRes = manager.initialize(config, eventBus);
  if (!initRes.success) throw new Error("Init failed");

  const createRes = manager.tessellateBody({
    id: "mesh_1",
    sourceBodyId: "brep_55",
    vertexCount: 1500,
    indexCount: 4500,
    material: { albedo: 0xFFFFFF, metallic: 0.5, roughness: 0.5 }
  });
  if (!createRes.success) throw new Error("Create failed");

  const geometryFailRes = manager.tessellateBody({
    id: "mesh_2",
    sourceBodyId: "brep_56",
    vertexCount: 0,
    indexCount: 0,
    material: { albedo: 0xFF0000, metallic: 0.5, roughness: 0.5 }
  });
  if (geometryFailRes.success) throw new Error("Geometry invariant failure missed");

  const pbrFailRes = manager.updateMaterial("mesh_1", {
    albedo: 0x00FF00,
    metallic: 1.5,
    roughness: 0.2
  });
  if (pbrFailRes.success) throw new Error("PBR bounds invariant failure missed");

  const validMaterialRes = manager.updateMaterial("mesh_1", {
    albedo: 0x000000,
    metallic: 1.0,
    roughness: 0.0
  });
  if (!validMaterialRes.success) throw new Error("Valid material update failed");

  const destroyRes = manager.destroyMesh("mesh_1");
  if (!destroyRes.success) throw new Error("Destroy failed");

  const snapshot = manager.getStateSnapshot();
  if (snapshot.length !== 0) throw new Error("Snapshot invalid state");

  console.log("All invariant constraints & atomic tests passed.");
}

