
struct Particle {
    float2 position;
    float2 velocity;
    float3 colour;
    int radius;
};

bool outOfBounds(float2 position) {
    if (position.x > 1920 || position.x < 0 || position.y > 1080 || position.y < 0)
        return true;

    return false;
}

struct Push {
    Particle* particles;
    int maxParticles;
    float dt;
}

[vk::push_constant]
Push push;

[shader("compute")]
[numthreads(32, 1, 1)]
void main(uint3 threadId: SV_DispatchThreadID) {
    const uint idx = threadId.x;
    if (idx >= push.maxParticles)
        return;
    Particle particle = push.particles[idx];

    float2 newPosition = particle.position + particle.velocity * push.dt / 10;
    if (outOfBounds(newPosition)) {
        const float2 screenCenter = float2(1920 / 2, 1080 / 2);

        float2 direction = normalize(screenCenter - newPosition);
        float2 newVelocity = direction * length(particle.velocity);
        particle.velocity = newVelocity;
    }

    particle.position = newPosition;
    push.particles[idx] = particle;
}
