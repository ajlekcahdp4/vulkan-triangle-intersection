#version 460

// from vertex shader
in vec3 vColor;

void main() {  
	gl_FragColor = vec4(vColor, 1.0);
}