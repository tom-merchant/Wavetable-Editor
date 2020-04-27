//
// Created by tmerchant on 04/04/2020.
//

#ifndef WAVETABLE_EDITOR_OPENGL_UTILS_H
#define WAVETABLE_EDITOR_OPENGL_UTILS_H


void multiply_matrices(const float matrix1[16], const float matrix2[16], float dest_matrix[16]);
void quaternion_to_rotation_matrix(const float quaternion[4], float matrix[16]);
void euler_to_quaternion(double rotateX, double rotateY, double rotateZ, float quaternion[4]);

void load_transform_matrix (
        float matrix[16],
        float scaleX, float scaleY, float scaleZ,
        float rotateX, float rotateY, float rotateZ,
        float translateX, float translateY, float translateZ
);

void load_identity_matrix ( float matrix[16] );

void load_projection_matrix ( float matrix[16], float ar, float zNear, float zFar, float fov );

void load_orthogonal_projection_matrix ( float *matrix, float zNear, float zFar, float width, float height );

void vertex_buffer_to_wavefront_obj(char* path, double* vertex_buffer, unsigned int* element_buffer,
                                    unsigned long long num_points, unsigned long long num_triangles);


#endif //WAVETABLE_EDITOR_OPENGL_UTILS_H
