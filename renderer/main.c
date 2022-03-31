#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "core/api.h"
#include "shaders/cache_helper.h"
#include "tests/test_blinn.h"
#include "tests/test_pbr.h"

#include "tests/test_helper.h"
#include "core/mesh.h"
#include "core/graphics.h"

#include "shaders/unlit_shader.h"

typedef void testfunc_t(int argc, char *argv[]);
typedef struct {const char *testname; testfunc_t *testfunc;} testcase_t;

static testcase_t g_testcases[] = {
    {"blinn", test_blinn},
    {"pbr", test_pbr},
};

program_t* program;

mesh_t* test_mesh;

static void test_update(program_t* program, perframe_t* perframe) {
    float ambient_intensity = perframe->ambient_intensity;
    float punctual_intensity = perframe->punctual_intensity;
    mat3_t normal_matrix;
    unlit_uniforms_t* uniforms;

    uniforms = (unlit_uniforms_t*)program_get_uniforms(program);
    uniforms->camera_pos = perframe->camera_pos;
    uniforms->model_matrix = mat4_scale(0.5f, 0.5f, 0.5f);
    uniforms->camera_vp_matrix = mat4_mul_mat4(perframe->camera_proj_matrix,
        perframe->camera_view_matrix);
}

static void test_draw_model(mesh_t* mesh, program_t* program, framebuffer_t* framebuffer) {
    int num_faces = mesh_get_num_faces(mesh);
    vertex_t* vertices = mesh_get_vertices(mesh);
    unlit_uniforms_t* uniforms;
    unlit_attribs_t* attribs;
    int i, j;

    const vec3_t colors[3] = {
       {1.0f, 0.0f, 0.0f},
       {0.0f, 1.0f, 0.0f},
       {0.0f, 0.0f, 1.0f},
    };

    for (i = 0; i < num_faces; i++) {
        for (j = 0; j < 3; j++) {
            vertex_t vertex = vertices[i * 3 + j];
            attribs = (unlit_attribs_t*)program_get_attribs(program, j);
            attribs->position = vertex.position;
            attribs->color = colors[j];
        }
        graphics_draw_triangle(framebuffer, program);
    }
}

static void tick_function(context_t* context, void* userdata) {
    //scene_t* scene = (scene_t*)userdata;
    
    vec3_t light_dir = vec3_normalize(context->light_dir);
    camera_t* camera = context->camera;
    perframe_t perframe;

    perframe.frame_time = context->frame_time;
    perframe.delta_time = context->delta_time;
    perframe.light_dir = light_dir;
    perframe.camera_pos = camera_get_position(camera);
    perframe.light_view_matrix = mat4_identity();
    perframe.light_proj_matrix = mat4_identity();;
    perframe.camera_view_matrix = camera_get_view_matrix(camera);
    perframe.camera_proj_matrix = camera_get_proj_matrix(camera);
    perframe.ambient_intensity = 0.5f;
    perframe.punctual_intensity = 1.0f;
    perframe.shadow_map = NULL;
    perframe.layer_view = -1;

    test_update(program, &perframe);

    framebuffer_clear_color(context->framebuffer, (vec4_t){1.0f, 0.5f, 0.8f, 1.0f});
    framebuffer_clear_depth(context->framebuffer, 1);

    test_draw_model(test_mesh, program, context->framebuffer);

    /*
    unlit_uniforms_t* uniforms = (unlit_uniforms_t*)program_get_uniforms(program);

    unlit_attribs_t* attribs;
    for (int i = 0; i < 1; i++) {
        for (int j = 0; j < 3; j++) {
            attribs = (unlit_attribs_t*)program_get_attribs(program, j);
            attribs->position = position[j];
            attribs->color = colors[j];
        }
        graphics_draw_triangle(context->framebuffer, program);
    }*/

    //test_draw_scene(scene, context->framebuffer, &perframe);
}

void test_scene(void)
{
    int sizeof_attribs = sizeof(unlit_attribs_t);
    int sizeof_varyings = sizeof(unlit_varyings_t);
    int sizeof_uniforms = sizeof(unlit_uniforms_t);
    program = program_create(
        unlit_vertex_shader,
        unlit_fragment_shader,
        sizeof_attribs, sizeof_varyings, sizeof_uniforms, 1, 0);

    unlit_uniforms_t* uniforms = (unlit_uniforms_t*)program_get_uniforms(program);

    uniforms->model_matrix = mat4_identity();
    uniforms->camera_vp_matrix = mat4_identity();

    test_mesh = mesh_load("susanne/susanne.obj");

    test_enter_mainloop(tick_function, NULL);

    mesh_release(test_mesh);
}

int main(int argc, char *argv[]) {
    int num_testcases = ARRAY_SIZE(g_testcases);
    const char *testname = NULL;
    testfunc_t *testfunc = NULL;
    int i;

    srand((unsigned int)time(NULL));
    platform_initialize();

    test_scene();

    platform_terminate();
    cache_cleanup();
    return 0;

    if (argc > 1) {
        testname = argv[1];
        for (i = 0; i < num_testcases; i++) {
            if (strcmp(g_testcases[i].testname, testname) == 0) {
                testfunc = g_testcases[i].testfunc;
                break;
            }
        }
    } else {
        i = rand() % num_testcases;
        testname = g_testcases[i].testname;
        testfunc = g_testcases[i].testfunc;
    }

    if (testfunc) {
        printf("test: %s\n", testname);
        testfunc(argc, argv);
    } else {
        printf("test not found: %s\n", testname);
        printf("available tests: ");
        for (i = 0; i < num_testcases; i++) {
            if (i != num_testcases - 1) {
                printf("%s, ", g_testcases[i].testname);
            } else {
                printf("%s\n", g_testcases[i].testname);
            }
        }
    }

    platform_terminate();
    cache_cleanup();

    return 0;
}
