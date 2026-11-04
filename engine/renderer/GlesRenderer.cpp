#include "GlesRenderer.hpp"
#include "aether/aether_core.h"
#include <GLES3/gl3.h>
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <random>
#include <vector>
namespace aether {
namespace {
struct Vertex { float position[3]; float normal[3]; float uv[2]; };
GLuint compileShader(GLenum type, const char *source) {
    GLuint shader=glCreateShader(type); glShaderSource(shader,1,&source,nullptr); glCompileShader(shader);
    GLint ok=GL_FALSE; glGetShaderiv(shader,GL_COMPILE_STATUS,&ok);
    if(ok!=GL_TRUE){char log[2048]{};glGetShaderInfoLog(shader,sizeof(log),nullptr,log);aether_log(AETHER_LOG_ERROR,"GLSL compile: %s",log);glDeleteShader(shader);return 0;}
    return shader;
}
GLuint createProgram(const char *vs_source,const char *fs_source) {
    GLuint vs=compileShader(GL_VERTEX_SHADER,vs_source), fs=compileShader(GL_FRAGMENT_SHADER,fs_source);
    if(!vs||!fs){if(vs)glDeleteShader(vs);if(fs)glDeleteShader(fs);return 0;}
    GLuint p=glCreateProgram();glAttachShader(p,vs);glAttachShader(p,fs);glLinkProgram(p);glDeleteShader(vs);glDeleteShader(fs);
    GLint ok=GL_FALSE;glGetProgramiv(p,GL_LINK_STATUS,&ok);
    if(ok!=GL_TRUE){char log[2048]{};glGetProgramInfoLog(p,sizeof(log),nullptr,log);aether_log(AETHER_LOG_ERROR,"GLSL link: %s",log);glDeleteProgram(p);return 0;}
    return p;
}
void makePerspective(float *m,float aspect) {
    const float f=1.0f/std::tan(0.3926990817f), n=0.1f, z=80.0f;
    for(int i=0;i<16;++i)m[i]=0.0f;
    m[0]=f/std::max(aspect,0.1f);m[5]=f;m[10]=(z+n)/(n-z);m[11]=-1.0f;m[14]=(2.0f*z*n)/(n-z);
}
const char *planet_vs=R"GLSL(#version 300 es
precision highp float;
layout(location=0) in vec3 aPosition;
layout(location=1) in vec3 aNormal;
layout(location=2) in vec2 aUv;
uniform mat4 uProjection;
uniform float uTime;
out vec3 vNormal; out vec3 vWorld; out vec2 vUv;
void main(){
 float a=uTime*0.12; mat2 spin=mat2(cos(a),-sin(a),sin(a),cos(a));
 vec3 p=aPosition; p.xz=spin*p.xz; vec3 n=aNormal; n.xz=spin*n.xz;
 vec3 world=p*1.18+vec3(0.0,0.0,-3.35);
 vNormal=normalize(n);vWorld=world;vUv=aUv;gl_Position=uProjection*vec4(world,1.0);
})GLSL";
const char *planet_fs=R"GLSL(#version 300 es
precision highp float;
in vec3 vNormal;in vec3 vWorld;in vec2 vUv;out vec4 outColor;
float hash31(vec3 p){p=fract(p*vec3(0.1031,0.1030,0.0973));p+=dot(p,p.yxz+33.33);return fract((p.x+p.y)*p.z);}
float noise3(vec3 p){
 vec3 i=floor(p),f=fract(p);f=f*f*(3.0-2.0*f);
 float a=hash31(i+vec3(0,0,0)),b=hash31(i+vec3(1,0,0));
 float c=hash31(i+vec3(0,1,0)),d=hash31(i+vec3(1,1,0));
 float e=hash31(i+vec3(0,0,1)),g=hash31(i+vec3(1,0,1));
 float h=hash31(i+vec3(0,1,1)),j=hash31(i+vec3(1,1,1));
 return mix(mix(mix(a,b,f.x),mix(c,d,f.x),f.y),mix(mix(e,g,f.x),mix(h,j,f.x),f.y),f.z);
}
float fbm(vec3 p){float v=0.0,a=0.53;for(int i=0;i<5;++i){v+=a*noise3(p);p=p*2.03+vec3(7.1,3.7,5.4);a*=0.49;}return v;}
void main(){
 vec3 N=normalize(vNormal),V=normalize(-vWorld),L=normalize(vec3(-0.48,0.34,0.82));
 float broad=fbm(N*3.2),detail=fbm(N*15.0),height=broad*0.82+detail*0.13;
 vec3 deep=vec3(0.012,0.065,0.15),shallow=vec3(0.025,0.29,0.43);
 vec3 low=vec3(0.19,0.34,0.18),high=vec3(0.52,0.40,0.23);
 vec3 albedo=mix(deep,shallow,smoothstep(0.36,0.54,height));
 albedo=mix(albedo,low,smoothstep(0.48,0.55,height));
 albedo=mix(albedo,high,smoothstep(0.65,0.79,height));
 float polar=smoothstep(0.72,0.96,abs(N.y)+detail*0.06);
 albedo=mix(albedo,vec3(0.70,0.81,0.87),polar*0.78);
 float diffuse=max(dot(N,L),0.0);vec3 H=normalize(L+V);
 float spec=pow(max(dot(N,H),0.0),42.0)*0.52;
 float rim=pow(1.0-max(dot(N,V),0.0),3.2);
 vec3 color=albedo*(0.095+diffuse*1.55)+vec3(0.24,0.43,0.65)*spec+vec3(0.13,0.43,0.95)*rim*0.88;
 color=color/(color+vec3(1.0));color=pow(color,vec3(1.0/2.2));outColor=vec4(color,1.0);
})GLSL";
const char *star_vs=R"GLSL(#version 300 es
precision highp float;
layout(location=0) in vec4 aStar;
uniform float uTime;out float vAlpha;
void main(){gl_Position=vec4(aStar.xy,0.99,1.0);gl_PointSize=aStar.z*(0.78+0.22*sin(uTime*1.4+aStar.w));vAlpha=0.42+0.48*fract(aStar.w*0.17);}
)GLSL";
const char *star_fs=R"GLSL(#version 300 es
precision mediump float;
in float vAlpha;out vec4 outColor;
void main(){float d=length(gl_PointCoord-vec2(0.5));float glow=1.0-smoothstep(0.12,0.5,d);outColor=vec4(vec3(0.62,0.76,1.0),glow*vAlpha);}
)GLSL";
}
GlesRenderer &GlesRenderer::instance(){static GlesRenderer r;return r;}
bool GlesRenderer::initialize(){
 if(planet_program_!=0)return true;
 planet_program_=createProgram(planet_vs,planet_fs);star_program_=createProgram(star_vs,star_fs);
 if(!planet_program_||!star_program_)return false;
 projection_location_=glGetUniformLocation(planet_program_,"uProjection");
 time_location_=glGetUniformLocation(planet_program_,"uTime");
 constexpr int lat=48,lon=72;std::vector<Vertex> vertices;std::vector<std::uint32_t> indices;
 vertices.reserve((lat+1)*(lon+1));
 for(int y=0;y<=lat;++y){float v=(float)y/(float)lat,phi=v*3.14159265359f;
  for(int x=0;x<=lon;++x){float u=(float)x/(float)lon,theta=u*6.28318530718f;
   float px=std::sin(phi)*std::cos(theta),py=std::cos(phi),pz=std::sin(phi)*std::sin(theta);
   vertices.push_back({{px,py,pz},{px,py,pz},{u,v}});
  }
 }
 for(int y=0;y<lat;++y)for(int x=0;x<lon;++x){
  std::uint32_t a=(std::uint32_t)(y*(lon+1)+x),b=a+(std::uint32_t)(lon+1);
  indices.insert(indices.end(),{a,b,a+1u,a+1u,b,b+1u});
 }
 index_count_=(int)indices.size();glGenVertexArrays(1,&planet_vao_);glGenBuffers(1,&planet_vbo_);glGenBuffers(1,&planet_ibo_);
 glBindVertexArray(planet_vao_);glBindBuffer(GL_ARRAY_BUFFER,planet_vbo_);
 glBufferData(GL_ARRAY_BUFFER,(GLsizeiptr)(vertices.size()*sizeof(Vertex)),vertices.data(),GL_STATIC_DRAW);
 glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,planet_ibo_);
 glBufferData(GL_ELEMENT_ARRAY_BUFFER,(GLsizeiptr)(indices.size()*sizeof(std::uint32_t)),indices.data(),GL_STATIC_DRAW);
 glEnableVertexAttribArray(0);glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,sizeof(Vertex),(void*)offsetof(Vertex,position));
 glEnableVertexAttribArray(1);glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,sizeof(Vertex),(void*)offsetof(Vertex,normal));
 glEnableVertexAttribArray(2);glVertexAttribPointer(2,2,GL_FLOAT,GL_FALSE,sizeof(Vertex),(void*)offsetof(Vertex,uv));glBindVertexArray(0);
 std::mt19937 rng(0xAE7E2u);std::uniform_real_distribution<float> xy(-1.0f,1.0f),sz(1.0f,2.8f),ph(0.0f,6.2831853f);
 std::vector<float> stars;stars.reserve(420*4);
 for(int i=0;i<420;++i){stars.push_back(xy(rng));stars.push_back(xy(rng));stars.push_back(sz(rng));stars.push_back(ph(rng));}
 star_count_=420;glGenVertexArrays(1,&star_vao_);glGenBuffers(1,&star_vbo_);glBindVertexArray(star_vao_);
 glBindBuffer(GL_ARRAY_BUFFER,star_vbo_);glBufferData(GL_ARRAY_BUFFER,(GLsizeiptr)(stars.size()*sizeof(float)),stars.data(),GL_STATIC_DRAW);
 glEnableVertexAttribArray(0);glVertexAttribPointer(0,4,GL_FLOAT,GL_FALSE,sizeof(float)*4,nullptr);glBindVertexArray(0);
 glEnable(GL_DEPTH_TEST);glDisable(GL_CULL_FACE);
 aether_log(AETHER_LOG_INFO,"OpenGL ES 3 renderer initialized");return true;
}
void GlesRenderer::resize(int width,int height){
 int w=std::max(width,1),h=std::max(height,1);glViewport(0,0,w,h);makePerspective(projection_,(float)w/(float)h);
}
void GlesRenderer::render(float time){
 glClearColor(0.012f,0.018f,0.038f,1.0f);glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
 glDisable(GL_DEPTH_TEST);glEnable(GL_BLEND);glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
 glUseProgram(star_program_);glUniform1f(glGetUniformLocation(star_program_,"uTime"),time);
 glBindVertexArray(star_vao_);glDrawArrays(GL_POINTS,0,star_count_);glBindVertexArray(0);glDisable(GL_BLEND);
 glEnable(GL_DEPTH_TEST);glUseProgram(planet_program_);glUniformMatrix4fv(projection_location_,1,GL_FALSE,projection_);
 glUniform1f(time_location_,time);glBindVertexArray(planet_vao_);glDrawElements(GL_TRIANGLES,index_count_,GL_UNSIGNED_INT,nullptr);glBindVertexArray(0);
}
}
