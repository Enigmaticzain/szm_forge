# Implementation Plan 5: Machine Learning System Setup (AI Generation)

## Objective
SZM Forge has a powerful AI architecture outlined in `MACHINE_LEARNING_SYSTEM.md`. We need to wire up the newly introduced `MachineLearningSystem` inside the C++ backend and Python AI service so that the AI can ingest CAD/Machine data, learn from it, and generate new machines (e.g., CNC Mills) autonomously.

## Proposed Changes

### 1. Python ML Training Integration (`ai_service/machine_learning_training.py`)
- **Action:** Ensure the `machine_data/` directory exists with valid JSON datasets (e.g., sample CNC Mills, Robot Arms).
- **Action:** Expose a FastAPI endpoint `/ai/train` that triggers the dataset ingestion and pattern learning.
- **Action:** Expose a FastAPI endpoint `/ai/generate` that accepts a `type` and `parameters` and returns a generated JSON machine definition.

### 2. C++ AI Integration (`src/AI/MachineLearningSystem/MachineLearningSystem.hpp`)
- **Action:** In `Application.cpp`, during initialization, ensure the ML system is initialized (`SZM::AI::Machines::MachineLearningSystem::GetInstance().Initialize()`).
- **Action:** Expose the AI generation capabilities to the React UI through the `FastAPIBridge`. When a user asks the AI to "Generate a CNC Mill", the C++ backend routes this to the Python `/ai/generate` endpoint.

### 3. Generated Machine to ECS Conversion
- **Action:** When the AI generates a new machine JSON, write a parser that converts this definition into a collection of ECS entities.
- **Action:** For example, a generated CNC mill creates a parent `Entity` (the base) and child entities for the Spindle, Table, and Axes, populated with correct `TransformComponent`s and `MeshComponent`s.

## Verification Plan
1. Send a request to `/ai/train` and verify the Python service successfully learns from the `machine_data` folder.
2. Request a generation via the UI chat interface: "Generate a CNC mill with 6000 RPM".
3. Verify the generated machine is added to the ECS Scene Graph and renders properly in the 3D viewport.
