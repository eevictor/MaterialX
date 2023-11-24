//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXRenderGlsl/GLFramebuffer.h>

#include <MaterialXRenderGlsl/GlslProgram.h>
#include <MaterialXRenderGlsl/GlslRenderer.h>
#include <MaterialXRenderGlsl/GLTextureHandler.h>

#include <MaterialXRenderGlsl/External/Glad/glad.h>
#include "GLUtil.h"

MATERIALX_NAMESPACE_BEGIN

//
// GLFramebuffer methods
//

GLFramebufferPtr GLFramebuffer::create(unsigned int width, unsigned int height, unsigned channelCount, Image::BaseType baseType)
{
    return GLFramebufferPtr(new GLFramebuffer(width, height, channelCount, baseType));
}

GLFramebuffer::GLFramebuffer(unsigned int width, unsigned int height, unsigned int channelCount, Image::BaseType baseType) :
    _width(width),
    _height(height),
    _channelCount(channelCount),
    _baseType(baseType),
    _encodeSrgb(false),
    _framebuffer(0),
    _colorTexture(0),
    _depthTexture(0), _depthrenderbuffer(0)
{
	if (!glGenFramebuffers)
	{
		gladLoadGL();
	}
	
	// Convert texture format to OpenGL.
	int glType, glFormat, glInternalFormat;
	//GLTextureHandler::mapTextureFormatToGL(baseType, channelCount, true, glType, glFormat, glInternalFormat);
	GLTextureHandler::mapTextureFormatToGL_forFramebuffer(baseType, channelCount, true, glType, glFormat, glInternalFormat);
	
	// Create and bind framebuffer.
	checkGlErrors("before glGenFramebuffers");
	glGenFramebuffers(1, &_framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, _framebuffer);

	// Create the offscreen color target and attach to the framebuffer.
	glGenTextures(1, &_colorTexture);
	glBindTexture(GL_TEXTURE_2D, _colorTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	checkGlErrors("before glTexImage2D");
#ifdef TARGET_OS_IOS
	// glType for texture used by framebuffer should be GL_UNSIGNED_BYTE?
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, _width, _height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	glTexImage2D(GL_TEXTURE_2D, 0, glInternalFormat, _width, _height, 0, glFormat, glType, nullptr);
#else
	glTexImage2D(GL_TEXTURE_2D, 0, glInternalFormat, _width, _height, 0, glFormat, glType, nullptr);
#endif
	checkGlErrors("after glTexImage2D");
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, _width, _height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_RG32F, _width, _height, 0, GL_RG, GL_FLOAT, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _colorTexture, 0);
	checkGlErrors("after glFramebufferTexture2D");
#ifdef TARGET_OS_IOS
	// The depth buffer
	// Create the offscreen depth target and attach to the framebuffer.
	glGenTextures(1, &_depthTexture);
	glBindTexture(GL_TEXTURE_2D, _depthTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, _width, _height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, 0);
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, _width, _height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
	checkGlErrors("after GL_DEPTH_COMPONENT");
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, _depthTexture, 0);
	checkGlErrors("after GL_DEPTH_ATTACHMENT");

	/*
		glGenRenderbuffers(1, &_depthrenderbuffer);
		glBindRenderbuffer(GL_RENDERBUFFER, _depthrenderbuffer);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8_OES, _width, _height);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, _depthrenderbuffer);
	*/
#else
	// Create the offscreen depth target and attach to the framebuffer.
	glGenTextures(1, &_depthTexture);
	glBindTexture(GL_TEXTURE_2D, _depthTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, _width, _height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
	checkGlErrors("after GL_DEPTH_COMPONENT24");
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, _depthTexture, 0);
	checkGlErrors("after GL_DEPTH_ATTACHMENT");
#endif
	
	glBindTexture(GL_TEXTURE_2D, GlslProgram::UNDEFINED_OPENGL_RESOURCE_ID);
#ifdef TARGET_OS_IOS
	//GLenum colorList[1] = { GL_FRONT };
	//glDrawBuffers(1, colorList);
	checkGlErrors("after glDrawBuffers");
#else
	glDrawBuffer(GL_NONE);
#endif
	
    // Validate the framebuffer.
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, GlslProgram::UNDEFINED_OPENGL_RESOURCE_ID);
        glDeleteFramebuffers(1, &_framebuffer);
        _framebuffer = GlslProgram::UNDEFINED_OPENGL_RESOURCE_ID;

        string errorMessage;
        switch (status)
        {
        case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
            errorMessage = "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT";
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
            errorMessage = "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT";
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
            errorMessage = "GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER";
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
            errorMessage = "GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER";
            break;
        case GL_FRAMEBUFFER_UNSUPPORTED:
            errorMessage = "GL_FRAMEBUFFER_UNSUPPORTED";
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
            errorMessage = "GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE";
            break;
        case GL_FRAMEBUFFER_UNDEFINED:
            errorMessage = "GL_FRAMEBUFFER_UNDEFINED";
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
            errorMessage = "GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS";
            break;
        default:
            errorMessage = std::to_string(status);
            break;
        }

        throw ExceptionRenderError("Frame buffer object setup failed: " + errorMessage);
    }

    // Unbind on cleanup
    glBindFramebuffer(GL_FRAMEBUFFER, GlslProgram::UNDEFINED_OPENGL_RESOURCE_ID);
	checkGlErrors("after glBindFramebuffer Unbind on cleanup");
}

GLFramebuffer::~GLFramebuffer()
{
    if (_framebuffer)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, GlslProgram::UNDEFINED_OPENGL_RESOURCE_ID);
        glDeleteTextures(1, &_colorTexture);
        if (_depthTexture) glDeleteTextures(1, &_depthTexture);
		if (_depthrenderbuffer) glDeleteRenderbuffers(1, &_depthrenderbuffer);
        glDeleteFramebuffers(1, &_framebuffer);
    }
}

// avoid changing existing fbo binding, if possible.
static signed int fboBeforeBind = GlslProgram::UNDEFINED_OPENGL_RESOURCE_ID;

void GLFramebuffer::bind()
{
    if (!_framebuffer)
    {
        throw ExceptionRenderError("No framebuffer exists to bind");
    }
	
	// avoid changing existing fbo binding, if possible.
	glGetIntegerv( GL_FRAMEBUFFER_BINDING, &fboBeforeBind);

    glBindFramebuffer(GL_FRAMEBUFFER, _framebuffer);
    GLenum colorList[1] = { GL_COLOR_ATTACHMENT0 };
    glDrawBuffers(1, colorList);

#ifdef TARGET_OS_IOS
	// not applicable for GLES
#else
    if (_encodeSrgb)
    {
        glEnable(GL_FRAMEBUFFER_SRGB);
    }
    else
    {
        glDisable(GL_FRAMEBUFFER_SRGB);
    }
#endif

    glViewport(0, 0, _width, _height);
}

void GLFramebuffer::unbind()
{
#ifdef TARGET_OS_IOS
	glBindFramebuffer(GL_FRAMEBUFFER, fboBeforeBind);
	GLenum colorList[1] = { GL_NONE };
	glDrawBuffers(1, colorList);
#else
	glBindFramebuffer(GL_FRAMEBUFFER, GlslProgram::UNDEFINED_OPENGL_RESOURCE_ID);
	glDrawBuffer(GL_NONE);
#endif
}

ImagePtr GLFramebuffer::getColorImage(ImagePtr image)
{
    int glType, glFormat, glInternalFormat;
    //GLTextureHandler::mapTextureFormatToGL(_baseType, _channelCount, false, glType, glFormat, glInternalFormat);
	GLTextureHandler::mapTextureFormatToGL_forFramebuffer(_baseType, _channelCount, false, glType, glFormat, glInternalFormat);

    bind();
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadBuffer(GL_COLOR_ATTACHMENT0);
#ifdef TARGET_OS_IOS
	//glReadPixels(0, 0, image->getWidth(), image->getHeight(), GL_RGBA, GL_UNSIGNED_BYTE, image->getResourceBuffer());
	{
		int fmt, type;
		glGetIntegerv(GL_IMPLEMENTATION_COLOR_READ_FORMAT, &fmt);
		glGetIntegerv(GL_IMPLEMENTATION_COLOR_READ_TYPE, &type);
		if ( type != glType && fmt == glFormat ) {
			glType = type;
		}
	}
	
	if (!image)
	{
		image = Image::create(_width, _height, _channelCount, _baseType);
		image->createResourceBuffer();
	}
	glReadPixels(0, 0, image->getWidth(), image->getHeight(), glFormat, glType, image->getResourceBuffer());
#else
    glReadPixels(0, 0, image->getWidth(), image->getHeight(), glFormat, glType, image->getResourceBuffer());
#endif
	{
		// debug
		// GL_INVALID_ENUM is generated if format or type is not an accepted value.
		std::vector<GLenum> errors;
		for (GLenum error = glGetError(); error; error = glGetError())
		{
			errors.push_back( error );
			if ( error == GL_INVALID_ENUM ) {
				std::cout << "GL error : GL_INVALID_ENUM format or type is not an accepted value. " << std::endl;
				printf("Requesting glReadPixels format/type, 0x%4x / 0x%4x \n", glFormat, glType);
				int fmt, type;
				glGetIntegerv(GL_IMPLEMENTATION_COLOR_READ_FORMAT, &fmt);
				glGetIntegerv(GL_IMPLEMENTATION_COLOR_READ_TYPE, &type);
				printf("Supported glReadPixels color format/type, 0x%4x / 0x%4x \n", fmt, type);
			} else {
				std::cout << "GL error : " << error << std::endl;
			}
		}
	}
	
	
    unbind();

    return image;
}

void GLFramebuffer::blit()
{
    glBindFramebuffer(GL_READ_FRAMEBUFFER, _framebuffer);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
#ifdef TARGET_OS_IOS
	GLenum colorList[1] = { GL_BACK };
	glDrawBuffers(1, colorList);
#else
	glDrawBuffer(GL_BACK);
#endif

    glBlitFramebuffer(0, 0, _width, _height, 0, 0, _width, _height,
                      GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, GL_NEAREST);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

MATERIALX_NAMESPACE_END
