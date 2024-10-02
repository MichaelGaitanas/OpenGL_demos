#ifndef SKYBOX_HPP
#define SKYBOX_HPP

#include<GL/glew.h>
#include<iostream>
#include<string>

#include"stb_image.h"

class skybox
{

public:

    unsigned int vao, vbo, ebo, tao;

    skybox()
    {
        float verts[] = { -1.0f, -1.0f,  1.0f,
                           1.0f, -1.0f,  1.0f,
                           1.0f, -1.0f, -1.0f,
                          -1.0f, -1.0f, -1.0f,
                          -1.0f,  1.0f,  1.0f,
                           1.0f,  1.0f,  1.0f,
                           1.0f,  1.0f, -1.0f,
                          -1.0f,  1.0f, -1.0f };

        unsigned int inds[] = { //Right
                                1, 2, 6,
                                6, 5, 1,
                                //Left
                                0, 4, 7,
                                7, 3, 0,
                                //Top
                                4, 5, 6,
                                6, 7, 4,
                                //Bottom
                                0, 3, 2,
                                2, 1, 0,
                                //Back
                                0, 1, 5,
                                5, 4, 0,
                                //Front
                                3, 7, 6,
                                6, 2, 3  };

        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);
        glGenBuffers(1, &vbo);
        glGenBuffers(1, &ebo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(verts), &verts, GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(inds), &inds, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glBindVertexArray(0); //Unbind the vao.


        //Skybox faces. Make sure they are in this exact order.
        std::string path[6] = { "../images/skybox/starfield_4k/right.jpg",
                                "../images/skybox/starfield_4k/left.jpg",
                                "../images/skybox/starfield_4k/top.jpg",
                                "../images/skybox/starfield_4k/bottom.jpg",
                                "../images/skybox/starfield_4k/front.jpg",
                                "../images/skybox/starfield_4k/back.jpg" };
        
        //Create the skybox (cubemap) texture object.
        unsigned int tao;
        glGenTextures(1, &tao);
        glBindTexture(GL_TEXTURE_CUBE_MAP, tao);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        //glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        // This might help with seams on some systems
        //glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

        //Loop through all the textures and attach them to the tao object.
        for (int i = 0; i < 6; i++)
        {
            int width, height, nchannels;
            unsigned char *data = stbi_load(path[i].c_str(), &width, &height, &nchannels, 0);
            if (data)
            {
                stbi_set_flip_vertically_on_load(false);
                glTexImage2D ( GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                               0,
                               GL_RGB,
                               width,
                               height,
                               0,
                               GL_RGB,
                               GL_UNSIGNED_BYTE,
                               data );
                stbi_image_free(data);
            }
            else
            {
                printf("Failed to load texture '%s'\n", path[i]);
                stbi_image_free(data);
            }
        }
    }

    //Delete the skybox.
    ~skybox()
    {
        glDeleteVertexArrays(1, &vao);
        glDeleteBuffers(1, &ebo);
        glDeleteBuffers(1, &vbo);
        glDeleteTextures(1, &tao);
    }

    //Draw the skybox cube mesh.
    void draw()
    {
        glBindVertexArray(vao);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, tao);
		glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_2D, 0); //Unbind the tao.
    }
};

#endif