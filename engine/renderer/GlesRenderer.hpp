#pragma once
#include "aether/runtime/ProjectLoader.hpp"
namespace aether {
class GlesRenderer final {
public:
    static GlesRenderer &instance();
    bool initialize();
    void resize(int width,int height);
    void render(const ProjectSettings &project,float time_seconds,bool debug_tools);
    int drawCalls() const noexcept { return draw_calls_; }
    int visibleObjects() const noexcept { return visible_objects_; }
private:
    GlesRenderer()=default;
    unsigned int program_=0,cube_vao_=0,sphere_vao_=0,plane_vao_=0,grid_vao_=0;
    int sphere_index_count_=0,grid_vertex_count_=0;
    int model_location_=-1,mvp_location_=-1,color_location_=-1;
    int draw_calls_=0,visible_objects_=0;
    float projection_[16]{};
    int width_=1,height_=1;
};
}
