//Shader:BasicShader, VS_Entry: main, PS_Entry: main, Defines: Test, Check, Something, another

#ifdef VS

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 0) out vec3 fragColor;

// per instance stuff
// eventual object organisation should be array of structs rather then individual arrays
layout(binding = 0, set = 0) buffer StorageBuffer {
	mat4 globalTransform[];
} sbo;

// probably gonna end up as a camera kind of thing + other per pass stuff
layout(binding = 0, set = 1) uniform UniformBufferObject {
    mat4 view;
    mat4 proj;
} ubo;

void main() 
{
    gl_Position = ubo.proj * ubo.view * sbo.globalTransform[gl_InstanceIndex] * vec4(inPosition, 0.0, 1.0);
    fragColor = inColor;
}

#endif

#ifdef PS


layout(location = 0) in vec3 fragColor;
layout(location = 0) out vec4 outColor;

void main() {
    outColor = vec4(fragColor, 1.0);
}

#endif
