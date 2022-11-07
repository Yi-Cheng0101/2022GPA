#include "../Include/Common.h"
#include <math.h>
#include <ctime>

//For GLUT to handle 
#define MENU_TIMER_START 1
#define MENU_TIMER_STOP 2
#define MENU_EXIT 3
#define MENU_HELLO 4
#define MENU_WORLD 5

enum TransMode
{
    GeoTranslation = 0,
    GeoRotation = 1,
};


enum BoneNum
{
    Torso = 0,
    Head  = 1,
    Right_hand_up  = 2,
    Right_hand_down= 3,
    Left_hand_up  = 4,
    Left_hand_down= 5,
    Right_leg_up  = 6,
    Right_leg_down= 7,
    Left_leg_up  = 8,
    Left_leg_down= 9,
};

// yi
bool mouse_pressed = false;
TransMode cur_trans_mode = GeoTranslation;


//GLUT timer variable
float timer_cnt = 0;
bool timer_enabled = true;
unsigned int timer_speed = 16;

using namespace glm;
using namespace std;

mat4 view(1.0f);			// V of MVP, viewing matrix
mat4 projection(1.0f);		// P of MVP, projection matrix
mat4 model(1.0f);			// M of MVP, model matrix
vec3 temp = vec3();			// a 3 dimension vector which represents how far did the ladybug should move

GLint um4p;
GLint um4mv;

GLuint program;			// shader program id

typedef struct
{
	GLuint vao;			// vertex array object
	GLuint vbo;			// vertex buffer object

	int materialId;
	int vertexCount;
	GLuint m_texture;
} Shape;


typedef struct
{
    GLuint vao;            // vertex array object
    GLuint vbo;            // vertex buffer object
    
    // initial the parameters
    vec3 pos = vec3(0.0f, 0.0f, 0.0f);
    vec3 rot = vec3(0.0f, 0.0f, 0.0f);
    vec3 scal = vec3(1.0f, 1.0f, 1.0f);
    
    int materialId;
    int vertexCount;
    GLuint m_texture;
} Bone;

vector<Bone*> Robot;

Bone m_torso;
Bone m_head;
Bone m_hand[4];
Bone m_leg[4];
//Bone m_bottom[2];

void organize(){
    //0
    Robot.push_back(&m_torso);
    //1
    Robot.push_back(&m_head);
    //2~5
    for(int i=0; i<4; i++){
        Robot.push_back(&m_hand[i]);
    }
    //6~9
    for(int i=0; i<4; i++){
        Robot.push_back(&m_leg[i]);
    }
 
    return;
}

// Load shader file to program
char** loadShaderSource(const char* file)
{
	FILE* fp = fopen(file, "rb");
	fseek(fp, 0, SEEK_END);
	long sz = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	char *src = new char[sz + 1];
	fread(src, sizeof(char), sz, fp);
	src[sz] = '\0';
	char **srcp = new char*[1];
	srcp[0] = src;
	return srcp;
}

// Free shader file
void freeShaderSource(char** srcp)
{
	delete srcp[0];
	delete srcp;
}

// Load .obj model
void My_LoadModels(Bone &m_shape, char* obj)
{
	tinyobj::attrib_t attrib;
	vector<tinyobj::shape_t> shapes;
	vector<tinyobj::material_t> materials;
	string warn;
	string err;
	bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, obj);
	if (!warn.empty()) {
		cout << warn << endl;
	}
	if (!err.empty()) {
		cout << err << endl;
	}
	if (!ret) {
		exit(1);
	}

	vector<float> vertices, texcoords, normals;  // if OBJ preserves vertex order, you can use element array buffer for memory efficiency
	for (int s = 0; s < shapes.size(); ++s) {  // for 'ladybug.obj', there is only one object
		int index_offset = 0;
		for (int f = 0; f < shapes[s].mesh.num_face_vertices.size(); ++f) {
			int fv = shapes[s].mesh.num_face_vertices[f];
			for (int v = 0; v < fv; ++v) {
				tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
				vertices.push_back(attrib.vertices[3 * idx.vertex_index + 0]);
				vertices.push_back(attrib.vertices[3 * idx.vertex_index + 1]);
				vertices.push_back(attrib.vertices[3 * idx.vertex_index + 2]);
				texcoords.push_back(attrib.texcoords[2 * idx.texcoord_index + 0]);
				texcoords.push_back(attrib.texcoords[2 * idx.texcoord_index + 1]);
				normals.push_back(attrib.normals[3 * idx.normal_index + 0]);
				normals.push_back(attrib.normals[3 * idx.normal_index + 1]);
				normals.push_back(attrib.normals[3 * idx.normal_index + 2]);
			}
			index_offset += fv;
			m_shape.vertexCount += fv;
		}
	}

	glGenVertexArrays(1, &m_shape.vao);
	glBindVertexArray(m_shape.vao);

	glGenBuffers(1, &m_shape.vbo);
	glBindBuffer(GL_ARRAY_BUFFER, m_shape.vbo);

	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float) + texcoords.size() * sizeof(float) + normals.size() * sizeof(float), NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(float), vertices.data());
	glBufferSubData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), texcoords.size() * sizeof(float), texcoords.data());
	glBufferSubData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float) + texcoords.size() * sizeof(float), normals.size() * sizeof(float), normals.data());

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid*)(vertices.size() * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)(vertices.size() * sizeof(float) + texcoords.size() * sizeof(float)));
	glEnableVertexAttribArray(2);

	shapes.clear();
	shapes.shrink_to_fit();
	materials.clear();
	materials.shrink_to_fit();
	vertices.clear();
	vertices.shrink_to_fit();
	texcoords.clear();
	texcoords.shrink_to_fit();
	normals.clear();
	normals.shrink_to_fit();

	cout << "Load " << m_shape.vertexCount << " vertices" << endl;

	texture_data tdata = loadImg("pee_bao.png");

	glGenTextures(1, &m_shape.m_texture);
	glBindTexture(GL_TEXTURE_2D, m_shape.m_texture);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, tdata.width, tdata.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, tdata.data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	delete tdata.data;
}

// OpenGL initialization
void My_Init()
{
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	// Create Shader Program
	program = glCreateProgram();

	// Create customize shader by tell openGL specify shader type 
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

	// Load shader file
	char** vertexShaderSource = loadShaderSource("vertex.vs.glsl");
	char** fragmentShaderSource = loadShaderSource("fragment.fs.glsl");

	// Assign content of these shader files to those shaders we created before
	glShaderSource(vertexShader, 1, vertexShaderSource, NULL);
	glShaderSource(fragmentShader, 1, fragmentShaderSource, NULL);

	// Free the shader file string(won't be used any more)
	freeShaderSource(vertexShaderSource);
	freeShaderSource(fragmentShaderSource);

	// Compile these shaders
	glCompileShader(vertexShader);
	glCompileShader(fragmentShader);

	// Logging
	shaderLog(vertexShader);
	shaderLog(fragmentShader);

	// Assign the program we created before with these shaders
	glAttachShader(program, vertexShader);
	glAttachShader(program, fragmentShader);
	glLinkProgram(program);

	// Get the id of inner variable 'um4p' and 'um4mv' in shader programs
	um4p = glGetUniformLocation(program, "um4p");
	um4mv = glGetUniformLocation(program, "um4mv");

	// Tell OpenGL to use this shader program now
	glUseProgram(program);

    // initial the position
    
    // oraganize Robot body
    organize();
    
    
    // load
    //torso
    My_LoadModels(m_torso, "Cube.obj");
    //head
    My_LoadModels(m_head, "Cube.obj");
    //hand & leg
    for(int i=0;i<4;i++){
        if( (i%2)==0){
            My_LoadModels(m_hand[i], "Cube.obj");
            My_LoadModels(m_leg[i], "Cube.obj");
        }else{
            My_LoadModels(m_hand[i], "Sphere.obj");
            My_LoadModels(m_leg[i], "Cylinder.obj");
            
        }
    }
    
    // position and scal
    //torso
    m_torso.pos = vec3(2.0f, 1.0f, 0.0f);
    m_torso.scal      = vec3(4.0f, 5.0f, 4.0f);
    //head
    m_head.scal       = vec3(2.75f, 2.75f, 2.75f);
    //hand & leg
    for(int i=0;i<4;i++){
        if( (i%2)==0){
            m_hand[i].scal = vec3(1.0f, 2.5f, 1.2f);
            m_leg[i].scal  = vec3(1.6f, 2.5f, 1.6f);
        }else{
            m_hand[i].scal = vec3(1.5f);
            m_leg[i].scal  = vec3(1.6f, 1.7f, 1.6f);
        }
    }
    

    //My_LoadModels();
}

void gl_rotate(mat4& model, Bone& shape) {
    model = translate(mat4(1.0f), shape.pos);
    model = rotate(model, shape.rot.x, vec3(1, 0, 0));
    model = rotate(model, shape.rot.y, vec3(0, 1, 0));
    model = rotate(model, shape.rot.z, vec3(0, 0, 1));
}

void gl_scale(mat4& model, Bone* shape) {
    model = scale(model, shape->scal);
}


// GLUT callback. Called to draw the scene.
void My_Display()
{
	
 
    vector<mat4> m_mat(10);
    vector<vec3> body_trans{
        vec3(0.0f, 4.0f, 0.0f),
        vec3(0.0f, 1.0f, 3.0f),
        vec3(0.0f, -2.0f, -0.2f),
        vec3(0.0f, -3.8f, 1.2f),
        vec3(0.0, -3.0f, -0.05f)
    };
    

    vector<float> body_rotates {-0.8f, 0.2f};
    
    float timer = sin(6 * timer_cnt / 180.f * 3.14f);
  
    // Clear display buffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // Bind a vertex array for OpenGL (OpenGL will apply operation only to the vertex array objects it bind)
    //glBindVertexArray(m_shape.vao);

    // Tell openGL to use the shader program we created before
    glUseProgram(program);
    
    // axis !!!
    vec3 rotate_axis = vec3(0.0, 1.0, 0.0);
    vec3 rotate_axis_n = vec3(1.0, 0.0, 0.0);
    // torso -> 0
    gl_rotate(m_mat[0], m_torso);
    m_mat[0] = rotate(m_mat[0], radians(timer_cnt), rotate_axis);
    
    // head  -> 1
    gl_rotate(m_mat[1], m_head);
    m_mat[1] = m_mat[1] * translate(m_mat[0], body_trans[0]);
    m_mat[1] = rotate(m_mat[1], radians(timer_cnt), rotate_axis);
    m_mat[1] = rotate(m_mat[1], radians(timer_cnt), rotate_axis_n);
    
    // upper arm -> 2, 4
    for(int i=0; i<2; i++){
        int mul = pow(-1,i%2);
        int index = 2 + 2*i;
        gl_rotate(m_mat[index], m_hand[0]);
        m_mat[index] = m_mat[index] * translate(m_mat[0], vec3(body_trans[1].x, body_trans[1].y, -1 * mul * body_trans[1].z));
        m_mat[index]= rotate(m_mat[index], -0.3f + mul * 0.7f * timer, vec3(0, 0, 1));
        m_mat[index] = rotate(m_mat[index], radians(timer_cnt), rotate_axis);
    }
   
    // lower arm -> 3, 5
    for(int i=0; i<2; i++){
        int mul = pow(-1,i%2); // 0-> + 1-> -
        int index = 3 + 2*i;
        gl_rotate(m_mat[index], m_hand[index-2]);
        m_mat[index] = m_mat[index] * translate(m_mat[index-1], vec3(body_trans[2].x, body_trans[2].y, -1* mul *body_trans[2].z));
        m_mat[index] = rotate(m_mat[index], body_rotates[0],vec3(1, 0, 0));
        m_mat[index]= rotate(m_mat[index], -0.5f + mul * 0.5f * timer, vec3(0, 0, 1));
    }
    
    // upper leg -> 6, 8
    for(int i=0; i<2; i++){
        int mul = pow(-1,i%2); // 0-> + 1-> -
        int index = 6 + 2*i;
        gl_rotate(m_mat[index], m_leg[0]);
        m_mat[index] = m_mat[index] * translate(m_mat[0], vec3(body_trans[3].x, body_trans[3].y, -1 * mul *body_trans[3].z));
        m_mat[index] = rotate(m_mat[index], mul * body_rotates[1],vec3(1, 0, 0));
        m_mat[index]= rotate(m_mat[index], -0.1f -1 * mul* 0.35f * timer, vec3(0, 0, 1));
    }
    
    // lower leg -> 7, 9
    for(int i=0; i<2; i++){
        int mul = pow(-1,i%2); // 0-> + 1-> -
        int index = 7 + 2*i;
        gl_rotate(m_mat[index], m_leg[index-6]);
        m_mat[index] = m_mat[index] * translate(m_mat[index-1], vec3(body_trans[4].x, body_trans[4].y, -1 * mul *body_trans[4].z));
        m_mat[index] = rotate(m_mat[index], 0.40f + -1 * mul * 0.35f * timer, vec3(0, 0, 1));
        m_mat[index] = rotate(m_mat[index], radians(timer_cnt), rotate_axis);
    }
   
    // draw the body -> 0~9
    for(int i=0; i<Robot.size(); i++){
        glBindVertexArray(Robot[i]->vao);
        gl_scale(m_mat[i], Robot[i]);
        glUniformMatrix4fv(um4mv, 1, GL_FALSE, value_ptr(view * m_mat[i]));
        glUniformMatrix4fv(um4p, 1, GL_FALSE, value_ptr(projection));
        glDrawArrays(GL_TRIANGLES, 0, Robot[i]->vertexCount);
    }
    
    
    // bottom
  
    /*
    for(int i){
        
    }
    
    
    */

	// Todo
	// Practice 2 : Build 2 matrix (translation matrix and rotation matrix)
	// then multiply these matrix with proper order
	// final result should restore in the variable 'model'
    
 
   
	// Transfer value of (view*model) to both shader's inner variable 'um4mv'; 
	//glUniformMatrix4fv(um4mv, 1, GL_FALSE, value_ptr(view * model));

	// Transfer value of projection to both shader's inner variable 'um4p';
	//glUniformMatrix4fv(um4p, 1, GL_FALSE, value_ptr(projection));

	// Tell openGL to draw the vertex array we had binded before
	//glDrawArrays(GL_TRIANGLES, 0, m_shape.vertexCount);

	// Change current display buffer to another one (refresh frame) 
	glutSwapBuffers();
}

// Setting up viewing matrix
void My_Reshape(int width, int height)
{
	glViewport(0, 0, width, height);

	float viewportAspect = (float)width / (float)height;

	// perspective(fov, aspect_ratio, near_plane_distance, far_plane_distance)
	// ps. fov = field of view, it represent how much range(degree) is this camera could see 
	projection = perspective(radians(60.0f), viewportAspect, 0.1f, 1000.0f);

	// lookAt(camera_position, camera_viewing_vector, up_vector)
	// up_vector represent the vector which define the direction of 'up'
	view = lookAt(vec3(-10.0f, 5.0f, 0.0f), vec3(1.0f, 1.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));
}

void My_Timer(int val)
{
	timer_cnt += 1.0f;
	glutPostRedisplay();
	if (timer_enabled)
	{
		glutTimerFunc(timer_speed, My_Timer, val+1);
	}
}

void My_Keyboard(unsigned char key, int x, int y)
{
	printf("Key %c is pressed at (%d, %d)\n", key, x, y);
    switch (key)
    {
    case 't':
        cur_trans_mode = GeoTranslation;
        break;
    case 'r':
        cur_trans_mode = GeoRotation;
        break;
    default:
        break;
    }
	if (key == 'd')
	{
		temp = temp + vec3(0, 0, 1);
	}
	else if (key == 'a')
	{
		temp = temp - vec3(0, 0, 1);
	}
	else if (key == 'w')
	{
		temp = temp + vec3(1, 0, 0);
	}
	else if (key == 's')
	{
		temp = temp - vec3(1, 0, 0);
	}
}

void My_SpecialKeys(int key, int x, int y)
{
	switch (key)
	{
	case GLUT_KEY_F1:
		printf("F1 is pressed at (%d, %d)\n", x, y);
		break;
	case GLUT_KEY_PAGE_UP:
		printf("Page up is pressed at (%d, %d)\n", x, y);
		break;
	case GLUT_KEY_LEFT:
		printf("Left arrow is pressed at (%d, %d)\n", x, y);
		break;
	default:
		printf("Other special key is pressed at (%d, %d)\n", x, y);
		break;
	}
}

int last_xpos = INT_MIN, last_ypos = INT_MIN;



void My_Menu(int id)
{
	switch (id)
	{
	case MENU_TIMER_START:
		if (!timer_enabled)
		{
			timer_enabled = true;
			glutTimerFunc(timer_speed, My_Timer, timer_cnt+1);
		}
		break;
	case MENU_TIMER_STOP:
		timer_enabled = false;
		break;
	case MENU_EXIT:
		exit(0);
		break;
	case MENU_HELLO:
		// do something
		break;
	case MENU_WORLD:
		// do something
		break;
	default:
		break;
	}
}


void My_Mouse(int button, int state, int x, int y)
{
	if (button == GLUT_LEFT_BUTTON)
	{
		if (state == GLUT_DOWN)
		{
			printf("Mouse %d is pressed at (%d, %d)\n", button, x, y);
            mouse_pressed = true;
		}
		else if (state == GLUT_UP)
		{
			printf("Mouse %d is released at (%d, %d)\n", button, x, y);
            mouse_pressed = true;
            last_xpos = last_ypos = INT_MIN;
		}
	}
}







int main(int argc, char *argv[])
{
#ifdef __APPLE__
	// Change working directory to source code path
	chdir(__FILEPATH__("/../Assets/"));
#endif
	// Initialize GLUT and GLEW, then create a window.
	glutInit(&argc, argv);
#ifdef _MSC_VER
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
#else
	glutInitDisplayMode(GLUT_3_2_CORE_PROFILE | GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
#endif
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(600, 600);
	glutCreateWindow("Practice"); // You cannot use OpenGL functions before this line;
								  // The OpenGL context must be created first by glutCreateWindow()!
#ifdef _MSC_VER
	glewInit();
#endif
	dumpInfo();
	My_Init();

	int menu_main = glutCreateMenu(My_Menu);
	int menu_timer = glutCreateMenu(My_Menu);
	int menu_new = glutCreateMenu(My_Menu);

	glutSetMenu(menu_main);
	// yi
    glutAttachMenu(GLUT_RIGHT_BUTTON);
    glutAddSubMenu("Timer", menu_timer);
	glutAddSubMenu("New", menu_new);
	glutAddMenuEntry("Exit", MENU_EXIT);

	glutSetMenu(menu_timer);
	glutAddMenuEntry("Start", MENU_TIMER_START);
	glutAddMenuEntry("Stop", MENU_TIMER_STOP);

	glutSetMenu(menu_new);						// Tell GLUT to design the menu which id==menu_new now
	glutAddMenuEntry("Hello", MENU_HELLO);		// Add submenu "Hello" in "New"(a menu which index is equal to menu_new)
	glutAddMenuEntry("World", MENU_WORLD);		// Add submenu "Hello" in "New"(a menu which index is equal to menu_new)

	

	// Register GLUT callback functions.
	glutDisplayFunc(My_Display);
	glutReshapeFunc(My_Reshape);
	glutKeyboardFunc(My_Keyboard);
	glutSpecialFunc(My_SpecialKeys);
	glutTimerFunc(timer_speed, My_Timer, 0);
	// Todo
	// Practice 1 : Register new GLUT event listner here
	// ex. glutXXXXX(my_Func);
	// Remind : you have to implement my_Func
    glutMouseFunc(My_Mouse);
  
	// Enter main event loop.
	glutMainLoop();

	return 0;
}
