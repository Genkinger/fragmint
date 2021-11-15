#include "gl.h"
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

#define internal static

#define MAX_TITLE_LENGTH 64
#define MAX_LOG_LENGTH 512

internal int Width;
internal int Height;
internal float Aspect;
internal float Time,LastTime,Delta;
internal char Title[MAX_TITLE_LENGTH];
internal GLFWwindow *Window;
internal char *VertexPath;
internal time_t VertexLastModified;
internal time_t FragmentLastModified;
internal char *FragmentPath;
internal char Log[MAX_LOG_LENGTH];
internal int LogLength;
internal unsigned int VertexShader;
internal unsigned int FragmentShader;
internal unsigned int Program;

internal float MouseX,MouseY;
internal int ButtonLeft,ButtonRight;
internal float dragX, dragY;

time_t LastModified(const char* Path){
    struct stat FileStat;
    stat(Path,&FileStat);
    return FileStat.st_mtime;
}

int LoadFile(const char* Path, unsigned char* Dest){
    FILE *File = fopen(Path,"rb");
    fseek(File,0,SEEK_END);
    int Size = ftell(File);

    if(Dest == NULL) goto end;
    rewind(File);
    fread(Dest,1,Size,File);

end:
    fclose(File);    
    return Size;
}

void Resize(GLFWwindow *Window, int _Width, int _Height){
    Width = _Width;
    Height = _Height;
    Aspect =  (float)Width/(float)Height;
    glViewport(0,0,Width,Height);
}

void MouseMove(GLFWwindow *Window,double X, double Y){
    MouseX = X;
    MouseY = Y;

}

void MouseButtons(GLFWwindow *Window, int Button, int Action, int Mods){
   if(Button  == GLFW_MOUSE_BUTTON_LEFT){
       ButtonLeft = ((Action == GLFW_PRESS) ? 1 : 0);
   }
   if(Button == GLFW_MOUSE_BUTTON_RIGHT){
       ButtonRight = ((Action == GLFW_PRESS) ? 1 : 0);
   }

}

unsigned int CompileShader(const char* Path, unsigned int Type, const char* Title){
    unsigned int Status;
    unsigned int Shader = glCreateShader(Type);
    int FileSize = LoadFile(Path,NULL);
    unsigned char *FileBuffer = malloc(FileSize);
    LoadFile(Path,FileBuffer);
    glShaderSource(Shader,1,&FileBuffer,&FileSize);
    glCompileShader(Shader);
    glGetShaderiv(Shader,GL_COMPILE_STATUS,&Status);
    if(Status != GL_TRUE){
        glGetShaderInfoLog(Shader,MAX_LOG_LENGTH,&LogLength,Log);
        printf("[%s]\n===\n%.*s\n",Title,LogLength,Log);
        glDeleteShader(Shader);
        Shader = 0;
    }

    free(FileBuffer);
    return Shader;
}

unsigned int LinkProgram(unsigned int VertexShader,unsigned int FragmentShader){
    unsigned int Status;
    unsigned int Program = glCreateProgram();

    glAttachShader(Program,VertexShader);
    glAttachShader(Program,FragmentShader);
    glLinkProgram(Program);

    glGetProgramiv(Program,GL_LINK_STATUS,&Status);
    if(Status != GL_TRUE){
        glGetProgramInfoLog(Program,MAX_LOG_LENGTH,&LogLength,Log);
        printf("[LINK]\n===\n%.*s\n",LogLength,Log);
        glDeleteProgram(Program);
        Program = 0;
    }
    return Program;
}

void CheckForModification(){
    time_t VertexModified = LastModified(VertexPath);
    time_t FragmentModified = LastModified(FragmentPath);
    if(VertexModified != VertexLastModified || FragmentModified != FragmentLastModified){
        glUseProgram(0);
        glDeleteProgram(Program);
        glDeleteShader(VertexShader);
        glDeleteShader(FragmentShader);
        VertexShader = CompileShader(VertexPath,GL_VERTEX_SHADER,"VERTEX");
        FragmentShader = CompileShader(FragmentPath,GL_FRAGMENT_SHADER,"FRAGMENT");
        Program = LinkProgram(VertexShader,FragmentShader);
        VertexLastModified = VertexModified;
        FragmentLastModified = FragmentModified;
    }
}

void Init(){
    Width = 800;
    Height = 600;
    LogLength = 0;
    MouseX = 0;
    MouseY = 0;
    ButtonLeft = 0;
    ButtonRight = 0;
    Aspect = (float)Width/(float)Height;
    
    strcpy(Title,"GLSL Playground");
    
    glfwInit();    
    Window = glfwCreateWindow(Width,Height,Title,NULL,NULL);
    glfwSetFramebufferSizeCallback(Window,Resize);
    glfwSetCursorPosCallback(Window,MouseMove);
    glfwSetMouseButtonCallback(Window,MouseButtons);
    glfwMakeContextCurrent(Window);
    
    Time = glfwGetTime();
    LastTime = Time;
    Delta = 0;
    
    FragmentPath = "./fragment.glsl";
    VertexPath = "./vertex.glsl";
    CheckForModification();
}

void DeInit(){
    glUseProgram(0);
    glDeleteProgram(Program);
    glDeleteShader(VertexShader);
    glDeleteShader(FragmentShader);
    glfwDestroyWindow(Window);
    glfwTerminate();

}

int main(){
    Init();

    while(!glfwWindowShouldClose(Window)){
        CheckForModification();
        
        Time = glfwGetTime();
        Delta = Time - LastTime;
        LastTime = Time;

        glfwPollEvents();
        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(Program);
        glUniform1f(glGetUniformLocation(Program,"Time"),Time);
        glUniform1f(glGetUniformLocation(Program,"Delta"),Delta);
        glUniform1f(glGetUniformLocation(Program,"Aspect"),Aspect);
        glUniform2i(glGetUniformLocation(Program,"Resolution"),Width,Height);
        glUniform2f(glGetUniformLocation(Program,"Mouse"),MouseX,MouseY);
        glUniform2i(glGetUniformLocation(Program,"Buttons"),ButtonLeft,ButtonRight);
        glDrawArrays(GL_TRIANGLES,0,6);
        glfwSwapBuffers(Window);
    }    

    DeInit();
    return 0;
}