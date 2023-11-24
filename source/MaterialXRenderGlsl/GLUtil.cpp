//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#include <MaterialXRenderGlsl/External/Glad/glad.h>

#include <MaterialXRenderGlsl/GLUtil.h>

#include <iostream>
#include <iomanip>

MATERIALX_NAMESPACE_BEGIN

void checkGlErrors(const string& context)
{
    for (GLenum error = glGetError(); error; error = glGetError())
    {
		std::cerr << "OpenGL error " << context << ": 0x" << std::setfill ('0') << std::setw(4) << std::hex << error << std::endl;
    }
}

void checkGlErrors()
{
	for (GLenum error = glGetError(); error; error = glGetError())
	{
		std::cerr << "OpenGL error : 0x" << std::setfill ('0') << std::setw(4) << std::hex << error << std::endl;
	}
}

void checkGlFramebuffer()
{
	// Validate the framebuffer.
	GLint fboId = 0;
	glGetIntegerv( GL_FRAMEBUFFER_BINDING, &fboId);
	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE)
	{
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

		std::cerr << "Frame buffer object setup, " << fboId << ": " << errorMessage << std::endl;
	} else {
		std::cerr << "Frame buffer object setup, " << fboId << ": GL_FRAMEBUFFER_COMPLETE " << std::endl;
	}
}
MATERIALX_NAMESPACE_END
