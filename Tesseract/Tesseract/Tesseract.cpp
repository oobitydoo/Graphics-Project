//
// Perspective view of a color cube using LookAt() and Perspective()
//
// Colors are assigned to each vertex and then the rasterizer interpolates
//   those colors across the triangles.
//

#include "Angel.h"
#include <iostream>
#include <sstream>
#include <string>

typedef Angel::vec4  color4;
typedef Angel::vec4  point4;


// Vertices of the "base_square" floor
vec4 base_square[4] = {vec4(-10.0, -0.75, -10.0, 1.0), vec4(-10.0, -0.75, 10.0, 1.0), vec4(10.0, -0.75, -10.0, 1.0), vec4(10.0, -0.75, 10.0, 1.0)};
vec4 base_square_colors[4] = {vec4(1.0, 1.0, 1.0, 0.0), vec4(1.0, 0.0, 1.0, 0.0), vec4(1.0, 1.0, 0.0, 0.0), vec4(0.0, 1.0, 1.0, 0.0)};


// Parameters for LookAt -- spherical coordinates (r, phi, theta, w)
// phi = angle down from from +y
// theta = angle from +x towards +z
// Keep the eye stationary at the camera-space origin
const point4  sph_eye( 0.0, 0.0, 0.0, 1.0 );
point4		  sph_at( 1.0, 90.0, -90.0, 1.0 );
vec4		  sph_up( 1.0, 0.0, -90.0, 0.0 );

// Cartesian coordinates position for eye
point4	eye_offset( 0.0, 0.0, 1.0, 1.0 );

// Rotation angles
GLfloat Angles[6] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
GLfloat angle_step_ratios[6] = { 1.0, 2.0, 3.0, 5.0, 7.0, 11.0 };
GLfloat angle_step = 0.1;


// Previous mouse coordinates -- default to somewhere mid-screen
int x_prev = 256;
int y_prev = 256;
GLfloat step = 0.1;


// Perspective parameters
GLfloat  fovy = 45.0;  // Field-of-view in Y direction angle (in degrees)
GLfloat  aspect;       // Viewport aspect ratio
GLfloat  zNear = 0.5, zFar = 3.0;

GLuint  model_view;  // model-view matrix uniform shader variable location
GLuint  projection; // projection matrix uniform shader variable location
GLuint  sines;   // sines and cosines of 4D rotation angles
GLuint  cosines;

GLuint solid_program, wireframe_program, floor_program;
GLuint solid, wireframe, floor_buffer;

//----------------------------------------------------------------------------

int  Index = 0;
int VerticesUsed = 0;
const int NumVertices = 6*24;      // 6 vertices / face

vec4 points[NumVertices];
vec4 normals[NumVertices];


int  face_index = 0;
int FaceVerticesUsed = 0;
const int FaceVertices = 8*24;
vec4 face_vertices[FaceVertices];


//----------------------------------------------------------------------------

void
reset_params()
{
	// Parameters for LookAt
	sph_at = vec4( 1.0, 90.0, -90.0, 1.0 );
	sph_up = vec4( 1.0, 0.0, -90.0, 0.0 );
	eye_offset = vec4( 0.0, 0.0, 1.0, 1.0 );

	step = 0.1;
}

//----------------------------------------------------------------------------

// Spherical coordinates represented as (r, phi, theta, w)
vec4
sph_to_cart( const vec4& v )
{
	return vec4( v.x*sin(v.y*DegreesToRadians)*cos(v.z*DegreesToRadians),
				 v.x*cos(v.y*DegreesToRadians),
				 v.x*sin(v.y*DegreesToRadians)*sin(v.z*DegreesToRadians),
				 v.w );
}

//----------------------------------------------------------------------------

vec4
get_normal( const vec4& a, const vec4& b, const vec4& c )
{
	return 
//	return normalize( cross( b-a, c-a ) );
}


//----------------------------------------------------------------------------
vec4 colors[24] = { vec4(1.0, 0.0, 0.0, 1.0), vec4(0.0, 1.0, 0.0, 1.0), vec4(0.0, 0.0, 1.0, 1.0), vec4(1.0, 1.0, 0.0, 1.0),
					vec4(1.0, 0.0, 1.0, 1.0), vec4(0.0, 1.0, 1.0, 1.0), vec4(0.1, 0.2, 0.3, 1.0), vec4(0.2, 0.1, 0.3, 1.0),
					vec4(0.1, 0.3, 0.2, 1.0), vec4(0.2, 0.3, 0.1, 0.0), vec4(0.3, 0.2, 0.1, 1.0), vec4(0.3, 0.1, 0.2, 1.0),
					vec4(0.7, 0.2, 1.0, 1.0), vec4(0.7, 1.0, 0.2, 1.0), vec4(0.2, 0.7, 1.0, 1.0), vec4(0.2, 1.0, 0.7, 1.0),
					vec4(1.0, 0.2, 0.7, 1.0), vec4(1.0, 0.7, 0.2, 1.0), vec4(0.5, 0.3, 0.0, 1.0), vec4(0.5, 0.0, 0.3, 1.0),
					vec4(0.0, 0.3, 0.5, 1.0), vec4(0.0, 0.5, 0.3, 1.0), vec4(0.3, 0.0, 0.5, 1.0), vec4(0.3, 0.5, 0.0, 1.0)
				  };

void
triangle( const vec4& a, const vec4& b, const vec4& c, int color )
{
	vec4 n = get_normal( a, b, c );
    points[Index] = a;  normals[Index] = colors[color]; normals[Index].w = 0.3;  Index++;
    points[Index] = b;  normals[Index] = colors[color]; normals[Index].w = 0.3;  Index++;
    points[Index] = c;  normals[Index] = colors[color]; normals[Index].w = 0.3;  Index++;
	VerticesUsed += 3;
}

/*****************************************************
Make a cube by creating 6 square faces from triangles

  h_______g
 /|      /|		
d-------c |
| e-----|-f
|/      |/
a-------b

******************************************************/
int face_num = 0;

void
face( const vec4& a, const vec4& b, const vec4& c, const vec4& d )
{
	triangle(a,b,d, face_num);
	triangle(d,b,c, face_num);
	face_num++;

	face_vertices[face_index] = a; face_index++;
	face_vertices[face_index] = b; face_index++;

	face_vertices[face_index] = b; face_index++;
	face_vertices[face_index] = c; face_index++;

	face_vertices[face_index] = c; face_index++;
	face_vertices[face_index] = d; face_index++;
	
	face_vertices[face_index] = d; face_index++;
	face_vertices[face_index] = a; face_index++;
	FaceVerticesUsed += 8;
}

void
tesseract( const vec4& center, GLfloat face_dist )
{
	face_index = 0;

	vec4 a = center + vec4(-1.0*face_dist,-1.0*face_dist,-1.0*face_dist,1.0*face_dist);
	vec4 b = center + vec4(1.0*face_dist,-1.0*face_dist,-1.0*face_dist,1.0*face_dist);
	vec4 c = center + vec4(1.0*face_dist,1.0*face_dist,-1.0*face_dist,1.0*face_dist);
	vec4 d = center + vec4(-1.0*face_dist,1.0*face_dist,-1.0*face_dist,1.0*face_dist);
	vec4 e = center + vec4(-1.0*face_dist,1.0*face_dist,1.0*face_dist,1.0*face_dist);
	vec4 f = center + vec4(1.0*face_dist,1.0*face_dist,1.0*face_dist,1.0*face_dist);
	vec4 g = center + vec4(1.0*face_dist,-1.0*face_dist,1.0*face_dist,1.0*face_dist);
	vec4 h = center + vec4(-1.0*face_dist,-1.0*face_dist,1.0*face_dist,1.0*face_dist);
	vec4 i = center + vec4(-1.0*face_dist,-1.0*face_dist,1.0*face_dist,-1.0*face_dist);
	vec4 j = center + vec4(1.0*face_dist,-1.0*face_dist,1.0*face_dist,-1.0*face_dist);
	vec4 k = center + vec4(1.0*face_dist,1.0*face_dist,1.0*face_dist,-1.0*face_dist);
	vec4 l = center + vec4(-1.0*face_dist,1.0*face_dist,1.0*face_dist,-1.0*face_dist);
	vec4 m = center + vec4(-1.0*face_dist,1.0*face_dist,-1.0*face_dist,-1.0*face_dist);
	vec4 n = center + vec4(1.0*face_dist,1.0*face_dist,-1.0*face_dist,-1.0*face_dist);
	vec4 o = center + vec4(1.0*face_dist,-1.0*face_dist,-1.0*face_dist,-1.0*face_dist);
	vec4 p = center + vec4(-1.0*face_dist,-1.0*face_dist,-1.0*face_dist,-1.0*face_dist);


	face(a,b,c,d);
	face(d,c,f,e);
	face(a,d,e,h);
	face(h,e,f,g);
	face(g,f,c,b);
	face(b,a,h,g);

	face(a,p,o,b);
	face(b,o,j,g);
	face(g,j,i,h);
	face(h,i,p,a);

	face(c,n,m,d);
	face(d,m,l,e);
	face(e,l,k,f);
	face(f,k,n,c);

	face(d,m,p,a);
	face(e,l,i,h);
	face(f,k,j,g);
	face(c,n,o,b);

	face(o,p,m,n);
	face(n,m,l,k);
	face(k,l,i,j);
	face(j,i,p,o);
	face(m,p,i,l);
	face(o,j,k,n);
	

	glBindBuffer( GL_ARRAY_BUFFER, wireframe );
    glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof(face_vertices), face_vertices );
	glBindBuffer( GL_ARRAY_BUFFER, 0 );

	glBindBuffer( GL_ARRAY_BUFFER, solid );
    glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof(points), points );
	glBufferSubData( GL_ARRAY_BUFFER, sizeof(points), sizeof(normals), normals );
	glBindBuffer( GL_ARRAY_BUFFER, 0 );
}

//----------------------------------------------------------------------------

void
cube( const vec4& center, GLfloat face_dist )
{
	vec4 a = center + vec4(-1.0*face_dist,-1.0*face_dist,-1.0*face_dist, 0.0);
	vec4 b = center + vec4(1.0*face_dist,-1.0*face_dist,-1.0*face_dist, 0.0);
	vec4 c = center + vec4(1.0*face_dist,1.0*face_dist,-1.0*face_dist, 0.0);
	vec4 d = center + vec4(-1.0*face_dist,1.0*face_dist,-1.0*face_dist, 0.0);
	vec4 e = center + vec4(-1.0*face_dist,-1.0*face_dist,1.0*face_dist, 0.0);
	vec4 f = center + vec4(1.0*face_dist,-1.0*face_dist,1.0*face_dist, 0.0);
	vec4 g = center + vec4(1.0*face_dist,1.0*face_dist,1.0*face_dist, 0.0);
	vec4 h = center + vec4(-1.0*face_dist,1.0*face_dist,1.0*face_dist, 0.0);

	face(a,b,c,d);
	face(d,c,g,h);
	face(g,c,b,f);
	face(b,f,e,a);
	face(e,a,d,h);
	face(e,f,g,h);
}

//----------------------------------------------------------------------------

// OpenGL initialization
void
init()
{
    // Load shaders and use the resulting shader program
    solid_program = InitShader( "vshader_solid.glsl", "fshader.glsl" );
    wireframe_program = InitShader( "vshader_wireframe.glsl", "fshader.glsl" );
	floor_program = InitShader( "vshader_floor.glsl", "fshader.glsl" );
    //glUseProgram( wireframe_program );

    // Create a vertex array object
	GLuint vao;
    glGenVertexArrays( 1, &vao );
    glBindVertexArray( vao );

    // Create and initialize buffer objects for tesseract, wireframe, and floor
    glGenBuffers( 1, &wireframe );
	glBindBuffer( GL_ARRAY_BUFFER, wireframe );
	glBufferData( GL_ARRAY_BUFFER, sizeof(face_vertices),
		NULL, GL_STATIC_DRAW );
	glBindBuffer( GL_ARRAY_BUFFER, 0 );
	
    glGenBuffers( 1, &solid );
	glBindBuffer( GL_ARRAY_BUFFER, solid );
	glBufferData( GL_ARRAY_BUFFER, sizeof(points) + sizeof(normals),
		NULL, GL_STATIC_DRAW );
	glBindBuffer( GL_ARRAY_BUFFER, 0 );

	glGenBuffers( 1, &floor_buffer );
	glBindBuffer( GL_ARRAY_BUFFER, floor_buffer );	
	glBufferData( GL_ARRAY_BUFFER, sizeof(base_square) + sizeof(base_square_colors),
		NULL, GL_STATIC_DRAW );
    glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof(base_square), base_square );
    glBufferSubData( GL_ARRAY_BUFFER, sizeof(base_square), sizeof(base_square_colors), base_square_colors );
	glBindBuffer( GL_ARRAY_BUFFER, 0 );


	//cube( vec3(0.0, 0.0, 0.0), 0.5 );
	tesseract( vec4(0.0,0.0,0.0,0.0), 0.3 );

    glEnable( GL_DEPTH_TEST );
    glClearColor( 0.0, 0.0, 0.0, 1.0 ); 
}

//----------------------------------------------------------------------------

void
display( void )
{
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	GLuint vPosition, vColor, vNormal;

	// Bring camera into Cartesian coordinates
	vec4 cart_eye = Translate( eye_offset ) * sph_to_cart( sph_eye );
	vec4 cart_at = Translate( eye_offset ) * sph_to_cart( sph_at );
	vec4 cart_up = sph_to_cart( sph_up );
    

	mat4 mv = LookAt( cart_eye, cart_at, cart_up );
	mat4 p = Perspective( fovy, aspect, zNear, zFar );

	// Calculate sines and cosines of rotation angles
	GLfloat sine_values[6];
	GLfloat cosine_values[6];

	for( int i=0; i<6; i++ )
	{
		sine_values[i] = sin(Angles[i]*DegreesToRadians);
		cosine_values[i] = cos(Angles[i]*DegreesToRadians);
	}


	// Set up uniforms for the floor
	glUseProgram( floor_program );
	glUniformMatrix4fv( model_view, 1, GL_TRUE, mv );
	glUniformMatrix4fv( projection, 1, GL_TRUE, p );

	// Bind floor_buffer and display floor
	glBindBuffer( GL_ARRAY_BUFFER, floor_buffer );

	vPosition = glGetAttribLocation( floor_program, "vPosition" );
    glEnableVertexAttribArray( vPosition );
    glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0,
			   BUFFER_OFFSET(0) );

    vColor = glGetAttribLocation( floor_program, "vColor" ); 
    glEnableVertexAttribArray( vColor );
    glVertexAttribPointer( vColor, 4, GL_FLOAT, GL_FALSE, 0,
			   BUFFER_OFFSET(sizeof(base_square)) );

	glDrawArrays( GL_TRIANGLE_STRIP, 0, sizeof(base_square)/sizeof(base_square[0]) );
	glBindBuffer( GL_ARRAY_BUFFER, 0 );

	
	// Set up uniforms for the wireframe
	glUseProgram( wireframe_program );
    model_view = glGetUniformLocation( wireframe_program, "ModelView" );
    projection = glGetUniformLocation( wireframe_program, "Projection" );
    sines = glGetUniformLocation( wireframe_program, "sines" );
    cosines = glGetUniformLocation( wireframe_program, "cosines" );

	glUniformMatrix4fv( model_view, 1, GL_TRUE, mv );
	glUniformMatrix4fv( projection, 1, GL_TRUE, p );
	glUniform1fv( sines, 6, sine_values );
	glUniform1fv( cosines, 6, cosine_values );

	// Bind buffer and display wireframe
	glBindBuffer( GL_ARRAY_BUFFER, wireframe );

	vPosition = glGetAttribLocation( wireframe_program, "vPosition" );
    glEnableVertexAttribArray( vPosition );
    glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0,
			   BUFFER_OFFSET(0) );

    glDrawArrays( GL_LINES, 0, FaceVerticesUsed );
	glBindBuffer( GL_ARRAY_BUFFER, 0 );


	// Set up uniforms for the solid object
	glUseProgram( solid_program );
    model_view = glGetUniformLocation( solid_program, "ModelView" );
    projection = glGetUniformLocation( solid_program, "Projection" );
    sines = glGetUniformLocation( solid_program, "sines" );
    cosines = glGetUniformLocation( solid_program, "cosines" );

	glUniformMatrix4fv( model_view, 1, GL_TRUE, mv );
	glUniformMatrix4fv( projection, 1, GL_TRUE, p );
	glUniform1fv( sines, 6, sine_values );
	glUniform1fv( cosines, 6, cosine_values );

	// Bind buffer and display solid
	glBindBuffer( GL_ARRAY_BUFFER, solid );

	vPosition = glGetAttribLocation( solid_program, "vPosition" );
    glEnableVertexAttribArray( vPosition );
    glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0,
			   BUFFER_OFFSET(0) );
    vNormal = glGetAttribLocation( solid_program, "vNormal" ); 
    glEnableVertexAttribArray( vNormal );
    glVertexAttribPointer( vNormal, 4, GL_FLOAT, GL_FALSE, 0,
			   BUFFER_OFFSET(sizeof(points)) );

    glDrawArrays( GL_TRIANGLES, 0, VerticesUsed );
	glBindBuffer( GL_ARRAY_BUFFER, 0 );


	
    glutSwapBuffers();
}

//----------------------------------------------------------------------------

void
keyboard( unsigned char key, int x, int y )
{
	vec4 temp;

	switch( key ) {

	case 033: // Escape Key
	    exit( EXIT_SUCCESS );
	    break;

	// Move the camera
	case 'w':
		//temp = sph_to_cart(sph_at);
		temp = -1.0*sph_to_cart(sph_up);
		temp.w = 0.0;
		eye_offset += step*normalize(temp);
		break;
	case 's':
		//temp = sph_to_cart(sph_at);
		temp = -1.0*sph_to_cart(sph_up);
		temp.w = 0.0;
		eye_offset -= step*normalize(temp);
		break;

	case 'a':
		temp = cross(sph_to_cart(sph_at),sph_to_cart(sph_up));
		temp.w = 0;
		eye_offset -= step*normalize(temp);
		break;
	case 'd':
		temp = cross(sph_to_cart(sph_at),sph_to_cart(sph_up));
		temp.w = 0;
		eye_offset += step*normalize(temp);
		break;

	// Change camera movement speed
	case ']':
		step += 0.05;
		break;
	case '[':
		step -= 0.05;
		if (step < 0)
			step = 0;
		break;


	case ' ':  // reset values to their defaults
		reset_params();
	    break;
    }

    glutPostRedisplay();
}

//----------------------------------------------------------------------------

void
idle( void )
{
	for( int i=0; i<6; i++ )
	{
		Angles[i] += angle_step_ratios[i]*angle_step;
		if (Angles[i] >= 360)
		{
			Angles[i] -= 360;
		}
	}

	glutPostRedisplay();
}

//----------------------------------------------------------------------------

void
move_mouse( int x, int y )
{
	int width = glutGet(GLUT_WINDOW_WIDTH);
	int height = glutGet(GLUT_WINDOW_HEIGHT);

	// Look horizontally
	sph_at.z += 5.0*((GLfloat)(x-x_prev))/(GLfloat)height;
	sph_up.z += 5.0*((GLfloat)(x-x_prev))/(GLfloat)height;
	if (sph_at.z > 360.0)
		sph_at.z -= 360.0;
	if (sph_at.z < 0.0)
		sph_at.z += 360.0;
	if (sph_up.z > 360.0)
		sph_up.z -= 360.0;
	if (sph_up.z < 0.0)
		sph_up.z += 360.0;

	// Look vertically
	sph_at.y -= 5.0*((GLfloat)(y-y_prev))/(GLfloat)height;
	sph_up.y -= 5.0*((GLfloat)(y-y_prev))/(GLfloat)height;
	if (sph_at.y > 360.0)
		sph_at.y -= 360.0;
	if (sph_at.y < 0.0)
		sph_at.y += 360.0;
	if (sph_up.y > 360.0)
		sph_up.y -= 360.0;
	if (sph_up.y < 0.0)
		sph_up.y += 360.0;

	glutPostRedisplay();
}

void mouse( int button, int state, int x, int y )
{
	if( button == GLUT_LEFT_BUTTON )
	{
		if( state == GLUT_DOWN )
		{
			//glutMotionFunc( move_mouse );
			glutIdleFunc( idle );
		}
		else
		{
			x_prev = glutGet( GLUT_WINDOW_WIDTH ) / 2;
			y_prev = glutGet( GLUT_WINDOW_HEIGHT ) / 2;
		}
	}
	else
	{
		glutMotionFunc( NULL );
	}
}

//----------------------------------------------------------------------------

void
reshape( int width, int height )
{
    glViewport( 0, 0, width, height );

    aspect = GLfloat(width)/height;
}

//----------------------------------------------------------------------------

int
main( int argc, char **argv )
{
    glutInit( &argc, argv );
    glutInitDisplayMode( GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH );
    glutInitWindowSize( 512, 512 );
    glutInitContextVersion( 3, 2 );
    glutInitContextProfile( GLUT_CORE_PROFILE );
    glutCreateWindow( "Teseseract" );

	glewExperimental = GL_TRUE;
    glewInit();

    init();

    glutDisplayFunc( display );
    glutKeyboardFunc( keyboard );
    glutReshapeFunc( reshape );
	glutMouseFunc( mouse );

    glutMainLoop();
    return 0;
}
