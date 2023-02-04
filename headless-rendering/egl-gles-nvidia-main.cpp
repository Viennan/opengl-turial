#include "render.hpp"
#include "glad/glad_egl.h"
#include "glad/glad.h"
#include <iostream>

static const EGLint configAttribs[] = {
        EGL_SURFACE_TYPE, EGL_PBUFFER_BIT,
        EGL_BLUE_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_RED_SIZE, 8,
        EGL_DEPTH_SIZE, 8,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
        EGL_NONE
};    

static const int pbufferWidth = 9;
static const int pbufferHeight = 9;

static const EGLint pbufferAttribs[] = {
    EGL_WIDTH, pbufferWidth,
    EGL_HEIGHT, pbufferHeight,
    EGL_NONE,
};

int main() {
    // 使用glad可以屏蔽OpenGL、OpenglES之间头文件名称的差异，便于移植
    gladLoadEGL();

    static const int MAX_DEVICES = 128;
    EGLDeviceEXT eglDevs[MAX_DEVICES];
    EGLint numDevices;
    eglQueryDevicesEXT(MAX_DEVICES, eglDevs, &numDevices);
    std::cout << "Detect " << numDevices << " devices" << std::endl;
    if (numDevices == 0) {
        std::cout << "no device" << std::endl;
        return -1;
    }

    int nvidia_id = -1;
    for (int i=0;i<numDevices;++i) {
        auto c_name = eglQueryDeviceStringEXT(eglDevs[i], EGL_DRIVER_NAME_EXT);
        if (!c_name) {
            continue;
        }
        auto name = std::string(c_name);
        if (name.find("nvidia") != std::string::npos) {
            std::cout << "nvidia at " << i << std::endl;
            nvidia_id = i;
            break;
        }
    }
    if (nvidia_id == -1) {
        std::cout << "no nvidia device" << std::endl;
        return -1;
    }

    // 如果linux系统(例如debian10及以上的发行版)以glvnd作为opengl dispatcher，需要使用eglGetPlatformDisplayEXT才能基于n卡驱动创建EGL环境
    // EGLDisplay eglDpy = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    EGLDisplay eglDpy = eglGetPlatformDisplayEXT(EGL_PLATFORM_DEVICE_EXT, eglDevs[nvidia_id], 0);

    EGLint major, minor;

    eglInitialize(eglDpy, &major, &minor);
    std::cout << "EGL " << major << "." << minor << std::endl;

    EGLint numConfigs;
    EGLConfig eglCfg;
    if (!eglChooseConfig(eglDpy, configAttribs, &eglCfg, 1, &numConfigs)) {
        std::cout << "EGL find expired config failed" << std::endl;
        return -1;
    }

    auto eglBindFlag = eglBindAPI(EGL_OPENGL_ES_API);
    if (!eglBindAPI) {
        std::cout << "egl bind opengl es api failed" << std::endl;
        return -1;
    }

    EGLint contextAttribs[] = {EGL_CONTEXT_CLIENT_VERSION, 3, EGL_NONE};
    EGLContext eglCtx = eglCreateContext(eglDpy, eglCfg, EGL_NO_CONTEXT, contextAttribs);

    // headless渲染下推荐设置EGL_NO_SURFACE避免从EGL surface与CUDA-accessible OpenGL buffer之间的数据拷贝
    // 参考https://developer.nvidia.com/blog/egl-eye-opengl-visualization-without-x-server/
    if (!eglMakeCurrent(eglDpy, EGL_NO_SURFACE, EGL_NO_SURFACE, eglCtx)) {
        std::cout << "egl make headless context current failed" << std::endl;
        return -1;
    }

    // 根据需要在glad网站上配置OpenGL / OpenGLES 版本，会自动生成对应loader
    // 只要正常安装最新版n卡驱动，就无需过于在意底层实际的gl版本，如果系统的GLES版本是3.0，那么这里也只会load ES2标准内的API
    auto gladLoadFlag = gladLoadGLES2Loader((GLADloadproc)eglGetProcAddress);
    if (!gladLoadFlag) {
        std::cout << "glad load opengl es api failed" << std::endl;
        return -1;
    }

    // Render to FBO and save imags
    render(1280, 720, ".");

    eglTerminate(eglDpy);

    return 0;
}