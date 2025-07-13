#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <vector>
#include <random>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);
void gameOver(GLFWwindow *window);
void generate_food();
bool inRange(glm::vec2);


// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;
const int GRID_SIZE = 16;
const float TICK_RATE = 0.1f;

// Snake properties
enum Direction { UP, DOWN, LEFT, RIGHT };
Direction dir = RIGHT;
Direction next_dir = RIGHT;
std::vector<glm::vec2> snake_body;
glm::vec2 food_pos;

const char *vertexShaderSource = "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "uniform mat4 model;\n"
    "uniform mat4 view;\n"
    "uniform mat4 projection;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = projection * view * model * vec4(aPos, 1.0);\n"
    "}\0";
const char *fragmentShaderSource = "#version 330 core\n"
    "out vec4 FragColor;\n"
    "uniform vec4 ourColor;\n"
    "void main()\n"
    "{\n"
    "   FragColor = ourColor;\n"
    "}\0";

int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Snake", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // build and compile our shader program
    // ------------------------------------
    // vertex shader
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    // check for shader compile errors
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    // fragment shader
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    // check for shader compile errors
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    // link shaders
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    // check for linking errors
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    float vertices[] = {
        // positions         
         0.5f,  0.5f, 0.0f,  // top right
         0.5f, -0.5f, 0.0f,  // bottom right
        -0.5f, -0.5f, 0.0f,  // bottom left
        -0.5f,  0.5f, 0.0f   // top left 
    };
    unsigned int indices[] = {  
        0, 1, 3,  // first triangle
        1, 2, 3   // second triangle
    }; 

    unsigned int VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0); 

    glBindVertexArray(0); 

    // Initialize snake and food
    snake_body.push_back(glm::vec2(GRID_SIZE / 2, GRID_SIZE / 2));
    generate_food();

    // Game loop timer
    double lastTime = glfwGetTime();
    double timer = lastTime;

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        // per-frame time logic
        // --------------------
        double currentTime = glfwGetTime();
        float deltaTime = float(currentTime - lastTime);
        lastTime = currentTime;

        // input
        // -----
        processInput(window);

        // update
        // ------
        if (currentTime - timer > TICK_RATE)
        {
            timer = currentTime;
            dir = next_dir;

            glm::vec2 new_head_pos = snake_body[0];
            if (dir == UP)
                new_head_pos.y += 1;
            if (dir == DOWN)
                new_head_pos.y -= 1;
            if (dir == LEFT)
                new_head_pos.x -= 1;
            if (dir == RIGHT)
                new_head_pos.x += 1;

            snake_body.insert(snake_body.begin(), new_head_pos);
            
            std::cout<<snake_body[0].x<<std::endl<<snake_body[0].y<<std::endl;
            
            if (!inRange(snake_body[0]))
            {
               gameOver(window); 
            }

            for (size_t i = 0; i<snake_body.size()-1; ++i)
            {
                if (snake_body[0] == snake_body[i+1])
                {
                    gameOver(window);
                }
            }

            if (snake_body[0] == food_pos)
            {
                generate_food();
            }
            else
            {
                snake_body.pop_back();
            }
        }

        // render
        // ------
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        framebuffer_size_callback(window, width, height);

        // draw our first triangle
        glUseProgram(shaderProgram);

        glm::mat4 view = glm::mat4(1.0f);
        glm::mat4 projection = glm::ortho(0.0f, (float)GRID_SIZE, 0.0f, (float)GRID_SIZE, -1.0f, 1.0f);

        unsigned int viewLoc = glGetUniformLocation(shaderProgram, "view");
        unsigned int projectionLoc = glGetUniformLocation(shaderProgram, "projection");

        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

        glBindVertexArray(VAO); 

        // Draw background
        int colorLoc = glGetUniformLocation(shaderProgram, "ourColor");
        glUniform4f(colorLoc, 0.82f, 0.71f, 0.55f, 1.0f);
        glm::mat4 bg_model = glm::mat4(1.0f);
        bg_model = glm::translate(bg_model, glm::vec3(GRID_SIZE / 2.0f, GRID_SIZE / 2.0f, 0.0f));
        bg_model = glm::scale(bg_model, glm::vec3(GRID_SIZE, GRID_SIZE, 1.0f));
        unsigned int modelLoc = glGetUniformLocation(shaderProgram, "model");
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(bg_model));
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        // Draw snake
        glUniform4f(colorLoc, 0.0f, 1.0f, 0.0f, 1.0f);
        for (glm::vec2 pos : snake_body)
        {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(pos.x + 0.5f, pos.y + 0.5f, 0.0f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        }

        // Draw food
        glUniform4f(colorLoc, 1.0f, 0.0f, 0.0f, 1.0f);
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(food_pos.x + 0.5f, food_pos.y + 0.5f, 0.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
 
        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteProgram(shaderProgram);

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS && dir != DOWN)
        next_dir = UP;
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS && dir != UP)
        next_dir = DOWN;
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS && dir != RIGHT)
        next_dir = LEFT;
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS && dir != LEFT)
        next_dir = RIGHT;
}

// TODO: game over screen instead of closure

void gameOver(GLFWwindow *window)
{
    glfwSetWindowShouldClose(window, true);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    int size = std::min(width, height);
    int x_offset = (width - size) / 2;
    int y_offset = (height - size) / 2;
    glViewport(x_offset, y_offset, size, size);
}

// TODO: make food generate outside snake 

void generate_food()
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, GRID_SIZE - 1);
    food_pos.x = dis(gen);
    food_pos.y = dis(gen);

    // maybe check if it is inside snake if so change to closest free cell
}

bool inRange(glm::vec2 point)
{
    if (point.x<0 || point.y<0 || point.x>GRID_SIZE-1 || point.y>GRID_SIZE-1)
    {
        return false;
    }
    return true;
}
