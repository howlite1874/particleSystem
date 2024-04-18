#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aVel;
layout (location = 2) in float aSize; 
layout (location = 3) in float aLifetime;
layout (location = 4) in float aCurtime;

uniform float u_time;
uniform vec3 u_acceleration; 

void main()
{            
    float deltaTime = u_time - aCurtime;                          
    if ( deltaTime <= aLifetime )                                 
    {                                                              
        vec3 velocity = aVel + deltaTime * u_acceleration;    
        vec3 position = aPos + deltaTime * velocity;          
        gl_Position = vec4( position.xy,0.0,1.0 );                   
        gl_PointSize = aSize * ( 1.0 - deltaTime / aLifetime );   
    }                                                              
    else                                                           
    {                                                              
        gl_Position = vec4( -1000, -1000, 0, 0 );                   
        gl_PointSize = 0.0;    
    }
}