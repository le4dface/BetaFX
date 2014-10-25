#version 430

layout(triangles) in;
layout(triangle_strip, max_vertices = 48) out;

layout(std430) buffer Camera {
	mat4 P;
	mat4 V;
	mat4 M;
};

layout(std430) buffer Rand {
	vec4 rand;
};



in vec4 vert_Color[];
in vec4 vert_Normal[];
in vec2 random[];

in gl_PerVertex
{
    vec4 gl_Position;
    float gl_PointSize;
    float gl_ClipDistance[];

} gl_in[];

out vec4 vertColor;
out vec2 texCoordinate;

out gl_PerVertex
{
    vec4 gl_Position;
    float gl_PointSize;
    float gl_ClipDistance[];
};

uniform float billSize; //billboard size
uniform float offset; //offset size

void main() {

//if ( vert_Normal[0].z > 0) {

vec4 boardPosition = gl_in[0].gl_Position * 0.33 +
			gl_in[1].gl_Position * 0.33 +
			gl_in[2].gl_Position * 0.33;

	vertColor = vert_Color[0] * 0.33 +
				vert_Color[1] * 0.33 +
				vert_Color[2] * 0.33;

	vec4 myRand = vec4(random[0], 0.0, 0.0);

	float trisize = billSize*(random[1].x  + 1) * sqrt(distance(gl_in[0].gl_Position, gl_in[2].gl_Position) * distance(gl_in[0].gl_Position, gl_in[1].gl_Position));
	//myRand.x = (gl_in[0].gl_Position.x);
	//myRand.y = (gl_in[0].gl_Position.y);

	vec4 randomOffset = vec4(0.0);
	float rscale = trisize * offset;

	for(int i = 0; i < 12; ++i) {

		myRand.y = myRand.y * (1.0 - myRand.y) * 4.0;
		myRand.x = myRand.x * (1.0 - myRand.x) * 4.0;

		randomOffset = vec4(rscale*(myRand.x - 0.5), rscale*(myRand.y - 0.5), 0.0, 0.0);

		gl_Position = (vec4(-trisize,-trisize, 0.0, 0.0) + boardPosition + randomOffset);
		texCoordinate = vec2(0.0,0.0);
		EmitVertex();

		gl_Position = (vec4(trisize, -trisize, 0.0, 0.0) + boardPosition + randomOffset);
		texCoordinate = vec2(1.0,0.0);
		EmitVertex();

		gl_Position = (vec4(-trisize, trisize, 0.0, 0.0) + boardPosition + randomOffset);
		texCoordinate = vec2(0.0,1.0);
		EmitVertex();

		gl_Position = (vec4(trisize, trisize, 0.0, 0.0) + boardPosition + randomOffset);
		texCoordinate = vec2(1.0,1.0);
		EmitVertex();

		EndPrimitive();

	}

//}


}