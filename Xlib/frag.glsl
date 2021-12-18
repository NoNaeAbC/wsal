#version 460

layout(location = 0) out vec4 outColor;
layout(location = 0) in vec3 inColor;

float mapLinear(const float value, const float lowerInput, const float upperInput, const float lowerOutput,
const float upperOutput) {
	return ((value - lowerInput) * ((upperOutput - lowerOutput) / (upperInput - lowerInput))) + lowerOutput;
}

void main() {
	outColor = vec4(inColor, 1.0);
}
