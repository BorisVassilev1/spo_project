
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

layout(location = 0) in vec4 particleData0;
layout(location = 1) in vec4 particleData1;

vec3 positions[3] = vec3[3](
	vec3(0.0, 0.0, 0.0),
	vec3(1.0, 0.0, 0.0),
	vec3(0.5, 1.0, 0.0)
);


void main() {
	vec3 particlePosition = particleData0.xyz;
	float particleMass = particleData0.w;
	vec3 particleVelocity = particleData1.xyz;

	// Calculate the position of the vertex
	//vec3 position = particlePosition + positions[gl_VertexID] * .1;
	vec3 position = particlePosition;
	//position.xy += .2 * positions[gl_VertexID % 3].xy;

	// Set the gl_Position
	//gl_Position = projectionMatrix * viewMatrix * vec4(position, 1.0);
	gl_Position = projectionMatrix * viewMatrix * worldMatrix * vec4(position, 1.0);

	gl_PointSize = 2.0;

	vVertexPos = position;


	if(drawMode == 0) {
		vColor = vec4(1.0, 1.0, 1.0, .3);
	}

	if(drawMode == 1) {
		float mass = 3000 * particleMass + 1000;

		float escapeVelocity = sqrt(2 * mass / length(particlePosition));
		vec3 normal = normalize(particlePosition);
		float dotProduct = dot(normal, particleVelocity);
		if(dotProduct > escapeVelocity) {
			vColor = vec4(0.0, 1.0, 0.0, 1.0);
		} 
	}

	if(drawMode == 2) {
		vec4 col2 = vec4(0.0, 1.0, 0.0, 1.0);
		vec4 col1 = vec4(1.0, 0.0, 0.0, .3);
		vColor = mix(col1, col2, particleMass / 10);
	}
}


