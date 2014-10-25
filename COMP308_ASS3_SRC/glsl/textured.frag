#version 430

uniform sampler2D frameTexture;
uniform float texScale;

struct Light {
	vec4 LightPosition;
	vec4 La; //Ambient light intensity
	vec4 Ld; //Diffuse light intensity
	vec4 Ls; //Specular light intensity
	vec4 direction;
	float cutoffAngle;
	float cutoffAngleOut;
	float a, b;
};

layout(std430) buffer LightData {
	Light lights[4];
};


layout(std430) buffer MaterialInfo {
	vec4 Ka;
	vec4 Kd;
	vec4 Ks;
	float shininess;
	float billSize;
	float offsetSize;
	float pad;
};

layout(std430) buffer Camera {
	mat4 P;
	mat4 V;
	mat4 M;
};

in vec2 texco;
in vec4 vert_Position;
in vec4 vert_Normal;
in mat4 MVP;
in mat4 modelView;

out vec4 color;

void main(){




	vec4 Iamb = vec4(0);
	vec4 Idiff = vec4(0);
	vec4 Ispec = vec4(0);

	// light position in eye coords
	vec4 lightpos_eyecoord = V * lights[0].LightPosition;
	vec4 lightpos_eyecoord1 = V * lights[1].LightPosition;
	vec4 lightpos_eyecoord2 = V * lights[2].LightPosition;
	vec4 lightpos_eyecoord3 = V * lights[3].LightPosition;

	vec4 N = vert_Normal;


	vec4 lightdir_eyecoord = normalize(V * (lights[0].direction - lights[0].LightPosition));

	vec4 E = normalize(-vert_Position);
	vec4 L = normalize(lightpos_eyecoord - vert_Position); //direction of incoming light
	vec4 R = normalize(-reflect(L,N));

	vec4 L1 = normalize(lightpos_eyecoord1 - vert_Position);
	vec4 L2 = normalize(lightpos_eyecoord2 - vert_Position);
	vec4 L3 = normalize(lightpos_eyecoord3 - vert_Position);

	vec4 R1 = normalize(-reflect(L1,N));
	vec4 R2 = normalize(-reflect(L2,N));
	vec4 R3 = normalize(-reflect(L3,N));

	float angle = dot(-L,lightdir_eyecoord);
	angle = degrees(acos(angle));

	if(angle < lights[0].cutoffAngle) {

		float intensity = (lights[0].cutoffAngle - angle)/(lights[0].cutoffAngleOut- lights[0].cutoffAngle);
		intensity = clamp(intensity, 0.0, 1.0);

		//calculate Ambient Term:
		Iamb += intensity * Ka * lights[0].La;

		//calculate Diffuse Term:
		Idiff += intensity * Kd * lights[0].Ld * max( dot(N,L), 0.0 );

		// calculate Specular Term:
		Ispec += intensity * Ks * lights[0].Ls * pow(max(dot(R,E), 0.0), shininess);

	}

	Iamb += Ka * lights[1].La;
	Iamb += Ka * lights[2].La;
	Iamb += Ka * lights[3].La;

	Idiff += Kd * lights[1].Ld * max( dot(N,L1), 0.0 );
	Idiff += Kd * lights[2].Ld * max( dot(N,L2), 0.0 );
	Idiff += Kd * lights[3].Ld * max( dot(N,L3), 0.0 );

	Ispec += Ks * lights[1].Ls * pow(max(dot(R1,E), 0.0), shininess);
	Ispec += Ks * lights[2].Ls * pow(max(dot(R2,E), 0.0), shininess);
	Ispec += Ks * lights[3].Ls * pow(max(dot(R3,E), 0.0), shininess);

	Iamb = clamp(Iamb, 0.0, 1.0);
	Idiff = clamp(Idiff, 0.0, 1.0);
	Ispec = clamp(Ispec, 0.0, 1.0);

	// write Total Color:
	color = Iamb + vec4(Idiff * texture2D(frameTexture, (texco*texScale))) + Ispec;
}
















