// 着色器管理
#pragma once
#include <glad/glad.h>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

class Shader {
public:
    GLuint ID = 0;
    Shader() = default;

    bool loadFromFile(const std::string& vp, const std::string& fp) {
        std::string v = read(vp), f = read(fp);
        if (v.empty() || f.empty()) return false;
        return compile(v.c_str(), f.c_str());
    }
    bool loadFromMemory(const char* vs, const char* fs) {
        return compile(vs, fs);
    }

    void use()  const { glUseProgram(ID); }
    void setFloat(const char* n, float v) const
        { glUniform1f(glGetUniformLocation(ID,n),v); }
    void setInt(const char* n, int v) const
        { glUniform1i(glGetUniformLocation(ID,n),v); }
    void setVec2(const char* n, float x, float y) const
        { glUniform2f(glGetUniformLocation(ID,n),x,y); }
    void setMat4(const char* n, const float* p) const
        { glUniformMatrix4fv(glGetUniformLocation(ID,n),1,GL_FALSE,p); }
    void destroy() { if(ID){glDeleteProgram(ID);ID=0;} }

private:
    static std::string read(const std::string& p) {
        std::ifstream f(p);
        if(!f.is_open()){std::cerr<<"Cannot open "<<p<<"\n";return "";}
        std::ostringstream s; s<<f.rdbuf(); return s.str();
    }
    bool compile(const char* vs, const char* fs) {
        GLuint v = stage(vs,GL_VERTEX_SHADER,"VERT");
        GLuint f = stage(fs,GL_FRAGMENT_SHADER,"FRAG");
        if(!v||!f){if(v)glDeleteShader(v);if(f)glDeleteShader(f);return false;}
        ID=glCreateProgram(); glAttachShader(ID,v); glAttachShader(ID,f);
        glLinkProgram(ID);
        GLint ok; glGetProgramiv(ID,GL_LINK_STATUS,&ok);
        if(!ok){char l[512];glGetProgramInfoLog(ID,512,0,l);
            std::cerr<<"Link: "<<l<<"\n";glDeleteProgram(ID);ID=0;}
        glDeleteShader(v); glDeleteShader(f);
        return ID!=0;
    }
    static GLuint stage(const char* s, GLenum t, const char* tag) {
        GLuint sh=glCreateShader(t);
        glShaderSource(sh,1,&s,0); glCompileShader(sh);
        GLint ok; glGetShaderiv(sh,GL_COMPILE_STATUS,&ok);
        if(!ok){char l[512];glGetShaderInfoLog(sh,512,0,l);
            std::cerr<<tag<<": "<<l<<"\n";glDeleteShader(sh);return 0;}
        return sh;
    }
};
