#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "stb_image/stb_image.h"

#include "GLDebug.hpp"
#include "VertexBuffer.hpp"
#include "IndexBuffer.hpp"
#include "VertexArray.hpp"
#include "Shader.hpp"
#include "Camera.hpp"
#include "Renderer.hpp"
#include "Texture.hpp"
#include "Mesh.hpp"
#include "Model.hpp"
#include "Light.hpp"
#include "Transform.hpp"
#include "OutlineRenderer.hpp"
#include "PickingTexture.hpp"
#include "FrameBuffers.hpp"

/* include imgui
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"
 */

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <vector>
#include <memory>

void processInput(GLFWwindow *window, Camera& camera); 
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
glm::mat4 GetLightSpaceMatrix(glm::vec3 position, glm::vec3 lookat, float near, float far);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

const unsigned int SHADOW_WIDTH = 1024;
const unsigned int ShADOW_HEIGHT = 1024;


// timing
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;

bool firstMouse = true;

float lastX = 400.0f, lastY = 300.0f;

Camera camera;

int main(){
    glfwInit();

    // set OpenGL version to 4.1
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    
    // set OpenGL profile to core-profile
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // set OpenGL forward compatibility to true, 
    // which means application uses only the modern features of OpenGL
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // for macOS
    
    // creating window
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL){
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    // window -> context
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    
    // glad: load all OpenGL function pointers
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }    

    /*  -----   GLobal Opengl  -----   */
    GLCall(glEnable(GL_DEPTH_TEST));

    GLCall(glEnable(GL_BLEND));
    GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

    // GLCall(glEnable(GL_FRAMEBUFFER_SRGB)); // gamma correction

    /* -----   Stencil Buffer for outlining -----  
    GLCall(glEnable(GL_STENCIL_TEST));
    GLCall(glStencilFunc(GL_NOTEQUAL, 1, 0xFF));
    GLCall(glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE));
    */

/*  -----   -----   -----   -----   */

    // HDR Framebuffer
    HDRFrameBuffer hdrFrameBuffer(SCR_WIDTH, SCR_HEIGHT);
    PingpongFrameBuffer pingpongFrameBuffer(SCR_WIDTH, SCR_HEIGHT);
	
	// Depth FrameBUffer for shadow
	DepthMapFrameBuffer depthFrameBuffer(SHADOW_WIDTH, SHADOW_WIDTH);

/*  -----   Shader  -----   */
    Shader planetShader("src/shaders/PlanetShader.shader");
    Shader starShader("src/shaders/StarShader.shader");
    Shader hdrShader("src/shaders/HDRShader.shader");
    Shader blurShader("src/shaders/BlurShader.shader");
	Shader depthShader("src/shaders/DepthShader.shader");
/*  -----   -----  -----   */

	/* Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui_ImplOpenGL3_Init("#version 410");

    // Setup Style
    ImGui::StyleColorsDark();
    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);

    bool show_demo_window = false;
	*/


/*          ****    ****    ****        */
/*  ----- vertices indices textures --- */
/*          ****    ****    ****        */

    // model
    Model moonModel("res/models/moon/moon.obj");
    Model earthModel("res/models/earth/earth.obj");
    Model sunModel("res/models/sun/sun.obj");
    
/*  -----   define light uniform   -----   */
    glm::vec3 sun_position = glm::vec3( 0.0f,  0.0f,  0.0f);
    glm::vec3 sun_color = glm::vec3(1.0f, 1.0f, 1.0f);
    DirectionalLight sunLight(sun_position, sun_color);


/*  -----   -------   -----   */

    // Camera
//	camera = Camera(glm::vec3(190.0f, 20.0f, 10.0f));
    camera = Camera(glm::vec3(39.0f, 5.0f, 2.0f));
	
	glm::vec3 earth_position = glm::vec3(40.0f, 0.0f, 0.0f);
	glm::vec3 moon_position = glm::vec3(39.0f, 0.0f, 0.0f);
	
	float sun_rotate_speed_factor = 0.001f;
	float earth_rotate_speed_factor = 20.0f;
	float earth_orbit_speed_factor = 20 / 36.5f; // 1 / 365
	float moon_rotate_orbit_speed_factor = 20 / 2.73f; // 1 / 27.3f
	
	Transform sunTrans(sun_position, 0.0f, 2.2f);
    Transform earthTrans(sun_position, 40.0f, 0.05f);
    Transform moonTrans(earth_position, -1.0f, 0.06f);
	
	// set all textures
    blurShader.Use();
    blurShader.setInt("image", 0);
	
    hdrShader.Use();
    hdrFrameBuffer.SetBufferToTexture(&hdrShader, 0);
    pingpongFrameBuffer.SetBufferToTexture(&hdrShader, 1);
	
	planetShader.Use();
	// diffuse 0, specular 1, emission 2
	depthFrameBuffer.SetBufferToTexture(&planetShader, 3);

/*          ****    ****    ****        */
/*       -----   Render Loop  -----     */
    Renderer renderer;
    while(!glfwWindowShouldClose(window)){
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        processInput(window, camera);
		
		float time = glfwGetTime();

        sunTrans.UpdateRotation(glm::vec3(0.0f, time * sun_rotate_speed_factor, 0.0f));
		glm::mat4 sun_model = sunTrans.GetModelMatrix();
		
		earthTrans.UpdateRotation(glm::vec3(0.0f, -30.0f + time * earth_rotate_speed_factor, 23.5f));
//		earthTrans.UpdateOrbition(glm::vec3(0.0f, time * earth_orbit_speed_factor, 0.0f));
		glm::mat4 earth_model = earthTrans.GetModelMatrix();
		earth_position = glm::vec3(earth_model[3]);
		
//      moonTrans.UpdateRotation(glm::vec3(0.0f, time *  moon_rotate_orbit_speed_factor, 0.0f));
//		moonTrans.UpdateOrbition(glm::vec3(0.0f, time *  moon_rotate_orbit_speed_factor, 0.0f));
		moonTrans.UpdateCenter(earth_model[3]);
		glm::mat4 moon_model = moonTrans.GetModelMatrix();
		moon_position = glm::vec3(moon_model[3]);
		
//		camera.earthCameraTracking(earth_model[3]);
								  
        glm::mat4 view = camera.getViewMatrix();
        glm::mat4 projection = glm::perspective(glm::radians(FOV), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.01f, 100.f);
		
        renderer.Clear();
        // Depth -> Shadow

		glm::mat4 lightSpaceMat = glm::mat4(0.0f);
		GLCall(glViewport(0, 0, SHADOW_WIDTH, ShADOW_HEIGHT));
		depthFrameBuffer.Bind();
		depthFrameBuffer.Clear();
			depthShader.Use();
			lightSpaceMat = GetLightSpaceMatrix(sun_position, earth_position, 1.0f, 100.0f);
			depthFrameBuffer.SetupShader(&depthShader, lightSpaceMat);
			depthShader.setMat4("model", earth_model);
			earthModel.Render(&depthShader);
		
			depthShader.Use();
			lightSpaceMat = GetLightSpaceMatrix(sun_position, moon_position, 1.0f, 100.0f);
			depthFrameBuffer.SetupShader(&depthShader, lightSpaceMat);
			depthShader.setMat4("model", moon_model);
			moonModel.Render(&depthShader);
		
			depthShader.Use();
			lightSpaceMat = GetLightSpaceMatrix(sun_position, sun_position, 1.0f, 100.0f);
			depthShader.setMat4("model", sun_model);
			sunModel.Render(&depthShader);
		depthFrameBuffer.Unbind();
		
		depthFrameBuffer.ActiveTexture(3);
		depthFrameBuffer.BindTexture();
		
		// HDR
		GLCall(glViewport(0, 0, SCR_WIDTH*2, SCR_HEIGHT*2));
		hdrFrameBuffer.Bind();
        hdrFrameBuffer.Clear();

            // the Earth
            sunLight.updatgeTarget(earth_position);
            sunLight.SetupShader(&planetShader);
		
            planetShader.Use();
			lightSpaceMat = GetLightSpaceMatrix(sun_position, earth_position, 1.0f, 100.0f);
			depthFrameBuffer.SetupShader(&planetShader, lightSpaceMat);
		
            planetShader.setMat4("model", earth_model);
            planetShader.setMat4("projection", projection);
            planetShader.setMat4("view", view);
            planetShader.setVec3("viewPos", camera.Position);
            planetShader.UnUse();

            earthModel.Render(&planetShader);

            // the Moon
            sunLight.updatgeTarget(moon_position);
            sunLight.SetupShader(&planetShader);

            planetShader.Use();
			lightSpaceMat = GetLightSpaceMatrix(sun_position, moon_position, 1.0f, 100.0f);
			depthFrameBuffer.SetupShader(&planetShader, lightSpaceMat);
            planetShader.setMat4("model", moon_model);
            planetShader.UnUse();

            moonModel.Render(&planetShader);

            // the Sun
            starShader.Use();
            starShader.setMat4("model", sun_model);
            starShader.setMat4("projection", projection);
            starShader.setMat4("view", view);
            starShader.UnUse();

            sunModel.Render(&starShader);

        hdrFrameBuffer.Unbind();

        // blur
        bool horizontal = true, first_iteration = true;
        unsigned int rounds = 16;
        
        for (int i = 0; i < rounds; i ++) {
            pingpongFrameBuffer.Bind(horizontal);
            blurShader.Use();
            blurShader.setInt("horizontal", horizontal);
            if (first_iteration) {
                hdrFrameBuffer.BindTexture(1); // use brightness texture
            } else {
                pingpongFrameBuffer.BindTexture(!horizontal);
            }
            hdrFrameBuffer.RenderBufferToScreen(&blurShader);
            horizontal = !horizontal;
            if (first_iteration) {
                first_iteration = false;
            }
        }
        pingpongFrameBuffer.Unbind();

        // render to default framebuffer
        hdrFrameBuffer.Clear();
        hdrShader.Use();
        // FrameBuffer Texture
		hdrFrameBuffer.ActiveTexture(0);
        hdrFrameBuffer.BindTexture(0);
        pingpongFrameBuffer.ActiveTexture(1);
        pingpongFrameBuffer.BindTexture(!horizontal);
		
        hdrFrameBuffer.SetupShader(&hdrShader, 1.0f);
        hdrFrameBuffer.RenderBufferToScreen(&hdrShader);

        // poll IO events
        glfwPollEvents();

        // swap buffers ( from back to front screen)
        glfwSwapBuffers(window);

    }
/*  -----   -----   -----   -----   */
    
	/* Imgui end
    ImGui_ImplGlfw_Shutdown();
    ImGui_ImplOpenGL3_Shutdown();
    ImGui::DestroyContext();
    glfwTerminate();
    return 0;
	 */

}

// handling user input for a given window 
// query GLFW whether relevant keys are pressed/released this frame and react accordingly
void processInput(GLFWwindow *window, Camera& camera) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }

    if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        camera.processKeyboard(FORWARD, deltaTime);
    }
    if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        camera.processKeyboard(BACKWARD, deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        camera.processKeyboard(RIGHT, deltaTime);
    }
    if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        camera.processKeyboard(LEFT, deltaTime);
    }
}

// this callback function executes whenever the window size changed 
//  adjust the OpenGL viewport to match the new window size
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    // make sure the viewport matches the new window dimensions
    glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = ypos - lastY;
    lastX = xpos;
    lastY = ypos;

    camera.processMouse(xoffset, yoffset);
}

glm::mat4 GetLightSpaceMatrix(glm::vec3 position, glm::vec3 lookat, float near, float far) {
	glm::mat4 lightProjection, lightView;
	glm::mat4 lightSpaceMatrix;
	lightProjection = glm::ortho(-50.0f, 50.0f, -50.0f, 50.0f, near, far);
	lightView = glm::lookAt(position, lookat, glm::vec3(0.0, 1.0, 0.0));
	lightSpaceMatrix = lightProjection * lightView;
	
	return lightSpaceMatrix;
}
