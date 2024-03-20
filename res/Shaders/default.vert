#version 460 core

layout(location = 0) in vec2 position;
layout(location = 1) in vec2 tex_coords;

uniform mat4 u_projection = mat4(1.0);
uniform float u_scale = 1.0f;
out vec2 v_tex_coords;

void main() {
    mat4 model = mat4(
        u_scale, 0.0f, 0.0f, 0.0f,
        0.0f, u_scale, 0.0f, 0.0f,
        0.0f, 0.0f, u_scale, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    );
    gl_Position = u_projection * model * vec4(position.x, position.y, 0.0, 1.0);
    v_tex_coords = tex_coords;
}
