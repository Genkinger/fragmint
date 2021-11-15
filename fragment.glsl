#version 330
in vec2 vTexCoord;
out vec4 Color;
uniform float Time;
uniform ivec2 Resolution;
uniform vec2 Mouse;
uniform ivec2 Buttons;
uniform float Aspect;

#define MAX_ITER 512
#define MAX_DIST 128
#define EPS 0.00001

float Sphere(vec3 P, float R){
    return length(P) - R;
}

float Plane( vec3 P, vec3 N, float H ){
  return dot(P,N) + H;
}

float Union(float A, float B){
    return min(A,B);
}

float Intersect(float A, float B){
    return max(A,B);
}

float Subtract(float A, float B){
    return max(-A,B);
}

vec3 Rep(vec3 P, vec3 C){
    return mod(P+0.5*C,C)-0.5*C;
}

float Map(vec3 P){
    //P.z -=Time;
    float S1 = Sphere(P,0.5) + sin(20*P.y + Time)*sin(40*P.z + Time*20)*0.01;
    float P1 = Plane(P ,vec3(0,1,0),0.5);
    return Union(P1,S1);
}

vec3 Normal( vec3 P ) 
{
    const float H = EPS;
    const vec2 K = vec2(1,-1);
    return normalize( K.xyy*Map( P + K.xyy*H ) + 
                      K.yyx*Map( P + K.yyx*H ) + 
                      K.yxy*Map( P + K.yxy*H ) + 
                      K.xxx*Map( P + K.xxx*H ));
}


vec4 RayCast(vec3 RO, vec3 RD, float MinT, float MaxT){
    vec4 Res;
    int i = 0;
    float t = 0;
    for(t = MinT; t < MaxT; ){
        float H = Map(RO + t * RD);
        if(H < EPS){
            break;
        }
        t += H;
        i++;
    }
    return vec4(RO + t*RD,i);
}

float SoftShadow(vec3 RO, vec3 RD, float MinT, float MaxT, float K ){
    float Res = 1.0;
    for( float t=MinT; t<MaxT; )
    {
        float H = Map(RO + RD*t);
        if( H < EPS )
            return 0.0;
        Res = min( Res, K*H/t );
        t += H;
    }
    return Res;
}

float Shadow(vec3 RO, vec3 RD, float MinT, float MaxT){
    for( float t=MinT; t<MaxT; )
    {
        float H = Map(RO + RD*t);
        if( H < EPS )
            return 0.0;
        t += H;
    }
    return 1;
}

void main(){
    vec3 Col = vec3(0);

    vec3 LightPos = vec3(5*cos(Time*1.3),5,8*sin(Time*2));
    vec3 RO = vec3(0,0,1.2);
    
    vec3 RD = normalize(vec3(vTexCoord,-1));
    vec4 Res = RayCast(RO,RD,EPS,MAX_DIST);
    vec3 LightToPoint = normalize(Res.xyz-LightPos);
    vec3 N = Normal(Res.xyz);
    float Theta = max(0.1,dot(-LightToPoint,N));
    Col += (1 - Res.w/MAX_ITER);
    float S = max(0.1, Shadow(LightPos,LightToPoint,0,distance(Res.xyz,LightPos)-0.075));
    Color = vec4(Col * Theta* vec3(0.6,0.5,0.7) * S ,1);
}