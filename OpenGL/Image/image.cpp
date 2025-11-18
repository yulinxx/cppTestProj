#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>
#include <vector>
#include <string>
#include <stdexcept>
#include <iostream>

// stb_image implementation
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

//////////////////////////////////////////////////
// Image Class
class Image
{
public:
    Image(const std::string& path)
    {
        data = stbi_load(path.c_str(), &width, &height, &channels, 0);
        if (!data)
        {
            throw std::runtime_error("Failed to load image: " + path);
        }
    }

    ~Image()
    {
        if (data)
        {
            stbi_image_free(data);
        }
    }

    unsigned char* getData() const
    {
        return data;
    }
    int getWidth() const
    {
        return width;
    }
    int getHeight() const
    {
        return height;
    }
    int getChannels() const
    {
        return channels;
    }

private:
    unsigned char* data = nullptr;
    int width = 0;
    int height = 0;
    int channels = 0;
};

//////////////////////////////////////////////////
// ImageDatas Class
class ImageDatas
{
public:
    struct ImageData
    {
        unsigned char* data;
        int width;
        int height;
        int channels;
    };

    void addImage(const Image& image)
    {
        ImageData imgData;
        imgData.data = image.getData();
        imgData.width = image.getWidth();
        imgData.height = image.getHeight();
        imgData.channels = image.getChannels();
        vecImgDatas.push_back(imgData);
    }

    const std::vector<ImageData>& getImageDatas() const
    {
        return vecImgDatas;
    }

private:
    std::vector<ImageData> vecImgDatas;
};

//////////////////////////////////////////////////
// ImageRender Class
class ImageRender
{
public:
    ImageRender()
    {
        // Vertex Shader
        const char* vertexShaderSource = R"(
            #version 330 core
            layout (location = 0) in vec3 aPos;
            layout (location = 1) in vec2 aTexCoord;

            out vec2 TexCoord;

            void main() {
                gl_Position = vec4(aPos, 1.0);
                TexCoord = aTexCoord;
            }
        )";

        // Fragment Shader
        const char* fragmentShaderSource = R"(
            #version 330 core
            out vec4 FragColor;
            in vec2 TexCoord;

            uniform sampler2D texture1;

            void main() {
                FragColor = texture(texture1, TexCoord);
            }
        )";

        // Compile shaders
        unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
        glCompileShader(vertexShader);

        unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
        glCompileShader(fragmentShader);

        // Link shader program
        shaderProgram = glCreateProgram();
        glAttachShader(shaderProgram, vertexShader);
        glAttachShader(shaderProgram, fragmentShader);
        glLinkProgram(shaderProgram);

        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);

        // Set up vertex data and buffers
        float vertices[] = {
            // positions        // texture coords
             0.5f,  0.5f, 0.0f, 1.0f, 1.0f, // top right
             0.5f, -0.5f, 0.0f, 1.0f, 0.0f, // bottom right
            -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, // bottom left
            -0.5f,  0.5f, 0.0f, 0.0f, 1.0f  // top left
        };

        unsigned int indices[] = {
            0, 1, 3,
            1, 2, 3
        };

        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);

        glBindVertexArray(0);
    }

    ~ImageRender()
    {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &EBO);
        glDeleteProgram(shaderProgram);

        for (unsigned int texture : vecTextures)
            glDeleteTextures(1, &texture);
    }

    void loadImages(const ImageDatas& imageDatas)
    {
        vecTextures.clear();
        for (const auto& imgData : imageDatas.getImageDatas())
        {
            unsigned int texture;
            glGenTextures(1, &texture);
            glBindTexture(GL_TEXTURE_2D, texture);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            GLenum format = imgData.channels == 3 ? GL_RGB : GL_RGBA;
            glTexImage2D(GL_TEXTURE_2D, 0, format, imgData.width, imgData.height, 0, format, GL_UNSIGNED_BYTE, imgData.data);
            glGenerateMipmap(GL_TEXTURE_2D);

            vecTextures.push_back(texture);
        }
    }

    void render()
    {
        glUseProgram(shaderProgram);
        glBindVertexArray(VAO);

        for (unsigned int texture : vecTextures)
        {
            glBindTexture(GL_TEXTURE_2D, texture);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        }

        glBindVertexArray(0);
    }

private:
    unsigned int VBO, VAO, EBO;
    unsigned int shaderProgram;
    std::vector<unsigned int> vecTextures;
};

//////////////////////////////////////////////////
// Main function
int main()
{
    // Initialize GLFW
    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create window
    GLFWwindow* window = glfwCreateWindow(1280, 720, "OpenGL Image Renderer", NULL, NULL);
    if (!window)
    {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    // Initialize GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        glfwTerminate();
        return -1;
    }
    
    // 输出 OpenGL 信息
    {
        std::cout << "=== OpenGL Information ===" << std::endl;
        std::cout << "Version: " << glGetString(GL_VERSION) << std::endl;
        std::cout << "Vendor: " << glGetString(GL_VENDOR) << std::endl;
        std::cout << "Renderer: " << glGetString(GL_RENDERER) << std::endl;
        std::cout << "Shading Language Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
        std::cout << "===================" << std::endl;
    }

    glViewport(0, 0, 800, 600);

    try
    {
        // Load images
        Image image1("D:/xx/Pictures/34.png");
        Image image2("D:/xx/Pictures/40w.jpg");

        ImageDatas imageDatas;
        imageDatas.addImage(image2);
        imageDatas.addImage(image1);

        // Initialize renderer
        ImageRender renderer;
        renderer.loadImages(imageDatas);

        // Render loop
        while (!glfwWindowShouldClose(window))
        {
            glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            renderer.render();

            glfwSwapBuffers(window);
            glfwPollEvents();
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }

    glfwTerminate();
    return 0;
}