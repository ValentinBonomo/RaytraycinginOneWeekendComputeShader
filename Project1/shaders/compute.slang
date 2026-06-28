[[vk::binding(0, 0)]] Sampler2D   textures[];
[[vk::binding(1, 0)]] RWTexture2D storageTextures[];

struct Sphere
{
    float3 center;
    float  radius;
    float3 albedo;
    float  fuzz;
    uint   matType;
    // Refractive index in vacuum or air, or the ratio of the material's refractive index over
    // the refractive index of the enclosing media
    float  refractionIndex;
};
[[vk::binding(2, 0)]] StructuredBuffer<Sphere> scenes[];

struct PushConstants
{
    uint2 imageSize;
    uint  outputId;
    uint  sceneId;
    uint  sphereCount;
    uint  samplesPerPixel;
    uint  maxDepth;

    float3 cameraCenter;
    float3 pixel00Loc;
    float3 pixelDeltaU;
    float3 pixelDeltaV;
    float3 defocusDiskU;
    float3 defocusDiskV;
};
[[vk::push_constant]] ConstantBuffer<PushConstants> pc;

static const float INFINITY = 1e30;

uint pcg_hash(uint v)
{
    uint state = v * 747796405u + 2891336453u;
    uint word  = ((state >> ((state >> 28u) + 4u)) ^ state) * 277803737u;
    return (word >> 22u) ^ word;
}

float random_float(inout uint state)
{
    state = pcg_hash(state);
    return float(state) * (1.0 / 4294967296.0);
}

// Returns the vector to a random point in the [-.5,-.5]-[+.5,+.5] unit square.
float2 sample_square(inout uint state)
{
    return float2(random_float(state) - 0.5, random_float(state) - 0.5);
}

float3 random_unit_vector(inout uint state)
{
    for (int i = 0; i < 64; i++)
    {
        float3 p = float3(random_float(state) * 2.0 - 1.0,
                          random_float(state) * 2.0 - 1.0,
                          random_float(state) * 2.0 - 1.0);
        float lensq = dot(p, p);
        if (lensq > 1e-30 && lensq <= 1.0)
            return p / sqrt(lensq);
    }
    return float3(0.0, 0.0, 1.0);
}

// Returns a random point in the camera defocus disk.
float3 random_in_unit_disk(inout uint state)
{
    for (int i = 0; i < 64; i++)
    {
        float3 p = float3(random_float(state) * 2.0 - 1.0,
                          random_float(state) * 2.0 - 1.0,
                          0.0);
        if (dot(p, p) < 1.0)
            return p;
    }
    return float3(0.0, 0.0, 0.0);
}

// Return true if the vector is close to zero in all dimensions.
bool near_zero(float3 v)
{
    const float s = 1e-8;
    return abs(v.x) < s && abs(v.y) < s && abs(v.z) < s;
}

// Use Schlick's approximation for reflectance.
float reflectance(float cosine, float refraction_index)
{
    float r0 = (1.0 - refraction_index) / (1.0 + refraction_index);
    r0 = r0 * r0;
    return r0 + (1.0 - r0) * pow(1.0 - cosine, 5.0);
}

struct Ray
{
    float3 origin;
    float3 direction;
};

float3 ray_at(Ray r, float t)
{
    return r.origin + t * r.direction;
}

struct HitRecord
{
    float3 p;
    float3 normal;
    float  t;
    bool   front_face;
    float3 albedo;
    float  fuzz;
    uint   matType;
    float  refractionIndex;
};

void set_face_normal(inout HitRecord rec, Ray r, float3 outward_normal)
{
    // Sets the hit record normal vector.
    // NOTE: the parameter `outward_normal` is assumed to have unit length.

    rec.front_face = dot(r.direction, outward_normal) < 0.0;
    rec.normal     = rec.front_face ? outward_normal : -outward_normal;
}

bool hit_sphere(Sphere s, Ray r, float t_min, float t_max, inout HitRecord rec)
{
    float3 oc = s.center - r.origin;
    float a = dot(r.direction, r.direction);
    float h = dot(r.direction, oc);
    float c = dot(oc, oc) - s.radius * s.radius;

    float discriminant = h * h - a * c;
    if (discriminant < 0.0)
        return false;

    float sqrtd = sqrt(discriminant);

    // Find the nearest root that lies in the acceptable range.
    float root = (h - sqrtd) / a;
    if (root <= t_min || t_max <= root)
    {
        root = (h + sqrtd) / a;
        if (root <= t_min || t_max <= root)
            return false;
    }

    rec.t = root;
    rec.p = ray_at(r, root);
    float3 outward_normal = (rec.p - s.center) / s.radius;
    set_face_normal(rec, r, outward_normal);

    rec.albedo          = s.albedo;
    rec.fuzz            = s.fuzz;
    rec.matType         = s.matType;
    rec.refractionIndex = s.refractionIndex;
    return true;
}

bool hit_world(Ray r, float t_min, float t_max, inout HitRecord rec)
{
    HitRecord temp;
    bool  hit_anything = false;
    float closest      = t_max;

    for (uint i = 0; i < pc.sphereCount; i++)
    {
        Sphere s = scenes[pc.sceneId][i];
        if (hit_sphere(s, r, t_min, closest, temp))
        {
            hit_anything = true;
            closest      = temp.t;
            rec          = temp;
        }
    }

    return hit_anything;
}

float3 ray_color(Ray r, inout uint state)
{
    float3 throughput = float3(1.0, 1.0, 1.0);
    Ray    current    = r;

    for (uint depth = 0; depth < pc.maxDepth; depth++)
    {
        HitRecord rec;
        if (hit_world(current, 0.001, INFINITY, rec))
        {
            float3 dir;
            if (rec.matType == 1u)
            {
                float3 reflected = reflect(normalize(current.direction), rec.normal);
                dir = reflected + rec.fuzz * random_unit_vector(state);
                if (dot(dir, rec.normal) <= 0.0)
                    return float3(0.0, 0.0, 0.0);
            }
            else if (rec.matType == 2u)
            {
                float  ri        = rec.front_face ? (1.0 / rec.refractionIndex) : rec.refractionIndex;
                float3 unit_dir  = normalize(current.direction);
                float  cos_theta = min(dot(-unit_dir, rec.normal), 1.0);
                float  sin_theta = sqrt(1.0 - cos_theta * cos_theta);

                bool cannot_refract = ri * sin_theta > 1.0;
                if (cannot_refract || reflectance(cos_theta, ri) > random_float(state))
                    dir = reflect(unit_dir, rec.normal);
                else
                    dir = refract(unit_dir, rec.normal, ri);
            }
            else
            {
                dir = rec.normal + random_unit_vector(state);
                // Catch degenerate scatter direction
                if (near_zero(dir))
                    dir = rec.normal;
            }

            current.origin    = rec.p;
            current.direction = dir;
            throughput       *= rec.albedo;
        }
        else
        {
            float3 unit_direction = normalize(current.direction);
            float  a   = 0.5 * (unit_direction.y + 1.0);
            float3 sky = (1.0 - a) * float3(1.0, 1.0, 1.0) + a * float3(0.5, 0.7, 1.0);
            return throughput * sky;
        }
    }

    return float3(0.0, 0.0, 0.0);
}

[shader("compute")]
[numthreads(16, 16, 1)]
void cs_main(uint3 threadId : SV_DispatchThreadID)
{
    if (threadId.x >= pc.imageSize.x || threadId.y >= pc.imageSize.y)
        return;

    uint image_width = pc.imageSize.x;

    uint rngState = pcg_hash(threadId.x + threadId.y * image_width);

    uint   samples     = max(pc.samplesPerPixel, 1u);
    float3 pixel_color = float3(0, 0, 0);

    for (uint s = 0; s < samples; s++)
    {
        // Construct a camera ray originating from the defocus disk and directed at a randomly
        // sampled point around the pixel location i, j.
        float2 offset = sample_square(rngState);
        float3 pixel_sample = pc.pixel00Loc
                            + (float(threadId.x) + offset.x) * pc.pixelDeltaU
                            + (float(threadId.y) + offset.y) * pc.pixelDeltaV;

        float3 lens   = random_in_unit_disk(rngState);
        float3 origin = pc.cameraCenter + lens.x * pc.defocusDiskU + lens.y * pc.defocusDiskV;

        Ray r;
        r.origin    = origin;
        r.direction = pixel_sample - origin;

        pixel_color += ray_color(r, rngState);
    }
    pixel_color /= float(samples);

    storageTextures[pc.outputId][threadId.xy] = float4(pixel_color, 1.0);
}
