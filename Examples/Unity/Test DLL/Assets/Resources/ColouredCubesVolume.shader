Shader "ColouredCubesVolume"
{	
    SubShader
    {
      Tags { "RenderType" = "Opaque" }
      CGPROGRAM
      #pragma surface surf Lambert vertex:vert
      #pragma target 3.0
      #pragma only_renderers d3d9
      
      struct Input
      {
          float4 color : COLOR;
          float4 modelPos;
          float3 worldPos;
          float3 customColor;
      };
      
      #include "ColoredCubesVolumeUtilities.cginc"

		
		float positionBasedNoise(float4 positionAndStrength)
		{
			//'floor' is more widely supported than 'round'. Offset consists of:
			//  - An integer to push us away from the origin (divide by zero causes a ringing artifact
			//    at one point in the world, and we want to pushthis somewhere it won't be seen.)
			//  - 0.5 to perform the rounding
			//  - A tiny offset to prevent sparkes as faces are exactly on rounding boundary.
			float3 roundedPos = floor(positionAndStrength.xyz + vec3(1000.501));
		
			//Our noise function generate banding for high inputs, so wrap them
			roundedPos = fmod(roundedPos, float3(17.0, 19.0, 23.0));
		
			//Large number is arbitrary, but smaller number lead to banding. '+ 1.0' prevents divide-by-zero
			float noise = 100000000.0 / (dot(roundedPos, roundedPos) + 1.0);
			noise = fract(noise);
		
			//Scale the noise
			float halfNoiseStrength = positionAndStrength.w * 0.5;
			noise = -halfNoiseStrength + positionAndStrength.w * noise; //http://www.opengl.org/wiki/GLSL_Optimizations#Get_MAD
		
			return noise;
		}
      
      void vert (inout appdata_full v, out Input o)
      {
      	UNITY_INITIALIZE_OUTPUT(Input,o);
      	float4 unpackedPosition = float4(unpackPosition(v.vertex.x), 1.0f);
      	float4 unpackedColor = float4(unpackColor(v.vertex.y), 1.0f);
      	
      	v.vertex = unpackedPosition;
      	
      	v.normal = float3 (0.0f, 0.0f, 1.0f);

    v.tangent = float4 (1.0f, 0.0f, 0.0f, 1.0f);     
          
          
          o.modelPos = v.vertex;
          
          //o.customColor = float3(0.0, v.texcoord.x, 0.0);
          //o.customColor = floatToRGB(v.texcoord.x);
          o.customColor = unpackedColor.xyz;
      }
      
      void surf (Input IN, inout SurfaceOutput o)
      {
      	// Compute the surface normal in the fragment shader.
      	float3 surfaceNormal = normalize(cross(ddx(IN.worldPos.xyz), ddy(IN.worldPos.xyz)));
      	
	    //Add noise
	    float noise = positionBasedNoise(float4(IN.modelPos.xyz, 0.1));
        
        o.Albedo = IN.customColor + noise;
        o.Normal = surfaceNormal;
      }
      ENDCG
    }
    Fallback "Diffuse"
  }
  