#version 430

layout(location = 0) in vec3 vert_position;
layout(location = 1) in vec2 vert_texcoord;
layout(location = 2) in vec3 vert_normal;

layout(std430) buffer Camera {
	mat4 P;
	mat4 V;
	mat4 M;
};

layout(std430) buffer CurrentTransform {
	mat4 CT;
};

out gl_PerVertex {
	vec4 gl_Position;
    float gl_PointSize;
    float gl_ClipDistance[];
};

out vec2 texco;
out vec4 vert_Position;
out vec4 vert_Normal;
out mat4 MVP;
out mat4 modelView;

void main(){

	modelView = V * CT;
	MVP = P * modelView;

	// vertex position and normal in eye space
	vert_Position = modelView * vec4(vert_position, 1.0);
   	vert_Normal = normalize(modelView * vec4(vert_normal, 0.0));
	gl_Position = P * V * CT * vec4(vert_position, 1.0);
	texco = vert_texcoord;
}
