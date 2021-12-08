#include <glad/glad.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

GLuint load_shader_file(const char* path, GLenum shader_type)
{
    //Open file
    GLuint shaderID = 0;
    std::string shader_string;
    std::ifstream source_file;
    source_file.open(path);

    //Source file loaded
    if(source_file) {
        //Get shader source
        shader_string.assign((std::istreambuf_iterator<char>(source_file)),
                std::istreambuf_iterator<char>());
        //Create shader ID
        shaderID = glCreateShader( shader_type );

        //Set shader source
        const GLchar* shaderSource = shader_string.c_str();
        glShaderSource( shaderID, 1, (const GLchar**)&shaderSource, NULL );

        //Compile shader source
        glCompileShader( shaderID );

        //Check shader for errors
        GLint shader_compiled = GL_FALSE;
        glGetShaderiv( shaderID, GL_COMPILE_STATUS, &shader_compiled );
        if(shader_compiled != GL_TRUE) {
            printf( "Unable to compile shader %d!\n\nSource:\n%s\n", shaderID, shaderSource );

            GLint len;
            glGetShaderiv(shaderID, GL_INFO_LOG_LENGTH, &len);
            std::vector<char> error_msg(len+1);
            glGetShaderInfoLog(shaderID, len, NULL, &error_msg[0]);
            printf("%s\n", &error_msg[0]);

            glDeleteShader( shaderID );
            shaderID = 0;
        }
    }
    else {
        printf( "Unable to open file %s\n", path);
    }

    return shaderID;
}

GLuint load_shaders(const char* vert, const char* frag) {
    //Generate program
    GLuint program = glCreateProgram();

    //Load vertex shader
    GLuint vert_shader = load_shader_file(vert, GL_VERTEX_SHADER );

    //Check for errors
    if( vert_shader == 0 )
    {
        glDeleteProgram(program);
        program = 0;
        return false;
    }

    //Attach vertex shader to program
    glAttachShader(program, vert_shader );


    //Create fragment shader
    GLuint frag_shader = load_shader_file(frag, GL_FRAGMENT_SHADER );

    //Check for errors
    if(frag_shader == 0)
    {
        glDeleteShader( vert_shader );
        glDeleteProgram(program);
        program = 0;
        return false;
    }

    //Attach fragment shader to program
    glAttachShader( program, frag_shader );

    //Link program
    glLinkProgram( program );

    //Check for errors
    GLint programSuccess = GL_TRUE;
    glGetProgramiv( program, GL_LINK_STATUS, &programSuccess );
    if( programSuccess != GL_TRUE )
    {
        printf( "Error linking program %d!\n", program );
        glDeleteShader( vert_shader );
        glDeleteShader( frag_shader );
        glDeleteProgram( program );
        program = 0;
        return false;
    }

    //Clean up excess shader references
    glDeleteShader( vert_shader );
    glDeleteShader( frag_shader );

    return program;
}
