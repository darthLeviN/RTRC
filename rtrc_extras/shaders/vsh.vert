#version 460

layout(location = 0) in vec3 vPos;
layout(location = 1) in vec2 vTexPos;

layout(location = 2) in mat4 modelM;
//layout(location = 2) in mat4 m_0;
//layout(location = 3) in vec4 m_1;
//layout(location = 4) in vec4 m_2;
//layout(location = 5) in vec4 m_3;

layout(location = 0) out vec2 fTexPos;

layout(binding = 0) uniform UniformBufferObject
{
	mat4 vp;
} mats;

void main() {
	//gl_Position = mats.vp * mat4(m_0, m_1, m_2, m_3) * vec4(vPos.xyz, 1.0);
	gl_Position = mats.vp * modelM * vec4(vPos.xyz, 1.0);
	//gl_Position = mats.vp * vec4(vPos.xyz, 1.0);
	fTexPos = vTexPos;
}