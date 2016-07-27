#include <cstdio>
#include <iostream>
#include <string>
#include <array>

#include <unistd.h> // usleep

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>

const GLchar* vertex_code = R"(
precision mediump float;
attribute mediump vec4 vPosition;

void main(void) {
  gl_Position = vPosition;
}
)";

const GLchar* fragment_code = R"(
void main() {
gl_FragColor = vec4(0.0, 1.0, 0.0, 1.0);
}
)";

std::array<GLfloat, 8> vtxcoord = {
    1.0f, -1.0f,
    1.0f,  1.0f,
    -1.0f, -1.0f,
    -1.0f,  1.0f,
};

std::array<GLfloat, 8> texcoord = {
    1.0f, 1.0f,
    1.0f, 0.0f,
    0.0f, 1.0f,
    0.0f, 0.0f,
};

GLuint CreateGLShader(const GLchar* shaderCode, GLenum shaderType) {
    GLuint shader = 0;
    GLint stat = 0;

    shader = glCreateShader(shaderType);
    glShaderSource(shader, 1, &shaderCode, 0);
    glCompileShader(shader);
    // Check the compile error
    glGetShaderiv(shader, GL_COMPILE_STATUS, (GLint*)&stat);
    if (GL_FALSE == stat) {
        GLint logLen = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, (GLint*)&logLen);

        char* log = new char[logLen];
        int ans = 0;
        glGetShaderInfoLog(shader, logLen, NULL, log);
        std::cout << "SHADER INFO:" << log << std::endl;

        glGetShaderiv(shader, GL_COMPILE_STATUS, &ans);
        std::cout << "SHADER BUILD INFO:" << ans << std::endl;

        delete [] log;

        exit(1);
    }

    return shader;
}

GLuint CreateGLProgram(GLuint shaderVtx, GLuint shaderFrg) {
    GLuint program = 0;
    GLint stat = 0;

    program = glCreateProgram();
    glAttachShader(program, shaderFrg);
    glAttachShader(program, shaderVtx);

    // Link program to two shaders
    glLinkProgram(program);
    // Check the link error
    glGetProgramiv(program, GL_LINK_STATUS, (GLint*)&stat);
    if (GL_FALSE == stat) {
        GLint logLen = 0;
        glGetShaderiv(program, GL_INFO_LOG_LENGTH, (GLint*)&logLen);

        char* log = new char[logLen];
        int ans = 0;
        glGetShaderInfoLog(program, logLen, NULL, log);
        std::cout << "PRG INFO:" << log << std::endl;

        glGetShaderiv(program, GL_COMPILE_STATUS, &ans);
        std::cout << "PRG BUILD INFO:" << ans << std::endl;

        delete [] log;

        exit(1);
    }

    glReleaseShaderCompiler();

    return program;
}

int main() {
    int w = 640;
    int h = 480;
    EGLNativeDisplayType native_display_ = XOpenDisplay(NULL);
    Window root_win = DefaultRootWindow(static_cast<Display*>(native_display_));

    int screen = DefaultScreen(native_display_);
    unsigned long white = WhitePixel(native_display_, screen);
    unsigned long black = BlackPixel(native_display_, screen);

    EGLNativeWindowType native_window_ = XCreateSimpleWindow(static_cast<Display*>(native_display_),
                                         root_win,
                                         100, 100,
                                         640, 480,
                                         2,
                                         black, white);

    XMapWindow(static_cast<Display*>(native_display_),
               static_cast<Window>(native_window_));


    EGLDisplay display = eglGetDisplay(native_display_);
	EGLint stat = EGL_SUCCESS;
    stat = eglGetError();
    if ((EGL_NO_DISPLAY == display) || (EGL_SUCCESS != stat)) {
        std::cout << "ERROR: eglGetDisplay() failed. code=" << stat << std::endl;
        std::abort();
    }

    EGLint major = 0;
    EGLint minor = 0;
    eglInitialize(display, &major, &minor);
    printf("egl version: %d,%d\n", major, minor);
    if (major < 1 && minor < 4)
        return -1;

    eglBindAPI(EGL_OPENGL_ES_API);

    EGLConfig auto_config = nullptr;
    constexpr EGLint attrs[] = {
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,    // type = gles2
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,           // type = onscreen
        EGL_RED_SIZE, 8,                            // RGB888 32depth
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        EGL_DEPTH_SIZE, 24,
        EGL_NONE
    };
    eglChooseConfig(display, attrs, &auto_config, 1, nullptr);

    // EGLSurface pbuf_surface = eglCreatePbufferSurface(display, auto_config, nullptr);
	EGLSurface pbuf_surface = eglCreateWindowSurface(display, auto_config, native_window_, nullptr);

    EGLint ctxAttr[] = {
        EGL_CONTEXT_CLIENT_VERSION,	2,
        EGL_NONE
    };
    EGLContext ctx = eglCreateContext(display, auto_config, nullptr, ctxAttr);
    eglMakeCurrent(display, pbuf_surface, pbuf_surface, ctx);
    glViewport(0, 0, w, h);

    GLuint vertex = CreateGLShader(vertex_code, GL_VERTEX_SHADER);
    GLuint fragment = CreateGLShader(fragment_code, GL_FRAGMENT_SHADER);
    GLuint shader_prg = CreateGLProgram(vertex, fragment);
    glUseProgram(shader_prg);

    GLuint loc_vtxcoord = glGetAttribLocation(shader_prg, "vPosition");

    glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    const GLfloat vertices[] = {
        0.0f,  0.5f,
        -0.5f, -0.5f,
        0.5f, -0.5f
    };
    glEnableVertexAttribArray(loc_vtxcoord);
    glVertexAttribPointer(loc_vtxcoord, 2, GL_FLOAT, GL_FALSE, 0, vertices);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    // eglSwapBuffers(display, pbuf_surface);
    // glUseProgram(0);

#if 0
    GLuint fbo = 0;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    GLuint rtex = 0;
    glGenTextures(1, &rtex);
    glBindTexture(GL_TEXTURE_2D, rtex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);

    GLuint depth_rfb = 0;
    glGenRenderbuffers(1, &depth_rfb);
    glBindRenderbuffer(GL_RENDERBUFFER, depth_rfb);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEGLPTH_COMPONENT16, w, h);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, rtex, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_rfb);

    // glClearColor(1, 0, 0, 1);
    // glClear(GL_COLOR_BUFFER_BIT);
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        printf("Problem with OpenGL framebuffer after specifying color render buffer: %x\n", status);
    }

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
#endif
    while (true) {
        sleep(1);
    }
    return 0;
}
