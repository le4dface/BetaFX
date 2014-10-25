#version 430


uniform sampler2D frameTexture;

in vec4 vertColor;
in vec2 texCoordinate;

out vec4 color;

void main(){

	// write Total Color:
	// texturing still goes here
	vec4 tex = texture(frameTexture, texCoordinate*1);
	color = vec4(vertColor * tex);
	color.a = tex.a;
	if(tex.a < 0.1)
		discard;

}

