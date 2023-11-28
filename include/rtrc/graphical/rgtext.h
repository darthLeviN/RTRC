/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   rgtext.h
 * Author: roohi
 *
 * Created on August 10, 2019, 8:37 PM
 */
#include "ropengl.h"
#include "../rstring.h"
#include "../rexception.h"
#include <vector>
#include <freetype/freetype.h>
#include <glm/glm.hpp>
#pragma once

namespace rtrc { namespace textl {
	
	struct gchartexInfo
	{
		glm::ivec2 size;
		glm::ivec2 bearing;
		GLuint advance;
	};
	
	struct FT_mLib
	{
		
		FT_mLib(const FT_mLib &) = delete;
		FT_mLib & operator=(const FT_mLib &) = delete;
		inline FT_mLib(FT_mLib &&ml) : libPtr(ml.libPtr) {			
			ml.libPtr = nullptr;
		}
		inline FT_mLib()
		{
			if(int error = FT_Init_FreeType(&libPtr))
			{
				printf("ERROR %d :could not create FT library\n",error);
				getchar();
				exit(-1);
			}
		}
		
		inline ~FT_mLib() { 
			if(int error = FT_Done_FreeType(libPtr))
			{
				printf("ERROR %d : could not destroy FT library\n", error);
				getchar();
				exit(-1);
			}
		}
		inline operator FT_Library() { return libPtr; }
	private:
		FT_Library libPtr;
	};
	
	struct FT_mFace
	{
		inline FT_mFace(const FT_mFace &) = delete;
		inline FT_mFace &operator=(const FT_mFace &) = delete;
		inline FT_mFace(FT_Library ft, const char *fontPath,FT_UInt pixel_height , FT_UInt pixel_width = 0, FT_Long index = 0) {
			if(int error = FT_New_Face(ft,fontPath, index, &facePtr))
			{
				printf("ERROR %d : could not create face with FT_New_Face\n", error);
				getchar();
				exit(-1);
			}
			if(int error = FT_Set_Pixel_Sizes(facePtr,pixel_width,pixel_height))
			{
				printf("ERROR %d : could not set pixel size of face\n", error);
				getchar();
				exit(-1);
			}
		}
		inline FT_mFace(FT_mFace &&mf) : facePtr(mf.facePtr) {
			mf.facePtr = nullptr;
		}
		inline ~FT_mFace()
		{
			if(int error = FT_Done_Face(facePtr))
			{
				printf("ERROR %d : could not destroy face with FT_Done_Face\n", error);
				getchar();
				exit(-1);
			}
		}
		
		inline operator FT_Face() {return facePtr;}
		inline FT_Face operator->() { return facePtr; }
	private:
		FT_Face facePtr;
	};
	
	//
	//
	//
	struct gltextRenderer
	{
		struct glsl_objIs_data_t
		{
			glm::mat4x4 m;
			int32_t vpIndex; // determines vp matrix index.
			int32_t padding[3];
		};
		
		struct charVertex
		{
			/* font code 0 is used to represent an inactive character. inactive 
			 * characters can be represented in verious ways, this way was 
			 * choosen due to no clear advantage for any of the ways.
			 */
			uint32_t charCode;
			uint32_t fontCode;
			glm::vec2 charOff;
			glm::vec2 charScale;
			glm::vec4 charColor;
		};
		
		typedef gll::commonRenderResource<glsl_objIs_data_t,charVertex> renderResource_t;
		
		inline gltextRenderer( gltextRenderer &&) = delete;
		gltextRenderer( const gltextRenderer &) = delete;
		gltextRenderer &operator=(const gltextRenderer &) = delete;
		
		inline gltextRenderer(//gll::masterRenderResource &masterRenderRs, 
		FT_Library ft, const char *fontPath, FT_UInt pixel_height, 
		GLuint uniformBufferBindIndex, GLuint textureBindIndex, size_t maxStrings = 200)
			: //_maxStrings(maxStrings), _maxInstances(maxStrings),
			//_masterRenderRs(masterRenderRs),
			_renderResource(maxStrings, maxStrings, maxStrings*_maxStringLength),
			//_objsIs(_maxInstances), _indirectRenderer(_maxStrings),
			_face(ft, fontPath, pixel_height),
			_vrsh(vertex_shader_text), 
			_gosh(stringsl::Rsnprintf<char,sizeof(geometry_shader_text)+20>(
				geometry_shader_text,_renderResource.objInstances.dataCount).data),
				_frsh(fragment_shader_text), 
			_ubBindPoint(uniformBufferBindIndex), _texBindPoint(textureBindIndex),
			_textures(_texBindPoint,1,GL_RGBA8,pixel_height,pixel_height,charCount)
		{
			//
			// loading textures
			//
			//glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			//glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			//glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			//glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			_textures.Parameteri(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			_textures.Parameteri(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			_textures.Parameteri(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			_textures.Parameteri(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			for(GLubyte c = startChar; c <= endChar; ++c)
			{
				// load character glyph
				if(int error = FT_Load_Char(_face,c,FT_LOAD_RENDER))
				{
					printf("ERROR %d : could not load char [%u]\n from \"%s\"", error, c, fontPath);
					getchar();
					exit(-1);
				}
				//printf("loading char %u with %ux%u bitmap\n", c, _face->glyph->bitmap.width, _face->glyph->bitmap.rows);
				_textures.loadTexture(c, 0, 0, 0, _face->glyph->bitmap.width, _face->glyph->bitmap.rows, GL_RED, GL_UNSIGNED_BYTE, _face->glyph->bitmap.buffer);
				_textures.clearTexture(0);
				_charinfos[c] = {glm::ivec2(_face->glyph->bitmap.width, _face->glyph->bitmap.rows)/*size*/,
				glm::ivec2(_face->glyph->bitmap_left, _face->glyph->bitmap_top)/*bearing*/,
				_face->glyph->advance.x/*advance*/};
			}
			
			//
			// preparing glProgram
			//
			_glprog.attach(_vrsh);
			_glprog.attach(_gosh);
			_glprog.attach(_frsh);
			_glprog.link();
			_glprog.bindUniformBlock("objs", _ubBindPoint, _renderResource.objInstances); // now _objsIs can be modified freely
			
			//
			// vertex array object
			//
			_vao.bind();
			auto charCodeLocation = _glprog.getAttribLocation("vcharCode");
			glVertexAttribPointer(charCodeLocation, 1, GL_UNSIGNED_INT, 
				GL_FALSE, sizeof(charVertex),(void *)offsetof(charVertex,charCode));
			auto charColorLocation = _glprog.getAttribLocation("vcharColor");
			glVertexAttribPointer(charColorLocation, 4, GL_FLOAT, 
				GL_FALSE, sizeof(charVertex),(void *)offsetof(charVertex,charColor));
			auto charOffLocation = _glprog.getAttribLocation("vcharOff");
			glVertexAttribPointer(charOffLocation, 2, GL_FLOAT, 
				GL_FALSE, sizeof(charVertex),(void *)offsetof(charVertex,charOff));
			//auto fontCodeLocation = _glprog.getAttribLocation("vfontCode");
			//glVertexAttribPointer(fontCodeLocation, 1, GL_UNSIGNED_INT, 
				//GL_FALSE, sizeof(charVertex),(void *)offsetof(charVertex,fontCode));
			auto charScaleLocation = _glprog.getAttribLocation("vcharScale");
			glVertexAttribPointer(charScaleLocation, 2, GL_FLOAT, 
				GL_FALSE, sizeof(charVertex),(void *)offsetof(charVertex,charScale));
			gll::checkError("setting up vertex buffer for gltextRenderer");
			
			glEnableVertexAttribArray(charCodeLocation);
			gll::checkError("enabling charCodeLocation vertex attribute");
			
			glEnableVertexAttribArray(charColorLocation);
			gll::checkError("enabling charColorLocation vertex attribute");
			
			glEnableVertexAttribArray(charOffLocation);
			gll::checkError("enabling charOffLocation vertex attribute");
			
			//glEnableVertexAttribArray(fontCodeLocation);
			//gll::checkError("enabling fontCodeLocation vertex attribute");
			
			glEnableVertexAttribArray(charScaleLocation);
			gll::checkError("enabling charScaleLocation vertex attribute");
		}
		
		/*encodeBasicString(charVertex *dst, char *src, glm::mat4x4 m)
		{
			while(*src != '\0')
			{
				charVertex &cv = *dst;
				cv.charCode = (uint32_t)*src;
				cv.charColor = glm::vec4(0.0,1.0,1.0,1.0);
				cv.charOff 
			}
		}*/
		
		
		inline void _renderAll()
		{
			//
			// setting up pre render options and bindings
			//
			gll::withgl<GL_BLEND> glblend;
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			_renderResource.reloadAllVertices(); // also binds
			_vao.bind();
			//
			// main render command
			//
			_renderResource.indirectRenderer.renderCmd(GL_POINTS, 0, _renderResource.objInstances.dataCount);
		}
		
		
		static constexpr FT_ULong startChar = 0;
		static constexpr FT_ULong endChar = 127;
		static constexpr size_t charCount = 128;
		//const size_t _maxStrings = 0;
		//const size_t _maxInstances = _maxStrings;
		const size_t _maxStringLength = 1024;
		const GLuint _ubBindPoint = 0;
		const GLuint _texBindPoint = 0;
		//const size_t _maxTotalCharVertices = _maxStringLength * _maxStrings;
		//const size_t _maxVboSize = _maxTotalCharVertices * sizeof(charVertex);
		
	private:
		
		static constexpr const char vertex_shader_text[] = 
		"#version 460 core\n"
		"in vec2 vcharOff;\n"
		"in uint vcharCode;\n"
		"in uint vfontCode;\n"
		"in vec2 vcharScale;\n"
		"in vec4 vcharColor;\n"
		"out uint gcharCode;\n"
		"out uint gfontCode;\n"
		"out vec2 gcharScale;\n"
		"out vec4 gcharColor;\n"
		"out int gIID;\n"
		"\n"
		"void main()\n"
		"{\n"
		"	gl_Position = vec4(vcharOff.xy, 0.0, 1.0);\n"
		"	gcharCode = vcharCode;\n"
		"	gcharScale = vcharScale;\n"
		"	gfontCode = vfontCode;\n"
		"	gcharColor = vcharColor;\n"
		"	gIID = gl_InstanceID;\n"
		"}\n";
		
		static constexpr const char geometry_shader_text[] =
		"#version 460 core\n"
		"layout (points) in;\n"
		"layout (triangle_strip, max_vertices = 4) out;\n"
		"in uint gcharCode[];\n"
		"in vec2 gcharScale[];\n"
		"in uint gfontCode[];\n"
		"in vec4 gcharColor[];\n"
		"in int gIID[];"
		"out vec3 ftexCoords;\n"
		"out vec4 fcharColor;\n"
		"\n"
		"struct charMapInfo\n"
		"{\n"
		"	vec4 charScEcLi; //<startCode,endCode,languageIndex>\n"
		"};\n"
		
		"struct objdata\n"
		"{\n"
		"	mat4 m;\n"
		"	int vpIndex;\n"
		"};\n"
		"layout(std140) uniform objs\n"
		"{\n"
		"	objdata data[%llu];\n"
		"} objsi;\n"
		"\n"
		"\n"
		"void main()\n"
		"{\n"
		"	if(gfontCode[0] != 0){\n"
		"	mat4 m = objsi.data[gIID[0]].m;\n"
		"	vec4 basePos = gl_in[0].gl_Position;\n"
		"	float texZ = float(gcharCode[0]);\n"
		"	fcharColor = gcharColor[0];\n"
		"	ftexCoords = vec3(1.0,0.0,texZ);\n"
		"	gl_Position = m * vec4(basePos.x + gcharScale[0].x,basePos.yzw);\n"
		"	EmitVertex();\n"
		"	gl_Position = m * vec4(basePos.xy + gcharScale[0],basePos.zw);\n"
		"	ftexCoords = vec3(1.0,1.0,texZ);\n"
		"	EmitVertex();\n"
		"	gl_Position = m * basePos;\n"
		"	ftexCoords = vec3(0.0,0.0,texZ);\n"
		"	EmitVertex();\n"
		"	gl_Position = m * vec4(basePos.x, basePos.y + gcharScale[0].y, basePos.zw);\n"
		"	ftexCoords = vec3(0.0,1.0,texZ);\n"
		"	EndPrimitive();}\n"
		"}\n"
		"\n";
		
		static constexpr const char fragment_shader_text[] =
		"#version 460 core\n"
		"in vec3 ftexCoords;\n"
		"in vec4 fcharColor;\n"
		"out vec4 color;\n"
		"uniform sampler2DArray textsmp;\n"
		"void main()\n"
		"{\n"
		"	vec4 sampled = vec4(1.0, 1.0, 1.0, texture(textsmp,ftexCoords).r);\n"
		"	color = fcharColor*sampled;\n"
		"}\n";
		
		
		//GLuint TextureID[charCount];
		FT_mFace _face;
		gchartexInfo _charinfos[charCount];
		//gll::masterRenderResource &_masterRenderRs;
		renderResource_t _renderResource;
		//gll::glslUBO<glsl_objIs_data_t> _objsIs;
		//gll::multiArrayIndirectRenderer _indirectRenderer;
		gll::glshader<GL_VERTEX_SHADER> _vrsh;
		gll::glshader<GL_GEOMETRY_SHADER> _gosh;
		gll::glshader<GL_FRAGMENT_SHADER> _frsh;
		gll::singleVAO _vao;
		//gll::singleGLBuffer<GL_ARRAY_BUFFER, GL_DYNAMIC_DRAW> _charVertices;
		GLint _vertex_location = 0;
		
		gll::glTexArray _textures;
		gll::glprogram _glprog;
		
		
		
	};
}
}
