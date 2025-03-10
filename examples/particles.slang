import canta;

layout (constant_id = 0) const int x_size = 32;
layout (constant_id = 1) const int screen_width = 1920;

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
[numthreads(x_size, 1, 1)]
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

    particle.position = newPosition;
    particles[idx] = particle;
}

[shader("compute")]
[numthreads(x_size, 1, 1)]
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

    var outputImage = RWImage2D<float4>(imageIndex);

    for (int x = -particle.radius; x < particle.radius; x++) {
        for (int y = -particle.radius; y < particle.radius; y++) {
            int2 position = int2(particle.position) + int2(x, y);
            if (distance(particle.position, position) > particle.radius)
                continue;
            outputImage[position] = colour;
        }
    }
}