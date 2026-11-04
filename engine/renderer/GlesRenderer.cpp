#include "GlesRenderer.hpp"
#include "aether/aether_core.h"
#include <GLES3/gl3.h>
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <vector>
namespace aether {
namespace {
struct Vertex{float p[3];float n[3];};
GLuint shader(GLenum type,const char *src){
    GLuint s=glCreateShader(type);glShaderSource(s,1,&src,nullptr);glCompileShader(s);
    GLint ok=GL_FALSE;glGetShaderiv(s,GL_COMPILE_STATUS,&ok);
    if(ok!=GL_TRUE){char log[2048]{};glGetShaderInfoLog(s,sizeof(log),nullptr,log);aether_log(AETHER_LOG_ERROR,"Shader compilation: %s",log);glDeleteShader(s);return 0;}return s;
}
GLuint program(const char *vs,const char *fs){
    GLuint v=shader(GL_VERTEX_SHADER,vs),f=shader(GL_FRAGMENT_SHADER,fs);if(!v||!f){if(v)glDeleteShader(v);if(f)glDeleteShader(f);return 0;}
    GLuint p=glCreateProgram();glAttachShader(p,v);glAttachShader(p,f);glLinkProgram(p);glDeleteShader(v);glDeleteShader(f);
    GLint ok=GL_FALSE;glGetProgramiv(p,GL_LINK_STATUS,&ok);if(ok!=GL_TRUE){char log[2048]{};glGetProgramInfoLog(p,sizeof(log),nullptr,log);aether_log(AETHER_LOG_ERROR,"Shader link: %s",log);glDeleteProgram(p);return 0;}return p;
}
void identity(float *m){for(int i=0;i<16;++i)m[i]=0.0f;m[0]=m[5]=m[10]=m[15]=1.0f;}
void multiply(float *o,const float *a,const float *b){
    float r[16];for(int c=0;c<4;++c)for(int row=0;row<4;++row){r[c*4+row]=0;for(int k=0;k<4;++k)r[c*4+row]+=a[k*4+row]*b[c*4+k];}
    for(int i=0;i<16;++i)o[i]=r[i];
}
void perspective(float *m,float aspect){
    const float f=1.0f/std::tan(0.5235987756f),n=0.08f,z=250.0f;
    for(int i=0;i<16;++i)m[i]=0.0f;
    m[0]=f/std::max(aspect,0.1f);m[5]=f;m[10]=(z+n)/(n-z);m[11]=-1.0f;m[14]=(2.0f*z*n)/(n-z);
}
void lookAt(float *m){
    const float eye[3]={0.0f,3.2f,9.0f},target[3]={0.0f,0.0f,0.0f},worldUp[3]={0.0f,1.0f,0.0f};
    float f[3]={target[0]-eye[0],target[1]-eye[1],target[2]-eye[2]};
    float fl=std::sqrt(f[0]*f[0]+f[1]*f[1]+f[2]*f[2]);for(float &v:f)v/=fl;
    float s[3]={f[1]*worldUp[2]-f[2]*worldUp[1],f[2]*worldUp[0]-f[0]*worldUp[2],f[0]*worldUp[1]-f[1]*worldUp[0]};
    float sl=std::sqrt(s[0]*s[0]+s[1]*s[1]+s[2]*s[2]);for(float &v:s)v/=sl;
    float u[3]={s[1]*f[2]-s[2]*f[1],s[2]*f[0]-s[0]*f[2],s[0]*f[1]-s[1]*f[0]};
    identity(m);m[0]=s[0];m[4]=s[1];m[8]=s[2];m[1]=u[0];m[5]=u[1];m[9]=u[2];m[2]=-f[0];m[6]=-f[1];m[10]=-f[2];
    m[12]=-(s[0]*eye[0]+s[1]*eye[1]+s[2]*eye[2]);m[13]=-(u[0]*eye[0]+u[1]*eye[1]+u[2]*eye[2]);m[14]=f[0]*eye[0]+f[1]*eye[1]+f[2]*eye[2];
}
void modelMatrix(const SceneObject &o,float *m){
    float t[16],rx[16],ry[16],rz[16],s[16],a[16],b[16];
    identity(t);t[12]=o.position[0];t[13]=o.position[1];t[14]=o.position[2];
    identity(rx);identity(ry);identity(rz);
    const float x=o.rotation[0]*0.01745329252f,y=o.rotation[1]*0.01745329252f,z=o.rotation[2]*0.01745329252f;
    rx[5]=std::cos(x);rx[6]=std::sin(x);rx[9]=-std::sin(x);rx[10]=std::cos(x);
    ry[0]=std::cos(y);ry[2]=-std::sin(y);ry[8]=std::sin(y);ry[10]=std::cos(y);
    rz[0]=std::cos(z);rz[1]=std::sin(z);rz[4]=-std::sin(z);rz[5]=std::cos(z);
    identity(s);s[0]=o.scale[0];s[5]=o.scale[1];s[10]=o.scale[2];
    multiply(a,rx,s);multiply(b,ry,a);multiply(a,rz,b);multiply(m,t,a);
}
void uploadMesh(GLuint &vao,const std::vector<Vertex>&v,GLenum usage=GL_STATIC_DRAW){
    GLuint buffer=0;glGenVertexArrays(1,&vao);glGenBuffers(1,&buffer);glBindVertexArray(vao);glBindBuffer(GL_ARRAY_BUFFER,buffer);
    glBufferData(GL_ARRAY_BUFFER,(GLsizeiptr)(v.size()*sizeof(Vertex)),v.data(),usage);
    glEnableVertexAttribArray(0);glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,sizeof(Vertex),(void*)offsetof(Vertex,p));
    glEnableVertexAttribArray(1);glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,sizeof(Vertex),(void*)offsetof(Vertex,n));glBindVertexArray(0);
}
void addFace(std::vector<Vertex>&v,const float n[3],const float a[3],const float b[3],const float c[3],const float d[3]){
    const float *p[6]={a,b,c,a,c,d};for(const float *q:p)v.push_back({{q[0],q[1],q[2]},{n[0],n[1],n[2]}});
}
const char *vs=R"GLSL(#version 300 es
precision highp float;
layout(location=0) in vec3 aPosition;
layout(location=1) in vec3 aNormal;
uniform mat4 uModel;
uniform mat4 uMvp;
out vec3 vNormal;
out vec3 vWorld;
void main(){vec4 w=uModel*vec4(aPosition,1.0);vWorld=w.xyz;vNormal=normalize(mat3(uModel)*aNormal);gl_Position=uMvp*vec4(aPosition,1.0);}
)GLSL";
const char *fs=R"GLSL(#version 300 es
precision highp float;
in vec3 vNormal;in vec3 vWorld;
uniform vec4 uColor;
out vec4 outColor;
void main(){
 vec3 N=normalize(vNormal),L=normalize(vec3(-0.45,0.82,0.36)),V=normalize(vec3(0.0,3.0,9.0)-vWorld);
 float diffuse=max(dot(N,L),0.0);
 float spec=pow(max(dot(N,normalize(L+V)),0.0),32.0)*0.18;
 vec3 base=uColor.rgb*(0.22+0.82*diffuse)+vec3(spec);
 float rim=pow(1.0-max(dot(N,V),0.0),3.0)*0.08;
 vec3 color=base+vec3(0.22,0.38,0.62)*rim;
 color=color/(color+vec3(1.0));color=pow(color,vec3(1.0/2.2));
 outColor=vec4(color,uColor.a);
}
)GLSL";
}
GlesRenderer&GlesRenderer::instance(){static GlesRenderer r;return r;}
bool GlesRenderer::initialize(){
    if(program_!=0)return true;program_=program(vs,fs);if(!program_)return false;
    model_location_=glGetUniformLocation(program_,"uModel");mvp_location_=glGetUniformLocation(program_,"uMvp");color_location_=glGetUniformLocation(program_,"uColor");
    std::vector<Vertex> cube;cube.reserve(36);
    const float p000[3]={-0.5f,-0.5f,-0.5f},p001[3]={-0.5f,-0.5f,0.5f},p010[3]={-0.5f,0.5f,-0.5f},p011[3]={-0.5f,0.5f,0.5f};
    const float p100[3]={0.5f,-0.5f,-0.5f},p101[3]={0.5f,-0.5f,0.5f},p110[3]={0.5f,0.5f,-0.5f},p111[3]={0.5f,0.5f,0.5f};
    const float nx[3]={-1,0,0},px[3]={1,0,0},ny[3]={0,-1,0},py[3]={0,1,0},nz[3]={0,0,-1},pz[3]={0,0,1};
    addFace(cube,nx,p000,p001,p011,p010);addFace(cube,px,p101,p100,p110,p111);
    addFace(cube,ny,p000,p100,p101,p001);addFace(cube,py,p010,p011,p111,p110);
    addFace(cube,nz,p100,p000,p010,p110);addFace(cube,pz,p001,p101,p111,p011);
    uploadMesh(cube_vao_,cube);
    std::vector<Vertex> sphere;std::vector<std::uint32_t> indices;constexpr int lat=24,lon=32;
    for(int y=0;y<=lat;++y){float phi=(float)y/(float)lat*3.14159265f;for(int x=0;x<=lon;++x){
        float theta=(float)x/(float)lon*6.2831853f;float pxv=std::sin(phi)*std::cos(theta),pyv=std::cos(phi),pzv=std::sin(phi)*std::sin(theta);
        sphere.push_back({{pxv*0.5f,pyv*0.5f,pzv*0.5f},{pxv,pyv,pzv}});
    }}
    for(int y=0;y<lat;++y)for(int x=0;x<lon;++x){std::uint32_t a=(std::uint32_t)(y*(lon+1)+x),b=a+(lon+1u);indices.insert(indices.end(),{a,b,a+1u,a+1u,b,b+1u});}
    glGenVertexArrays(1,&sphere_vao_);GLuint sv=0,si=0;glGenBuffers(1,&sv);glGenBuffers(1,&si);glBindVertexArray(sphere_vao_);
    glBindBuffer(GL_ARRAY_BUFFER,sv);glBufferData(GL_ARRAY_BUFFER,(GLsizeiptr)(sphere.size()*sizeof(Vertex)),sphere.data(),GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,si);glBufferData(GL_ELEMENT_ARRAY_BUFFER,(GLsizeiptr)(indices.size()*sizeof(std::uint32_t)),indices.data(),GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,sizeof(Vertex),(void*)offsetof(Vertex,p));
    glEnableVertexAttribArray(1);glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,sizeof(Vertex),(void*)offsetof(Vertex,n));glBindVertexArray(0);
    sphere_index_count_=(int)indices.size();
    std::vector<Vertex> plane;const float n[3]={0,1,0};const float a[3]={-0.5f,0,-0.5f},b[3]={-0.5f,0,0.5f},c[3]={0.5f,0,0.5f},d[3]={0.5f,0,-0.5f};
    addFace(plane,n,a,b,c,d);uploadMesh(plane_vao_,plane);
    std::vector<Vertex> grid;const float gn[3]={0,1,0};
    for(int i=-10;i<=10;++i){float q=(float)i;grid.push_back({{-10,-0.505f,q},{gn[0],gn[1],gn[2]}});grid.push_back({{10,-0.505f,q},{gn[0],gn[1],gn[2]}});
        grid.push_back({{q,-0.505f,-10},{gn[0],gn[1],gn[2]}});grid.push_back({{q,-0.505f,10},{gn[0],gn[1],gn[2]}});}
    grid_vertex_count_=(int)grid.size();uploadMesh(grid_vao_,grid);
    glEnable(GL_DEPTH_TEST);glDisable(GL_CULL_FACE);glClearDepthf(1.0f);
    aether_log(AETHER_LOG_INFO,"Generic GLES 3 scene renderer initialized");return true;
}
void GlesRenderer::resize(int w,int h){width_=std::max(w,1);height_=std::max(h,1);glViewport(0,0,width_,height_);perspective(projection_,(float)width_/(float)height_);}
void GlesRenderer::render(const ProjectSettings&project,float time,bool debug){
    (void)time;draw_calls_=0;visible_objects_=0;
    glClearColor(project.clear_color[0],project.clear_color[1],project.clear_color[2],project.clear_color[3]);glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    float view[16];lookAt(view);glUseProgram(program_);
    if(project.show_grid&&debug){
        float model[16],mvp[16];identity(model);multiply(mvp,projection_,view);glUniformMatrix4fv(model_location_,1,GL_FALSE,model);glUniformMatrix4fv(mvp_location_,1,GL_FALSE,mvp);
        glUniform4f(color_location_,0.20f,0.29f,0.43f,1.0f);glBindVertexArray(grid_vao_);glDrawArrays(GL_LINES,0,grid_vertex_count_);glBindVertexArray(0);++draw_calls_;
    }
    for(const auto&o:project.objects){
        if(!o.visible)continue;float model[16],pv[16],mvp[16];modelMatrix(o,model);multiply(pv,projection_,view);multiply(mvp,pv,model);
        glUniformMatrix4fv(model_location_,1,GL_FALSE,model);glUniformMatrix4fv(mvp_location_,1,GL_FALSE,mvp);
        glUniform4f(color_location_,o.color[0],o.color[1],o.color[2],o.color[3]);
        if(o.mesh=="sphere"||o.mesh=="uv_sphere"){glBindVertexArray(sphere_vao_);glDrawElements(GL_TRIANGLES,sphere_index_count_,GL_UNSIGNED_INT,nullptr);}
        else if(o.mesh=="plane"){glBindVertexArray(plane_vao_);glDrawArrays(GL_TRIANGLES,0,6);}
        else {glBindVertexArray(cube_vao_);glDrawArrays(GL_TRIANGLES,0,36);}
        glBindVertexArray(0);++draw_calls_;++visible_objects_;
    }
    if(debug)glFinish();
}
}
