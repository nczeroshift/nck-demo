
/*
* Output shader for
* FXAA Brightness, Contrast and Saturation Shader
* 
* FXAA from  Matt DesLauriers 
* https://github.com/mattdesl/glsl-fxaa
*/

#pragma vertex_shader_glx2

#include "fxaa.cpp"
uniform vec4 texResolution;
	
void main(){
	vec2 fCoord = gl_MultiTexCoord0.xy * texResolution.xy;
    fxaa_map(fCoord, texResolution.xy);
	gl_TexCoord[0] = gl_MultiTexCoord0;
	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
}

#pragma fragment_shader_glx2
#include "grain.cpp"
#include "fxaa.cpp"


uniform sampler2D gphTexture0;
uniform vec4 texResolution;

uniform float contrast;
uniform float brightness;
uniform float saturation;

uniform float time;

vec3 rgb2hsv(vec3 c)
{
    vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
    vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
    vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));

    float d = q.x - min(q.w, q.y);
    float e = 1.0e-10;
    return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
}

vec3 hsv2rgb(vec3 c)
{
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}


void main()
{
	vec2 uv = gl_TexCoord[0].xy;
	vec2 fCoord = uv * texResolution.xy; 
    vec4 col = fxaa_apply(gphTexture0, fCoord, texResolution.xy);
    vec3 grain = grain_apply(col.xyz,uv,time,texResolution.x,texResolution.y,0.025,false,0.6,1.6,1.0);
    
	vec3 hsv = rgb2hsv(grain.xyz);
	hsv = hsv * vec3(1.0, 1.0 + saturation, 1.0);
	vec3 rgb = hsv2rgb(hsv);

    
	rgb *= (1.0 + contrast);
	rgb += brightness;
 
	gl_FragColor = vec4(0.5*rgb,col.a);
}