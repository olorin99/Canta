import canta;

struct Ray {
    float3 origin;
    float3 direction;

    float3 at(float t) {
        return origin + (direction * t);
    }
}

struct RayCamera {

    func getRay(float u, float v) -> Ray {
        return { _origin, _lowerLeft + (_horizonal * u) + (_vertical * v) - _origin };
    }

    float3 _origin;
    float3 _lowerLeft;
    float3 _horizonal;
    float3 _vertical;

};

struct Hit {
    float3 point;
    float3 normal;
    float t;
    bool frontFace;

    [mutating]
    void setFaceNormal(Ray ray, float3 outNormal) {
        frontFace = dot(ray.direction, outNormal) < 0;
        normal = frontFace ? outNormal : outNormal * -1;
    }
}

struct Sphere {
    float4 value;

    bool hit(Ray ray, float tMin, float tMax, inout Hit hit) {
        var oc = ray.origin - value.xyz;
        var a = length(ray.direction) * length(ray.direction);
        var b = dot(oc, ray.direction);
        var c = length(oc) * length(oc) - (value.w * value.w);
        var discriminant = b * b - a * c;
        if (discriminant < 0) return false;
        var sqrtd = sqrt(discriminant);
        var root = (-b - sqrtd) / a;
        if (root < tMin || tMax < root) {
            root = (-b + sqrtd) / a;
            if (root < tMin || tMax < root) return false;
        }

        hit.t = root;
        hit.point = ray.at(root);
        var outNormal = (hit.point - value.xyz) / value.w;
        hit.setFaceNormal(ray, outNormal);
        return true;
    }
}

bool checkHit(Sphere* spheres, uint count, Ray ray, float tMin, float tMax, inout Hit hit) {
    bool hitAnything = false;
    var closest = tMax;
    Hit tmp;

    for (uint i = 0; i < count; i++) {
        Sphere sphere = spheres[i];
        if (sphere.hit(ray, tMin, closest, tmp)) {
            hitAnything = true;
            closest = hit.t;
            hit = tmp;
        }
    }

    return hitAnything;
}


[shader("compute")]
[numthreads(32, 32, 1)]
void traceMain(
    uint3 threadId : SV_DispatchThreadID,
    uniform Sphere* spheres,
    uniform canta.RWImage2D<float4> backbuffer,
    uniform uint sphereCount,
    uniform RayCamera camera,
    uniform uint imageWidth,
    uniform uint imageHeight
) {

    if (any(threadId.xy >= uint2(imageWidth, imageHeight)))
        return;

    float u = (float)threadId.x / (imageWidth - 1);
    float v = (float)threadId.y / (imageHeight - 1);
    var ray = camera.getRay(u, v);

    Hit hit;
    var colour = float4(0);
    if (checkHit(spheres, sphereCount, ray, 0, 100000, hit)) {
        colour = float4((hit.normal + 1) * 0.5, 1.0);
    } else {
        var unitDirection = normalize(ray.direction);
        hit.t = 0.5f * (unitDirection.y + 1.f);
        colour = lerp(float4(1.0, 1.0, 1.0, 1.0), float4(0.5f, 0.7f, 1.0f, 1.0f), hit.t);
    }

    backbuffer[threadId.xy] = colour;
}