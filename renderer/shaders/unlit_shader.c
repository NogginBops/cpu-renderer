#include <math.h>
#include <stdlib.h>
#include "../core/api.h"
#include "unlit_shader.h"
#include "cache_helper.h"

vec4_t unlit_vertex_shader(void* attribs_, void* varyings_, void* uniforms_)
{
    unlit_attribs_t* attribs = (unlit_attribs_t*)attribs_;
    unlit_varyings_t* varyings = (unlit_varyings_t*)varyings_;
    unlit_uniforms_t* uniforms = (unlit_uniforms_t*)uniforms_;

    varyings->world_position = attribs->position;
    varyings->color = attribs->color;

    vec3_t p = attribs->position;
    return (vec4_t) { p.x, p.y, p.z, 1.0f };
}

vec4_t unlit_fragment_shader(void* varyings_, void* uniforms_, int* discard, int backface) 
{
    unlit_varyings_t* varyings = (unlit_varyings_t*)varyings_;
    unlit_uniforms_t* uniforms = (unlit_uniforms_t*)uniforms_;

    return vec4_from_vec3(varyings->color, 1.0f);
}

static void update_model(model_t* model, perframe_t* perframe) 
{
    float ambient_intensity = perframe->ambient_intensity;
    float punctual_intensity = perframe->punctual_intensity;
    mat4_t model_matrix = model->transform;
    unlit_uniforms_t* uniforms;

    uniforms = (unlit_uniforms_t*)program_get_uniforms(model->program);
    
    uniforms->camera_pos = perframe->camera_pos;
    uniforms->model_matrix = model_matrix;
    uniforms->camera_vp_matrix = mat4_mul_mat4(perframe->camera_proj_matrix,
        perframe->camera_view_matrix);
}

static void draw_model(model_t* model, framebuffer_t* framebuffer, int shadow_pass) 
{
    mesh_t* mesh = model->mesh;
    int num_faces = mesh_get_num_faces(mesh);
    vertex_t* vertices = mesh_get_vertices(mesh);
    program_t* program = model->program;
    unlit_uniforms_t* uniforms;
    unlit_attribs_t* attribs;
    int i, j;

    const vec3_t colors[3] = {
        {1.0f, 0.0f, 0.0f},
        {0.0f, 1.0f, 0.0f},
        {0.0f, 0.0f, 1.0f},
    };

    uniforms = (unlit_uniforms_t*)program_get_uniforms(model->program);
    uniforms->shadow_pass = shadow_pass;
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

static void release_model(model_t* model) {
    unlit_uniforms_t* uniforms;
    uniforms = (unlit_uniforms_t*)program_get_uniforms(model->program);
    program_release(model->program);
    cache_release_mesh(model->mesh);
    free(model);
}

model_t* unlit_create_model(const char* mesh, mat4_t transform)
{
	int sizeof_attribs = sizeof(unlit_attribs_t);
	int sizeof_varyings = sizeof(unlit_varyings_t);
	int sizeof_uniforms = sizeof(unlit_uniforms_t);
	unlit_uniforms_t* uniforms;
	program_t* program;
	model_t* model;

	program = program_create(
		unlit_vertex_shader,
		unlit_fragment_shader,
		sizeof_attribs, sizeof_varyings, sizeof_uniforms, 1, 0);

	uniforms = (unlit_uniforms_t*)program_get_uniforms(program);
	
	model = (model_t*)malloc(sizeof(model_t));
	model->mesh = cache_acquire_mesh(mesh);
	model->program = program;
	model->transform = transform;
	model->skeleton = NULL;
	model->attached = 0;
	model->opaque = 1;
	model->distance = 0;
	model->update = update_model;
	model->draw = draw_model;
    model->release = release_model;
}



