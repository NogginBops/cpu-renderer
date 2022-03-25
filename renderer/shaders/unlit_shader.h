#ifndef UNLIT_SHADER_H
#define UNLIT_SHADER_H

typedef struct {
	vec3_t position;
	vec3_t color;
} unlit_attribs_t;

typedef struct {
	vec3_t world_position;
	vec3_t color;
} unlit_varyings_t;

typedef struct {
	vec3_t camera_pos;
	mat4_t model_matrix;
	mat4_t camera_vp_matrix;
	int shadow_pass;
} unlit_uniforms_t;

vec4_t unlit_vertex_shader(void* attribs, void* varyings, void* uniforms);
vec4_t unlit_fragment_shader(void* varyings, void* uniforms, int* discard, int backface);

model_t* unlit_create_model(const char* mesh, mat4_t transform);

#endif