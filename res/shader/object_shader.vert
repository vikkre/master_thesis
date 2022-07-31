#version 450


layout(set=0, binding=0) uniform GlobalData {
	mat4 view;
	mat4 proj;
	vec3 lightPosition;
} globalData;

layout(set=1, binding = 0) uniform ObjectData {
	mat4 model;
	vec3 color;
} objectData;

layout(location = 0) in vec3 vertPosition;
layout(location = 1) in vec3 vertNormal;

layout(location = 0) out vec3 fragColor;

void main() {
	gl_Position = globalData.proj * globalData.view * objectData.model * vec4(vertPosition, 1.0);

	vec3 cameraNormal = normalize((globalData.proj * globalData.view * objectData.model * vec4(vertNormal, 0.0)).xyz);
	vec3 cameraLightDirection = normalize((globalData.proj * globalData.view * vec4(globalData.lightPosition, 1.0)).xyz);
	float light = clamp(dot(cameraNormal, cameraLightDirection), 0, 1);

  vec3 cameraEyeDirection = normalize(vec3(0, 0, 0) - (globalData.proj * globalData.view * objectData.model * vec4(vertPosition, 1.0)).xyz);
	vec3 reflectNormal = reflect(-cameraLightDirection, cameraNormal);
	float specularLight = clamp(dot(cameraEyeDirection, reflectNormal), 0, 1);

	fragColor = vec3(0.2, 0.2, 0.2) * objectData.color            // ambient color
	          + vec3(0.3, 0.3, 0.3) * pow(specularLight, 2) * 0.3 // specular color
	          + objectData.color * light * 2.0;                   // diffuse color
}
