
/**
* NCtoolKit © 2007-2015 Luís F.Loureiro, under zlib software license.
* https://github.com/nczeroshift/nctoolkit
*/

#pragma vertex_shader_glx2

void main()
{
	gl_TexCoord[0] = gl_MultiTexCoord0;
	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
}

#pragma fragment_shader_glx2

uniform sampler2D gphTexture0;
uniform vec4 direction;
uniform float kernelSize;
uniform vec4 invImageSize;
uniform float kernelWeights[33];

void main()
{
	vec4 res = vec4(0,0,0,0);
    float dist = clamp(0.5/length((gl_TexCoord[0].xy-vec2(0.5,0.5))*2),0,1);
    //float ks = min(floor(kernelSize * (1-dist)),1);
	float k = -kernelSize;
	int i = 0;
	for(;k <= kernelSize; k ++, i++){
        //
        //float kW = 1.0/(ks*2);
		res += kernelWeights[i] * texture2D(gphTexture0, gl_TexCoord[0].xy + direction.xy * k * invImageSize.xy);
	}
	gl_FragColor = mix(texture2D(gphTexture0, gl_TexCoord[0].xy),
        vec4(res.xyz,1.0),1-pow(dist,4.0));
}
