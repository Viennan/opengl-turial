set(HEADER_LIST include/EGL/eglplatform.h include/glad/glad.h include/glad/glad_egl.h include/KHR/khrplatform.h)
add_library(glad STATIC src/glad.c src/glad_egl.c ${HEADER_LIST})
target_include_directories(glad PUBLIC include)