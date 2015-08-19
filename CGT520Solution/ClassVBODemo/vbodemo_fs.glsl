#version 150
in vec4 vColor; //varying variable
in vec3 vPos;
out vec4 fragcolor;
void main(void)
{
 
  vec2 f = abs(fract(vPos.xy)-vec2(0.5, 0.5));
  float d = min(f.x, f.y);
  float a = smoothstep(0.0, 0.1, d);
  fragcolor = mix(vec4(0.0, 0.0, 0.0, 1.0), vColor, a);
}
