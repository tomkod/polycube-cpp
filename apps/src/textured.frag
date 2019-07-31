#version 430

uniform sampler2D uTexture0;
uniform vec3 uLightDir;
uniform vec4 uColor;

in vec4 v, c, t, p;
in vec3 n;

out vec4 fragColor;

void main()
{
   vec4 base = texture2D(uTexture0, t.xy) * uColor;
   vec3 norm = normalize(n);
   float diffuse = max(0.25, dot(norm, uLightDir));
   float spec = max(0.0,dot(reflect(uLightDir,norm),normalize(p.xyz)));
   spec = pow(spec, 16.0)*.5;
   fragColor = base*c*diffuse + spec*vec4(1.0)*c.w;
}

