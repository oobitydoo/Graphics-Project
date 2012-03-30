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


// Vertices of the "base_square" square
vec4 base_square[4] = {vec4(-10.0, -0.75, -10.0, 1.0), vec4(-10.0, -0.75, 10.0, 1.0), vec4(10.0, -0.75, -10.0, 1.0), vec4(10.0, -0.75, 10.0, 1.0)};
// These are not actually the normal vector for the floor, I just wanted better coloration for the floor
vec4 base_square_normals[4] = {vec4(1.0, 1.0, 1.0, 0.0), vec4(1.0, 0.0, 1.0, 0.0), vec4(1.0, 1.0, 0.0, 0.0), vec4(0.0, 1.0, 1.0, 0.0)};


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

GLuint program;
GLuint buffer, floor_buffer;

//----------------------------------------------------------------------------

//int NumTimesToSubdivide = 0;
//int VerticesUsed = 0;
//const int NumCubes = 20*20*20*20; // 20^n cubes
//const int NumTriangles = 12*NumCubes; // 12 triangles/cube
//const int NumVertices = 3*NumTriangles;      // 3 vertices / triangle
//
//vec4 points[NumVertices];
//vec4 normals[NumVertices];

int  Index = 0;

int FaceVerticesUsed = 0;
const int FaceVertices = 8*24;
vec4 face_vertices[FaceVertices];

int  face_index;

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
	return normalize( cross( b-a, c-a ) );
}


//----------------------------------------------------------------------------

void
triangle( const vec4& a, const vec4& b, const vec4& c )
{
	//vec4 n = get_normal( a, b, c );
 //   points[Index] = a;  normals[Index] = n;  Index++;
 //   points[Index] = b;  normals[Index] = n;  Index++;
 //   points[Index] = c;  normals[Index] = n;  Index++;
	//VerticesUsed += 3;
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
void
face( const vec4& a, const vec4& b, const vec4& c, const vec4& d )
{
	//triangle(a,b,d);
	//triangle(d,b,c);

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
	//VerticesUsed = 0;
	//Index = 0;
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
	

	glBindBuffer( GL_ARRAY_BUFFER, buffer );
    glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof(face_vertices), face_vertices );
//    glBufferSubData( GL_ARRAY_BUFFER, sizeof(points), sizeof(normals), normals );
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

void divide_cube( const vec3& center, const GLfloat face_dist, const int count )
{
	if(count > 0)
	{
		GLfloat temp = face_dist*2.0/3.0;
		for( int i=-1; i<2; i++ )
		{
			for( int j=-1; j<2; j++ )
			{
				for( int k=-1; k<2; k++ )
				{
					if( !( (i==0 && j==0) || (i==0 && k==0) || (j==0 && k==0) ) )
					{
						vec3 offset( i*temp, j*temp, k*temp );
						divide_cube( center+offset, temp/2.0, count - 1 );
					}
				}
			}
		}
	}
	else
	{
		cube(center, face_dist);
	}
}

void inverted_sponge( const vec3& center, const GLfloat face_dist, const int count )
{
	if(count > 0)
	{
		GLfloat temp = face_dist*2.0/3.0;
		for( int i=-1; i<2; i++ )
		{
			for( int j=-1; j<2; j++ )
			{
				for( int k=-1; k<2; k++ )
				{
					if( !( (i==0 && j==0) || (i==0 && k==0) || (j==0 && k==0) ) )
					{
						vec3 offset( i*temp, j*temp, k*temp );
						inverted_sponge( center+offset, temp/2.0, count - 1 );
					}
					else
					{
						vec3 offset( i*temp, j*temp, k*temp );
						cube( center+offset, temp/2.0 );
					}
				}
			}
		}
	}
}
//----------------------------------------------------------------------------

void
make_fractal( void )
{
//	VerticesUsed = 0;
//	Index = 0;

	//switch( fract )
	//{
	//case SPONGE:
//		divide_cube(vec3(0.0,0.0,0.0),0.5, NumTimesToSubdivide);
	//	break;

	//case INVERT:
	//	inverted_sponge(vec3(0.0,0.0,0.0),0.5, NumTimesToSubdivide);
	//	break;

	//case TETRA:
	//	divide_tetra( 
	//		0.7*vec3( 0.0, 0.0, -1.0 ),
	//		0.7*vec3( 0.0, 0.942809, 0.333333 ),
	//		0.7*vec3( -0.816497, -0.471405, 0.333333 ),
	//		0.7*vec3( 0.816497, -0.471405, 0.333333 ),
	//			NumTimesToSubdivide );
	//	break;
	//}

	//glBindBuffer( GL_ARRAY_BUFFER, buffer );
 //   glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof(points), points );
 //   glBufferSubData( GL_ARRAY_BUFFER, sizeof(points), sizeof(normals), normals );
	//glBindBuffer( GL_ARRAY_BUFFER, 0 );
}

//----------------------------------------------------------------------------

// OpenGL initialization
void
init()
{
    // Load shaders and use the resulting shader program
    program = InitShader( "vshader.glsl", "fshader.glsl" );
    glUseProgram( program );

    // Create a vertex array object
	GLuint vao;
    glGenVertexArrays( 1, &vao );
    glBindVertexArray( vao );

    // Create and initialize buffer objects for fractal and floor
    glGenBuffers( 1, &buffer );
	glBindBuffer( GL_ARRAY_BUFFER, buffer );
	glBufferData( GL_ARRAY_BUFFER, sizeof(face_vertices),
		NULL, GL_STATIC_DRAW );
	glBindBuffer( GL_ARRAY_BUFFER, 0 );

	glGenBuffers( 1, &floor_buffer );
	glBindBuffer( GL_ARRAY_BUFFER, floor_buffer );	
	glBufferData( GL_ARRAY_BUFFER, sizeof(base_square) + sizeof(base_square_normals),
		NULL, GL_STATIC_DRAW );
    glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof(base_square), base_square );
    glBufferSubData( GL_ARRAY_BUFFER, sizeof(base_square), sizeof(base_square_normals), base_square_normals );
	glBindBuffer( GL_ARRAY_BUFFER, 0 );

    //make_fractal();
	//cube( vec3(0.0, 0.0, 0.0), 0.5 );
	tesseract( vec4(0.0,0.0,0.0,0.0), 0.3 );

    model_view = glGetUniformLocation( program, "ModelView" );
    projection = glGetUniformLocation( program, "Projection" );
    sines = glGetUniformLocation( program, "sines" );
    cosines = glGetUniformLocation( program, "cosines" );
    
    glEnable( GL_DEPTH_TEST );
    glClearColor( 0.0, 0.0, 0.0, 1.0 ); 
}

//----------------------------------------------------------------------------

void
display( void )
{
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	GLuint vPosition, vNormal;

	// Bring camera into Cartesian coordinates
	vec4 cart_eye = Translate( eye_offset ) * sph_to_cart( sph_eye );
	vec4 cart_at = Translate( eye_offset ) * sph_to_cart( sph_at );
	vec4 cart_up = sph_to_cart( sph_up );

	//std::cout << eye_offset << std::endl << 
	//	cart_eye << std::endl <<
	//	cart_at << std::endl <<
	//	cart_up << std::endl << std::endl;

	mat4 mv = LookAt( cart_eye, cart_at, cart_up );
	mat4 p = Perspective( fovy, aspect, zNear, zFar );

	glUniformMatrix4fv( model_view, 1, GL_TRUE, mv );
	glUniformMatrix4fv( projection, 1, GL_TRUE, p );

	// Bind floor_buffer and display floor
	glBindBuffer( GL_ARRAY_BUFFER, floor_buffer );

	vPosition = glGetAttribLocation( program, "vPosition" );
    glEnableVertexAttribArray( vPosition );
    glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0,
			   BUFFER_OFFSET(0) );

    vNormal = glGetAttribLocation( program, "vNormal" ); 
    glEnableVertexAttribArray( vNormal );
    glVertexAttribPointer( vNormal, 4, GL_FLOAT, GL_FALSE, 0,
			   BUFFER_OFFSET(sizeof(base_square)) );

	//glDrawArrays( GL_TRIANGLE_STRIP, 0, sizeof(base_square)/sizeof(base_square[0]) );
	glBindBuffer( GL_ARRAY_BUFFER, 0 );

	// Calculate sines and cosines of rotation angles
	GLfloat sine_values[6];
	GLfloat cosine_values[6];

	for( int i=0; i<6; i++ )
	{
		sine_values[i] = sin(Angles[i]*DegreesToRadians);
		cosine_values[i] = cos(Angles[i]*DegreesToRadians);
	}

	// Update the uniform variables for the shaders (model_view now first applies transformations to the object)
	glUniformMatrix4fv( model_view, 1, GL_TRUE, mv );
	glUniformMatrix4fv( projection, 1, GL_TRUE, p );
	glUniform1fv( sines, 6, sine_values );
	glUniform1fv( cosines, 6, cosine_values );

	// Bind buffer and display wireframe
	glBindBuffer( GL_ARRAY_BUFFER, buffer );

	vPosition = glGetAttribLocation( program, "vPosition" );
    glEnableVertexAttribArray( vPosition );
    glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0,
			   BUFFER_OFFSET(0) );

    //vNormal = glGetAttribLocation( program, "vNormal" ); 
    //glEnableVertexAttribArray( vNormal );
    //glVertexAttribPointer( vNormal, 4, GL_FLOAT, GL_FALSE, 0,
			 //  BUFFER_OFFSET(sizeof(points)) );

    glDrawArrays( GL_LINES, 0, FaceVerticesUsed );
	glBindBuffer( GL_ARRAY_BUFFER, 0 );

	
    glutSwapBuffers();
}

//----------------------------------------------------------------------------

void
keyboard( unsigned char key, int x, int y )
{
	vec4 temp;

	switch( key ) {
		/************************ Things to remove eventually ****************************/
	case '1':
//		NumTimesToSubdivide = (NumTimesToSubdivide + 1) % 5;
		make_fractal();
		break;
		/*********************************************************************************/

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
			glutMotionFunc( move_mouse );
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
	glutIdleFunc( idle );

    glutMainLoop();
    return 0;
}
