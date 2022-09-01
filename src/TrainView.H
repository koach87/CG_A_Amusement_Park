/************************************************************************
     File:        TrainView.H

     Author:     
                  Michael Gleicher, gleicher@cs.wisc.edu

     Modifier
                  Yu-Chi Lai, yu-chi@cs.wisc.edu
     
     Comment:     
						The TrainView is the window that actually shows the 
						train. Its a
						GL display canvas (Fl_Gl_Window).  It is held within 
						a TrainWindow
						that is the outer window with all the widgets. 
						The TrainView needs 
						to be aware of the window - since it might need to 
						check the widgets to see how to draw

	  Note:        we need to have pointers to this, but maybe not know 
						about it (beware circular references)

     Platform:    Visio Studio.Net 2003/2005

*************************************************************************/

#pragma once

#include "RenderUtilities/BufferObject.h"
#include "RenderUtilities/Shader.h"
#include "RenderUtilities/Texture.h"

// Preclarify for preventing the compiler error
class TrainWindow;
class CTrack;


//#######################################################################
// TODO
// You might change the TrainView in order to add different objects to
// be drawn, or adjust the lighting, or ...
//#######################################################################


// The TrainView also needs to handle its user interface events.
// since this is an FLTk Window, we need to include FlTk Headers
// the pragmas turn off the warnings from FlTk - our code should have no warnings,
// but their code doesn't meet that standard
#pragma warning(push)
#pragma warning(disable:4312)
#pragma warning(disable:4311)
#include <Fl/Fl_Gl_Window.h>
#pragma warning(pop)

// this uses the old ArcBall Code
#include "Utilities/ArcBallCam.H"
#include "TrainWindow.H"
#include<iostream>



class WaterFrameBuffers
{
public:
	GLuint REFLECTION_WIDTH = 590;
	GLuint REFLECTION_HEIGHT = 590;

	GLuint REFRACTION_WIDTH = 590;
	GLuint REFRACTION_HEIGHT = 590;

	GLuint reflectionFrameBuffer;
	GLuint reflectionTexture;
	GLuint reflectionDepthBuffer;
	Texture2D reflectionTexture2D;

	GLuint refractionFrameBuffer;
	GLuint refractionTexture;
	GLuint refractionDepthTexture;
	Texture2D refractionTexture2D;



	WaterFrameBuffers() {//call when loading the game
		initialiseReflectionFrameBuffer();
		initialiseRefractionFrameBuffer();
	}

	void cleanUp() {//call when closing the game
		glDeleteFramebuffers(1, &reflectionFrameBuffer);
		glDeleteTextures(1, &reflectionTexture);
		glDeleteRenderbuffers(1, &reflectionDepthBuffer);
		glDeleteFramebuffers(1, &refractionFrameBuffer);
		glDeleteTextures(1, &refractionTexture);
		glDeleteTextures(1, &refractionDepthTexture);
	}
	void bindReflectionFrameBuffer() {//call before rendering to this FBO
		bindFrameBuffer(reflectionFrameBuffer, REFLECTION_WIDTH, REFLECTION_HEIGHT);
		// glClearColor(1, 0, 0, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	void bindRefractionFrameBuffer() {//call before rendering to this FBO
		bindFrameBuffer(refractionFrameBuffer, REFRACTION_WIDTH, REFRACTION_HEIGHT);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	void unbindCurrentFrameBuffer() {//call to switch to default frame buffer
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, 590, 590);
	}

	GLuint getReflectionTexture() {//get the resulting texture
		return reflectionTexture;
	}

	GLuint getRefractionTexture() {//get the resulting texture
		return refractionTexture;
	}

	GLuint getRefractionDepthTexture() {//get the resulting depth texture
		return refractionDepthTexture;
	}

	void initialiseReflectionFrameBuffer() {
		reflectionFrameBuffer = createFrameBuffer();
		reflectionTexture = createTextureAttachment(REFLECTION_WIDTH, REFLECTION_HEIGHT);
		reflectionTexture2D.setID(reflectionTexture);
		reflectionDepthBuffer = createDepthBufferAttachment(REFLECTION_WIDTH, REFLECTION_HEIGHT);
		unbindCurrentFrameBuffer();
	}

	void initialiseRefractionFrameBuffer() {
		refractionFrameBuffer = createFrameBuffer();
		refractionTexture = createTextureAttachment(REFRACTION_WIDTH, REFRACTION_HEIGHT);
		refractionTexture2D.setID(refractionTexture);
		refractionDepthTexture = createDepthTextureAttachment(REFRACTION_WIDTH, REFRACTION_HEIGHT);
		unbindCurrentFrameBuffer();
	}

	void bindFrameBuffer(int frameBuffer, int width, int height) {
		glBindTexture(GL_TEXTURE_2D, 0);
		glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
		glViewport(0, 0, width, height);
	}

	GLuint createFrameBuffer() {
		GLuint frameBuffer;
		glGenFramebuffers(1, &frameBuffer);
		glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
		glDrawBuffer(GL_COLOR_ATTACHMENT0);
		return frameBuffer;
	}

	GLuint createTextureAttachment(int width, int height) {
		GLuint texture;
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, texture, 0);
		return texture;
	}

	GLuint createDepthTextureAttachment(int width, int height) {
		GLuint texture;
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
		glTextureParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, texture, 0);
		return texture;
	}

	GLuint createDepthBufferAttachment(int width, int height) {
		GLuint depthBuffer;
		glGenRenderbuffers(1, &depthBuffer);
		glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);
		return depthBuffer;
	}
};

class TrainView : public Fl_Gl_Window
{
	public:
		// note that we keep the "standard widget" constructor arguments
		TrainView(int x, int y, int w, int h, const char* l = 0);

		// overrides of important window things
		virtual int handle(int);
		virtual void draw();

		// all of the actual drawing happens in this routine
		// it has to be encapsulated, since we draw differently if
		// we're drawing shadows (no colors, for example)
		void drawStuff(bool doingShadows=false);

		// setup the projection - assuming that the projection stack has been
		// cleared for you
		void setProjection();

		// Reset the Arc ball control
		void resetArcball();

		// pick a point (for when the mouse goes down)
		void doPick();

		//drawTrack
		void drawTrack(bool doingShadow);

		//drawTrain
		void drawTrain(bool doingShadow, Pnt3f pos0, Pnt3f pos1, Pnt3f ori, bool head = false);

		//drawWheel
		void drawWheel();
		//void drawWheel(bool doingShadow, Pnt3f pos0, Pnt3f pos1, Pnt3f ori);

		//void drawModel();
		void drawTunnel();

		void drawCube(Pnt3f p0, Pnt3f p1, Pnt3f p2, Pnt3f p3);

		

	public:
		ArcBallCam		arcball;			// keep an ArcBall for the UI
		int				selectedCube;  // simple - just remember which cube is selected
		
		TrainWindow*	tw;				// The parent of this display window
		CTrack*			m_pTrack;		// The track of the entire scene


		//add by koach
		vector<Pnt3f> point_list;
		vector<Pnt3f> orient_list;
		int nonArcCnt = 0;
		int point_index = 0;
		int carAmount = 3;
		int carInterval = 30;
		float POINT_INTERVAL_LEN = 1.0f;
		float POINT_CNT_LEN = 0.0f;
		float RAIL_INTERVAL_LEN = 10.0f;
		float RAIL_WIDTH = 3.0f;
		float RAIL_CNT_LEN = 0.0f;
		float DIS = 0.0f;
		float DIVIDE_LINE = 1000.0f;

		// adaptive 
		bool adaptiveLock = false;
		bool firstLock = true;
		bool secondLock = true;
		Pnt3f lastTwoPoint;
		Pnt3f lastPoint;
		Pnt3f lastVec;
		Pnt3f nowVec;
		int cc = 0;


		/* shader */

		// skybox & cubeMap
		void initSkyboxShader();
		void drawSkybox();
		unsigned int loadCubemap(std::vector<std::string> faces);

		unsigned int cubemapTexture;
		Shader* skyboxShader = nullptr;
		Texture2D* skyBoxTexture = nullptr;
		unsigned int skyboxVAO;
		unsigned int skyboxVBO;

		/*
		proj & cam matrix 
		*/
		// set ubo
		void initUBO();
		void setUBO();
		UBO* commom_matrices = nullptr;
		glm::vec3 source_pos;



		/* 
		water 
		*/
		const float WATER_HEIGHT = 0.0f;

		// inverse
		GLfloat* inverse(GLfloat*);

		// fbo
		WaterFrameBuffers* fbos = nullptr;

		//campos
		glm::vec3 cameraPosition;
		glm::vec3 lightColor;
		glm::vec3 lightPosition;

		// new_view_matrix
		glm::mat4 new_view_matrix;

		Texture2D* texture = nullptr;
		//sine
		void initSineWater();
		void drawSineWater();

		Shader* sineWaterShader = nullptr;
		VAO* sineWater = nullptr;
		float sinWaterCounter = 0;
		float moveFactor = 0.0f;
		float WAVE_SPEED = 0.03f;
		float t_time = 0.0f;

		/*
		Monitor
		*/
		// Monitor
		void initMonitor();
		void drawMonitor(int);
		// Monitor
		Shader* monitorShader = nullptr;
		VAO* monitor = nullptr;
		Texture2D* monitorTexture = nullptr;

		// particle
		void initParticle();
		void drawParticle();
		//Shader* particleShader = nullptr;
		//VAO* particle = nullptr;
		unsigned int MaxParticles = 20;
		unsigned int ParticlesCount = 20;
		GLuint billboard_vertex_buffer;
		GLuint particles_position_buffer;
		GLuint particles_color_buffer;

};