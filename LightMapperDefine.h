#pragma once

#define GLM_ENABLE_EXPERIMENTAL
#include <glm.hpp>
#include <gtx/quaternion.hpp>
#include <gtc/matrix_transform.hpp>

static float gInfinity = std::numeric_limits<float>::infinity();
static float gNegInfinity = -gInfinity;
static float gEpsilon = std::numeric_limits<float>::epsilon();

#define SAFE_DELETE(x) \
    { \
        delete x; \
        x = nullptr; \
    }

#define SAFE_DELETE_ARRAY(x) \
    { \
        delete[] x; \
        x = nullptr; \
    }

#define MAX_GRID_SIZE 128

inline uint32_t murmur3(const uint32_t* key, size_t wordCount, uint32_t seed) noexcept {
    uint32_t h = seed;
    size_t i = wordCount;
    do {
        uint32_t k = *key++;
        k *= 0xcc9e2d51u;
        k = (k << 15u) | (k >> 17u);
        k *= 0x1b873593u;
        h ^= k;
        h = (h << 13u) | (h >> 19u);
        h = (h * 5u) + 0xe6546b64u;
    } while (--i);
    h ^= wordCount;
    h ^= h >> 16u;
    h *= 0x85ebca6bu;
    h ^= h >> 13u;
    h *= 0xc2b2ae35u;
    h ^= h >> 16u;
    return h;
}

template<typename T>
struct MurmurHash {
    uint32_t operator()(const T& key) const noexcept {
        static_assert(0 == (sizeof(key) & 3u), "Hashing requires a size that is a multiple of 4.");
        return murmur3((const uint32_t*) &key, sizeof(key) / 4, 0);
    }
};

#define LIGHT_TYPE_DIRECTIONAL 0
#define LIGHT_TYPE_OMNI 1
#define LIGHT_TYPE_SPOT 2

struct Light {
    glm::vec3 position;
    uint32_t type;
    glm::vec4 direction_energy;
    glm::vec4 color;
    float range;
    float attenuation;
    float cos_spot_angle;
    float inv_spot_attenuation;
};

struct Vertex {
    glm::vec4 position;
    glm::vec4 normal;
    glm::vec2 uv0;
    glm::vec2 uv1;
};

struct MeshVertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 uv0;
    glm::vec2 uv1;
};

struct Edge {
    glm::vec3 a;
    glm::vec3 b;
    glm::vec3 na;
    glm::vec3 nb;

    bool operator==(const Edge &p_seam) const {
        return a == p_seam.a && b == p_seam.b && na == p_seam.na && nb == p_seam.nb;
    }

    Edge() {
    }

    Edge(const glm::vec3& a, const glm::vec3& b, const glm::vec3& na, const glm::vec3& nb) {
        this->a = a;
        this->b = b;
        this->na = na;
        this->nb = nb;
    }
};

struct EdgeUV {
    glm::vec2 a;
    glm::vec2 b;
    glm::ivec2 indices;
    bool seam_found = false;

    bool operator==(const EdgeUV& uv) const {
        return a == uv.a && b == uv.b;
    }

    EdgeUV() {}

    EdgeUV(const glm::vec2& a, const glm::vec2& b, const glm::ivec2& indices) {
        this->a = a;
        this->b = b;
        this->indices = indices;
    }
};

struct EdgeEq  {
    bool operator()(const Edge& edge_a, const Edge& edge_b) const {
        if (edge_a.a.x != edge_b.a.x) return false;
        if (edge_a.a.y != edge_b.a.y) return false;
        if (edge_a.a.z != edge_b.a.z) return false;
        if (edge_a.b.x != edge_b.b.x) return false;
        if (edge_a.b.y != edge_b.b.y) return false;
        if (edge_a.b.z != edge_b.b.z) return false;
        return true;
    }
};

struct Seam {
    glm::ivec2 a;
    glm::ivec2 b;
};

struct Triangle {
    uint32_t indices[4] = {};
    float min_bounds[4] = {};
    float max_bounds[4] = {};
};

struct TriangleSort {
    uint32_t cell_index = 0;
    uint32_t triangle_index = 0;
    bool operator<(const TriangleSort &p_triangle_sort) const {
        return cell_index < p_triangle_sort.cell_index;
    }
};

class AABB {
public:
    glm::vec3 min = glm::vec3(gInfinity, gInfinity, gInfinity);
    glm::vec3 max = glm::vec3(gNegInfinity, gNegInfinity, gNegInfinity);

    bool operator==(const AABB &other) const {
        return ((min == other.min) && (max == other.max));
    }

    bool operator!=(const AABB &other) const {
        return ((min != other.min) || (max != other.max));
    }

    void Merge(const AABB &other) {
        glm::vec3 beg_1, beg_2;
        glm::vec3 end_1, end_2;
        beg_1 = min;
        beg_2 = other.min;
        end_1 = max;
        end_2 = other.max;

        min.x = (beg_1.x < beg_2.x) ? beg_1.x : beg_2.x;
        min.y = (beg_1.y < beg_2.y) ? beg_1.y : beg_2.y;
        min.z = (beg_1.z < beg_2.z) ? beg_1.z : beg_2.z;

        max.x = (end_1.x > end_2.x) ? end_1.x : end_2.x;
        max.y = (end_1.y > end_2.y) ? end_1.y : end_2.y;
        max.z = (end_1.z > end_2.z) ? end_1.z : end_2.z;
    }

    void Grow(float amount) {
        min.x -= amount;
        min.y -= amount;
        min.z -= amount;
        max.x += amount;
        max.y += amount;
        max.z += amount;
    }

    void Expand(const glm::vec3& point) {
        min.x = glm::min(min.x, point.x);
        min.y = glm::min(min.y, point.y);
        min.z = glm::min(min.z, point.z);
        max.x = glm::max(max.x, point.x);
        max.y = glm::max(max.y, point.y);
        max.z = glm::max(max.z, point.z);
    }

    glm::vec3 GetSize() {
        return max - min;
    }

    glm::vec3 GetCenter() {
        return (max + min) * 0.5f;
    }

    glm::vec3 GetSize() const {
        return max - min;
    }

    glm::vec3 GetCenter() const {
        return (max + min) * 0.5f;
    }
};