#version 430

uniform mat4 uModelViewMatrix;
uniform mat4 uProjectionMatrix;
uniform mat3 uNormalMatrix;

layout (location = 0) in vec3 aVertex;
layout (location = 1) in vec2 aTexCoord;
layout (location = 2) in vec3 aNormal;

out vec4 v, c, t, p;
out vec3 n;

void main()
{
   v = vec4(aVertex, 1);
   c = vec4(1);
   t = vec4(aTexCoord, 0, 1);
   n = uNormalMatrix * aNormal;
   p = uModelViewMatrix * v;
   gl_Position = uProjectionMatrix * p;
}
