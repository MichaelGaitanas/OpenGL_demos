#include<GL/glew.h>
#include<GLFW/glfw3.h>
#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include<glm/gtc/type_ptr.hpp>
#include<cstdio>

#include"../include/shader.hpp"
#include"../include/mesh.hpp"
#include"../include/skybox.hpp"
#include"../include/camera.hpp"

camera cam(glm::vec3(-10.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), 0.0f);

float time_tick; //Elapsed time per frame update.

float xpos_previous, ypos_previous;
bool first_time_entered_the_window = true;

bool cursor_visible = false;

int win_width = 1200, win_height = 900;

//For 'continuous' events, i.e. at every frame in the while() loop.
void event_tick(GLFWwindow *win)
{
    bool move_key_pressed = false;
    if (glfwGetKey(win, GLFW_KEY_W) == GLFW_PRESS)
    {
        cam.accelerate(time_tick, cam.front);
        move_key_pressed = true;
    }
    if (glfwGetKey(win, GLFW_KEY_S) == GLFW_PRESS)
    {
        cam.accelerate(time_tick, -cam.front);
        move_key_pressed = true;
    }
    if (glfwGetKey(win, GLFW_KEY_D) == GLFW_PRESS)
    {
        cam.accelerate(time_tick, cam.right);
        move_key_pressed = true;
    }
    if (glfwGetKey(win, GLFW_KEY_A) == GLFW_PRESS)
    {
        cam.accelerate(time_tick, -cam.right);
        move_key_pressed = true;
    }
    if (glfwGetKey(win, GLFW_KEY_E) == GLFW_PRESS)
    {
        cam.accelerate(time_tick, cam.world_up);
        move_key_pressed = true;
    }
    if (glfwGetKey(win, GLFW_KEY_Q) == GLFW_PRESS)
    {
        cam.accelerate(time_tick, -cam.world_up);
        move_key_pressed = true;
    }

    //If no keys are pressed, decelerate.
    if (!move_key_pressed)
        cam.decelerate(time_tick);
}

//For discrete keyboard events.
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_RELEASE)
        glfwSetWindowShouldClose(window, true);
}

//When a mouse button is pressed, do the following :
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    //Toggle cursor visibility via the mouse right click.
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE)
    {
        cursor_visible = !cursor_visible;
        if (cursor_visible)
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        else
        {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            first_time_entered_the_window = true;
        }
    }
}

//When the mouse moves, do the following :
void cursor_pos_callback(GLFWwindow *win, double xpos, double ypos)
{
    if (cursor_visible)
        return;

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
    win_width = w;
    win_height = h;
    glViewport(0,0,w,h);
}

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4);

    glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);

    GLFWwindow *window = glfwCreateWindow(win_width, win_height, "Skybox", NULL, NULL);
    if (window == NULL)
    {
        printf("Failed to create glfw window. Exiting...\n");
        glfwTerminate();
        return 0;
    }
    glfwMakeContextCurrent(window);
    glfwSetWindowSizeLimits(window, 400, 400, GLFW_DONT_CARE, GLFW_DONT_CARE);

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, cursor_pos_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); //Hide the mouse initially.

    glfwGetWindowSize(window, &win_width, &win_height);

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
    glm::vec3 mesh_col = glm::vec3(0.1f,0.6f,0.0f);
    shadsuz.use();
    shadsuz.set_vec3_uniform("light_pos", light_pos);
    shadsuz.set_vec3_uniform("light_col", light_col);
    shadsuz.set_vec3_uniform("mesh_col", mesh_col);

    skybox sb;
    shader shadsb("../shaders/vertex/skybox.vert","../shaders/fragment/skybox.frag");

    glEnable(GL_DEPTH_TEST);
    glClearColor(0.1f,0.1f,0.1f,1.0f);

    glm::mat4 projection, view, model;

    float t1 = 0.0f, t2;
    while (!glfwWindowShouldClose(window))
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        t2 = (float)glfwGetTime(); //Elapsed time [sec] since glfwInit().
        time_tick = t2 - t1;
        t1 = t2;

        event_tick(window);

        projection = glm::perspective(glm::radians(45.0f),(float)win_width/(float)win_height, 0.01f,100.0f);
        cam.move(time_tick);
        view = cam.view();
        model = glm::mat4(1.0f);
        shadsuz.use();
        shadsuz.set_mat4_uniform("projection", projection);
        shadsuz.set_mat4_uniform("view", view);
        shadsuz.set_mat4_uniform("model", model);
        suzanne.draw_triangles();

        glDepthFunc(GL_LEQUAL);
            projection = glm::perspective(glm::radians(45.0f), (float)win_width/win_height, 0.01f,100.0f);
            view = glm::mat4(glm::mat3(glm::lookAt(cam.pos, cam.pos+cam.front, cam.up)));
            shadsb.use();
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
