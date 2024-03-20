#version 460 core

in vec2 v_tex_coords;
out vec4 final;

uniform bool u_use_texture = true;
uniform sampler2D u_texture;

void main() {
    if (u_use_texture) {
        final = texture(u_texture, v_tex_coords).rgba;
    } else {
        final = vec4(1.0);
    }
}
