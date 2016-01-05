#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <OpenGL/gl3.h>

#include "platform.h"

#define LEN(e) (sizeof(e) / sizeof(e[0]))

typedef struct {
    bool is_initialized;

    // GL State
    GLuint shader;
    GLuint vao;
    GLuint vbo;
    int num_vertices;
} State;

const char* vertex_shader_source =
    "#version 150 core\n"
    
    "in vec4 position;"

    "uniform vec3 modelColor;"
    "uniform mat4 projection;"
    "uniform mat4 view;"

    "out vec3 vertColor;"

    "void main(void) {"
    "    vertColor = vec3(1,0,0);"
    "    gl_Position = projection * view * position;"
    "}"
    "";

const char* fragment_shader_source = {
    "#version 150 core\n"

    "in vec3 vertColor;"

    "out vec4 color;"

    "void main(void) {"
    "    color = vec4(vertColor.x, vertColor.y, vertColor.z, 1);"
    "}"
};

const float cube_vertices[] = {
    0,0,0,
    0,0,1,
    0,1,1,
    1,1,0,
    0,0,0,
    0,1,0,
    1,0,1,
    0,0,0,
    1,0,0,
    1,1,0,
    1,0,0,
    0,0,0,
    0,0,0,
    0,1,1,
    0,1,0,
    1,0,1,
    0,0,1,
    0,0,0,
    0,1,1,
    0,0,1,
    1,0,1,
    1,1,1,
    1,0,0,
    1,1,0,
    1,0,0,
    1,1,1,
    1,0,1,
    1,1,1,
    1,1,0,
    0,1,0,
    1,1,1,
    0,1,0,
    0,1,1,
    1,1,1,
    0,1,1,
    1,0,1,
};

typedef union {
    struct {
        float x; float y;
    };
    float e[2];
} V2;

typedef union {
    struct {
        float x; float y; float z;
    };
    struct {
        float r; float theta; float phi;
    };
    struct {
        V2 xy;
        float _z;
    };
    float e[3];
} V3;

V3 v3(float x, float y, float z)
{
    V3 result;
    
    result.x = x;
    result.y = y;
    result.z = z;

    return result;
}

typedef union {
    struct {
        float x; float y; float z; float w;
    };
    struct {
        float r; float g; float b; float a;
    };
    struct {
        float sw, se, nw, ne;
    };
    float e[4];
} V4;

typedef union {
    struct {
        V4 c1, c2, c3, c4;
    };
    float e[16]; // column major
} Mat4;

Mat4
mat4(float a11, float a21, float a31, float a41,
     float a12, float a22, float a32, float a42,
     float a13, float a23, float a33, float a43,
     float a14, float a24, float a34, float a44)
{
    Mat4 result;

    result.e[0] = a11;
    result.e[1] = a12;
    result.e[2] = a13;
    result.e[3] = a14;

    result.e[4] = a21;
    result.e[5] = a22;
    result.e[6] = a23;
    result.e[7] = a24;
    
    result.e[8] = a31;
    result.e[9] = a32;
    result.e[10] = a33;
    result.e[11] = a34;
    
    result.e[12] = a41;
    result.e[13] = a42;
    result.e[14] = a43;
    result.e[15] = a44;

    return result;
}

#define PI 3.14159265358979323846

float deg_to_rad(float degrees)
{
    return degrees * (float)(PI / 180);
}

float cotan(float x)
{
    return 1.0 / tan(x);
}

V3
v3_sub(V3 a, V3 b)
{
    V3 result;

    result.x = a.x - b.x;
    result.y = a.y - b.y;
    result.z = a.z - b.z;
    
    return result;
}

V3
normalize(V3 v)
{
    float magnitude = sqrt((v.x * v.x) + (v.y * v.y) + (v.z * v.z));
    if (magnitude != 0) {
        v.x = v.x / magnitude;
        v.y = v.y / magnitude;
        v.z = v.z / magnitude;
    }
    return v;
}

V3
cross(V3 a, V3 b)
{
    V3 result;

    result.x = a.y * b.z - a.z * b.y;
    result.y = a.z * b.x - a.x * b.z;
    result.z = a.x * b.y - a.y * b.x;

    return result;
}

float
dot(V3 a, V3 b)
{
    return a.x*b.x + a.y*b.y + a.z*b.z;
}

Mat4
perspective(float field_of_view,
            float display_ratio,
            float near_clip,
            float far_clip)
{
    float y_scale = cotan(deg_to_rad(field_of_view / 2.0));
    float x_scale = y_scale / display_ratio;
    float frustrum_length = far_clip - near_clip;

    Mat4 result = mat4(
        x_scale, 0,       0,                                              0,
        0,       y_scale, 0,                                              0,
        0,       0,      -(far_clip + near_clip)/(far_clip - near_clip),
        -((2 * near_clip * far_clip) / frustrum_length),
        0,       0,      -1,                                              0
    );
    
    return result;
}

Mat4
look_at(V3 eye,
        V3 center,
        V3 up)
{
    V3 f = normalize(v3_sub(center, eye));
    V3 s = normalize(cross(f, up));
    V3 u = normalize(cross(s, f));

    const Mat4 IDENTITY_MATRIX = mat4(
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
        );
    
    Mat4 result = IDENTITY_MATRIX;

    result.e[0*4+0] = s.x;
    result.e[1*4+0] = s.y;
    result.e[2*4+0] = s.z;
    result.e[0*4+1] = u.x;
    result.e[1*4+1] = u.y;
    result.e[2*4+1] = u.z;
    result.e[0*4+2] =-f.x;
    result.e[1*4+2] =-f.y;
    result.e[2*4+2] =-f.z;
    result.e[3*4+0] =-dot(s, eye);
    result.e[3*4+1] =-dot(u, eye);
    result.e[3*4+2] = dot(f, eye);
    
    return result;
}

void
game_update_and_render(PlatformMemory *memory)
{
    State *state = (State*)memory->raw_memory;
    if (!state->is_initialized) {
        fprintf(stderr, "Init\n");

        /* fprintf(stderr, "Shaders\n %s\n %s\n", vertex_shader_source, fragment_shader_source); */

        // Set up opengl shader
        GLint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertex_shader, 1, &vertex_shader_source, NULL);
        glCompileShader(vertex_shader);

        int shader_ok;
        glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &shader_ok);
        if (!shader_ok) {
            fprintf(stderr, "Failed to compile vertex shader\n");
            int log_length;
            char* log;

            glGetShaderiv(vertex_shader, GL_INFO_LOG_LENGTH, &log_length);

            log = (char*)malloc(log_length);
            glGetShaderInfoLog(vertex_shader, log_length, NULL, log);
            fprintf(stderr, "%s", log);
            free(log);
            exit(1);
        }

        GLint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragment_shader, 1, &fragment_shader_source, NULL);
        glCompileShader(fragment_shader);

        glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &shader_ok);
        if (!shader_ok) {
            fprintf(stderr, "Failed to compile fragment shader\n");
            int log_length;
            char* log;

            glGetShaderiv(fragment_shader, GL_INFO_LOG_LENGTH, &log_length);

            log = (char*)malloc(log_length);
            glGetShaderInfoLog(fragment_shader, log_length, NULL, log);
            fprintf(stderr, "%s", log);
            free(log);
            exit(1);
        }

        state->shader = glCreateProgram();
        glAttachShader(state->shader, vertex_shader);
        glAttachShader(state->shader, fragment_shader);
        glLinkProgram(state->shader);

        int program_ok;
        glGetProgramiv(state->shader, GL_LINK_STATUS, &program_ok);
        if (!program_ok) {
            fprintf(stderr, "Failed to link shader");

            int log_length;
            char* log;
            glGetProgramiv(state->shader, GL_INFO_LOG_LENGTH, &log_length);
            log = (char*)malloc(log_length);
            glGetProgramInfoLog(state->shader, log_length, NULL, log);
            fprintf(stderr, "%s", log);
            free(log);
            exit(1);
        }

        glDeleteShader(vertex_shader);
        glDeleteShader(fragment_shader);

        glUseProgram(state->shader);

        // set up opengl state.
        glGenVertexArrays(1, &state->vao);
        glBindVertexArray(state->vao);

        int position_i = glGetAttribLocation(state->shader, "position");
        
        glGenBuffers(1, &state->vbo);
        glBindBuffer(GL_ARRAY_BUFFER, state->vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vertices), cube_vertices, GL_STATIC_DRAW);
        glVertexAttribPointer(position_i, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(0);

        glBindVertexArray(0);
        glUseProgram(0);

        glEnable(GL_DEPTH_TEST);
        glClearColor(0.0f, 0.5f, 1.0f, 1.0f);
        
        state->is_initialized = true;
    }

    float color[3] = {1,0,0};

    V3 camera_lookat = v3(0,0,0);
    V3 camera_position = v3(5,5,10);

    Mat4 view = look_at(camera_position,
                        camera_lookat,
                        v3(0,0,1));


    float angle_of_view = 45;
    float near = 0.1;
    float far = 100;

    Mat4 projection = perspective(angle_of_view,
                                  memory->display_width / memory->display_height,
                                  near, far);

    glUseProgram(state->shader);
    glBindVertexArray(state->vao);
    
    int color_i = glGetUniformLocation(state->shader, "modelColor");
    glUniform3fv(color_i, 1, color);

    int view_i = glGetUniformLocation(state->shader, "view");
    glUniformMatrix4fv(view_i, 1, GL_FALSE, view.e);

    
    int projection_i = glGetUniformLocation(state->shader, "projection");
    glUniformMatrix4fv(projection_i, 1, GL_FALSE, projection.e);

    glDrawArrays(GL_TRIANGLES, 0, LEN(cube_vertices)/3);
    glBindVertexArray(0);    
}

Game game = {.update_and_render = game_update_and_render,
             .raw_memory_size = sizeof(State)};
