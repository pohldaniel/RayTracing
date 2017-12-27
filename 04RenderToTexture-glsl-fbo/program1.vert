#version 410 core

// calles for every vertex between glUseProgram(program); ---> glUseProgram(0);
in vec4 v_coord;

void main(void )
{
  gl_Position = v_coord;
}
