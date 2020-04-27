//
// Created by tmerchant on 04/04/2020.
//

#include "morph_preview.h"
#include <iostream>
#include "control_structures.h"

morph_preview_context morph_ctx = {};

bool glInitialized = false;

double* morph_surface_vertex_buffer = nullptr;
unsigned long long vertex_buffer_size;
unsigned int* morph_surface_element_buffer = nullptr;
unsigned long long element_buffer_size;

double triangle[] = {
        0.0, 1.0, 0.0,
        -1.0, -1.0, 0.0,
        1.0, -1.0, 0.0
};

unsigned int triangle_EBO[] = {0, 1, 2};

const GLchar fragment_source[] =
        "#version 150\n"
        "precision highp float;\n"
        "in vec4 colour;\n"
        "out vec4 fragColor;\n"
        "void main()\n"
        "{\n"
        "fragColor = colour;\n"
        "}\n";

const GLchar* fragment_sources[] = {fragment_source};
const GLint fragment_source_sizes[] = {sizeof(fragment_source) / sizeof(fragment_source[0])};


const GLchar vertex_source[] =
        "#version 330 core\n"
        "layout (location = 0) in vec3 in_Position;\n"
        "out vec4 colour;\n"
        "uniform float playbackPos;\n"
        "uniform mat4 viewMatrix;\n"
        "uniform mat4 modelMatrix;\n"
        "uniform mat4 projectionMatrix;\n"
        "float wavetable_num;\n"
        "void main()\n"
        "{\n"
        "gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(in_Position.x, in_Position.y, in_Position.z, 1.0);\n"
        "if( abs(playbackPos - (in_Position.z + 1) / 2) > 0.02)\n"
        "{\n"
        "colour = vec4(1 * in_Position.y, in_Position.y, 1.0 * -in_Position.y, 0.5);\n"
        "}\n"
        "else\n"
        "{\n"
        "colour = vec4(1, 0, 1, 1);\n"
        "}\n"
        "}\n";

const GLchar* vertex_sources[] = {vertex_source};
const GLint vertex_source_sizes[] = {sizeof(vertex_source) / sizeof(vertex_source[0])};

bool draw_morph_preview_control(const Glib::RefPtr<Gdk::GLContext>& ctx)
{
    if (glInitialized)
    {
        if(morph_ctx.update_vertex_buffers)
        {
            update_morph_surface (morph_surface_vertex_buffer, vertex_buffer_size, morph_surface_element_buffer, element_buffer_size);
            free(morph_surface_vertex_buffer);
            free(morph_surface_element_buffer);
            morph_surface_vertex_buffer = nullptr;
            morph_surface_element_buffer = nullptr;
            morph_ctx.update_vertex_buffers = false;
        }

        glClearColor ( 0, 0, 0, 1 );
        glClear ( GL_COLOR_BUFFER_BIT + GL_DEPTH_BUFFER_BIT + GL_STENCIL_BUFFER_BIT );

        glUseProgram ( morph_ctx.shader_program );

        morph_ctx.translateX = -5;
        morph_ctx.translateZ = 12;
        morph_ctx.scaleX = 20;
        morph_ctx.scaleY = 3;
        morph_ctx.scaleZ = 6;
        morph_ctx.rotateX = M_PI / 4;

        morph_ctx.camera_yaw = M_PI / 16;
        //morph_ctx.camera_pitch = M_PI / 32;
        morph_ctx.cameraY = -1;
        morph_ctx.cameraX = 1;

        // Set up matrix transformations
        float model[16], view[16], projection[16];
        //load_projection_matrix ( projection, morph_ctx.ar, morph_ctx.zNear, morph_ctx.zFar, morph_ctx.tanHalfFOV );
        load_orthogonal_projection_matrix ( projection, morph_ctx.zNear, morph_ctx.zFar, morph_ctx.view_width,
                                            morph_ctx.view_height );
        load_transform_matrix ( model, morph_ctx.scaleX, morph_ctx.scaleY, morph_ctx.scaleZ, morph_ctx.rotateX, morph_ctx.rotateY, morph_ctx.rotateZ, morph_ctx.translateX, morph_ctx.translateY, morph_ctx.translateZ );
        load_transform_matrix ( view, 1, 1, 1, -morph_ctx.camera_roll, -morph_ctx.camera_yaw, -morph_ctx.camera_pitch, -morph_ctx.cameraX, -morph_ctx.cameraY, -morph_ctx.cameraZ );

        // Upload matrices to vertex shader
        GLint modelMatrixPos = glGetUniformLocation (morph_ctx.shader_program, "modelMatrix");
        GLint viewMatrixPos = glGetUniformLocation (morph_ctx.shader_program, "viewMatrix");
        GLint projectionMatrixPos = glGetUniformLocation (morph_ctx.shader_program, "projectionMatrix");
        glProgramUniformMatrix4fv (morph_ctx.shader_program, modelMatrixPos, 1, GL_FALSE, model);
        glProgramUniformMatrix4fv (morph_ctx.shader_program, viewMatrixPos, 1, GL_FALSE, view);
        glProgramUniformMatrix4fv (morph_ctx.shader_program, projectionMatrixPos, 1, GL_FALSE, projection);

        if(ctrls.mwm)morph_ctx.playback_position = (float)(morph_ctx.playback_position + 0.01);
        if(morph_ctx.playback_position >= 1) morph_ctx.playback_position = -1;

        GLint playbackPos = glGetUniformLocation (morph_ctx.shader_program, "playbackPos");
        glProgramUniform1f (morph_ctx.shader_program, playbackPos, fabsf(morph_ctx.playback_position));

        // Draw triangles
        glBindVertexArray ( morph_ctx.morph_surface_VAO_id );
        glEnableVertexAttribArray ( 0 );
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, morph_ctx.morph_surface_EBO_id);
        glDrawElements (GL_TRIANGLES, morph_ctx.morph_surface_EBO_element_count, GL_UNSIGNED_INT, nullptr);

        // Unbind everything
        glDisableVertexAttribArray ( 0 );
        glBindVertexArray ( 0 );
        glUseProgram ( 0 );

        glFlush ();

        ctrls.wavetable_morph_preview->queue_render ();
    }
    else
    {
        morph_ctx.vertex_shader   = glCreateShader (GL_VERTEX_SHADER  );
        morph_ctx.fragment_shader = glCreateShader (GL_FRAGMENT_SHADER);

        glShaderSource (morph_ctx.vertex_shader, 1, vertex_sources, vertex_source_sizes);
        glShaderSource (morph_ctx.fragment_shader, 1, fragment_sources, fragment_source_sizes);

        int success;

        glCompileShader (morph_ctx.vertex_shader);
        glGetShaderiv(morph_ctx.vertex_shader, GL_COMPILE_STATUS, &success);

        char infoLog[513] = {};

        if(!success)
        {
            glGetShaderInfoLog(morph_ctx.vertex_shader, 512, nullptr, infoLog);
            std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
        }

        glCompileShader (morph_ctx.fragment_shader);
        glGetShaderiv(morph_ctx.fragment_shader, GL_COMPILE_STATUS, &success);

        if(!success)
        {
            glGetShaderInfoLog(morph_ctx.fragment_shader, 512, nullptr, infoLog);
            std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
        }

        morph_ctx.shader_program = glCreateProgram();
        glAttachShader(morph_ctx.shader_program, morph_ctx.vertex_shader);
        glAttachShader(morph_ctx.shader_program, morph_ctx.fragment_shader);
        glBindAttribLocation (morph_ctx.shader_program, 0, "in_Position");

        glLinkProgram(morph_ctx.shader_program);


        morph_ctx.width = ctrls.wavetable_morph_preview->get_window ()->get_width ();
        morph_ctx.height = ctrls.wavetable_morph_preview->get_window ()->get_height ();
        morph_ctx.FOV = 75;

        morph_ctx.ar = morph_ctx.width / (double)morph_ctx.height;
        morph_ctx.zNear = 1.0f;
        morph_ctx.zFar = 40.0f;
        morph_ctx.view_width = 60;
        morph_ctx.view_height = 20;
        morph_ctx.tanHalfFOV = (float)(tan((M_PI/180) * (morph_ctx.FOV / 2)));


        glGenVertexArrays(1, &morph_ctx.morph_surface_VAO_id);
        glGenBuffers (1, &morph_ctx.morph_surface_VBO_id);
        glGenBuffers (1, &morph_ctx.morph_surface_EBO_id);

        update_morph_surface (triangle, 9 * sizeof(double), triangle_EBO, 3 * sizeof(unsigned int));

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glInitialized = true;
    }

    return true;
}

void generate_morph_surface_vertices(morph_wavetable_mipmap *mwm)
{
    int i, j, k;

    if(mwm == nullptr)
    {
        if(morph_surface_vertex_buffer)free(morph_surface_vertex_buffer);
        if(morph_surface_element_buffer)free(morph_surface_element_buffer);

        morph_surface_vertex_buffer = (double*)malloc(sizeof(triangle) * sizeof(double));
        morph_surface_element_buffer = (unsigned int*)malloc(sizeof(triangle_EBO) * sizeof(unsigned int));

        memcpy (morph_surface_vertex_buffer, triangle, sizeof(triangle) * sizeof(double));
        memcpy (morph_surface_element_buffer, triangle_EBO, sizeof(triangle_EBO) * sizeof(unsigned int));

        vertex_buffer_size  = sizeof(triangle) * sizeof(double);
        element_buffer_size = sizeof(triangle_EBO) * sizeof(unsigned int);

        morph_ctx.update_vertex_buffers = true;
        return;
    }

    // TODO: Choose the mipmap level with the most samples under a certain threshold so we don't have to draw millions of tris

    int num_tables = mwm->total_tables;
    int samples_per_table = mwm->tables[0].tables[0].num_samples;
    long long num_points = num_tables * samples_per_table;

    double sample_distance_x = 2 / (double)samples_per_table; //Normalise samples into range -1...1
    double table_distance_z = 2 / (double)num_tables; //Normalise tables into range -1..1
    //Samples will already be in range -1..1 but perhaps we should squash them down so that they are in proportion
    //to the table, or we can just scale the table out after we've generated it in the z and x axis using a matrix

    int num_triangles = (2 * (samples_per_table - 1)) * (num_tables - 1);

    if(morph_surface_element_buffer)free(morph_surface_element_buffer);
    if(morph_surface_vertex_buffer)free(morph_surface_vertex_buffer);

    morph_surface_element_buffer = (unsigned int*)malloc(3 * num_triangles * sizeof(unsigned int));
    element_buffer_size = 3 * num_triangles * sizeof(int);
    morph_surface_vertex_buffer = (double*)malloc(3* num_points * sizeof(double));
    vertex_buffer_size = 3 * num_points * sizeof(double);

    //TODO: each table could be done in parallel and combined at the end, we'll see how long this takes
    k = 0;

    // Iterate backwards so alpha blending works
    // for(i = 0; i < num_tables; i++)
    for(i = num_tables - 1; i >= 0; i--)
    {
        //for(j = 0; j < samples_per_table; j++)
        for(j = samples_per_table - 1; j >= 0; j--)
        {
            morph_surface_vertex_buffer[i * samples_per_table * 3 + j * 3] = -1 + (double)j * sample_distance_x;
            morph_surface_vertex_buffer[i * samples_per_table * 3 + j * 3 + 1] = -mwm->tables[i].tables[0].table[j];
            morph_surface_vertex_buffer[i * samples_per_table * 3 + j * 3 + 2] = -1 + (double)i * table_distance_z;

            if(j < samples_per_table - 1 && i < num_tables - 1)
            {
                morph_surface_element_buffer[k] = i * samples_per_table + j;
                morph_surface_element_buffer[k + 1] = (i + 1) * samples_per_table + (j + 1);
                morph_surface_element_buffer[k + 2] = (i + 1) * samples_per_table + (j);
                morph_surface_element_buffer[k + 3] =  i * samples_per_table + j;
                morph_surface_element_buffer[k + 4] =  i * samples_per_table + (j + 1);
                morph_surface_element_buffer[k + 5] = (i + 1) * samples_per_table + (j + 1);
                k += 6;
            }
        }
    }

    //vertex_buffer_to_wavefront_obj("test.obj", morph_surface_vertex_buffer, morph_surface_element_buffer,
    //                                    num_points, num_triangles);

    morph_ctx.update_vertex_buffers = true;
}

void update_morph_surface ( double* vertex_buffer, unsigned long long buffer_size, unsigned int* element_buffer, unsigned long long element_buffer_sz )
{
    glBindBuffer (GL_ARRAY_BUFFER, morph_ctx.morph_surface_VBO_id);
    glBufferData (GL_ARRAY_BUFFER, buffer_size, vertex_buffer, GL_STATIC_DRAW);
    glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, morph_ctx.morph_surface_EBO_id);
    glBufferData (GL_ELEMENT_ARRAY_BUFFER, element_buffer_sz, element_buffer, GL_STATIC_DRAW);
    morph_ctx.morph_surface_VAO_vertex_count = (unsigned long long)(buffer_size / sizeof(double));
    morph_ctx.morph_surface_EBO_element_count = (unsigned long long)(element_buffer_sz / sizeof(int));
    glBindVertexArray (morph_ctx.morph_surface_VAO_id);
    glVertexAttribPointer (0, 3, GL_DOUBLE, GL_FALSE, 0, nullptr);
    glBindBuffer (GL_ARRAY_BUFFER, 0);
    glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void close_dialog(Gtk::MessageDialog *dialog)
{
    dialog->hide ();
    delete dialog;
}


void on_generate_morph_table()
{
    int num_tables = 0;
    unsigned long i;

    wavetable_morph_control_row *current_row;

    ctrls.morph_harmonics = {};

    if(ctrls.mwm)
    {
        free_morph_wavetable_mipmap (ctrls.mwm);
        ctrls.mwm = nullptr;
    }

    ctrls.morph_harmonics.tables_per_morph = (int)strtol(ctrls.morph_tables_per_morph_entry->get_text ().c_str (), nullptr, 10);
    ctrls.morph_harmonics.num_harmonics = (int)strtol ( ctrls.morph_num_harmonics_entry->get_text ().c_str (), nullptr, 10 );
    ctrls.morph_harmonics.oversample = (int)strtol(ctrls.morph_oversample_entry->get_text ().c_str (), nullptr, 10);
    ctrls.morph_harmonics.tables_per_octave = (int)strtol(ctrls.morph_tables_per_octave_entry->get_text().c_str (), nullptr, 10);

    std::map<int, wavetable_morph_control_row*> rowmap;
    std::vector<int> indices;
    std::map<int, wavetable_morph_control_row*> sorted_rowmap;

    for(current_row = &first_morph_row; current_row != nullptr; current_row = current_row->next)
    {
        if(current_row->include_btn->get_active ())
        {
            num_tables++;
            int insertpos = (int)strtol(current_row->morph_order->get_text ().c_str (), nullptr, 10);
            rowmap.insert (std::pair<int, wavetable_morph_control_row*>(insertpos, current_row));
            indices.insert(indices.begin (), insertpos);
        }
    }

    std::sort (indices.begin (), indices.end ());

    if(num_tables <= 1)
    {
        auto dialog = new Gtk::MessageDialog("Too few wavetables selected!");
        dialog->set_modal ( true );
        dialog->set_visible ( true );

        /* TODO: I'm pretty sure I'm not supposed to do it this way but the documentation isn't very good
         * so this works until I find out the correct way */
        reinterpret_cast<Gtk::Button*>(reinterpret_cast<Gtk::ButtonBox*>(reinterpret_cast<Gtk::Box*>
        (reinterpret_cast<Gtk::Box*>(dialog->get_child ())->get_children ().at (1))->get_children ().at(0))
                ->get_children ().at(0))->signal_clicked ().connect
                ( sigc::bind<Gtk::MessageDialog*>(sigc::ptr_fun ( close_dialog ), dialog) );

        dialog->show();
        return;
    }

    ctrls.morph_harmonics.num_wavetables = num_tables;

    if(ctrls.morph_harmonics.spectrums)free(ctrls.morph_harmonics.spectrums);

    ctrls.morph_harmonics.spectrums = (wavetable_harmonics*)calloc(num_tables, sizeof(wavetable_harmonics));

    for(i = 0; i < indices.size (); i++)
    {
        sorted_rowmap.insert (std::pair<int, wavetable_morph_control_row*>(i, rowmap[indices[i]]));
        memcpy(&ctrls.morph_harmonics.spectrums[i], rowmap[indices[i]]->wavetable->harmonics, sizeof(wavetable_harmonics));
    }

    gint morphtype;

    ctrls.morph_combobox->get_active ()->get_value (0, morphtype);

    ctrls.morph_harmonics.morph_style = (morph_type)morphtype;

    if(ctrls.morph_harmonics.morph_style == morph_type::MORPH_ENVELOPE)
    {
        ctrls.morph_harmonics.morph_env = ctrls.morph_envelope;
    }

    ctrls.mwm = generate_morph_wavetables (&ctrls.morph_harmonics, 44100);

    generate_morph_surface_vertices(ctrls.mwm);
}