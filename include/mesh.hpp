#ifndef MESH_HPP
#define MESH_HPP

#include<GL/glew.h>
#include<iostream>
#include<string>
#include<fstream>
#include<vector>
#define STB_IMAGE_IMPLEMENTATION
#include"stb_image.h"

class meshvf
{
public:
    unsigned int vao, vbo; //Vertex array and buffer object.
    std::vector<float> main_buffer; //Final form of the geometry data to draw.

    meshvf(const char *objpath)
    {
        std::ifstream fp;
        fp.open(objpath);
        if (!fp.is_open())
        {
            printf("'%s' not found. Exiting...\n", objpath);
            exit(EXIT_FAILURE);
        }

        //Vertices (format : v x y z).
        std::vector<std::vector<float>> verts;
        float x,y,z;

        //Indices (format : f i1 i2 i3).
        std::vector<std::vector<unsigned int>> inds;
        unsigned int i1,i2,i3;

        std::string line;
        while (getline(fp, line))
        {
            if (line[0] == 'v' && line[1] == ' ') //Then we have a vertex line.
            {
                const char *tmp_line = line.c_str();
                sscanf(tmp_line, "v %f %f %f", &x,&y,&z);
                verts.push_back({x,y,z});
            }
            else if (line[0] == 'f') //Then we have an index line.
            {
                const char *tmp_line = line.c_str();
                sscanf(tmp_line, "f %d %d %d", &i1,&i2,&i3);
                inds.push_back({i1-1, i2-1, i3-1}); //Subtract 1 to convert to 0-based indexing.
            }
        }

        //Now combine verts[][] and inds[][] to construct the main_buffer[]
        //which will have all the data needed for drawing.
        for (int i = 0; i < inds.size(); ++i)
        {
            main_buffer.push_back( verts[ inds[i][0] ][0] );
            main_buffer.push_back( verts[ inds[i][0] ][1] );
            main_buffer.push_back( verts[ inds[i][0] ][2] );

            main_buffer.push_back( verts[ inds[i][1] ][0] );
            main_buffer.push_back( verts[ inds[i][1] ][1] );
            main_buffer.push_back( verts[ inds[i][1] ][2] );

            main_buffer.push_back( verts[ inds[i][2] ][0] );
            main_buffer.push_back( verts[ inds[i][2] ][1] );
            main_buffer.push_back( verts[ inds[i][2] ][2] );
        }
        //Now main_buffer[] has the form : {x1,y1,z1, x2,y2,z2, x3,y3,z3, ... }, where
        //consecutive triads of vertices form triangles-faces of the object to be rendered.

        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, main_buffer.size()*sizeof(float), &main_buffer[0], GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glBindVertexArray(0); //Unbind the vao.
    }

    //Delete the mesh.
    ~meshvf()
    {
        glDeleteVertexArrays(1, &vao);
        glDeleteBuffers(1, &vbo);
    }

    //Draw the mesh (triangles).
    void draw_triangles()
    {
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, (int)(main_buffer.size()/3));
        glBindVertexArray(0); //Unbind the vao.
    }
};

class meshvfn
{
public:
    unsigned int vao, vbo; //Vertex array and buffer object.
    std::vector<float> main_buffer; //Final form of the geometry data to draw.

    meshvfn(const char *objpath)
    {
        std::ifstream fp;
        fp.open(objpath);
        if (!fp.is_open())
        {
            printf("'%s' not found. Exiting...\n", objpath);
            exit(EXIT_FAILURE);
        }

        //Vertices (format : v x y z).
        std::vector<std::vector<float>> verts;
        float x,y,z;

        //Normals (format : vn nx ny nz).
        std::vector<std::vector<float>> norms;
        float nx,ny,nz;

        //Indices (format : f i11//i12 i21//i22 i31//i32).
        std::vector<std::vector<unsigned int>> inds;
        unsigned int i11,i12, i21,i22, i31,i32;

        std::string line;
        while (getline(fp, line))
        {
            if (line[0] == 'v' && line[1] == ' ') //Then we have a vertex line.
            {
                const char *tmp_line = line.c_str();
                sscanf(tmp_line, "v %f %f %f", &x,&y,&z);
                verts.push_back({x,y,z});
            }
            else if (line[0] == 'v' && line[1] == 'n') //Then we have a normal line.
            {
                const char *tmp_line = line.c_str();
                sscanf(tmp_line, "vn %f %f %f", &nx,&ny,&nz);
                norms.push_back({nx,ny,nz});
            }
            else if (line[0] == 'f') //Then we have an index line.
            {
                const char *tmp_line = line.c_str();
                sscanf(tmp_line, "f %d//%d %d//%d %d//%d", &i11,&i12, &i21,&i22, &i31,&i32);
                inds.push_back({i11-1, i12-1, i21-1, i22-1, i31-1, i32-1});
            }
        }

        //Now combine verts[][], norms[][] and inds[][] to construct the main buffer main_buffer[],
        //which will have everything needed in the correct order for rendering.
        for (int i = 0; i < inds.size(); ++i)
        {
            main_buffer.push_back( verts[ inds[i][0] ][0] );
            main_buffer.push_back( verts[ inds[i][0] ][1] );
            main_buffer.push_back( verts[ inds[i][0] ][2] );
            main_buffer.push_back( norms[ inds[i][1] ][0] );
            main_buffer.push_back( norms[ inds[i][1] ][1] );
            main_buffer.push_back( norms[ inds[i][1] ][2] );

            main_buffer.push_back( verts[ inds[i][2] ][0] );
            main_buffer.push_back( verts[ inds[i][2] ][1] );
            main_buffer.push_back( verts[ inds[i][2] ][2] );
            main_buffer.push_back( norms[ inds[i][3] ][0] );
            main_buffer.push_back( norms[ inds[i][3] ][1] );
            main_buffer.push_back( norms[ inds[i][3] ][2] );

            main_buffer.push_back( verts[ inds[i][4] ][0] );
            main_buffer.push_back( verts[ inds[i][4] ][1] );
            main_buffer.push_back( verts[ inds[i][4] ][2] );
            main_buffer.push_back( norms[ inds[i][5] ][0] );
            main_buffer.push_back( norms[ inds[i][5] ][1] );
            main_buffer.push_back( norms[ inds[i][5] ][2] );
        }
        //main_buffer[] has now the form : {x1,y1,z1, nx1,ny1,nz1, x2,y2,z2, nx2,ny2,nz2 ... }.

        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, main_buffer.size()*sizeof(float), &main_buffer[0], GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*)(3*sizeof(float)));
        glEnableVertexAttribArray(1);
        glBindVertexArray(0); //Unbind the vao.
    }

    //Delete the mesh.
    ~meshvfn()
    {
        glDeleteVertexArrays(1, &vao);
        glDeleteBuffers(1, &vbo);
    }

    //Draw the mesh (triangles).
    void draw_triangles()
    {
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, (int)(main_buffer.size()/6));
        glBindVertexArray(0); //Unbind the vao.
    }
};

class meshvft
{
public:
    unsigned int vao, vbo, tao; //Vertex array, buffer and texture 'array object' (there's no tao officialy, but the name merges well with vao and vbo).
    std::vector<float> main_buffer; //Final form of the geometry data to draw.

    meshvft(const char *objpath, const char *imgpath)
    {
        std::ifstream fp;
        fp.open(objpath);
        if (!fp.is_open())
        {
            printf("'%s' not found. Exiting...\n", objpath);
            exit(EXIT_FAILURE);
        }

        //Vertices (format : v x y z)
        std::vector<std::vector<float>> verts;
        float x,y,z;

        //Texture (format : vt u v).
        std::vector<std::vector<float>> uvs;
        float u,v;

        //Indices (format : f i11/i12 i21/i22 i31/i32.
        std::vector<std::vector<unsigned int>> inds;
        unsigned int i11,i12, i21,i22, i31,i32;

        std::string line;
        while (getline(fp, line))
        {
            if (line[0] == 'v' && line[1] == ' ') //Then we have a vertex line.
            {
                const char *tmp_line = line.c_str();
                sscanf(tmp_line, "v %f %f %f", &x,&y,&z);
                verts.push_back({x,y,z});
            }
            else if (line[0] == 'v' && line[1] == 't') //Then we have a texture coordinate (uv) line.
            {
                const char *tmp_line = line.c_str();
                sscanf(tmp_line, "vt %f %f", &u,&v);
                uvs.push_back({u,v});
            }
            else if (line[0] == 'f') //Then we have a index line.
            {
                const char *tmp_line = line.c_str();
                sscanf(tmp_line, "f %d/%d %d/%d %d/%d", &i11,&i12, &i21,&i22, &i31,&i32);
                inds.push_back({i11-1, i12-1, i21-1, i22-1, i31-1, i32-1});
            }
        }

        //Now combine verts[][], uvs[][] and inds[][] to construct the main buffer main_buffer[]
        //which will have all the main_buffer needed for drawing.
        for (int i = 0; i < inds.size(); ++i)
        {
            main_buffer.push_back( verts[ inds[i][0] ][0] );
            main_buffer.push_back( verts[ inds[i][0] ][1] );
            main_buffer.push_back( verts[ inds[i][0] ][2] );
            main_buffer.push_back(   uvs[ inds[i][1] ][0] );
            main_buffer.push_back(   uvs[ inds[i][1] ][1] );

            main_buffer.push_back( verts[ inds[i][2] ][0] );
            main_buffer.push_back( verts[ inds[i][2] ][1] );
            main_buffer.push_back( verts[ inds[i][2] ][2] );
            main_buffer.push_back(   uvs[ inds[i][3] ][0] );
            main_buffer.push_back(   uvs[ inds[i][3] ][1] );

            main_buffer.push_back( verts[ inds[i][4] ][0] );
            main_buffer.push_back( verts[ inds[i][4] ][1] );
            main_buffer.push_back( verts[ inds[i][4] ][2] );
            main_buffer.push_back(   uvs[ inds[i][5] ][0] );
            main_buffer.push_back(   uvs[ inds[i][5] ][1] );
        }
        //main_buffer[] has now the form : {x1,y1,z1, u1,v1, x2,y2,z2, u2,v2 ... }

        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, main_buffer.size()*sizeof(float), &main_buffer[0], GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void*)(3*sizeof(float)));
        glEnableVertexAttribArray(1);
        glBindVertexArray(0); //Unbind the vao.

        //Load the image texture.
        int imgwidth, imgheight, imgchannels;
        stbi_set_flip_vertically_on_load(true);
        unsigned char *imgdata = stbi_load(imgpath, &imgwidth, &imgheight, &imgchannels, 0);
        if (!imgdata)
        {
            printf("'%s' not found. Exiting...\n", imgpath);
            exit(EXIT_FAILURE);
        }

        //Tell OpenGL how to apply the texture on the mesh.
        glGenTextures(1, &tao);
        glBindTexture(GL_TEXTURE_2D, tao);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1); //This is useful for textures with non-standard widths or single-channel textures (e.g. grayscale).

        //Determine the correct format for glTexImage2D based on the number of channels (imgchannels).
        GLenum format;
        if (imgchannels == 1)
            format = GL_RED; //Single-channel grayscale image.
        else if (imgchannels == 3)
            format = GL_RGB; //Classical 3-channel image (e.g. jpg).
        else if (imgchannels == 4)
            format = GL_RGBA; //4-channel image, i.e. RGB + alpha channel for opacity (e.g. png).

        glTexImage2D(GL_TEXTURE_2D, 0, format, imgwidth, imgheight, 0, format, GL_UNSIGNED_BYTE, imgdata);
        glGenerateMipmap(GL_TEXTURE_2D);
        stbi_image_free(imgdata); //Free image resources.
    }

    //Delete the mesh.
    ~meshvft()
    {
        glDeleteVertexArrays(1, &vao);
        glDeleteBuffers(1, &vbo);
        glDeleteTextures(1, &tao);
    }

    //Draw the mesh (triangles).
    void draw_triangles()
    {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, tao);
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, (int)(main_buffer.size()/5));
        glBindVertexArray(0); //Unbind the vao.
        glBindTexture(GL_TEXTURE_2D, 0); //Unbind the tao.
    }

};

class skybox
{

private:
    unsigned int vao, vbo, ebo, tao;

public:
    skybox(const char *path_to_right, const char *path_to_left, const char *path_to_top, const char *path_to_bottom, const char *path_to_front, const char *path_to_back)
    {   
        float verts[] = { -1.0f, -1.0f,  1.0f,
                           1.0f, -1.0f,  1.0f,
                           1.0f, -1.0f, -1.0f,
                          -1.0f, -1.0f, -1.0f,
                          -1.0f,  1.0f,  1.0f,
                           1.0f,  1.0f,  1.0f,
                           1.0f,  1.0f, -1.0f,
                          -1.0f,  1.0f, -1.0f };
      
        unsigned int inds[] = { //Right.
                                1, 2, 6,
                                6, 5, 1,
                                //Left.
                                0, 4, 7,
                                7, 3, 0,
                                //Top.
                                4, 5, 6,
                                6, 7, 4,
                                //Bottom.
                                0, 3, 2,
                                2, 1, 0,
                                //Front.
                                3, 7, 6,
                                6, 2, 3, 
                                //Back.
                                0, 1, 5,
                                5, 4, 0 };

        //Setup skybox's data in the memory.
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
        glBindVertexArray(0);

        //Create the skybox's texture.
        glGenTextures(1, &tao);
        glBindTexture(GL_TEXTURE_CUBE_MAP, tao);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1); //This is useful for textures with non-standard widths or single-channel textures (e.g. grayscale).
        //glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

        //Skybox's expected image names. Do not change their order..
        std::string paths[6] = { path_to_right, path_to_left, path_to_top, path_to_bottom, path_to_front, path_to_back };

        stbi_set_flip_vertically_on_load(false);
        for (int i = 0; i < 6; i++)
        {
            int width, height, nchannels;
            unsigned char *data = stbi_load(paths[i].c_str(), &width, &height, &nchannels, 0);
            if (!data)
                printf("Error : Failed to load texture '%s'\n", paths[i].c_str());

            //Determine the correct format for glTexImage2D based on the number of channels (nchannels).
            GLenum format;
            if (nchannels == 1)
                format = GL_RED; //Single-channel grayscale image.
            else if (nchannels == 3)
                format = GL_RGB; //Classical 3-channel image (e.g. jpg).
            else if (nchannels == 4)
                format = GL_RGBA; //4-channel image, i.e. RGB + alpha channel for opacity (e.g. png).

            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
    }

    //Delete the skybox's resources.
    ~skybox()
    {
        glDeleteVertexArrays(1, &vao);
        glDeleteBuffers(1, &ebo);
        glDeleteBuffers(1, &vbo);
        glDeleteTextures(1, &tao);
    }

    //Draw the skybox cube mesh.
    void draw_elements()
    {
        glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, tao);
        glBindVertexArray(vao);
        glDepthFunc(GL_LEQUAL); //Ensures that the skybox fragments will render behind everything else.
		glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
        glDepthFunc(GL_LESS); //Restore the default depth test function for rendering the rest of the scene.
		glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    }
};

class quadtex
{
    
public:
    unsigned int vao, vbo;
    std::vector<float> main_buffer;

    quadtex()
    {
        //Hard-coded quad vertices for a screen-space quad.
        main_buffer = {  //Positions.        //UVs.
                        -1.0f,  1.0f, 0.0f,  0.0f, 1.0f,
                        -1.0f, -1.0f, 0.0f,  0.0f, 0.0f,
                         1.0f, -1.0f, 0.0f,  1.0f, 0.0f,

                        -1.0f,  1.0f, 0.0f,  0.0f, 1.0f,
                         1.0f, -1.0f, 0.0f,  1.0f, 0.0f,
                         1.0f,  1.0f, 0.0f,  1.0f, 1.0f };

        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, main_buffer.size()*sizeof(float), &main_buffer[0], GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void*)0); //Positions.
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void*)(3*sizeof(float))); //UVs.
        glEnableVertexAttribArray(1);
        glBindVertexArray(0); //Unbind the vao.
    }

    //Delete the quad-mesh.
    ~quadtex()
    {
        glDeleteVertexArrays(1, &vao);
        glDeleteBuffers(1, &vbo);
    }

    //Draw the quad-mesh (triangles).
    void draw_triangles(unsigned int framebuffer_tex)
    {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, framebuffer_tex);
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
};

#endif
