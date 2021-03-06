/*
 * ShaderEngine.cpp
 *
 *  Created on: Jul 18, 2008
 *      Author: pete
 */
#include <fstream>
#include "PerlinNoise.hpp"
#include "ShaderEngine.hpp"
#include "BeatDetect.hpp"

#ifdef USE_GLES
    #define GLSL_VERSION "300 es"
#else
    #define GLSL_VERSION "410"
#endif

std::string v2f_c4f_vert(
    "#version "
    GLSL_VERSION
    "\n"
    ""
    "layout(location = 0) in vec2 vertex_position;\n"
    "layout(location = 1) in vec4 vertex_color;\n"
    ""
    "uniform mat4 vertex_transformation;\n"
    "uniform float vertex_point_size;\n"
    ""
    "out vec4 fragment_color;\n"
    ""
    "void main(){\n"
    "    gl_Position = vertex_transformation * vec4(vertex_position, 0.0, 1.0);\n"
    "    gl_PointSize = vertex_point_size;\n"
    "    fragment_color = vertex_color;\n"
    "}\n");

std::string v2f_c4f_frag(
        "#version "
        GLSL_VERSION
        "\n"
        "precision mediump float;\n"
        ""
        "in vec4 fragment_color;\n"
        "out vec4 color;\n"
        ""
        "void main(){\n"
        "    color = fragment_color;\n"
        "}\n");


std::string v2f_c4f_t2f_vert(
        "#version "
        GLSL_VERSION
        "\n"
        "layout(location = 0) in vec2 vertex_position;\n"
        "layout(location = 1) in vec4 vertex_color;\n"
        "layout(location = 2) in vec2 vertex_texture;\n"
        ""
        "uniform mat4 vertex_transformation;\n"
        ""
        "out vec4 fragment_color;\n"
        "out vec2 fragment_texture;\n"
        ""
        "void main(){\n"
        "    gl_Position = vertex_transformation * vec4(vertex_position, 0.0, 1.0);\n"
        "    fragment_color = vertex_color;\n"
        "    fragment_texture = vertex_texture;\n"
        "}\n");

std::string v2f_c4f_t2f_frag(
        "#version "
        GLSL_VERSION
        "\n"
        "precision mediump float;\n"
        ""
        "in vec4 fragment_color;\n"
        "in vec2 fragment_texture;\n"
        ""
        "uniform sampler2D texture_sampler;\n"
        ""
        "out vec4 color;\n"
        ""
        "void main(){\n"
        "    color = fragment_color * texture(texture_sampler, fragment_texture.st);\n"
        "}\n");


GLint ShaderEngine::UNIFORM_V2F_C4F_VERTEX_TRANFORMATION = 0;
GLint ShaderEngine::UNIFORM_V2F_C4F_VERTEX_POINT_SIZE = 0;
GLint ShaderEngine::UNIFORM_V2F_C4F_T2F_VERTEX_TRANFORMATION = 0;
GLint ShaderEngine::UNIFORM_V2F_C4F_T2F_FRAG_TEXTURE_SAMPLER = 0;



ShaderEngine::ShaderEngine()
{
#ifdef USE_CG
	SetupCg();
#endif
    
    // glValidateProgram needs a VAO for its checks
    GLuint m_temp_vao;
    glGenVertexArrays(1, &m_temp_vao);
    glBindVertexArray(m_temp_vao);
    
    programID_v2f_c4f = CompileShaderProgram(v2f_c4f_vert, v2f_c4f_frag);
    programID_v2f_c4f_t2f = CompileShaderProgram(v2f_c4f_t2f_vert, v2f_c4f_t2f_frag);

    UNIFORM_V2F_C4F_VERTEX_TRANFORMATION = glGetUniformLocation(programID_v2f_c4f, "vertex_transformation");
    UNIFORM_V2F_C4F_VERTEX_POINT_SIZE = glGetUniformLocation(programID_v2f_c4f, "vertex_point_size");
    UNIFORM_V2F_C4F_T2F_VERTEX_TRANFORMATION = glGetUniformLocation(programID_v2f_c4f_t2f, "vertex_transformation");
    UNIFORM_V2F_C4F_T2F_FRAG_TEXTURE_SAMPLER = glGetUniformLocation(programID_v2f_c4f_t2f, "texture_sampler");

    glDeleteVertexArrays(1, &m_temp_vao);
}

ShaderEngine::~ShaderEngine()
{
	// TODO Auto-generated destructor stub
}

#ifdef USE_CG

void ShaderEngine::setParams(const int texsize, const unsigned int texId, const float aspect, BeatDetect *beatDetect,
		TextureManager *textureManager)
{
	mainTextureId = texId;
	this->beatDetect = beatDetect;
	this->textureManager = textureManager;
	this->aspect = aspect;
	this->texsize = texsize;

	textureManager->setTexture("main", texId, texsize, texsize);

	glGenTextures(1, &blur1_tex);
	glBindTexture(GL_TEXTURE_2D, blur1_tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texsize/2, texsize/2, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL );
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glGenTextures(1, &blur2_tex);
	glBindTexture(GL_TEXTURE_2D, blur2_tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texsize / 4, texsize / 4, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL );
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glGenTextures(1, &blur3_tex);
	glBindTexture(GL_TEXTURE_2D, blur3_tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texsize / 8, texsize / 8, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL );
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	blur1_enabled = false;
	blur2_enabled = false;
	blur3_enabled = false;

	//std::cout << "Generating Noise Textures" << std::endl;

	PerlinNoise		*noise = new PerlinNoise;

	glGenTextures(1, &noise_texture_lq_lite);
	glBindTexture(GL_TEXTURE_2D, noise_texture_lq_lite);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, 32, 32, 0, GL_LUMINANCE, GL_FLOAT, noise->noise_lq_lite);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	textureManager->setTexture("noise_lq_lite", noise_texture_lq_lite, 32, 32);

	glGenTextures(1, &noise_texture_lq);
	glBindTexture(GL_TEXTURE_2D, noise_texture_lq);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, 256, 256, 0, GL_LUMINANCE, GL_FLOAT, noise->noise_lq);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	textureManager->setTexture("noise_lq", noise_texture_lq, 256, 256);

	glGenTextures(1, &noise_texture_mq);
	glBindTexture(GL_TEXTURE_2D, noise_texture_mq);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, 256, 256, 0, GL_LUMINANCE, GL_FLOAT, noise->noise_mq);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	textureManager->setTexture("noise_mq", noise_texture_mq, 256, 256);

	glGenTextures(1, &noise_texture_hq);
	glBindTexture(GL_TEXTURE_2D, noise_texture_hq);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, 256, 256, 0, GL_LUMINANCE, GL_FLOAT, noise->noise_hq);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	textureManager->setTexture("noise_hq", noise_texture_hq, 256, 256);

	glGenTextures(1, &noise_texture_perlin);
	glBindTexture(GL_TEXTURE_2D, noise_texture_perlin);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, 512, 512, 0, GL_LUMINANCE, GL_FLOAT, noise->noise_perlin);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	textureManager->setTexture("noise_perlin", noise_texture_perlin, 512, 512);

	 glGenTextures( 1, &noise_texture_lq_vol );
	 glBindTexture( GL_TEXTURE_3D, noise_texture_lq_vol );
	 glTexImage3D(GL_TEXTURE_3D,0,4,32,32,32,0,GL_LUMINANCE,GL_FLOAT,noise->noise_lq_vol);
	 glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	 glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	 glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	 glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	 textureManager->setTexture("noisevol_lq", noise_texture_lq_vol, 256, 256);

	 glGenTextures( 1, &noise_texture_hq_vol );
	 glBindTexture( GL_TEXTURE_3D, noise_texture_hq_vol );
	 glTexImage3D(GL_TEXTURE_3D,0,4,32,32,32,0,GL_LUMINANCE,GL_FLOAT,noise->noise_hq_vol);
	 glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	 glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	 glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	 glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	 textureManager->setTexture("noisevol_hq", noise_texture_hq_vol, 8, 8);
}

bool ShaderEngine::LoadCgProgram(Shader &shader, std::string &shaderFilename)
{
	//if (p != NULL) cgDestroyProgram(p);
	//p = NULL;
	std::string program = shader.programSource;

	if (program.length() > 0)
	{
		size_t found = program.rfind('}');
		if (found != std::string::npos)
		{
			//std::cout << "last '}' found at: " << int(found) << std::endl;
			program.replace(int(found), 1, "OUT.color.xyz=ret.xyz;\nOUT.color.w=1;\nreturn OUT;\n}");
		}
		else
			return false;
		found = program.rfind('{');
		if (found != std::string::npos)
		{
			//std::cout << "first '{' found at: " << int(found) << std::endl;
			program.replace(int(found), 1, "{\nfloat rad=getrad;\nfloat ang=getang;\n");
		}
		else
			return false;
		found = program.find("shader_body");
		if (found != std::string::npos)
		{
			//std::cout << "first 'shader_body' found at: " << int(found) << std::endl;
			program.replace(int(found), 11, "outtype projectm(float2 uv : TEXCOORD0)\n");
		}
		else
			return false;

		shader.textures.clear();

		found = 0;
		found = program.find("sampler_", found);
		while (found != std::string::npos)
		{
			found += 8;
			size_t end = program.find_first_of(" ;,\n\r)", found);

			if (end != std::string::npos)
			{

				std::string sampler = program.substr((int) found, (int) end - found);
				UserTexture* texture = new UserTexture(sampler);

				texture->texID = textureManager->getTexture(texture->name);
				if (texture->texID != 0)
				{
					texture->width = textureManager->getTextureWidth(texture->name);
					texture->height = textureManager->getTextureHeight(texture->name);
				}
				else
				{
					if (sampler.substr(0, 4) == "rand")
					{
						std::string random_name = textureManager->getRandomTextureName(texture->name);
						if (random_name.size() > 0)
						{
							texture->texID = textureManager->getTexture(random_name);
							texture->width = textureManager->getTextureWidth(random_name);
							texture->height = textureManager->getTextureHeight(random_name);
						}
					}
					else
					{
						std::string extensions[6];
						extensions[0] = ".jpg";
						extensions[1] = ".dds";
						extensions[2] = ".png";
						extensions[3] = ".tga";
						extensions[4] = ".bmp";
						extensions[5] = ".dib";

						for (int x = 0; x < 6; x++)
						{

							std::string filename = texture->name + extensions[x];
							texture->texID = textureManager->getTexture(filename);
							if (texture->texID != 0)
							{
								texture->width = textureManager->getTextureWidth(filename);
								texture->height = textureManager->getTextureHeight(filename);
								break;
							}
						}

					}
				}
				if (texture->texID != 0 && shader.textures.find(texture->qname) == shader.textures.end())
					shader.textures[texture->qname] = texture;

				else
					delete (texture);

			}

			found = program.find("sampler_", found);
		}
		textureManager->clearRandomTextures();

		found = 0;
		found = program.find("texsize_", found);
		while (found != std::string::npos)
		{
			found += 8;
			size_t end = program.find_first_of(" ;.,\n\r)", found);

			if (end != std::string::npos)
			{
				std::string tex = program.substr((int) found, (int) end - found);
				if (shader.textures.find(tex) != shader.textures.end())
				{
					UserTexture* texture = shader.textures[tex];
					texture->texsizeDefined = true;
					//std::cout << "texsize_" << tex << " found" << std::endl;
				}
			}
			found = program.find("texsize_", found);
		}

		found = program.find("GetBlur3");
		if (found != std::string::npos)
			blur1_enabled = blur2_enabled = blur3_enabled = true;
		else
		{
			found = program.find("GetBlur2");
			if (found != std::string::npos)
				blur1_enabled = blur2_enabled = true;
			else
			{
				found = program.find("GetBlur1");
				if (found != std::string::npos)
					blur1_enabled = true;
			}
		}

		std::string temp;

		temp.append(cgTemplate);
		temp.append(program);

		//std::cout << "Cg: Compilation Results:" << std::endl << std::endl;
		//std::cout << program << std::endl;

		CGprogram p = cgCreateProgram(myCgContext, CG_SOURCE, temp.c_str(),//temp.c_str(),
				myCgProfile, "projectm", NULL);

		checkForCgCompileError(shaderFilename, "creating shader program");
		if (p == NULL)
			return false;

		cgGLLoadProgram(p);

		if (checkForCgCompileError(shaderFilename, "loading shader program"))
		{
			p = NULL;
			return false;
		}

		programs[&shader] = p;

		return true;
	}
	else
		return false;
}

bool ShaderEngine::checkForCgCompileError(std::string &context, const char *situation)
{
	CGerror error;
	const char *string = cgGetLastErrorString(&error);
	error = cgGetError();
	if (error != CG_NO_ERROR)
	{
		std::cout << "Cg: Compilation Error For " << context << std::endl;
		std::cout << "Cg: " << situation << " - " << string << std::endl;
		if (error == CG_COMPILER_ERROR)
		{
			std::cout << "Cg: " << cgGetLastListing(myCgContext) << std::endl;

		}
		return true;
	}

	return false;
}

void ShaderEngine::checkForCgError(const char *situation)
{
	CGerror error;
	const char *string = cgGetLastErrorString(&error);

	if (error != CG_NO_ERROR)
	{
		std::cout << "Cg: %" << situation << " - " << string << std::endl;
		if (error == CG_COMPILER_ERROR)
		{
			std::cout << "Cg: " << cgGetLastListing(myCgContext) << std::endl;
		}
		exit(1);
	}
}

void ShaderEngine::SetupCg()
{
	std::string line;
	std::ifstream myfile(DATADIR_PATH "/shaders/projectM.cg");
	if (myfile.is_open())
	{
		while (!myfile.eof())
		{
			std::getline(myfile, line);
			cgTemplate.append(line + "\n");
		}
		myfile.close();
	}

	else
      std::cout << "Unable to load shader template \"" << DATADIR_PATH << "/shaders/projectM.cg\"" << std::endl;

	std::ifstream myfile2(DATADIR_PATH "/shaders/blur.cg");
	if (myfile2.is_open())
	{
		while (!myfile2.eof())
		{
			std::getline(myfile2, line);
			blurProgram.append(line + "\n");
		}
		myfile2.close();
	}

	else
		std::cout << "Unable to load blur template" << std::endl;

	myCgContext = cgCreateContext();
	checkForCgError("creating context");
	cgGLSetDebugMode(CG_FALSE);
	cgSetParameterSettingMode(myCgContext, CG_DEFERRED_PARAMETER_SETTING);

	myCgProfile = cgGLGetLatestProfile(CG_GL_FRAGMENT);

	// HACK breaks with buggy ati video drivers such as my own
	// -carmelo.piccione@gmail.com 7/26/2010
    cgGLSetOptimalOptions(myCgProfile);
	checkForCgError("selecting fragment profile");

	profileName = cgGetProfileString(myCgProfile);
	std::cout << "Cg: Initialized profile: " << profileName << std::endl;
//std::cout<< blurProgram.c_str()<<std::endl;
	blur1Program = cgCreateProgram(myCgContext, CG_SOURCE, blurProgram.c_str(), myCgProfile, "blur1", NULL);

    std::string blur1Filename = "blur1";
    checkForCgCompileError(blur1Filename, "creating blur1 program");
	if (blur1Program == NULL)
		exit(1);
	cgGLLoadProgram(blur1Program);

	checkForCgError("loading blur1 program");

	blur2Program = cgCreateProgram(myCgContext, CG_SOURCE, blurProgram.c_str(), myCgProfile, "blurVert", NULL);

    std::string blur2Filename = "blur2";
	checkForCgCompileError(blur2Filename, "creating blur2 program");
	if (blur2Program == NULL)
		exit(1);
	cgGLLoadProgram(blur2Program);

	checkForCgError("loading blur2 program");

}

void ShaderEngine::SetupCgVariables(CGprogram program, const Pipeline &pipeline, const PipelineContext &context)
{

	double slow_roam_cos[4] =	{ 0.5 + 0.5 * cos(context.time * 0.005), 0.5 + 0.5 * cos(context.time * 0.008), 0.5 + 0.5 * cos(context.time * 0.013), 0.5 + 0.5 * cos(context.time * 0.022) };
	double roam_cos[4] =	{ 0.5 + 0.5 * cos(context.time * 0.3), 0.5 + 0.5 * cos(context.time * 1.3), 0.5 + 0.5 * cos(context.time * 5), 0.5	+ 0.5 * cos(context.time * 20) };
	double slow_roam_sin[4] =	{ 0.5 + 0.5 * sin(context.time * 0.005), 0.5 + 0.5 * sin(context.time * 0.008), 0.5 + 0.5 * sin(context.time * 0.013), 0.5 + 0.5 * sin(context.time * 0.022) };
	double roam_sin[4] =	{ 0.5 + 0.5 * sin(context.time * 0.3), 0.5 + 0.5 * sin(context.time * 1.3), 0.5 + 0.5 * sin(context.time * 5), 0.5	+ 0.5 * sin(context.time * 20) };

	cgGLSetParameter4dv(cgGetNamedParameter(program, "slow_roam_cos"), slow_roam_cos);
	cgGLSetParameter4dv(cgGetNamedParameter(program, "roam_cos"), roam_cos);
	cgGLSetParameter4dv(cgGetNamedParameter(program, "slow_roam_sin"), slow_roam_sin);
	cgGLSetParameter4dv(cgGetNamedParameter(program, "roam_sin"), roam_sin);

	cgGLSetParameter1f(cgGetNamedParameter(program, "time"), context.time);
	cgGLSetParameter4f(cgGetNamedParameter(program, "rand_preset"), rand_preset[0], rand_preset[1], rand_preset[2],	rand_preset[3]);
	cgGLSetParameter4f(cgGetNamedParameter(program, "rand_frame"), (rand() % 100) * .01, (rand() % 100) * .01, (rand()% 100) * .01, (rand() % 100) * .01);
	cgGLSetParameter1f(cgGetNamedParameter(program, "fps"), context.fps);
	cgGLSetParameter1f(cgGetNamedParameter(program, "frame"), context.frame);
	cgGLSetParameter1f(cgGetNamedParameter(program, "progress"), context.progress);

	cgGLSetParameter1f(cgGetNamedParameter(program, "blur1_min"), pipeline.blur1n);
	cgGLSetParameter1f(cgGetNamedParameter(program, "blur1_max"), pipeline.blur1x);
	cgGLSetParameter1f(cgGetNamedParameter(program, "blur2_min"), pipeline.blur2n);
	cgGLSetParameter1f(cgGetNamedParameter(program, "blur2_max"), pipeline.blur2x);
	cgGLSetParameter1f(cgGetNamedParameter(program, "blur3_min"), pipeline.blur3n);
	cgGLSetParameter1f(cgGetNamedParameter(program, "blur3_max"), pipeline.blur3x);

	cgGLSetParameter1f(cgGetNamedParameter(program, "bass"), beatDetect->bass);
	cgGLSetParameter1f(cgGetNamedParameter(program, "mid"), beatDetect->mid);
	cgGLSetParameter1f(cgGetNamedParameter(program, "treb"), beatDetect->treb);
	cgGLSetParameter1f(cgGetNamedParameter(program, "bass_att"), beatDetect->bass_att);
	cgGLSetParameter1f(cgGetNamedParameter(program, "mid_att"), beatDetect->mid_att);
	cgGLSetParameter1f(cgGetNamedParameter(program, "treb_att"), beatDetect->treb_att);
	cgGLSetParameter1f(cgGetNamedParameter(program, "vol"), beatDetect->vol);
	cgGLSetParameter1f(cgGetNamedParameter(program, "vol_att"), beatDetect->vol);

	cgGLSetParameter4f(cgGetNamedParameter(program, "texsize"), texsize, texsize, 1 / (float) texsize, 1
			/ (float) texsize);
	cgGLSetParameter4f(cgGetNamedParameter(program, "aspect"), 1 / aspect, 1, aspect, 1);

	if (blur1_enabled)
	{
		cgGLSetTextureParameter(cgGetNamedParameter(program, "sampler_blur1"), blur1_tex);
		cgGLEnableTextureParameter(cgGetNamedParameter(program, "sampler_blur1"));
	}
	if (blur2_enabled)
	{
		cgGLSetTextureParameter(cgGetNamedParameter(program, "sampler_blur2"), blur2_tex);
		cgGLEnableTextureParameter(cgGetNamedParameter(program, "sampler_blur2"));
	}
	if (blur3_enabled)
	{
		cgGLSetTextureParameter(cgGetNamedParameter(program, "sampler_blur3"), blur3_tex);
		cgGLEnableTextureParameter(cgGetNamedParameter(program, "sampler_blur3"));
	}

}

void ShaderEngine::SetupUserTexture(CGprogram program, const UserTexture* texture)
{
	std::string samplerName = "sampler_" + texture->qname;

	CGparameter param = cgGetNamedParameter(program, samplerName.c_str());
	checkForCgError("getting parameter");
	cgGLSetTextureParameter(param, texture->texID);
	checkForCgError("setting parameter");
	cgGLEnableTextureParameter(param);
	checkForCgError("enabling parameter");
	//std::cout<<texture->texID<<" "<<samplerName<<std::endl;

	if (texture->texsizeDefined)
	{
		std::string texsizeName = "texsize_" + texture->name;
		cgGLSetParameter4f(cgGetNamedParameter(program, texsizeName.c_str()), texture->width, texture->height, 1
				/ (float) texture->width, 1 / (float) texture->height);
		checkForCgError("setting parameter texsize");
	}
}

void ShaderEngine::SetupUserTextureState( const UserTexture* texture)
{
	    glBindTexture(GL_TEXTURE_2D, texture->texID);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, texture->bilinear ? GL_LINEAR : GL_NEAREST);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, texture->bilinear ? GL_LINEAR : GL_NEAREST);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, texture->wrap ? GL_REPEAT : GL_CLAMP);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, texture->wrap ? GL_REPEAT : GL_CLAMP);
}

void ShaderEngine::SetupCgQVariables(CGprogram program, const Pipeline &q)
{
		cgGLSetParameter4f(cgGetNamedParameter(program, "_qa"), q.q[0], q.q[1], q.q[2], q.q[3]);
		cgGLSetParameter4f(cgGetNamedParameter(program, "_qb"), q.q[4], q.q[5], q.q[6], q.q[7]);
		cgGLSetParameter4f(cgGetNamedParameter(program, "_qc"), q.q[8], q.q[9], q.q[10], q.q[11]);
		cgGLSetParameter4f(cgGetNamedParameter(program, "_qd"), q.q[12], q.q[13], q.q[14], q.q[15]);
		cgGLSetParameter4f(cgGetNamedParameter(program, "_qe"), q.q[16], q.q[17], q.q[18], q.q[19]);
		cgGLSetParameter4f(cgGetNamedParameter(program, "_qf"), q.q[20], q.q[21], q.q[22], q.q[23]);
		cgGLSetParameter4f(cgGetNamedParameter(program, "_qg"), q.q[24], q.q[25], q.q[26], q.q[27]);
		cgGLSetParameter4f(cgGetNamedParameter(program, "_qh"), q.q[28], q.q[29], q.q[30], q.q[31]);
}

void ShaderEngine::setAspect(float aspect)
{
	this->aspect = aspect;
}
void ShaderEngine::RenderBlurTextures(const Pipeline &pipeline, const PipelineContext &pipelineContext,
		const int texsize)
{
	if (blur1_enabled || blur2_enabled || blur3_enabled)
	{
		float tex[4][2] =
		{
		{ 0, 1 },
		{ 0, 0 },
		{ 1, 0 },
		{ 1, 1 } };

		glBlendFunc(GL_ONE, GL_ZERO);
		glColor4f(1.0, 1.0, 1.0, 1.0f);

		glBindTexture(GL_TEXTURE_2D, mainTextureId);
		glEnable(GL_TEXTURE_2D);

		glEnableClientState(GL_VERTEX_ARRAY);
		glDisableClientState(GL_COLOR_ARRAY);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glTexCoordPointer(2, GL_FLOAT, 0, tex);

		cgGLEnableProfile(myCgProfile);
		checkForCgError("enabling profile");

		if (blur1_enabled)
		{
			cgGLSetParameter4f(cgGetNamedParameter(blur1Program, "srctexsize"), texsize/2, texsize/2, 2 / (float) texsize,
					2 / (float) texsize);
			cgGLSetParameter4f(cgGetNamedParameter(blur2Program, "srctexsize"), texsize/2 , texsize/2, 2 / (float) texsize,
								2 / (float) texsize);



			float pointsold[4][2] =
			{
			{ 0, 1 },
			{ 0, 0 },
			{ 1, 0 },
			{ 1, 1 } };
			float points[4][2] =
						{
						{ 0, 0.5 },
						{ 0, 0 },
						{ 0.5, 0 },
						{ 0.5, 0.5 } };

			cgGLBindProgram(blur1Program);
						checkForCgError("binding blur1 program");

			glVertexPointer(2, GL_FLOAT, 0, points);
			glBlendFunc(GL_ONE,GL_ZERO);
			glDrawArrays(GL_TRIANGLE_FAN, 0, 4);


			glBindTexture(GL_TEXTURE_2D, blur1_tex);
			glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, texsize/2, texsize/2);


		}

		if (blur2_enabled)
		{
			cgGLSetParameter4f(cgGetNamedParameter(blur1Program, "srctexsize"), texsize/2, texsize/2, 2 / (float) texsize,
								2 / (float) texsize);
			cgGLSetParameter4f(cgGetNamedParameter(blur2Program, "srctexsize"), texsize/2, texsize/2, 2 / (float) texsize,
					2 / (float) texsize);



			float points[4][2] =
			{
			{ 0, 0.25 },
			{ 0, 0 },
			{ 0.25, 0 },
			{ 0.25, 0.25 } };

			glVertexPointer(2, GL_FLOAT, 0, points);
			glBlendFunc(GL_ONE,GL_ZERO);
			glDrawArrays(GL_TRIANGLE_FAN, 0, 4);



						glBindTexture(GL_TEXTURE_2D, blur2_tex);
						glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, texsize/4, texsize/4);


		}

		if (blur3_enabled)
		{
			cgGLSetParameter4f(cgGetNamedParameter(blur2Program, "srctexsize"), texsize/4, texsize/4, 4 / (float) texsize,
								4/ (float) texsize);
			cgGLSetParameter4f(cgGetNamedParameter(blur2Program, "srctexsize"), texsize / 4, texsize / 4, 4
					/ (float) texsize, 4 / (float) texsize);
			float points[4][2] =
			{
			{ 0, 0.125 },
			{ 0, 0 },
			{ 0.125, 0 },
			{ 0.125, 0.125 } };

			glVertexPointer(2, GL_FLOAT, 0, points);
			glBlendFunc(GL_ONE,GL_ZERO);
			glDrawArrays(GL_TRIANGLE_FAN, 0, 4);



						glBindTexture(GL_TEXTURE_2D, blur3_tex);
						glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, texsize/8, texsize/8);


		}


			cgGLUnbindProgram(myCgProfile);
			checkForCgError("unbinding blur2 program");


		cgGLDisableProfile(myCgProfile);
		checkForCgError("disabling blur profile");

		glDisable(GL_TEXTURE_2D);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	}
}

void ShaderEngine::loadShader(Shader &shader, std::string &shaderFilename)
{
	if (shader.enabled)
	{
		cgDestroyProgram(programs[&shader]);
		programs.erase(&shader);
	}
	shader.enabled = LoadCgProgram(shader, shaderFilename);
}

void ShaderEngine::disableShader()
{
	if (enabled)
	{
		cgGLUnbindProgram(myCgProfile);
		checkForCgError("disabling fragment profile");
		cgGLDisableProfile(myCgProfile);
		checkForCgError("disabling fragment profile");
	}
	enabled = false;
}

void ShaderEngine::enableShader(Shader &shader, const Pipeline &pipeline, const PipelineContext &pipelineContext)
{
	enabled = false;
	if (shader.enabled)
	{

		for (std::map<std::string, UserTexture*>::const_iterator pos = shader.textures.begin(); pos	!= shader.textures.end(); ++pos)
							SetupUserTextureState( pos->second);


		CGprogram program = programs[&shader];
		for (std::map<std::string, UserTexture*>::const_iterator pos = shader.textures.begin(); pos
						!= shader.textures.end(); ++pos)
					SetupUserTexture(program, pos->second);

		cgGLEnableProfile(myCgProfile);
		checkForCgError("enabling warp profile");

		cgGLBindProgram(program);
		checkForCgError("binding warp program");

		SetupCgVariables(program, pipeline, pipelineContext);
		SetupCgQVariables(program, pipeline);

		enabled = true;
	}
}

void ShaderEngine::reset()
{
	rand_preset[0] = (rand() % 100) * .01;
	rand_preset[1] = (rand() % 100) * .01;
	rand_preset[2] = (rand() % 100) * .01;
	rand_preset[3] = (rand() % 100) * .01;
}

#endif


GLuint ShaderEngine::CompileShaderProgram(const std::string & VertexShaderCode, const std::string & FragmentShaderCode){

    // Create the shaders
    GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
    GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

    GLint Result = GL_FALSE;
    int InfoLogLength;


    // Compile Vertex Shader
    char const * VertexSourcePointer = VertexShaderCode.c_str();
    glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
    glCompileShader(VertexShaderID);

    // Check Vertex Shader
    glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if ( InfoLogLength > 0 ){
        std::vector<char> VertexShaderErrorMessage(InfoLogLength+1);
        glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
        fprintf(stderr, "Error compiling base vertex shader: %s\n", &VertexShaderErrorMessage[0]);
    }


    // Compile Fragment Shader
    char const * FragmentSourcePointer = FragmentShaderCode.c_str();
    glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
    glCompileShader(FragmentShaderID);

    // Check Fragment Shader
    glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if ( InfoLogLength > 0 ){
        std::vector<char> FragmentShaderErrorMessage(InfoLogLength+1);
        glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
        fprintf(stderr, "Error compiling base fragment shader: %s\n", &FragmentShaderErrorMessage[0]);
    }



    // Link the program
    GLuint programID = glCreateProgram();
    glAttachShader(programID, VertexShaderID);
    glAttachShader(programID, FragmentShaderID);
    glLinkProgram(programID);

    // Check the program
    glGetProgramiv(programID, GL_LINK_STATUS, &Result);
    glGetProgramiv(programID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if ( InfoLogLength > 0 ){
        std::vector<char> ProgramErrorMessage(InfoLogLength+1);
        glGetProgramInfoLog(programID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
        fprintf(stderr, "%s\n", &ProgramErrorMessage[0]);
    }


    glValidateProgram(programID);

    // Check the program
    glGetProgramiv(programID, GL_VALIDATE_STATUS, &Result);
    glGetProgramiv(programID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if ( InfoLogLength > 0 ){
        std::vector<char> ProgramErrorMessage(InfoLogLength+1);
        glGetProgramInfoLog(programID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
        fprintf(stderr, "%s\n", &ProgramErrorMessage[0]);
    }


    glDetachShader(programID, VertexShaderID);
    glDetachShader(programID, FragmentShaderID);

    glDeleteShader(VertexShaderID);
    glDeleteShader(FragmentShaderID);

    return programID;
}
