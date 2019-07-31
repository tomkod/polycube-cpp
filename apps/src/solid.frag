#version 430

in vec4 v, c, t, p;
in vec3 n;

uniform vec4 uColor;

out vec4 fragColor;

void main()
{
    fragColor = c * uColor;
}

