#version 430

uniform mat4 uModelViewMatrix;
uniform mat4 uProjectionMatrix;
uniform mat3 uNormalMatrix;

layout (location = 0) in vec4 aVertex;
layout (location = 1) in vec4 aTexCoord;
layout (location = 2) in vec3 aNormal;
layout (location = 3) in vec4 aColor;

out vec4 v, c, t, p;
out vec3 n;

void main()
{
   v = aVertex;
   c = aColor;
   t = aTexCoord;
   n = uNormalMatrix * aNormal;
   p = uModelViewMatrix * v;
   gl_Position = uProjectionMatrix * p;
}
