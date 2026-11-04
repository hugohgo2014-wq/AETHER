#pragma once
namespace aether {
class GlesRenderer final {
public:
    static GlesRenderer &instance();
    bool initialize();
    void resize(int width, int height);
    void render(float time_seconds);
private:
    GlesRenderer() = default;
    unsigned int planet_program_ = 0, star_program_ = 0;
    unsigned int planet_vao_ = 0, planet_vbo_ = 0, planet_ibo_ = 0;
    unsigned int star_vao_ = 0, star_vbo_ = 0;
    int index_count_ = 0, star_count_ = 0;
    int projection_location_ = -1, time_location_ = -1;
    float projection_[16] {};
};
}
