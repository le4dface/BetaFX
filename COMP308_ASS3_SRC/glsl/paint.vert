#version 430

layout(location = 0) in vec3 vert_position;
layout(location = 1) in vec2 vert_texcoord;
layout(location = 2) in vec3 vert_normal;
layout(location = 3) in vec2 vert_rand;

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


layout(std430) buffer CurrentTransform {
	mat4 CT;
};

out vec4 vert_Color;
out vec4 vert_Normal;
out vec2 random;

out gl_PerVertex
{
    vec4 gl_Position;
    float gl_PointSize;
    float gl_ClipDistance[];
};


void main() {

	//add ct back in later
	mat4 modelView = V * CT;
	mat4 MVP = P * modelView;

	// vertex position and normal in eye space
	vec4 vert_Position = modelView * vec4(vert_position, 1.0);
   	vert_Normal = normalize(modelView * vec4(vert_normal, 0.0));
	gl_Position =  MVP * vec4(vert_position, 1.0);

	// do the lighting calculations per vertex
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

	Iamb += 0.01 * Ka * lights[0].La;
	if(angle < lights[0].cutoffAngle) {

		float intensity = (lights[0].cutoffAngle - angle)/(lights[0].cutoffAngleOut- lights[0].cutoffAngle);
		intensity = clamp(intensity, 0.0, 1.0);

		//calculate Ambient Term:

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

	vert_Color = Iamb + Idiff + Ispec;
	vert_Color.a = 1.0;

	random = vert_rand;

}
