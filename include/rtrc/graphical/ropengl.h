/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#pragma once
#include <stdio.h>
#include <cstdlib>
#include <vector>
#ifndef RTRC_GLAD_INCLUDED
#define RTRC_GLAD_INCLUDED
#include <glad/glad.h>
#include "../rexception.h"
#include <vector>
#include "../compiletime.h"
#include "../glextensionMgr.h"
#include "../rstring.h"
#include <glm/glm.hpp>
#endif

//#define gltexUnit(x) GL_TEXTURE80
namespace rtrc { namespace gll {

inline void checkError(const char *cstr)
{
	GLenum error = glGetError();
	if(error != GL_NO_ERROR)
	{
		printf("error %llu while %s\n", error, cstr);
		getchar();
		exit(-1);
	}
}

template<GLenum textype>
inline bool isTextureAllocatedCorrectly(GLsizei width, GLsizei height)
{
	if(width == 0 || height == 0) return true;
	GLint _width;
	GLint _height;
	glGetTexLevelParameteriv(GL_PROXY_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &_width);
	glGetTexLevelParameteriv(GL_PROXY_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &_height);
	if( _width == width && _height == height ) return true;
	return false;
}

struct DrawArraysIndirectCommand
{
	GLuint count;
	GLuint instanceCount;
	GLuint first;
	GLuint baseInstance;
};
	
static struct
{
	
	init()
	{
		if(inited) throw virtualConstManipulation("trying to re init rgl");
		
		glGetIntegerv(GL_MAX_TEXTURE_SIZE, &_max_texture_size);
		glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &_max_comb_texture_image_units);
		glGetIntegerv(GL_MAX_ARRAY_TEXTURE_LAYERS, &_max_array_texture_layers);
		glGetIntegerv(GL_MAX_TEXTURE_BUFFER_SIZE, &_max_texture_buffer_size);
		glGetIntegerv(GL_MAX_UNIFORM_BUFFER_BINDINGS, &_max_uniform_buffer_bindings);
		glGetIntegerv(GL_MAX_VERTEX_UNIFORM_BLOCKS, &_max_vertex_uniform_blocks);
		glGetIntegerv(GL_MAX_GEOMETRY_UNIFORM_BLOCKS, &_max_geometry_uniform_blocks);
		glGetIntegerv(GL_MAX_FRAGMENT_UNIFORM_BLOCKS, &_max_fragment_uniform_blocks);
		glGetIntegerv(GL_MAX_COMBINED_VERTEX_UNIFORM_COMPONENTS, &_max_combined_vertex_uniform_components);
		glGetIntegerv(GL_MAX_COMBINED_GEOMETRY_UNIFORM_COMPONENTS, &_max_combined_gometry_uniform_components);
		glGetIntegerv(GL_MAX_COMBINED_FRAGMENT_UNIFORM_COMPONENTS, &_max_combined_fragment_uniform_components);
		inited = true;
	}
	void checkInit() { if(!inited) throw usingUninitializedObject("initialize rgl before use");}
	void ActiveTexture(size_t index)
	{
		glActiveTexture(GL_TEXTURE0 + index);
	}
	GLint max_combined_texture_image_units() { return _max_comb_texture_image_units;}
	GLint max_texture_size() { return _max_texture_size;}
	GLint max_texture_buffer_size() { return _max_texture_buffer_size; }
	GLint max_array_texture_layers() { return _max_array_texture_layers; }
	GLint max_uniform_buffer_bindings() {return _max_uniform_buffer_bindings; }
	GLint max_vertex_uniform_blocks() { return _max_vertex_uniform_blocks;}
	GLint max_gometry_uniform_blocks() { return _max_geometry_uniform_blocks;}
	GLint max_fragment_uniform_blocks() { return _max_fragment_uniform_blocks;}
	GLint max_combined_vertex_uniform_components() { return _max_combined_vertex_uniform_components;}
	GLint max_combined_gometry_uniform_components() { return _max_combined_gometry_uniform_components; }
	GLint max_combined_fragmend_uniform_components() { return _max_combined_fragment_uniform_components;}
	
	
	bool isTexture2DSupported(GLint level, GLint internalFormat, GLsizei width, GLsizei height,
		GLint border, GLenum format, GLenum type)
	{
		glTexImage2D(GL_PROXY_TEXTURE_2D, level, internalFormat, width, height, 
			border, format, type, nullptr);
		return isTextureAllocatedCorrectly<GL_PROXY_TEXTURE_2D>(width,height);
	}
	
private:
	GLint _max_texture_size = 0;
	GLint _max_texture_buffer_size = 0;
	GLint _max_comb_texture_image_units = 0;
	GLint _max_array_texture_layers = 0;
	GLint _max_uniform_buffer_bindings = 0;
	GLint _max_vertex_uniform_blocks = 0;
	GLint _max_geometry_uniform_blocks = 0;
	GLint _max_fragment_uniform_blocks = 0;
	GLint _max_combined_vertex_uniform_components = 0;
	GLint _max_combined_gometry_uniform_components = 0;
	GLint _max_combined_fragment_uniform_components = 0;
	bool inited = false;
} rgl;
	


struct singleVAO
{
	singleVAO(const singleVAO &) = delete;
	singleVAO(singleVAO &&) = delete;
	singleVAO &operator=(const singleVAO &) = delete;
	singleVAO() {
		glGenVertexArrays(1, &ptr);
		checkError("creating VAO");
	}
	void bind() {
		glBindVertexArray(ptr);
		checkError("binding VAO");
	}
	~singleVAO() {
		glDeleteVertexArrays(1,&ptr);
		checkError("deleting VAO");
	}
private:
	GLuint ptr;
};

template<GLenum target, GLenum usage>
struct singleGLBuffer
{
	
	inline singleGLBuffer() { 
		glGenBuffers(1,&ptr);
		checkError("creating singleGLBuffer"); 
	}
	
	singleGLBuffer(const singleGLBuffer &) = delete;
	singleGLBuffer(singleGLBuffer &&) = delete;
	singleGLBuffer &operator=(const singleGLBuffer &) = delete;
	
	inline void bind()
	{
		glBindBuffer(target, ptr);
		checkError("rebinding buffer");
	}
	
	inline void bindBase(GLuint index)
	{
		glBindBufferBase(target,index,ptr);
		checkError("binding to binding point");
	}
	
	inline void loadBuffer(const GLvoid *src, GLsizeiptr size)
	{	
		bind();
		glBufferData(target, size, src, usage);
		checkError("buffering data");
		_rawsize = size;
	}
	
	inline void updateBuffer(const GLintptr offset, GLsizeiptr size, const GLvoid *data)
	{
		bind();
		glBufferSubData(target,offset,size,data);
		checkError("updated buffer");
	}
	
	inline GLsizeiptr rawsize() { return _rawsize; }
	
	inline ~singleGLBuffer()
	{
		glDeleteBuffers(1,&ptr);
		checkError("deleting buffer");
	}
private:
	GLuint ptr;
	GLsizeiptr _rawsize;
};




struct multiArrayIndirectRenderer
{
	multiArrayIndirectRenderer( const multiArrayIndirectRenderer &) = delete;
	multiArrayIndirectRenderer( multiArrayIndirectRenderer &&) = delete;
	multiArrayIndirectRenderer &operator=(const multiArrayIndirectRenderer &)= delete;
	multiArrayIndirectRenderer(size_t count)
		: _cmdCount(count), _cmds( new DrawArraysIndirectCommand[count]())
	{
		buffer.loadBuffer(_cmds,_cmdCount*sizeof(DrawArraysIndirectCommand));
	}
	
	~multiArrayIndirectRenderer()
	{
		delete []_cmds;
	}
	
	inline void reloadAll()
	{
		buffer.updateBuffer(0, 
			_cmdCount*sizeof(DrawArraysIndirectCommand), _cmds);
	}
	
	
	DrawArraysIndirectCommand &operator[](size_t cmdIndex)
	{
		return *(_cmds+cmdIndex);
	}
	
	
	inline void renderCmd(GLenum mode, size_t firstCmdIndex, size_t count, size_t cmdStrideCount = 1)
	{
		buffer.bind();
		glMultiDrawArraysIndirect(mode, (void *)(firstCmdIndex*sizeof(DrawArraysIndirectCommand)), count, 
			sizeof(DrawArraysIndirectCommand)*(cmdStrideCount));
	}
	

private:
	const size_t _cmdCount = 0;
	singleGLBuffer<GL_DRAW_INDIRECT_BUFFER,GL_DYNAMIC_DRAW> buffer;
	DrawArraysIndirectCommand * const _cmds = nullptr;
};

// texture must be binded
// note : this is used because openGL API is not perfect...
template<GLenum textype>
void glLoadDefaultTexture()
{
	glTexImage2D(textype, 0, GL_RED, 1, 1, 
					0, GL_RED, GL_UNSIGNED_BYTE, nullptr);
}




struct glTexArray
{
	/* note : try making a fix for different edge parameters for when width or height is less than the maximum.
	 */
	static constexpr GLenum textype = GL_TEXTURE_2D_ARRAY;
	
	glTexArray(const glTexArray &) = delete;
	glTexArray(const glTexArray &&) = delete;
	glTexArray operator=(const glTexArray &) = delete;
	inline glTexArray(GLuint bindPoint, size_t mipLevelCount, GLenum internalformat, GLsizei maxWidth, GLsizei maxHeight, GLsizei count) 
	: _internalformat(internalformat), _maxWidth(maxWidth), _maxHeight(maxHeight), _mipLevelCount(mipLevelCount), _count(count) {
		glGenTextures(1,&ptr);
		//bind(bindPoint); unfortunetly for some reason this won't do when texture name is just a name
		checkError("creating textures");
		rgl.ActiveTexture(bindPoint);
		checkError("calling glActiveTexture");
		bind();
		rgl.ActiveTexture(0);
		if(!glIsTexture(ptr)) 
		{
			printf("generated texture %llu is not a texture\n", ptr);
			getchar();
			exit(-1);
		}
		glTextureStorage3D(ptr, mipLevelCount, internalformat, maxWidth, maxHeight, count);
		checkError("loading default texture");
		//std::vector<uint32_t> clear_data(maxWidth*maxHeight,0);
	}
	
	inline void cindex(size_t index) {
		if(index >= _count) throw invalidAccessRange("invalid texture index to bind");
	}
	
	inline void bind() {
		glBindTexture(textype,ptr);
		checkError("binding texture to target");
	}
	
	inline void bind(GLuint bindPoint)
	{
		glBindTextureUnit(bindPoint, ptr);
		checkError("binding texture to bind point");
	}
	
	inline void loadTexture(size_t index, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height,
	GLint format, GLenum pixeltype, const GLvoid *data)
	{
		
		cindex(index);
		glTextureSubImage3D(ptr,0,0,0,index,width,height,1, format, pixeltype, data);
		//glTexImage2D(textype, level, internalformat, width, height, border, format, pixeltype, data);
		checkError("calling glTexSubImage3D");
	}
	
	inline void clearTexture(GLint level)
	{
		std::vector<uint32_t> clear_data(_maxWidth*_maxHeight,0);
		for(size_t i = 0; i < _count; ++i)
		{
			loadTexture(i, level, 0, 0, _maxWidth, _maxHeight, GL_RGBA, GL_UNSIGNED_BYTE, clear_data.data());
		}
	}
	
	inline void Parameteri(GLenum pname, GLfloat param)
	{
		glTextureParameteri(ptr,pname,param);
		checkError("calling glTextureParameteri");
	}
	
	inline void Parameterf(GLenum pname, GLint param)
	{
		glTextureParameterf(ptr,pname,param);
		checkError("calling glTextureParameterf");
	}
	
	inline void Parameterfv(GLenum pname, const GLfloat *params)
	{
		glTextureParameterfv(ptr,pname,params);
		checkError("calling glTextureParameterfv");
	}
	inline void Parameteriv(GLenum pname, const GLint *params)
	{
		glTextureParameteriv(ptr,pname,params);
		checkError("calling glTextureParameteriv");
	}
	inline void ParameterIiv(GLenum pname, const GLint *params)
	{
		glTextureParameterIiv(ptr,pname,params);
		checkError("calling glTextureParameterIiv");
	}
	inline void ParameterIuiv(GLenum pname, const GLuint *params)
	{
		glTextureParameterIuiv(ptr,pname,params);
		checkError("calling glTextureParameterIuiv");
	}
	
	
	inline ~glTexArray() {
		glDeleteTextures(1, &ptr);
		checkError("deleting textures");
	}
private:
	void defaultInitializeTextures()
	{
		
	}
	const GLenum _internalformat;
	const GLsizei _maxWidth, _maxHeight;
	const size_t _count;
	size_t _mipLevelCount = 1;
	GLuint ptr;

};

template<typename data_t>
struct glslUBO
{
	glslUBO(const glslUBO &) = delete;
	glslUBO(glslUBO &&) = delete;
	glslUBO &operator=(const glslUBO &) = delete;

	

	glslUBO(size_t count)
		: dataCount(count), data(new data_t[count]())
	{
		ub.loadBuffer(data, dataCount*sizeof(data_t));
	}

	data_t &operator[](size_t instanceID)
	{
		return *(data+instanceID);
	}

	void reloadAll()
	{
		ub.updateBuffer(0, dataCount*sizeof(data_t), data);
	}
	
	void bindBase(GLuint index)
	{
		ub.bindBase(index);
	}

	~glslUBO()
	{
		delete []data;
	}

	const size_t dataCount;
private:
	data_t * const data = nullptr;
	gll::singleGLBuffer<GL_UNIFORM_BUFFER, GL_DYNAMIC_DRAW> ub;
};

template<typename glsldataT, typename vertexT, GLenum vertexUsage = GL_DYNAMIC_DRAW>
struct commonRenderResource
{
	typedef glsldataT glsl_data_t;
	typedef vertexT vertex_t;
	commonRenderResource(const commonRenderResource &) = delete;
	commonRenderResource(const commonRenderResource &&) = delete;
	commonRenderResource &operator=(const commonRenderResource &) = delete;
	commonRenderResource(size_t cmdCount, size_t dataCount, size_t vboVertexCount)
		: indirectRenderer(cmdCount), objInstances(dataCount), vbo_data(new vertex_t[vboVertexCount])
	{
		vbo.loadBuffer(vbo_data,sizeof(vertex_t)*vboVertexCount);
	}
	
	~commonRenderResource()
	{
		delete []vbo_data;
	}
	
	void reloadAllVertices()
	{
		vbo.updateBuffer(0,sizeof(vertex_t)*vboVertexCount,vbo_data);
	}
	
	size_t vboVertexCount = 0;
	multiArrayIndirectRenderer indirectRenderer;
	glslUBO<glsl_data_t> objInstances;
	vertex_t * vbo_data = nullptr;
	singleGLBuffer<GL_ARRAY_BUFFER, vertexUsage> vbo;
	
};

struct masterRenderResource
{
	/* contains resources that gets shared among commonRenderResource structs 
	 * for rendering. currently contains view*projection Matrices only.
	 */
	struct glsl_data_t
	{
		//glsl_data_t() : vp(glm::identity<glm::mat4x4>()) {}
		glm::mat4x4 vp;
		glm::vec4 xylimits; // <left,right,bottom,top>
		
	};
	
	enum reservedIndexes
	{
		inActive = -1, overlayUI0, camUI0, overlayUI1, camUI1, overlayUI2, camUI2
	};
	
	masterRenderResource(const masterRenderResource &) = delete;
	masterRenderResource( masterRenderResource &&) = delete;
	masterRenderResource &operator=( masterRenderResource &) = delete;
	masterRenderResource(GLuint bindIndex, size_t viewCount = 100/*keep it binded at all times*/) 
		: _bindIndex(bindIndex), _ubo(viewCount)
	{
		_ubo.bindBase(bindIndex);
	}
	
	const GLuint _bindIndex;
	glslUBO<glsl_data_t> _ubo;
};

template<GLenum shType>
struct glshader
{
	const GLuint ptr;
	glshader() : ptr(glCreateShader(shType)) {}
	glshader(const GLchar *cstr) : glshader() { setSourceAndCompile(cstr);}
	
	template<uint16_t buffSize, typename ...Args>
	glshader(const GLchar *cstr, Args ... args)
		: glshader(stringsl::Rsnprintf<char,buffSize>(cstr,args...).data)
	{
		
	}
	glshader(const glshader &) = delete;
	glshader(glshader &&) = delete;
	glshader &operator=(const glshader &) = delete;
	void setSource(const GLchar *cstr) {
		glShaderSource(ptr,1,&cstr,nullptr);
		checkError("setting shader source");
	}
	void compile() {
		glCompileShader(ptr);
		
		
		printf("%s\n",getLog().data());
		checkError("compiling shader");
		if(!isCompiled())
		{
			printf("failed to compile shader\n");
			getchar();
			exit(-1);
		}
		
	}
	
	bool isCompiled()
	{
		GLint res;
		glGetShaderiv(ptr, GL_COMPILE_STATUS, &res);
		return (bool)res;
	}
	
	std::vector<char> getLog()
	{
		GLint logLength;
		glGetShaderiv(ptr, GL_INFO_LOG_LENGTH, &logLength);
		std::vector<char> logStr(logLength+1);
		glGetShaderInfoLog(ptr,logLength, NULL, logStr.data());
		return std::move(logStr);
	}
	void setSourceAndCompile(const GLchar *cstr)
	{
		setSource(cstr);
		compile();
	}
	~glshader() { glDeleteShader(ptr);}
};


typedef GLint glObjLocation;

struct glprogram
{
	const GLuint ptr;
	glprogram() : ptr(glCreateProgram())
	{
		checkError("creating program");
	}
	glprogram(const glprogram &) = delete;
	glprogram(glprogram &&) = delete;
	glprogram &operator=(const glprogram &) = delete;
	~glprogram() { glDeleteProgram(ptr); }
	
	template<GLenum shType>
	void attach(glshader<shType> &sh)
	{
		glAttachShader(ptr, sh.ptr);
		switch(shType)
		{
			case GL_FRAGMENT_SHADER:
			{
				checkError("attaching fragment shader");
				break;
			}
			case GL_VERTEX_SHADER:
			{
				checkError("attaching vertex shader");
				break;
			}
			case GL_GEOMETRY_SHADER:
			{
				checkError("attaching geometry shader");
				break;
			}
			default:
			{
				printf("undefined shader type\n");
				getchar();
				exit(-1);
			}
		}
			
	}
	
	template<GLenum shType>
	void detach(glshader<shType> &sh)
	{
		glDetachShader(ptr, sh.ptr);
		checkError("detaching shader");
	}
	void link()
	{	
		glLinkProgram(ptr);
		printf("%s\n",getLog().data());
		checkError("linking program");
		if(!isLinked())
		{
			printf("failed to link program\b");
			getchar();
			exit(-1);
		}
		
	}
	
	bool isLinked()
	{
		GLint tmp;
		glGetProgramiv(ptr,GL_LINK_STATUS, &tmp);
		return (bool)tmp;
	}
	
	std::vector<char> getLog()
	{
		GLint logLength;
		glGetProgramiv(ptr, GL_INFO_LOG_LENGTH, &logLength);
		std::vector<char> logStr(logLength+1);
		glGetProgramInfoLog(ptr,logLength, NULL, logStr.data());
		return std::move(logStr);
	}
	
	GLint getUniformLocation(const GLchar *name) {
		GLint ret = glGetUniformLocation(ptr,name);
		checkError("getting uniform location");
		return ret; }
	
	GLint getAttribLocation(const GLchar *name) {
		GLint ret = glGetAttribLocation(ptr,name);
		checkError("getting attrib location");
		if(ret == -1)
		{
			printf("invalid attrib name in getAttribLocation\n");
			getchar();
			exit(-1);
		}
		return ret; }
	
	
	GLint getUniformBlockIndex(const GLchar *name) {
		GLint ret = glGetUniformBlockIndex(ptr,name);
		checkError("getting uniform block index");
		if(ret == -1)
		{
			printf("invalid attrib name in getUniformBlockIndex\n");
			getchar();
			exit(-1);
		}
		return ret; }
	
	// map bindingPoint to location of the shader
	void uniformBlockBinding(GLuint location, GLuint bindingPoint)
	{
		glUniformBlockBinding(ptr,location,bindingPoint);
		checkError("calling glUniformBlockBinding");
	}
	
	template<GLenum usage>
	void bindUniformBlock(const GLchar *name, GLuint index, singleGLBuffer<GL_UNIFORM_BUFFER,usage> &buffer)
	{
		auto location = getUniformBlockIndex(name);
		buffer.bindBase(index);
		uniformBlockBinding(location,index);
	}
	
	template<typename data_t>
	void bindUniformBlock(const GLchar *name, GLuint index, glslUBO<data_t> &glslobjs)
	{
		auto location = getUniformBlockIndex(name);
		glslobjs.bindBase(index);
		uniformBlockBinding(location,index);
	}
	
	void Use(){
		glUseProgram(ptr);
		checkError("assigning program to use state");
	}
};

template<GLenum cap>
struct withgl
{
	withgl(const withgl &) = delete;
	withgl(withgl &&) = delete;
	withgl &operator=(const withgl &) = delete;
	withgl() : disable(glIsEnabled(cap) == GL_FALSE)
	{
		if(disable) glEnable(cap);
	}
	~withgl()
	{
		if(disable) glDisable(cap);
	}
	bool disable;
};
template<GLenum cap>
struct withoutgl
{
	withoutgl(const withoutgl &) = delete;
	withoutgl(withoutgl &&) = delete;
	withoutgl &operator=(const withoutgl &) = delete;
	withoutgl() : enable(glIsEnabled(cap) == GL_TRUE)
	{
		if(enable) glDisable(cap);
	}
	~withoutgl()
	{
		if(enable) glEnable(cap);
	}
	bool enable;
};



/*struct glbindUse
{
	
};

template<typename firstT, typename ...Args>
struct glbindUse : glbindUse<Args...>
{
	inline glbindUse(firstT firstArg,Args &... args) : binded(firstArg), glbindUse<Args...>(args...)
	{
		binded.bind();
	}
	glbindUse(const glbindUse &) = delete;
	glbindUse(glbindUse &&) = delete;
	glbindUse &operator=(const glbindUse &) = delete;
	~glbindUse
	firstT &binded;
};*/

}
}