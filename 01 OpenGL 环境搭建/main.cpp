
#include "GLTools.h"
#include "GLFrustum.h"

#include "GLMatrixStack.h"
#include "GLGeometryTransform.h"
#ifdef __APPLE__
#include <glut/glut.h>
#else
#define FREEGLUT_STATIC
#include <GL/glut.h>
#endif


GLShaderManager        shaderManager;
//模型视图矩阵堆栈
GLMatrixStack        modelViewMatrix;

//投影视图矩阵堆栈
GLMatrixStack        projectionMatrix;

//观察者位置
GLFrame                cameraFrame;

//世界坐标位置
GLFrame             objectFrame;

//视景体，用来构造投影矩阵
GLFrustum            viewFrustum;

//三角形批次类
GLTriangleBatch     CC_Triangle;

//球
GLTriangleBatch     sphereBatch;
//环
GLTriangleBatch     torusBatch;
//圆柱
GLTriangleBatch     cylinderBatch;
//锥
GLTriangleBatch     coneBatch;
//磁盘
GLTriangleBatch     diskBatch;

//管道，用来管理投影视图矩阵堆栈和模型视图矩阵堆栈的
GLGeometryTransform    transformPipeline;

//颜色值，绿色、黑色
GLfloat vGreen[] = { 0.0f, 1.0f, 0.0f, 1.0f };
GLfloat vBlack[] = { 0.0f, 0.0f, 0.0f, 1.0f };

//空格的标记
int nStep = 0;


#define SMALL_STARS     100
#define MEDIUM_STARS     40
#define LARGE_STARS      15

#define SCREEN_X        800
#define SCREEN_Y        600

void DrawWireFramedBatch(GLTriangleBatch* pBatch)
{
    shaderManager.UseStockShader(GLT_SHADER_FLAT,transformPipeline.GetModelViewProjectionMatrix(),vGreen);
    pBatch->Draw();
    
    glEnable(GL_BLEND);
    glEnable(GL_LINE_SMOOTH);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    glPolygonOffset(-1.0, -1.0);
    
    glEnable(GL_POLYGON_OFFSET_LINE);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glLineWidth(2.0f);
    shaderManager.UseStockShader(GLT_SHADER_FLAT,transformPipeline.GetModelViewProjectionMatrix(),vBlack);
    
    pBatch->Draw();
    
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDisable(GL_POLYGON_OFFSET_LINE);
    glDisable(GL_BLEND);
    glLineWidth(1.0f);
    glDisable(GL_LINE_SMOOTH);
    
}

//场景召唤
void RenderScene()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    modelViewMatrix.PushMatrix();
    
    M3DMatrix44f Mcamea;
    cameraFrame.GetCameraMatrix(Mcamea);
    
    modelViewMatrix.MultMatrix(Mcamea);
    
    
    //世界坐标系
    M3DMatrix44f mObjectFrame;
    objectFrame.GetMatrix(mObjectFrame);
    
    modelViewMatrix.MultMatrix(mObjectFrame);
    
    switch (nStep) {
        case 0:
            DrawWireFramedBatch(&sphereBatch);
            break;
        case 1:
            DrawWireFramedBatch(&torusBatch);
            break;
        case 2:
            DrawWireFramedBatch(&cylinderBatch);
            break;
        case 3:
            DrawWireFramedBatch(&coneBatch);
            break;
        default:
            break;
    }
    
    modelViewMatrix.PopMatrix();
    glutSwapBuffers();
    
}


//对渲染进行必要的初始化。
void SetupRC()
{
    glClearColor(0.7f, 0.7f, 0.7f, 1.0f);
    
    shaderManager.InitializeStockShaders();
    
    //开启深度测试
    glEnable(GL_DEPTH);
    
    transformPipeline.SetMatrixStacks(modelViewMatrix, projectionMatrix);
    
    cameraFrame.MoveForward(-15);
    
    //球
    /*
     gltMakeSphere(GLTriangleBatch& sphereBatch, GLfloat fRadius, GLint iSlices, GLint iStacks);
     参数1:三角形批次类对象
     参数2:球体的半径
     参数3:从球体底部到顶部的三角形带的数量.其实球体是一圈一圈的三角形带组成的.
     参数4:围绕球体一圈排列的三角形对数
     */
    gltMakeSphere(sphereBatch, 3.0f, 10.0f, 20.0f);
    
    //🍩
    /*
     gltMakeTorus(GLTriangleBatch& torusBatch, GLfloat majorRadius, GLfloat minorRadius, GLint numMajor, GLint numMinor);
     参数1:三角形批次类
     参数2:甜甜圈的中心到外圈的半径
     参数3:甜甜圈的中心到内圈的半径
     参数4:沿着主半径的三角形的对数
     参数5:颜色内部较小半径的三角形对数.!!!尽量!!!:numMajor = 2 *numMinor
     */
    gltMakeTorus(torusBatch, 1.0f, 0.3f, 24, 12);
    
    //圆柱
    /*
     gltMakeCylinder(GLTriangleBatch& cylinderBatch, GLfloat baseRadius, GLfloat topRadius, GLfloat fLength, GLint numSlices, GLint numStacks);
     参数1:三角形批次对象
     参数2:底部半径
     参数3:头部半径
     参数4:圆柱的长度
     参数5:三角形对的数量
     参数6:底部堆叠到顶部圆环的三级形对数
     */
    gltMakeCylinder(cylinderBatch, 2.5f, 2.5f, 4.0f, 12, 2);
    
    //圆锥
    //圆柱体:从0开始向Z的正方向延伸
    //圆锥体:一端半径为0,另一端半径可指定
    gltMakeCylinder(coneBatch, 2.1f, 0, 3, 12, 2);
}

void ChangeSize(int w, int h)
{
    
    if(h == 0)
        h = 1;
    
    glViewport(0, 0, w, h);
    viewFrustum.SetPerspective(35.0f, float(w)/float(h), 1.0f, 500.0f);
    projectionMatrix.LoadMatrix(viewFrustum.GetProjectionMatrix());
    
    modelViewMatrix.LoadIdentity();
}

void specialKeyPress(int key, int x, int y)
{
    if (key == GLUT_KEY_UP) {
        objectFrame.RotateWorld(m3dDegToRad(-5), 1, 0, 0);
    }
    
    if (key == GLUT_KEY_DOWN) {
        objectFrame.RotateWorld(m3dDegToRad(5), 1.0f, 0, 0);
    }
    
    if (key == GLUT_KEY_LEFT) {
        objectFrame.RotateWorld(m3dDegToRad(-5.0f), 0, 1.0f, 0.0f);
    }
    
    if (key == GLUT_KEY_RIGHT) {
        objectFrame.RotateWorld(m3dDegToRad(5.0f), 0, 1.0f, 0.0f);
    }
    
    glutPostRedisplay();
}

void keyPressFunc(unsigned char key,int x ,int y)
{
    if (key == 32) {
        nStep++;
        if (nStep > 3) {
            nStep = 0;
        }
    }
    
    switch (nStep) {
        case 0:
            glutSetWindowTitle("球");
            break;
        case 1:
            glutSetWindowTitle("甜甜圈");
            break;
        case 2:
            glutSetWindowTitle("圆柱");
            break;
        case 3:
            glutSetWindowTitle("圆锥");
            break;
            
        default:
            break;
    }
    
    glutPostRedisplay();
}

int main(int argc, char* argv[])
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutCreateWindow("GYD ");
    
    //创建菜单
    glutReshapeFunc(ChangeSize);
    glutDisplayFunc(RenderScene);
    glutSpecialFunc(specialKeyPress);
    glutKeyboardFunc(keyPressFunc);
    
    GLenum err = glewInit();
    if (GLEW_OK != err) {
        fprintf(stderr, "GLEW Error: %s\n", glewGetErrorString(err));
        return 1;
    }
    
    SetupRC();
    glutMainLoop();
    
    return 0;
    
}
