//
// Created by tmerchant on 04/04/2020.
//

#include "opengl_utils.h"

#ifdef _GNUC_
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#endif
#include <cmath>
#include <cstring>
#include <cstdio>

#define COLUMN_MAJOR(x, y) ((y%4)*4 + (x%4))

void multiply_matrices(const float matrix1[16], const float matrix2[16], float dest_matrix[16])
{
    int i, j;

    for(i = 0; i < 4; i++)
    {
        for(j = 0; j < 4; j++)
        {
            dest_matrix[COLUMN_MAJOR(i, j)] =
                    matrix1[COLUMN_MAJOR(i, 0)] * matrix2[COLUMN_MAJOR(0, j)] +
                    matrix1[COLUMN_MAJOR(i, 1)] * matrix2[COLUMN_MAJOR(1, j)] +
                    matrix1[COLUMN_MAJOR(i, 2)] * matrix2[COLUMN_MAJOR(2, j)] +
                    matrix1[COLUMN_MAJOR(i, 3)] * matrix2[COLUMN_MAJOR(3, j)];
        }
    }
}

void quaternion_to_rotation_matrix(const float quaternion[4], float matrix[16])
{
    float tmp_matrix1[16];
    float tmp_matrix2[16];

    tmp_matrix1[COLUMN_MAJOR(0, 0)] = quaternion[3];
    tmp_matrix1[COLUMN_MAJOR(0, 1)] = quaternion[2];
    tmp_matrix1[COLUMN_MAJOR(0, 2)] = -quaternion[1];
    tmp_matrix1[COLUMN_MAJOR(0, 3)] = quaternion[0];

    tmp_matrix1[COLUMN_MAJOR(1, 0)] = -quaternion[2];
    tmp_matrix1[COLUMN_MAJOR(1, 1)] = quaternion[3];
    tmp_matrix1[COLUMN_MAJOR(1, 2)] = quaternion[0];
    tmp_matrix1[COLUMN_MAJOR(1, 3)] = quaternion[1];

    tmp_matrix1[COLUMN_MAJOR(2, 0)] = quaternion[1];
    tmp_matrix1[COLUMN_MAJOR(2, 1)] = -quaternion[0];
    tmp_matrix1[COLUMN_MAJOR(2, 2)] = quaternion[3];
    tmp_matrix1[COLUMN_MAJOR(2, 3)] = quaternion[2];

    tmp_matrix1[COLUMN_MAJOR(3, 0)] = -quaternion[0];
    tmp_matrix1[COLUMN_MAJOR(3, 1)] = -quaternion[1];
    tmp_matrix1[COLUMN_MAJOR(3, 2)] = -quaternion[2];
    tmp_matrix1[COLUMN_MAJOR(3, 3)] = quaternion[3];

    tmp_matrix2[COLUMN_MAJOR(0, 0)] = quaternion[3];
    tmp_matrix2[COLUMN_MAJOR(0, 1)] = quaternion[2];
    tmp_matrix2[COLUMN_MAJOR(0, 2)] = -quaternion[1];
    tmp_matrix2[COLUMN_MAJOR(0, 3)] = -quaternion[0];

    tmp_matrix2[COLUMN_MAJOR(1, 0)] = -quaternion[2];
    tmp_matrix2[COLUMN_MAJOR(1, 1)] = quaternion[3];
    tmp_matrix2[COLUMN_MAJOR(1, 2)] = quaternion[0];
    tmp_matrix2[COLUMN_MAJOR(1, 3)] = -quaternion[1];

    tmp_matrix2[COLUMN_MAJOR(2, 0)] = quaternion[1];
    tmp_matrix2[COLUMN_MAJOR(2, 1)] = -quaternion[0];
    tmp_matrix2[COLUMN_MAJOR(2, 2)] = quaternion[3];
    tmp_matrix2[COLUMN_MAJOR(2, 3)] = -quaternion[2];

    tmp_matrix2[COLUMN_MAJOR(3, 0)] = quaternion[0];
    tmp_matrix2[COLUMN_MAJOR(3, 1)] = quaternion[1];
    tmp_matrix2[COLUMN_MAJOR(3, 2)] = quaternion[2];
    tmp_matrix2[COLUMN_MAJOR(3, 3)] = quaternion[3];

    multiply_matrices (tmp_matrix1, tmp_matrix2, matrix);
}

void euler_to_quaternion(double rotateX, double rotateY, double rotateZ, float quaternion[4])
{
    double cy, sy, cp, sp, cr, sr;

#ifdef _GNUC_
    sincos(rotateZ * 0.5, sy, cy);
    sincos(rotateY * 0.5, sp, cp);
    sincos(rotateX * 0.5, sr, cr);
#else
    cy = cos(rotateZ * 0.5);
    sy = sin(rotateZ * 0.5);
    cp = cos(rotateY * 0.5);
    sp = sin(rotateY * 0.5);
    cr = cos(rotateX * 0.5);
    sr = sin(rotateX * 0.5);
#endif

    quaternion[0] = (float)(cy * cp * sr - sy * sp * cr);
    quaternion[1] = (float)(sy * cp * sr + cy * sp * cr);
    quaternion[2] = (float)(sy * cp * cr - cy * sp * sr);
    quaternion[3] = (float)(cy * cp * cr + sy * sp * sr);
}

void load_scaling_matrix (float matrix[16], float scaleX, float scaleY, float scaleZ)
{
    load_identity_matrix (matrix);
    matrix[COLUMN_MAJOR (0, 0)] = scaleX;
    matrix[COLUMN_MAJOR (1, 1)] = scaleY;
    matrix[COLUMN_MAJOR (2, 2)] = scaleZ;
}

void load_translation_matrix(float matrix[16], float translateX, float translateY, float translateZ)
{
    load_identity_matrix (matrix);

    matrix[COLUMN_MAJOR (0, 3)] = translateX;
    matrix[COLUMN_MAJOR (1, 3)] = translateY;
    matrix[COLUMN_MAJOR (2, 3)] = translateZ;
}

void load_transform_matrix (
        float matrix[16],
        float scaleX, float scaleY, float scaleZ,
        float rotateX, float rotateY, float rotateZ,
        float translateX, float translateY, float translateZ
)
{
    float tmp_matrix[16];
    float result_matrix[16];
    float rotation_quaternion[4];

    load_scaling_matrix(matrix, scaleX, scaleY, scaleZ);
    load_identity_matrix(tmp_matrix);

    euler_to_quaternion(rotateX, rotateY, rotateZ, rotation_quaternion);
    quaternion_to_rotation_matrix(rotation_quaternion, tmp_matrix);

    //Rotation * Scale
    multiply_matrices(tmp_matrix, matrix, result_matrix);

    load_translation_matrix(tmp_matrix, translateX, translateY, translateZ);

    //Translation * Rotation * Scale
    multiply_matrices (tmp_matrix, result_matrix, matrix);
}

void load_identity_matrix ( float matrix[16] )
{
    memset(matrix, 0, 16 * sizeof(float));

    matrix[COLUMN_MAJOR(0, 0)] = 1;
    matrix[COLUMN_MAJOR(1, 1)] = 1;
    matrix[COLUMN_MAJOR(2, 2)] = 1;
    matrix[COLUMN_MAJOR(3, 3)] = 1;
}

void load_projection_matrix ( float matrix[16], float ar, float zNear, float zFar, float fov )
{
    memset(matrix, 0, 16 * sizeof(float));

    matrix[COLUMN_MAJOR(0, 0)] = 1 / ( fov * ar );
    matrix[COLUMN_MAJOR(1, 1)] = 1 / fov;
    matrix[COLUMN_MAJOR(2, 2)] = - ( zNear + zFar ) / ( zNear - zFar );
    matrix[COLUMN_MAJOR(2, 3)] = -1;
    matrix[COLUMN_MAJOR(3, 2)] = -2 * ( zFar * zNear ) / ( zNear - zFar );

}

void load_orthogonal_projection_matrix ( float matrix[16], float zNear, float zFar, float width, float height )
{
    float scaleMatrix[16], translationMatrix[16];

    load_scaling_matrix (scaleMatrix, 2 / width, 2 / height, 2 / (zFar - zNear));
    load_translation_matrix (translationMatrix, 0, 0, - (zFar + zNear) / 2);

    multiply_matrices (scaleMatrix, translationMatrix, matrix);
}

void vertex_buffer_to_wavefront_obj(char* path, double* vertex_buffer, unsigned int* element_buffer,
        unsigned long long num_points, unsigned long long num_triangles)
{
    unsigned long long i;

    FILE *objfile = fopen(path, "w");

    for(i = 0; i < num_points; i++)
    {
        fprintf (objfile, "v %f %f %f\n", vertex_buffer[i * 3], vertex_buffer[i * 3 + 1], vertex_buffer[i * 3 + 2]);
    }

    fflush(objfile);

    fputs("\nf ", objfile);

    for(i = 0; i < 3 * num_triangles; i++)
    {
        fprintf(objfile, "%u ", element_buffer[i] + 1);
    }

    fflush(objfile);

    fclose(objfile);
}