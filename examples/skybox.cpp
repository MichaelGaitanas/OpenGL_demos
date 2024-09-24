#include<GL/glew.h>
#include<GLFW/glfw3.h>
#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include<glm/gtc/type_ptr.hpp>
#include<cstdio>

#include"../include/shader.hpp"
#include"../include/mesh.hpp"
#include"../include/camera.hpp"

camera cam(glm::vec3(0.0f,-10.0f,2.0f));

float t1 = 0.0f, t2, delta_time;
float xpos_previous, ypos_previous;
bool first_time_entered_the_window = true;

void raw_hardware_input(GLFWwindow *win)
{
    if (glfwGetKey(win, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(win, true);

    if (glfwGetKey(win, GLFW_KEY_W) == GLFW_PRESS)
        cam.translate_front(delta_time);
    if (glfwGetKey(win, GLFW_KEY_S) == GLFW_PRESS)
        cam.translate_back(delta_time);
    if (glfwGetKey(win, GLFW_KEY_D) == GLFW_PRESS)
        cam.translate_right(delta_time);
    if (glfwGetKey(win, GLFW_KEY_A) == GLFW_PRESS)
        cam.translate_left(delta_time);
}

void cursor_pos_callback(GLFWwindow *win, double xpos, double ypos)
{
    if (first_time_entered_the_window)
    {
        xpos_previous = xpos;
        ypos_previous = ypos;
        first_time_entered_the_window = false;
    }

    float xoffset = xpos - xpos_previous;
    float yoffset = ypos - ypos_previous;

    xpos_previous = xpos;
    ypos_previous = ypos;

    cam.rotate(xoffset, yoffset);
}

void framebuffer_size_callback(GLFWwindow *win, int w, int h)
{
    glViewport(0,0,w,h);
}

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);
    GLFWwindow *window = glfwCreateWindow(900, 900, "First person camera", NULL, NULL);
    if (window == NULL)
    {
        printf("Failed to create glfw window. Exiting...\n");
        glfwTerminate();
        return 0;
    }
    glfwMakeContextCurrent(window);
    glfwSetWindowSizeLimits(window, 400, 400, GLFW_DONT_CARE, GLFW_DONT_CARE);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, cursor_pos_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    int win_width, win_height;
    glfwGetWindowSize(window, &win_width, &win_height);
    xpos_previous = win_width/2.0f;
    ypos_previous = win_height/2.0f;
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
    {
        printf("Failed to initialize glew. Exiting...\n");
        return 0;
    }

    meshvfn suzanne("../obj/vfn/suzanne.obj");
    shader shadsuz("../shaders/vertex/trans_mvpn.vert","../shaders/fragment/point_light_ad.frag");
    glm::vec3 light_pos = glm::vec3(0.0f,-5.0f,60.0f);
    glm::vec3 light_col = glm::vec3(1.0f,1.0f,1.0f);
    glm::vec3 model_col = glm::vec3(0.1f,0.6f,0.0f);
    shadsuz.use();
    shadsuz.set_vec3_uniform("light_pos", light_pos);
    shadsuz.set_vec3_uniform("light_col", light_col);
    shadsuz.set_vec3_uniform("model_col", model_col);

    skybox sb;
    shader shadsb("../shaders/vertex/skybox.vert","../shaders/fragment/skybox.frag");

    glEnable(GL_DEPTH_TEST);
    glClearColor(0.1f,0.1f,0.1f,1.0f);
    while (!glfwWindowShouldClose(window))
    {
        t2 = glfwGetTime();
        delta_time = t2 - t1;
        t1 = t2;

        raw_hardware_input(window);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 projection = glm::perspective(glm::radians(45.0f),(float)win_width/(float)win_height, 0.01f,100.0f);
        glm::mat4 view = cam.view();
        glm::mat4 model = glm::mat4(1.0f);
        shadsuz.use();
        shadsuz.set_mat4_uniform("projection", projection);
        shadsuz.set_mat4_uniform("view", view);
        shadsuz.set_mat4_uniform("model", model);
        suzanne.draw_triangles();

        glDepthFunc(GL_LEQUAL);
            shadsb.use();
            projection = glm::perspective(glm::radians(45.0f), (float)win_width/win_height, 0.01f, 100.0f);
            view = glm::mat4(glm::mat3(glm::lookAt(cam.pos, cam.pos+cam.front, cam.up)));
            shadsb.set_mat4_uniform("projection", projection);
            shadsb.set_mat4_uniform("view", view);
            sb.draw();
        glDepthFunc(GL_LESS);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}