#pragma once

#include "ofMain.h"
using namespace glm;
class ofApp : public ofBaseApp{

	//No access from other files needed everything can be private.
private:
		void setup();
		void update();
		void draw();
		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void mouseEntered(int x, int y);
		void mouseExited(int x, int y);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
		ofBoxPrimitive SlimeAnimation(float amp = 0.5, float freq = 0.5, float yPlacement = 0.5, float scale = 1);
		void buildQuad(ofMesh& mesh, vec3 a, vec3 b, vec3 c, vec3 d);
		void drawRandParticles();
		float piApprox;
		int amountOfSlats = 20;
		ofEasyCam cam;
		ofShader shader;
		ofShader randLightShader;
		ofImage image;
		ofImage image2;
		ofImage image3;
		ofMesh quadSideRight;
		ofMesh quadSideLeft;
		ofMesh quadSideBackwall;
		ofMesh quadSideCeeling;
		ofMesh quadSideFloor;
		struct MeshStruct
		{
			ofMesh mesh;
			float size;
			float r, g, b;
		};
		vector<MeshStruct> mesh;
		float deltaTime;
		void createRandLights(float amountOfLights);
		float minecartPositionX;
		float slimeMovementX;
		vec3 mouseLightColour;
		int lightingMode = 1;
		bool changingLighting = true;
};
