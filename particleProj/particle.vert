#version 330                                    
uniform float u_time;                                
uniform vec3 u_centerPosition;                       
layout(location = 0) in vec3 a_Position;        
layout(location = 1) in vec3 a_Velocity;    

out vec3 vPos;

void main()                                          
{                                                                                             
    gl_Position.xyz = a_Position + u_time * a_Velocity;      
    gl_Position.w = 1.0;                                                                           
    gl_PointSize = ( u_time * u_time ) * 40.0;
    vPos = a_Position;
}