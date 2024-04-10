#version 330                                    
uniform vec4 u_color;                               
in float v_lifetime;
in vec4 vertexColor;
layout(location = 0) out vec4 fragColor;       

void main()                                         
{                                                   
  fragColor = u_color;
  fragColor.a *= v_lifetime;                        
}  