import canta;

GROUP_SIZE(32, 1, 1);
[vk::constant_id(SPEC_CONSTANT_INDEX(0))] const int screen_width = 1920;

struct Particle {
    float2 position;
    float2 velocity;
    float3 colour;
    int radius;
};

bool outOfBounds(float2 position) {
    if (position.x > screen_width || position.x < 0 || position.y > 1080 || position.y < 0)
        return true;

    return false;
}

struct Push {
    Particle* particles;
    int maxParticles;
    float dt;
}

[shader("compute")]
NUM_THREADS
void moveMain(
    uint3 threadId: SV_DispatchThreadID,
    uniform Particle* particles,
    uniform int maxParticles,
    uniform float dt
) {
    const uint idx = threadId.x;
    if (idx >= maxParticles)
        return;
    Particle particle = particles[idx];

    float2 newPosition = particle.position + particle.velocity * dt / 10;
    if (outOfBounds(newPosition)) {
        const float2 screenCenter = float2(screen_width / 2, 1080 / 2);

        float2 direction = normalize(screenCenter - newPosition);
        float2 newVelocity = direction * length(particle.velocity);
        particle.velocity = newVelocity;
    }

    float2 randDir = canta.rand2(float2(dt, idx));

    particle.position = newPosition + randDir;
    particles[idx] = particle;
}

[shader("compute")]
NUM_THREADS
void drawMain(
    uint3 threadId: SV_DispatchThreadID,
    uniform Particle* particles,
    uniform int imageIndex,
    uniform int maxParticles
) {
    const uint idx = threadId.x;
    if (idx >= maxParticles)
        return;

    Particle particle = particles[idx];
    float4 colour = float4(particle.colour, 1);

    var outputImage = canta.RWImage2D<float4>(imageIndex);

    for (int x = -particle.radius; x < particle.radius; x++) {
        for (int y = -particle.radius; y < particle.radius; y++) {
            int2 position = int2(particle.position) + int2(x, y);
            if (distance(particle.position, position) > particle.radius)
                continue;
            outputImage[position] = colour;
        }
    }
}