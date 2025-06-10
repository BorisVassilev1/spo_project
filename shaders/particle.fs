#include <rendering.glsl>

in vec4 vColor;
in vec2 vTexCoord;
in vec3 vVertexNormal;
in vec3 vVertexPos;

out vec4 fragColor;

uniform float particleRadius;

void main() {
	
	fragColor = vColor;
	
	//fragColor = vec4(vec3(sin(vVertexPos.x* 100 + vVertexPos.y* 100)), 1.0);
}
