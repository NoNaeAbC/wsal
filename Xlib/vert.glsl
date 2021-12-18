#version 460

layout(location = 0) out vec3 inColor;

out gl_PerVertex{
	vec4 gl_Position;
};

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 col;

layout(push_constant) uniform Offset{
	vec3 offset;
}push;

layout(binding = 0) uniform UBO{
	mat4 MVP;
} ubo;

void main() {
	inColor = col;
	
	gl_Position = ubo.MVP *  vec4(pos + push.offset, 1.0);
}
