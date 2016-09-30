#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <GL/gl3w.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

typedef struct _COORDS_ {
  GLfloat x;
  GLfloat y;
  GLfloat s;
  GLfloat t;
} CoordinatesSet;

typedef struct _RGBA_ {
  GLfloat Red;
  GLfloat Green;
  GLfloat Blue;
  GLfloat Alpha;
} RGBAValues;

void printWorkGroupsCapabilities() {
  int workgroup_count[3];
  int workgroup_size[3];
  int workgroup_invocations;

  glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, &workgroup_count[0]);
  glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1, &workgroup_count[1]);
  glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2, &workgroup_count[2]);

  printf ("Taille maximale des workgroups globaux:\n\tx:%u\n\ty:%u\n\tz:%u\n",
  workgroup_size[0], workgroup_size[1], workgroup_size[2]);

  glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0, &workgroup_size[0]);
  glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1, &workgroup_size[1]);
  glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2, &workgroup_size[2]);

  printf ("Taille maximale des workgroups locaux:\n\tx:%u\n\ty:%u\n\tz:%u\n",
  workgroup_size[0], workgroup_size[1], workgroup_size[2]);

  glGetIntegerv (GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, &workgroup_invocations);
  printf ("Nombre maximum d'invocation de workgroups locaux:\n\t%u\n", workgroup_invocations);
}

void compileShader(GLuint shaderID, char * sourcePointer) {
  glShaderSource(shaderID, 1, &sourcePointer , NULL);
  glCompileShader(shaderID);

  GLint Result = GL_FALSE;
  int InfoLogLength = 1024;
  char shaderErrorMessage[1024] = {0};

  glGetShaderiv(shaderID, GL_COMPILE_STATUS, &Result);
	
  glGetShaderInfoLog(shaderID, InfoLogLength, NULL, shaderErrorMessage);
  if (strlen(shaderErrorMessage) != 0) 
    std::cout <<  shaderErrorMessage << "\n" ;
}

void loadShader(char ** shaderBuffer, const char * fileName) {
  FILE * shader = fopen (fileName, "r");
  (*shaderBuffer) = new char[1280];
  for (int i=0;i<1280;i++) {
    (*shaderBuffer)[i] = (unsigned char ) fgetc(shader);
    if ((*shaderBuffer)[i] == EOF) {
      (*shaderBuffer)[i] = '\0';
      break;
    }
  }
  fclose(shader);
}

int main(int argc, char ** argv) {
  /* ----- Init window ----- */
  
  GLFWwindow * window;
  if (!glfwInit()) {
    exit(-1);
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

  window = glfwCreateWindow(640, 480, "Damn simple compute shader", NULL, NULL);
  
  if (!window)  {
    exit(-2);
  }

  glfwMakeContextCurrent(window);

  if(gl3wInit() != 0) {
    exit(-3);
  }
  
  /* ----- Quad Context ----- */

  RGBAValues * quadTexture = new RGBAValues[640*480];
  CoordinatesSet * quadVertex = new CoordinatesSet[4];
  unsigned short int * quadIndex = new unsigned short int[3];

  quadVertex[0].x = -1.0f;
  quadVertex[0].y = 1.0f;
  
  quadVertex[1].x = -1.0f;
  quadVertex[1].y = -1.0f;
  
  quadVertex[2].x = 1.0f;
  quadVertex[2].y = 1.0f;
  
  quadVertex[3].x = 1.0f;
  quadVertex[3].y = -1.0f;
  
  quadIndex[0] = 0;
  quadIndex[1] = 1;
  quadIndex[2] = 2;
  quadIndex[3] = 3;
  
  quadVertex[0].s = 0.0f;
  quadVertex[0].t = 0.0f;
  quadVertex[1].s = 0.0f;
  quadVertex[1].t = 1.0f;
  quadVertex[2].s = 1.0f;
  quadVertex[2].t = 0.0f;
  quadVertex[3].s = 1.0f;
  quadVertex[3].t = 1.0f;

  for(int x=0; x < 640; x++) {
    for(int y=0; y< 480; y++) {
      quadTexture[x + y * 640].Red = 1.0f;
      quadTexture[x + y * 640].Green = .5f;
      quadTexture[x + y * 640].Blue = .0f;
      quadTexture[x + y * 640].Alpha = 1.0f;
    }
  }

  /* ----- Render Context ----- */

  GLuint quadIBO;
  GLuint quadVBO;
  GLuint quadVAO;
  GLuint quadTextureID;

  /* Création de la texture et association à une image unit */
  glGenTextures(1, &quadTextureID);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, quadTextureID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glGenerateMipmap(GL_TEXTURE_2D);  
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 640, 480, 0, GL_RGBA, GL_FLOAT, NULL);
    glBindImageTexture (0, quadTextureID, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
    glBindImageTexture (0, 0, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
  glBindTexture(GL_TEXTURE_2D, 0);
  
  glGenBuffers(1, &quadIBO);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quadIBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 4 * sizeof(unsigned short int), quadIndex, GL_STATIC_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

  glGenBuffers(1, &quadVBO);
  glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(CoordinatesSet) * 4, 0, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(CoordinatesSet) * 4, quadVertex);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  
  glGenVertexArrays(1, &quadVAO);
  glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
      glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);
      glEnableVertexAttribArray(0);
      glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid *) (sizeof(GLfloat) * 2));
      glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);

  /* ----- Compute Shader ----- */

  printWorkGroupsCapabilities();

  GLuint computeShaderID;
  GLuint csProgramID;
  char * computeShader;

  GLint Result = GL_FALSE;
  int InfoLogLength = 1024;
  char ProgramErrorMessage[1024] = {0};
  
  computeShaderID = glCreateShader(GL_COMPUTE_SHADER);
  
  loadShader(&computeShader, "compute.shader");
  compileShader(computeShaderID, computeShader);
  
  csProgramID = glCreateProgram();

  glAttachShader(csProgramID, computeShaderID);
  glLinkProgram(csProgramID);
  glDeleteShader(computeShaderID);
  
  /* ----- Vertex shaders and Fragments shaders ----- */

  GLuint vertexShaderID;
  GLuint fragmentShaderID;
  GLuint programID;

  Result = GL_FALSE;
  InfoLogLength = 1024;
  ProgramErrorMessage[1024] = {0};

  char * vertexShader;
  char * fragmentShader;

  vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
  fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
  
  loadShader(&vertexShader, "vertex.shader");
  loadShader(&fragmentShader, "fragment.shader");
  
  compileShader(vertexShaderID, vertexShader);
  compileShader(fragmentShaderID, fragmentShader);

  programID = glCreateProgram();

  glAttachShader(programID, vertexShaderID);
  glAttachShader(programID, fragmentShaderID);

  glLinkProgram(programID);

  glDeleteShader(vertexShaderID);
  glDeleteShader(fragmentShaderID);

  glGetProgramiv(programID, GL_LINK_STATUS, &Result);
  glGetProgramiv(programID, GL_INFO_LOG_LENGTH, &InfoLogLength);
  glGetProgramInfoLog(programID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
  
  if (strlen(ProgramErrorMessage) != 0) 
    std::cout << ProgramErrorMessage << "\n";


  /* ----- Run Compute shader ----- */
  glUseProgram(csProgramID);
    glBindTexture(GL_TEXTURE_2D, quadTextureID);
      glBindImageTexture (0, quadTextureID, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
        glDispatchCompute(40,30,1);
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
      glBindImageTexture (0, 0, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
    glBindTexture(GL_TEXTURE_2D, 0);
  glUseProgram(0);

  /* ----- Render loop ----- */
  while(true) {
    usleep(40000);
    glEnable(GL_CULL_FACE);
      

    /* ----- Actual render ----- */
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glUseProgram(programID);
    
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quadIBO);
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, quadTextureID);
    glBindImageTexture (0, quadTextureID, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
    
    glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_SHORT, (GLvoid *) 0);
            
    glBindImageTexture (0, 0, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
    glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(0);
            
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    
    glUseProgram(0);
    

    glfwSwapBuffers(window);
  }

  return 0;
}
