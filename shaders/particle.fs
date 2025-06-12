#include <rendering.glsl>

in vec4 vColor;
in vec2 vTexCoord;
in vec3 vVertexNormal;
in vec3 vVertexPos;

out vec4 fragColor;

uniform float particleRadius;
uniform float sharpness;

void main() {
	
	fragColor = vColor;
	if(length(vVertexPos) > 1.f) {
		discard;
	}

	float fact = 1.0 - length(vVertexPos);
	fact = pow(fact, sharpness);

	fragColor = vec4(fact * vColor.rgb, vColor.a);
}
