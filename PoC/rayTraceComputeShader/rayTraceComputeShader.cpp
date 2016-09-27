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

typedef struct _RGB_ {
  unsigned char Red;
  unsigned char Green;
  unsigned char Blue;
} RGBValues;

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

  window = glfwCreateWindow(640, 480, "Ray Trace with compute shader", NULL, NULL);
  
  if (!window)  {
    exit(-2);
  }

  glfwMakeContextCurrent(window);

  if(gl3wInit() != 0) {
    exit(-3);
  }
  
  /* ----- Quad Context ----- */

  RGBValues * quadTexture = new RGBValues[640*480];
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
      quadTexture[x + y * 640].Red = 0xFF;
      quadTexture[x + y * 640].Green = 0x88;
      quadTexture[x + y * 640].Blue = 0x00;
    }
  }

  /* ----- Render Context ----- */

  GLuint quadIBO;
  GLuint quadVBO;
  GLuint quadVAO;
  GLuint quadTextureID;

  glGenTextures(1, &quadTextureID);
  glBindTexture(GL_TEXTURE_2D, quadTextureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 640, 480, 0, GL_RGB, GL_UNSIGNED_BYTE, quadTexture);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
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

  /* ----- Vertex shaders and Fragments shaders ----- */

  GLuint vertexShaderID;
  GLuint fragmentShaderID;
  GLuint programID;

  GLint Result = GL_FALSE;
  int InfoLogLength = 1024;
  char ProgramErrorMessage[1024] = {0};

  char * vertexShader;
  char * fragmentShader;

  vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
  fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
  
  loadShader(&vertexShader, "triangle.vs");
  loadShader(&fragmentShader, "triangle.fs");
  
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

  /* ----- Render loop ----- */

  while(true) {
    usleep(40000);
    glEnable(GL_CULL_FACE);  
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glUseProgram(programID);
      glBindTexture(GL_TEXTURE_2D, quadTextureID);
        glBindVertexArray(quadVAO);
          glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quadIBO);
            glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_SHORT, (GLvoid *) 0);
          glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
      glBindTexture(GL_TEXTURE_2D, 0);
    glUseProgram(0);
    
    glfwSwapBuffers(window);
  }

  return 0;
}