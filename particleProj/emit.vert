#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aVel;
layout (location = 2) in float aSize;    
layout (location = 3) in float aLifetime;
layout (location = 4) in float aCurtime;

out vec3  outPos;
out vec3  outVel;
out float outSize;
out float outLifetime;
out float outCurtime;

uniform float u_time;
uniform sampler3D s_noiseTex;
uniform float u_emissionRate;    

float randomValue( inout float seed )                              
{                                                                  
   float vertexId = float( gl_VertexID ) / float( 200 ); 
   vec3 texCoord = vec3( u_time, vertexId, seed );                 
   seed += 0.1;                                                    
   return texture( s_noiseTex, texCoord ).r;                       
}    

void main()
{
    float seed = u_time;  
    float deltaTime = u_time - aCurtime;
    float lifetime = aCurtime - u_time;  
    if(lifetime <= 0.0 && randomValue(seed) < u_emissionRate){
        outVel = vec3(randomValue(seed) * 2.0 - 1.0,    
                       randomValue(seed) * 1.4 + 1.0,0);   
        outPos = vec3(0,0,0);
        outSize = randomValue(seed) * 20.0 + 60.0;  
        outLifetime = 2.0;   
        outCurtime = u_time;                                     
    } 
    else{
        outPos = aPos;
        outVel = aVel;
        outSize = aSize;   
        outLifetime = aLifetime;
        outCurtime = aCurtime;
    }
    
}