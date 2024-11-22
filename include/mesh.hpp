#ifndef MESH_HPP
#define MESH_HPP

#include<GL/glew.h>
#include<iostream>
#include<string>
#include<fstream>
#include<vector>
#include<unordered_map>

#define STB_IMAGE_IMPLEMENTATION //This must happen only once.
#include"stb_image.h"



class meshvf
{
private:
    unsigned int vao, vbo, ebo; //Vertex array object, vertex buffer object, element (index) buffer object.
    std::vector<float> verts; //Mesh's vertices {x1,y1,z1, x2,y2,z2, ...}.
    std::vector<unsigned int> inds; //Mesh's indices {vi1,vi2,vi3, vi4,vi5,vi6, ...}.

public:
    //Load the obj file, construct the mesh vectors and do the gpu memory setup.
    meshvf(const char *obj_path)
    {
        std::ifstream fp;
        fp.open(obj_path);
        if (!fp.is_open())
        {
            fprintf(stderr, "Error : File '%s' was not found. Exiting...\n", obj_path);
            exit(EXIT_FAILURE);
        }

        float x,y,z;
        unsigned int vi1,vi2,vi3;
        std::string line;
        while (getline(fp, line))
        {
            if (line[0] == 'v' && line[1] == ' ') //Then we have a vertex line.
            {
                sscanf(line.c_str(), "v %f %f %f", &x,&y,&z);
                verts.push_back(x);
                verts.push_back(y);
                verts.push_back(z);
            }
            else if (line[0] == 'f') //Then we have an index line.
            {
                sscanf(line.c_str(), "f %u %u %u", &vi1,&vi2,&vi3); //This must be the faces format of the obj file, so that everything works.
                //Append indices to inds and subtract 1 from each, to convert to 0-based indexing. Obj files are 1-based.
                inds.push_back(vi1-1);
                inds.push_back(vi2-1);
                inds.push_back(vi3-1);
            }
        }

        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, verts.size()*sizeof(float), &verts[0], GL_STATIC_DRAW);

        glGenBuffers(1, &ebo); //OpenGL expects the indices stored in the ebo to reference positions in the verts[] buffer.
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, inds.size()*sizeof(unsigned int), &inds[0], GL_STATIC_DRAW);
        
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        
        glBindVertexArray(0);
    }

    //Cleanup memory.
    ~meshvf()
    {
        glDeleteVertexArrays(1, &vao);
        glDeleteBuffers(1, &vbo);
        glDeleteBuffers(1, &ebo);
    }

    //Draw the mesh in the form of individual triangles (filled).
    void draw_triangles()
    {
        glBindVertexArray(vao); //Bind the mesh's vao.
        glDrawElements(GL_TRIANGLES, (int)inds.size(), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0); //Unbind the vao.
    }

    //Draw the mesh in the form of individual lines (wireframe).
    void draw_lines(const float line_width = 1.0f)
    {
        glBindVertexArray(vao);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); //Switch to line mode for wireframe/edge only drawing.
        glLineWidth(line_width);
        glDrawElements(GL_TRIANGLES, (int)inds.size(), GL_UNSIGNED_INT, 0);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); //Restore fill mode.
        glBindVertexArray(0);
    }

    //Draw the mesh in the form of individual points (vertices).
    void draw_points(const float point_size = 2.0f)
    {
        glBindVertexArray(vao);
        glPointSize(point_size);
        glDrawElements(GL_POINTS, (int)inds.size(), GL_UNSIGNED_INT, 0); //Point mode.
        glBindVertexArray(0);
    }
};



class meshvfn
{
private:
    unsigned int vao, vbo, ebo; //Vertex array object, vertex buffer object, element (index) buffer object.
    std::vector<std::vector<float>> verts; //Mesh's vertices {{x1,y1,z1}, {x2,y2,z2}, ...}.
    std::vector<std::vector<float>> norms; //Mesh's normals {{nx1,ny1,nz1}, {nx2,ny2,nz2}, ...}.
    std::vector<unsigned int> inds; //Mesh's indices. Every index is used to reference BOTH vertex and normal attributes.
    std::vector<float> interleaved_buffer; //Interleaved buffer that contains vertex and normal coordinates as pairs {x1,y1,z1, nx1,ny1,nz1, x2,y2,z2, nx2,ny2,nz2, ...}.

    void process_inds_and_push_back(unsigned int vindex, unsigned int nindex, std::unordered_map<std::string, unsigned int> &combo_map)
    {
        //Create a key for the current vertex-normal pair.
        std::string key = std::to_string(vindex) + "//" + std::to_string(nindex);
        if (combo_map.find(key) == combo_map.end())
        {
            //This is a new vertex-normal combination, so store it.
            interleaved_buffer.push_back(verts[vindex][0]);
            interleaved_buffer.push_back(verts[vindex][1]);
            interleaved_buffer.push_back(verts[vindex][2]);

            interleaved_buffer.push_back(norms[nindex][0]);
            interleaved_buffer.push_back(norms[nindex][1]);
            interleaved_buffer.push_back(norms[nindex][2]);

            //Assign a new index for this unique vertex-normal combo.
            unsigned int new_index = (unsigned int)(interleaved_buffer.size()/6 - 1);
            combo_map[key] = new_index;
            inds.push_back(new_index);
        }
        else
        {
            //This vertex-normal pair already exists, so use its existing index.
            inds.push_back(combo_map[key]);
        }
    }

public:
    //Load the obj file, construct the mesh vectors and do the gpu memory setup.
    meshvfn(const char *obj_path)
    {
        std::ifstream fp;
        fp.open(obj_path);
        if (!fp.is_open())
        {
            fprintf(stderr, "Error : File '%s' was not found. Exiting...\n", obj_path);
            exit(EXIT_FAILURE);
        }

        std::unordered_map<std::string, unsigned int> combo_map; //Map to store unique vertex-normal pairs. Let's call them combos.
        float x,y,z, nx,ny,nz;
        unsigned int vi1,ni1, vi2,ni2, vi3,ni3;
        std::string line;
        while (getline(fp, line))
        {
            if (line[0] == 'v' && line[1] == ' ')
            {
                sscanf(line.c_str(), "v %f %f %f", &x,&y,&z);
                verts.push_back({x,y,z});
            }
            else if (line[0] == 'v' && line[1] == 'n')
            {
                sscanf(line.c_str(), "vn %f %f %f", &nx,&ny,&nz);
                norms.push_back({nx,ny,nz});
            }
            else if (line[0] == 'f')
            {
                sscanf(line.c_str(), "f %u//%u %u//%u %u//%u", &vi1,&ni1, &vi2,&ni2, &vi3,&ni3); //This must be the faces format of the obj file, so that everything works.
                process_inds_and_push_back(vi1-1, ni1-1, combo_map);
                process_inds_and_push_back(vi2-1, ni2-1, combo_map);
                process_inds_and_push_back(vi3-1, ni3-1, combo_map);
            }
        }

        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, interleaved_buffer.size()*sizeof(float), &interleaved_buffer[0], GL_STATIC_DRAW);
        
        glGenBuffers(1, &ebo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, inds.size()*sizeof(unsigned int), &inds[0], GL_STATIC_DRAW);
        
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*)0); //For vertices.
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*)(3*sizeof(float)));  //For normals.
        glEnableVertexAttribArray(1);
        
        glBindVertexArray(0);
    }

    //Free resources.
    ~meshvfn()
    {
        glDeleteVertexArrays(1, &vao);
        glDeleteBuffers(1, &vbo);
        glDeleteBuffers(1, &ebo);
    }

    void draw_triangles()
    {
        //Remember : glDrawElements() uses 1 index to reference all attributes like positions, normals, UVs, etc...
        glBindVertexArray(vao);
        glDrawElements(GL_TRIANGLES, (int)inds.size(), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }

    //Farthest vertex distance with respect to the local coordinate system.
    float get_farthest_vertex_distance()
    {
        float farthest = std::sqrt(verts[0][0]*verts[0][0] + verts[0][1]*verts[0][1] + verts[0][2]*verts[0][2]); //Assume that the farthest vertex distance is the first one.
        for (size_t i = 1; i < verts.size(); ++i)
        {
            float dist = std::sqrt(verts[i][0]*verts[i][0] + verts[i][1]*verts[i][1] + verts[i][2]*verts[i][2]);
            if (dist > farthest)
                farthest = dist;
        }
        return farthest;
    }

    //Nearest vertex distance with respect to the local coordinate system.
    float get_nearest_vertex_distance()
    {
        float nearest = std::sqrt(verts[0][0]*verts[0][0] + verts[0][1]*verts[0][1] + verts[0][2]*verts[0][2]); //Assume that the nearest vertex distance is the first one.
        for (size_t i = 1; i < verts.size(); ++i)
        {
            float dist = std::sqrt(verts[i][0]*verts[i][0] + verts[i][1]*verts[i][1] + verts[i][2]*verts[i][2]);
            if (dist < nearest)
                nearest = dist;
        }
        return nearest;
    }
};



class meshvft
{
private:
    unsigned int vao, vbo, ebo, tex; //Vertex array object, vertex buffer object, element (index) buffer object and texture ID.
    std::vector<std::vector<float>> verts; //Mesh's vertices {{x1,y1,z1}, {x2,y2,z2}, ...}.
    std::vector<std::vector<float>> uvs; //Mesh's texture coords (u,v) {{u1,v1}, {u2,v2}, ...}.
    std::vector<unsigned int> inds; //Mesh's indices. Every index is used to reference BOTH vertex and uv attributes.
    std::vector<float> interleaved_buffer; //Interleaved buffer that contains vertex and uv coordinates as pairs {x1,y1,z1, u1,v1, x2,y2,z2, u2,v2, ...}.

    void process_inds_and_push_back(unsigned int vindex, unsigned int tindex, std::unordered_map<std::string, unsigned int> &combo_map)
    {
        //Create a key for the current vertex-uv pair.
        std::string key = std::to_string(vindex) + "/" + std::to_string(tindex);
        if (combo_map.find(key) == combo_map.end())
        {
            //This is a new vertex-normal combination, so store it.
            interleaved_buffer.push_back(verts[vindex][0]);
            interleaved_buffer.push_back(verts[vindex][1]);
            interleaved_buffer.push_back(verts[vindex][2]);

            interleaved_buffer.push_back(uvs[tindex][0]);
            interleaved_buffer.push_back(uvs[tindex][1]);

            //Assign a new index for this unique vertex-normal combo.
            unsigned int new_index = (unsigned int)(interleaved_buffer.size()/5 - 1);
            combo_map[key] = new_index;
            inds.push_back(new_index);
        }
        else
        {
            //This vertex-uv pair already exists, so use its existing index.
            inds.push_back(combo_map[key]);
        }
    }

public:
    //Load the obj file, construct the mesh vectors and do the gpu memory setup regarding both the mesh data and the image attached to the mesh.
    meshvft(const char *obj_path, const char *img_path)
    {
        std::ifstream fp;
        fp.open(obj_path);
        if (!fp.is_open())
        {
            fprintf(stderr, "Error : File '%s' was not found. Exiting...\n", obj_path);
            exit(EXIT_FAILURE);
        }

        std::unordered_map<std::string, unsigned int> combo_map; //Map to store unique vertex-uv pairs. Let's call them combos.
        float x,y,z, u,v;
        unsigned int vi1,ti1, vi2,ti2, vi3,ti3;
        std::string line;
        while (getline(fp, line))
        {
            if (line[0] == 'v' && line[1] == ' ')
            {
                sscanf(line.c_str(), "v %f %f %f", &x,&y,&z);
                verts.push_back({x,y,z});
            }
            else if (line[0] == 'v' && line[1] == 't')
            {
                sscanf(line.c_str(), "vt %f %f", &u,&v);
                uvs.push_back({u,v});
            }
            else if (line[0] == 'f')
            {
                sscanf(line.c_str(), "f %u/%u %u/%u %u/%u", &vi1,&ti1, &vi2,&ti2, &vi3,&ti3); //This must be the faces format of the obj file, so that everything works.
                process_inds_and_push_back(vi1-1, ti1-1, combo_map);
                process_inds_and_push_back(vi2-1, ti2-1, combo_map);
                process_inds_and_push_back(vi3-1, ti3-1, combo_map);
            }
        }

        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, interleaved_buffer.size()*sizeof(float), &interleaved_buffer[0], GL_STATIC_DRAW);

        glGenBuffers(1, &ebo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, inds.size()*sizeof(unsigned int), &inds[0], GL_STATIC_DRAW);
        
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void*)0); //For vertices.
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void*)(3*sizeof(float))); //For uvs.
        glEnableVertexAttribArray(1);
        glBindVertexArray(0);

        //Tell OpenGL how to apply the texture on the mesh.
        glGenTextures(1, &tex);
        glBindTexture(GL_TEXTURE_2D, tex);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1); //This is useful for textures with non-standard widths or single-channel textures.

        //Load the image texture.
        int img_width, img_height, img_channels;
        stbi_set_flip_vertically_on_load(true);
        unsigned char *img_data = stbi_load(img_path, &img_width, &img_height, &img_channels, 0);
        if (!img_data)
        {
            fprintf(stderr, "Error : File '%s' was not found. Exiting...\n", img_path);
            exit(EXIT_FAILURE);
        }

        //Determine the correct format based on the number of channels (img_channels).
        GLenum format;
        if (img_channels == 1)
            format = GL_RED; //Single-channel (grayscale image).
        else if (img_channels == 3)
            format = GL_RGB; //Classical 3-channel image (e.g. jpg).
        else if (img_channels == 4)
            format = GL_RGBA; //4-channel image, i.e. RGB + alpha channel for opacity (e.g. png).

        glTexImage2D(GL_TEXTURE_2D, 0, format, img_width, img_height, 0, format, GL_UNSIGNED_BYTE, img_data);
        glGenerateMipmap(GL_TEXTURE_2D);
        stbi_image_free(img_data); //Free image resources.
    }

    //Free resources.
    ~meshvft()
    {
        glDeleteVertexArrays(1, &vao);
        glDeleteBuffers(1, &vbo);
        glDeleteBuffers(1, &ebo);
        glDeleteTextures(1, &tex);
    }

    void draw_triangles()
    {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, tex);
        glBindVertexArray(vao);
        glDrawElements(GL_TRIANGLES, (int)inds.size(), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
};



class skybox
{
private:
    unsigned int vao, vbo, ebo, tex; //Vertex array object, vertex buffer object, element (index) buffer object and texture ID.

public:
    //Construct the mesh procedurally (i.e. no geometry data like vertices or uvs are read from a file), setup the mesh in the gpu memory, load the 6 images and tell how to wrap them.
    //Note : Make sure that all 6 images have the same size in pixels (e.g. 2048x2048, 500x500, etc...) AND the same type of extensions (e.g. jpg, png, bmp, ...).
    skybox(const char *right_img_path, const char *left_img_path, const char *top_img_path, const char *bottom_img_path, const char *front_img_path, const char *back_img_path)
    {   
        //Cube vertices. This is basically the interleaved buffer itself.
        float verts[] = { -1.0f, -1.0f,  1.0f,
                           1.0f, -1.0f,  1.0f,
                           1.0f, -1.0f, -1.0f,
                          -1.0f, -1.0f, -1.0f,
                          -1.0f,  1.0f,  1.0f,
                           1.0f,  1.0f,  1.0f,
                           1.0f,  1.0f, -1.0f,
                          -1.0f,  1.0f, -1.0f };

        //Cube indices.
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
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(verts), &verts, GL_STATIC_DRAW);

        glGenBuffers(1, &ebo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(inds), &inds, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glBindVertexArray(0);

        //Create the skybox's texture.
        glGenTextures(1, &tex);
        glBindTexture(GL_TEXTURE_CUBE_MAP, tex);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1); //This is useful for textures with non-standard widths or single-channel textures (e.g. grayscale).
        //glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

        //Skybox's expected image names. Do not change their order!
        std::string paths[6] = { right_img_path, left_img_path, top_img_path, bottom_img_path, front_img_path, back_img_path };
        int img_widths[6], img_heights[6], img_channels[6];

        stbi_set_flip_vertically_on_load(false);
        for (int i = 0; i < 6; i++)
        {
            unsigned char *data = stbi_load(paths[i].c_str(), &img_widths[i], &img_heights[i], &img_channels[i], 0);
            if (!data)
                fprintf(stderr, "Error : Failed to load texture '%s'\n", paths[i].c_str());

            //Determine the correct format for glTexImage2D based on the number of channels (img_channels).
            GLenum format;
            if (img_channels[i] == 1)
                format = GL_RED; //Single-channel grayscale image.
            else if (img_channels[i] == 3)
                format = GL_RGB; //Classical 3-channel image (e.g. jpg).
            else if (img_channels[i] == 4)
                format = GL_RGBA; //4-channel image, i.e. RGB + alpha channel for opacity (e.g. png).

            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, format, img_widths[i], img_heights[i], 0, format, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }

        //Check if all images have the same width, height, and channels. Otherwise the skybox may not render.
        bool img_consistency = true;
        for (int i = 1; i < 6; i++)
        {
            if (img_widths[i] != img_widths[0] || img_heights[i] != img_heights[0] || img_channels[i] != img_channels[0])
            {
                img_consistency = false;
                break;
            }
        }
        if (!img_consistency)
            fprintf(stderr, "Error : All 6 images must have the same width, height, and channels.\n");

    }

    //Delete the skybox's resources.
    ~skybox()
    {
        glDeleteVertexArrays(1, &vao);
        glDeleteBuffers(1, &ebo);
        glDeleteBuffers(1, &vbo);
        glDeleteTextures(1, &tex);
    }

    //Draw the skybox.
    void draw_triangles()
    {
        glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, tex);
        glBindVertexArray(vao);
        glDepthFunc(GL_LEQUAL); //Ensures that the skybox fragments will render behind everything else. (A bit dangerous to place it here. Be cautious.)
		glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
        glDepthFunc(GL_LESS); //Restore the default depth test function for rendering the rest of the scene.
		glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    }
};



class quadtex
{
private:
    unsigned int vao, vbo;

public:
    quadtex()
    {
        //Procedural quad.
        float interleaved_buffer[] = {  //Positions.         //UVs.
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
        glBufferData(GL_ARRAY_BUFFER, sizeof(interleaved_buffer), &interleaved_buffer, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void*)0); //Positions.
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void*)(3*sizeof(float))); //UVs.
        glEnableVertexAttribArray(1);
        glBindVertexArray(0);
    }

    //Delete the quadtex mesh.
    ~quadtex()
    {
        glDeleteVertexArrays(1, &vao);
        glDeleteBuffers(1, &vbo);
    }

    //Draw the quadtex mesh (2 triangles).
    void draw_triangles(unsigned int fbo_tex)
    {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, fbo_tex);
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
};

#endif
