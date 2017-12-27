#version 410 core

in vec4 v_coord;
uniform mat4 u_projection;

void main(void)
{
   gl_Position = u_projection * v_coord;
}