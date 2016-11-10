#include <iostream>
#include <unistd.h>
#include "dsgl.hpp"
#include "dsglMeshes.hpp"
#include "dsglPng.hpp"
#include <cmath>
#define N 8
#define EPSILON 0.25

DSGL::Images::rgb32f imgMax(DSGL::Images::rgb8 * img, unsigned int size) {
	DSGL::Images::rgb32f max = {0,0,0};

	for (int i = 0; i < size; i++) {
		if (img[i].Red > max.Red) max.Red = img[i].Red;
		if (img[i].Green > max.Green) max.Green = img[i].Green;
		if (img[i].Blue > max.Blue) max.Blue = img[i].Blue;
	}

	return max;
}

int findClosest(DSGL::Images::rgb8 pixel, DSGL::Images::rgb32f * seeds) {
	
	float minDistance = 16581375.0;
	int closest=0;
	float tmp;
	for (int i=0; i< N; i++) {
		tmp = sqrt(
			pow(abs(pixel.Red - seeds[i].Red),2) +
			pow(abs(pixel.Green - seeds[i].Green),2) +
			pow(abs(pixel.Blue - seeds[i].Blue),2)
		);
		if (tmp < minDistance) {
			closest=i;
			minDistance = tmp;
		}
	}
	return closest;
}

int main(int argc, char ** argv) {
	DSGL::Images::Png png("2013_-_Denis_Salem_-_CC_By_SA_-_Fernando_Omeara_0x06_720p.png");

	DSGL::Images::rgb8 * pixels = (DSGL::Images::rgb8 *) png.rawData;

	DSGL::Images::rgb32f q = imgMax(pixels, png.Width() * png.Height());
	q.Red /= N;
	q.Green /= N;
	q.Blue /= N;

	DSGL::Images::rgb32f * seeds = new DSGL::Images::rgb32f[N]();
	DSGL::Images::rgba32f * newSeeds = new DSGL::Images::rgba32f[N]();

	float old = 0;
	float current = 0;
	for (int i=0; i < N; i++) {
		seeds[i].Red = q.Red * (i+1);
		seeds[i].Green = q.Green *(i+1);
		seeds[i].Blue = q.Blue *(i+1);
		old += sqrt(
			pow(abs(seeds[i].Red),2) +
			pow(abs(seeds[i].Green),2) +
			pow(abs(seeds[i].Blue),2)
		);
	}

	old /=N;

	int seedIndex = 0;

	// Main iteration loop
	while(true) {
		// Sum weight
		for (int i = 0; i < png.Width() * png.Height(); i++) {
			seedIndex = findClosest(pixels[i], seeds);
			newSeeds[seedIndex].Red += pixels[i].Red;
			newSeeds[seedIndex].Green += pixels[i].Green;
			newSeeds[seedIndex].Blue += pixels[i].Blue;
			newSeeds[seedIndex].Alpha += 1;
		}

		// compute mean points
		for (int i = 0; i < N; i++) {
			seeds[i].Red = newSeeds[i].Red / newSeeds[i].Alpha;
			seeds[i].Green = newSeeds[i].Green / newSeeds[i].Alpha;
			seeds[i].Blue = newSeeds[i].Blue / newSeeds[i].Alpha;
			current += sqrt(
				pow(abs(seeds[i].Red),2) +
				pow(abs(seeds[i].Green),2) +
				pow(abs(seeds[i].Blue),2)
			);
		}
		current /= N;

		// Reset new seeds
		for (int i = 0; i < N; i++) {
			newSeeds[i].Red = 0;
			newSeeds[i].Green = 0;
			newSeeds[i].Blue = 0;
			newSeeds[i].Alpha = 1;
		}

		if (abs(current - old) < EPSILON) {
			break;
		}
		old = current;
		current = 0;
	}

	// Finally Quantize output image
	for (int i = 0; i < png.Width() * png.Height(); i++) {
		seedIndex = findClosest(pixels[i], seeds);
		pixels[i].Red = seeds[seedIndex].Red;
		pixels[i].Green = seeds[seedIndex].Green;
		pixels[i].Blue = seeds[seedIndex].Blue;
	}


	/* OpenGL context */

	DSGL::Context context("LLOYD-MAX WITH DSGL", png.Width(), png.Height(), 4, 3);
	context.InitSimpleWindow();
	
	/* Regular shaders */
	DSGL::PipelineProgram pipelineProgram("vertex.shader","fragment.shader");

	/* DSGL provide built in simple quad for playing with and debugging. */
	DSGL::Meshes::Quad quad;



	/* Create buffers */
	DSGL::Textures texture(GL_TEXTURE_2D, png.Width(), png.Height(), png.rawData, png.GetFormat(), png.GetType());
	DSGL::Elements elements(4 * sizeof(GLuint), quad.index);
	DSGL::VertexBufferObject VBO(sizeof(GLfloat) * 12, quad.vertex);
	DSGL::VertexBufferObject texCoords(sizeof(GLfloat) * 8, quad.texCoords);
	DSGL::VertexArrayObject VAO(elements.ID, VBO.ID);

	/* Set up how vertex related memory is organized */
	VAO.AttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 0);

	/* Set up how texcoords is organized in memory */
  	VAO.AttribPointer(texCoords.ID, 1, 2, GL_FLOAT, GL_FALSE, 2*sizeof(GLfloat), 0);
	glEnable (GL_BLEND); glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	/* ----- Render loop ----- */
	while(!glfwWindowShouldClose(context.window)) {
		glfwSwapBuffers(context.window);

		glfwPollEvents();

    		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		/* ----- Actual render ----- */
    
		pipelineProgram.Use();
    
		VAO.Bind();
    
		texture.Bind();
    
		glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_INT, (GLvoid *) 0);
		
		usleep(40000); /* Let the GPU take a breath */
    
		texture.Unbind();
            
		glBindVertexArray(0);
    
		glUseProgram(0);
	}

	delete[] seeds;
	delete[] newSeeds;
	return DSGL_END_NICELY;
}
