#version 460
layout(early_fragment_tests) in;

layout(location = 0) in vec2 fTexPos;
layout(location = 0) out vec4 outColor;

layout(binding = 1) uniform sampler2D texSampler;

void main() {
	outColor = texture(texSampler, fTexPos);
	//outColor = vec4(1.0f,0.0f,1.0f,1.0f);

}