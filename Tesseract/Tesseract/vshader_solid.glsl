attribute  vec4 vPosition;
attribute  vec4 vNormal;
varying    vec4 color;

// 6-component vectors, split into two vec3 objects
// Angles for rotation about (XY, YZ, XZ, XW, YW, ZW)
uniform float sines[6];
uniform float cosines[6];

uniform mat4 ModelView;
uniform mat4 Projection;

void main()
{
	// Perform rotation in 4D about the given planes
	// XY
	mat4 rotation = mat4( cosines[0],	sines[0],		0.0,	0.0,
						  -sines[0],	cosines[0],		0.0,	0.0,
						  0.0,			0.0,			1.0,	0.0,
						  0.0,			0.0,			0.0,	1.0 );
						  
	// YZ
	rotation = mat4( 1.0,			0.0,			0.0,		0.0,
					 0.0,			cosines[1],		sines[1],	0.0,
					 0.0,			-sines[1],		cosines[1],	0.0,
					 0.0,			0.0,			0.0,		1.0 ) * rotation;
	
	// XZ
	rotation = mat4( cosines[2],	0.0,	-sines[2],	0.0,
					 0.0,			1.0,	0.0,		0.0,
					 sines[2],		0.0,	cosines[2],	0.0,
					 0.0,			0.0,	0.0,		1.0 ) * rotation;
	
	// XW
	rotation = mat4( cosines[3],	0.0,	0.0,	sines[3],
					 0.0,			1.0,	0.0,	0.0,
					 0.0,			0.0,	1.0,	0.0,
					 -sines[3],		0.0,	0.0,	cosines[3]) * rotation;
	
	// YW
	rotation = mat4( 1.0,			0.0,			0.0,	0.0,
					 0.0,			cosines[4],		0.0,	-sines[4],
					 0.0,			0.0,			1.0,	0.0,
					 0.0,			sines[4],		0.0,	cosines[4] ) * rotation;
	
	// ZW
	rotation = mat4( 1.0,	0.0,	0.0,			0.0,
					 0.0,	1.0,	0.0,			0.0,
					 0.0,	0.0,	cosines[5],		-sines[5],
					 0.0,	0.0,	sines[5],		cosines[5] ) * rotation;
	
	
	vec4 temp = rotation*vPosition;
    temp.w = temp.w + 1.0;
    temp.xyz = temp.xyz * temp.w;
    temp.w = 1.0;
    
    vec4 vColor = vNormal;
    vColor = vColor / vColor.w;
    color = vColor; //vec4(abs(vColor.x),abs(vColor.y),abs(vColor.z),1.0);
    gl_Position = Projection*ModelView*temp;
}

