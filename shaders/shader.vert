#version 400

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (std140, binding = 0) uniform buffer {
  mat4 mvp;
  vec4 colors[2];
} uniform_buffer;

layout (location = 0) in vec4 pos;
layout (location = 1) in uint color_index;

layout (location = 0) out vec4 outColor;

void main() {
  outColor = uniform_buffer.colors[color_index];
  gl_Position = uniform_buffer.mvp * pos;
}