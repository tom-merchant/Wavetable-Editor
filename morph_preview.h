//
// Created by tmerchant on 04/04/2020.
//

#ifndef WAVETABLE_EDITOR_MORPH_PREVIEW_H
#define WAVETABLE_EDITOR_MORPH_PREVIEW_H

#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <gtkmm.h>

extern "C"
{
#include <wavetable.h>
};

#include "opengl_utils.h"

typedef struct morph_preview_context
{
    int width;
    int height;
    double FOV;
    GLuint morph_surface_VAO_id;
    unsigned long long morph_surface_VAO_vertex_count;
    GLuint morph_surface_VBO_id;
    GLuint morph_surface_EBO_id;
    GLuint vertex_shader;
    GLuint fragment_shader;
    GLuint shader_program;
    double translateX;
    double translateY;
    double translateZ;
    double rotateX;
    double rotateY;
    double rotateZ;
    double scaleX;
    double scaleY;
    double scaleZ;
    double cameraX;
    double cameraY;
    double cameraZ;
    double camera_pitch;
    double camera_yaw;
    double camera_roll;
    double ar;
    float zNear;
    float zFar;
    float view_width;
    float view_height;
    float tanHalfFOV;
    float playback_position;
    unsigned long long morph_surface_EBO_element_count;
    bool update_vertex_buffers;
} morph_preview_context;

extern morph_preview_context morph_ctx;
extern double* morph_surface_vertex_buffer;
extern unsigned long long vertex_buffer_size;
extern unsigned int* morph_surface_element_buffer;
extern unsigned long long element_buffer_size;

bool draw_morph_preview_control(const Glib::RefPtr<Gdk::GLContext>& ctx);
void generate_morph_surface_vertices(morph_wavetable_mipmap *mwm);
void update_morph_surface ( double* vertex_buffer, unsigned long long buffer_size, unsigned int* element_buffer, unsigned long long element_buffer_size );
void on_generate_morph_table();

#endif //WAVETABLE_EDITOR_MORPH_PREVIEW_H
