#version 430
layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

in vec4 vert_Position;
in vec4 vert_Normal;
in mat4 MVP;
in mat4 modelView;

in gl_PerVertex
{
    vec4 gl_Position;
    float gl_PointSize;
    float gl_ClipDistance[];

} gl_in[];

out gl_PerVertex
{
    vec4 gl_Position;
    float gl_PointSize;
    float gl_ClipDistance[];
};

out vec4 vert_Position;
out vec4 vert_Normal;
out mat4 MVP;
out mat4 modelView;


void main() {
  for(int i = 0; i < 3; i++) { // You used triangles, so it's always 3
    gl_Position = gl_in[i].gl_Position;
    EmitVertex();
  }
  EndPrimitive();
}