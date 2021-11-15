#version 330

out vec2 vTexCoord;
uniform float Aspect;
void main(){
    vTexCoord = vec2((gl_VertexID << 1) & 2, gl_VertexID & 2);
	gl_Position = vec4(vTexCoord * vec2(2, -2) + vec2(-1, 1), 0, 1);
    vTexCoord = vTexCoord * 2 -  vec2(1);
    vTexCoord *= -vec2(Aspect,1);
}