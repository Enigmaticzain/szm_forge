const {
  app,
  advanceJobQueue,
  resetState,
  stopJobWorker,
} = require('./server');

function getRouteHandler(method, path) {
  const normalizedMethod = method.toLowerCase();
  const layer = app._router.stack.find(
    (entry) =>
      entry.route
      && entry.route.path === path
      && entry.route.methods[normalizedMethod],
  );

  if (!layer) {
    throw new Error(`Route ${method.toUpperCase()} ${path} not found`);
  }

  return layer.route.stack[layer.route.stack.length - 1].handle;
}

function createResponse() {
  return {
    statusCode: 200,
    body: null,
    status(code) {
      this.statusCode = code;
      return this;
    },
    json(payload) {
      this.body = payload;
      return this;
    },
  };
}

async function invokeRoute(method, path, request = {}) {
  const handler = getRouteHandler(method, path);
  const response = createResponse();

  await handler(
    {
      body: request.body || {},
      params: request.params || {},
      query: request.query || {},
    },
    response,
  );

  return response;
}

describe('SZM Forge backend integration routes', () => {
  beforeEach(() => {
    resetState();
  });

  afterAll(() => {
    stopJobWorker();
  });

  test('creates a custom material and exposes it in the catalog', async () => {
    const createResponse = await invokeRoute('post', '/api/materials', {
      body: {
        name: 'Carbon Fiber Composite',
        family: 'Composite',
        density: 1600,
        youngs_modulus: 135,
        poissons_ratio: 0.28,
        yield_strength: 620,
        ultimate_strength: 780,
        thermal_conductivity: 12,
        thermal_expansion: 0.000002,
        tags: ['Lightweight', 'High stiffness'],
      },
    });

    expect(createResponse.statusCode).toBe(201);
    expect(createResponse.body.material).toMatchObject({
      name: 'Carbon Fiber Composite',
      family: 'Composite',
      density: 1600,
    });

    const listResponse = await invokeRoute('get', '/api/materials', {
      query: { filter: 'carbon' },
    });

    expect(listResponse.statusCode).toBe(200);
    expect(
      listResponse.body.some((material) => material.name === 'Carbon Fiber Composite'),
    ).toBe(true);
  });

  test('creates a workshop component, adds it to the assembly, and removes it again', async () => {
    const createComponent = await invokeRoute('post', '/api/components', {
      body: {
        name: 'Test Gusset',
        shape: 'box',
        materialId: 'MAT-STEEL-STRUCT',
        category: 'Fixture',
        description: 'Created during integration testing.',
        size: [1.4, 0.6, 0.8],
      },
    });

    expect(createComponent.statusCode).toBe(201);
    const componentId = createComponent.body.component.id;

    const addInstance = await invokeRoute('post', '/api/assemblies/:assemblyId/instances', {
      params: { assemblyId: 'assy_actuated_bracket' },
      body: {
        partId: componentId,
        name: 'Test Gusset Instance',
      },
    });

    expect(addInstance.statusCode).toBe(201);
    expect(addInstance.body.assembly.instanceCount).toBeGreaterThan(6);

    const instanceId = addInstance.body.instance.id;

    const removeInstance = await invokeRoute(
      'delete',
      '/api/assemblies/:assemblyId/instances/:instanceId',
      {
        params: {
          assemblyId: 'assy_actuated_bracket',
          instanceId,
        },
      },
    );

    expect(removeInstance.statusCode).toBe(200);
    expect(removeInstance.body.status).toBe('removed');
    expect(
      removeInstance.body.assembly.instances.some((instance) => instance.id === instanceId),
    ).toBe(false);
  });

  test('submits a job against a selected assembly component and advances it to completion', async () => {
    const jobResponse = await invokeRoute('post', '/api/jobs', {
      body: {
        templateId: 'structural_static',
        componentId: 4821,
      },
    });

    expect(jobResponse.statusCode).toBe(201);
    const jobId = jobResponse.body.jobId;

    for (let index = 0; index < 20; index += 1) {
      advanceJobQueue();
    }

    const getJob = await invokeRoute('get', '/api/jobs/:jobId', {
      params: { jobId },
    });

    expect(getJob.statusCode).toBe(200);
    expect(getJob.body.status).toBe('completed');
    expect(getJob.body.componentName).toBe('Frame Base');
    expect(getJob.body.result).toMatchObject({
      success: true,
      target: 'Frame Base',
    });
  });

  test('returns enriched scene metadata for advanced viewer controls', async () => {
    const response = await invokeRoute('get', '/api/scene');

    expect(response.statusCode).toBe(200);
    expect(response.body).toMatchObject({
      title: 'Actuated Bracket Study',
      metrics: {
        partCount: 6,
        materialCount: 4,
        categoryCount: 4,
        analysisModeCount: 8,
      },
      spatial: {
        center: expect.any(Array),
        size: expect.any(Array),
        radius: expect.any(Number),
      },
    });

    expect(response.body.parts).toHaveLength(6);
    expect(response.body.parts[0]).toMatchObject({
      name: 'Frame Base',
      materialId: 'MAT-STEEL-STRUCT',
      materialName: 'Structural Steel',
      source: 'Workshop primitive',
      mass: 24.8,
      analysisModes: expect.arrayContaining(['Structural']),
      bounds: {
        min: expect.any(Array),
        max: expect.any(Array),
        size: expect.any(Array),
        center: expect.any(Array),
      },
      explodeDirection: expect.any(Array),
    });
  });
});
