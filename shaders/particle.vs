
layout(std140, binding = 0) uniform Matrices {
	mat4 projectionMatrix;
	mat4 viewMatrix;
	mat4 cameraWorldMatrix;
};

out vec4 vColor;
out vec2 vTexCoord;
out vec3 vVertexNormal;
out vec3 vVertexPos;

uniform mat4 worldMatrix;
uniform float time;
uniform int drawMode = 0;
uniform int numParticles = 0;
uniform int N;

uniform int particleSize = 2;
uniform float transparency = .3;

layout(location = 0) in vec4 particleData0;
layout(location = 1) in vec4 particleData1;
layout(location = 2) in vec4 vertexData;

void main() {
	vec3 particlePosition = particleData0.xyz;
	float particleMass = particleData0.w;
	vec3 particleVelocity = particleData1.xyz;

	vec3 position = particlePosition + particleSize * vertexData.xyz * .1;

	// Set the gl_Position
	//gl_Position = projectionMatrix * viewMatrix * vec4(position, 1.0);
	gl_Position = projectionMatrix * viewMatrix * worldMatrix * vec4(position, 1.0);

	gl_PointSize = particleSize;

	vVertexPos = vertexData.xyz;


	if(drawMode == 0) {
		vColor = vec4(1.0, 1.0, 1.0, transparency);
	}

	if(drawMode == 1) {
		float mass = N + .4*N;

		float escapeVelocity = sqrt(2 * mass / length(particlePosition));
		vec3 normal = normalize(particlePosition);
		float dotProduct = dot(normal, particleVelocity);
		if(dotProduct > escapeVelocity) {
			vColor = vec4(0.0, 1.0, 0.0, 1.0);
		} 
	}

	if(drawMode == 2) {
		vec4 col2 = vec4(100.0, 100.0, 100.0, 1.0);
		vec4 col1 = vec4(1.0, .2, .05, transparency);
		vColor = mix(col1, col2, float(particleMass > 100.f));
	}
}


