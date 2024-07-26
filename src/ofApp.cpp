#include "ofApp.h"
#include "ofGraphicsUtil.h"

using namespace glm;
//--------------------------------------------------------------
//Shaders converted into string functions to more easily read code and edit this in the future
//In development I found this extremely useful as I knew where everything was and could segment this
//I think for beginner programmers or new programmers to shader code that segmenting the shader code
//Into respective functions is a great idea.
static std::string glslLighting() {
	return R"(
		struct Light {
			vec3 pos ;
			float strength ;
			float halfDist ;
			float ambient ;
			vec3 diffuse ;
			vec3 specular ;
		};
		struct Material {
			vec3 diffuse ;
			vec3 specular ;
			float shine ;
		};
		struct LightFall {
			vec3 diffuse ;
			vec3 specular ;
		};

		// In - place addition : a += b
		void addTo ( inout LightFall a , in LightFall b){
			a.diffuse += b.diffuse ;
			a.specular += b.specular ;
		}
		// Compute light components falling on surface
		LightFall computeLightFall ( vec3 pos , vec3 N , vec3 eye , in Light lt , in Material mt ){
			vec3 lightDist = lt.pos - pos ;
			float hh = lt.halfDist * lt.halfDist ;
			float atten = lt.strength * hh /( hh + dot ( lightDist , lightDist ));
			vec3 L = normalize ( lightDist );

			// diffuse
			float d = max ( dot (N , L) , 0.) ;
			d += lt.ambient ;

			// specular
			vec3 V = normalize ( eye - pos );
			vec3 H = normalize (L + V);
			float s = pow ( max ( dot (N , H) , 0.) , mt.shine );
			LightFall fall ;

			fall.diffuse = lt.diffuse *( d* atten );
			fall.specular = lt.specular *( s* atten );
			return fall ;
		}

		// Get final color reflected off material
		vec3 lightColor (in LightFall f , in Material mt ){
			return f.diffuse * mt.diffuse + f.specular * mt.specular ;
		}
)";
}
//--------------------------------------------------------------
static std::string glslVertexLighting()
{
	ofShader shader;
	return R"(
	uniform mat4 projectionMatrix;
	uniform mat4 viewMatrix;
	uniform mat4 modelMatrix;

	in vec4 position;
	in vec3 normal;
	in vec3 color;

	out vec3 vposition;
	out vec3 vnormal;
	out vec3 vcolor;

	in vec2 texcoord;
	out vec2 vtexcoord;

	void main() {
		vcolor = color;
		vnormal = normal;
		vtexcoord = texcoord;
		vposition = (modelMatrix * position).xyz;
		gl_Position = projectionMatrix * viewMatrix * vec4(vposition, 1);
	}
	)";
}
//--------------------------------------------------------------
static std::string glslFragmentLighting()
{
	ofShader shader;
	return R"(
		uniform vec3 eye ;
		uniform vec3 mouseLightColour;
		uniform sampler2D tex;
		uniform float textureScale; // added uniform for texture scaling
		uniform float opacity;
		in vec2 vtexcoord;
		in vec3 vposition ;
		in vec3 vnormal ;
		in vec3 vcolor ;
		out vec4 fragColor ;

		void main () {
			vec3 pos = vposition;
			vec3 normal = normalize(vnormal);
			Light light1;
			light1.pos = vec3(3.5, 2,.5);
			light1.strength = 10;
			light1.halfDist = 1;
			light1.ambient = 0.2;
			light1.diffuse = mouseLightColour;
			light1.specular = light1.diffuse;
			Material mtrl;
			mtrl.diffuse = vec3 (1.0);
			mtrl.specular = vec3 (0.5);
			mtrl.shine = 100.0;
			
			//Added second Light
			Light light2 = light1;
			light2.pos = vec3(0,2,.5);
			light2.diffuse = mouseLightColour;
			light2.specular = light2.diffuse;

			//Added third Light
			Light light3 = light1;
			light3.pos = vec3(-3.5,2,.5);
			light3.diffuse = mouseLightColour;
			light3.specular = light3.diffuse;

			//Calculating Light
			LightFall fall = computeLightFall ( pos , normal , eye , light1 , mtrl );
			//Add additional lighting to the calculations.
			addTo ( fall , computeLightFall ( pos , normal , eye , light2 , mtrl ));
			addTo ( fall , computeLightFall ( pos , normal , eye , light3 , mtrl ));
			vec2 scaledTexCoord = vtexcoord * textureScale;
			vec3 col = lightColor (fall ,mtrl);
			col = col *texture(tex, scaledTexCoord).rgb;
			fragColor = vec4(col,opacity);
		})";
}
//--------------------------------------------------------------
static ofShader buildPointLightShader()
{
	// Build shader (from GLSL code)
	ofShader shader;
	build(shader, R"(
	uniform mat4 modelViewProjectionMatrix ;
	in vec4 position ;
	in vec2 texcoord ;
	out vec2 spriteCoord ; // sprite coordinate in [ -1 ,1]
	uniform float spriteRadius ;
	uniform mat4 cameraMatrix ;
	
	void main () {
		spriteCoord = texcoord * 2. - 1.;
		vec4 pos = position ;
		switch ( gl_VertexID % 4) {
		case 0: spriteCoord = vec2 ( -1. , -1.) ; break ;
		case 1: spriteCoord = vec2 ( 1. , -1.) ; break ;
		case 2: spriteCoord = vec2 ( 1. , 1.) ; break ;
		case 3: spriteCoord = vec2 ( -1. , 1.) ; break ;
		}
		vec4 offset = vec4 ( spriteCoord * spriteRadius , 0. , 0.) ;
		offset = cameraMatrix * offset ;
		pos += offset ;
		gl_Position = modelViewProjectionMatrix * pos ;

	}

	)", R"(
		// Fragment program
		in vec2 spriteCoord ; // sprite coordinate in [ -1 ,1]
		out vec4 fragColor;
		uniform float r,g,b;
		void main(){
			vec3 col = vec3(r,g,b);
			float rsqr = dot ( spriteCoord , spriteCoord );
			if( rsqr > 1.) discard ;
			float a = 1. - rsqr ; // inverted parabola
			a *= a; // smooth ends
			col *= a;
			fragColor = vec4 ( col , 1.) ;
		}
	)");
	return shader;
}
//--------------------------------------------------------------
//Handling point sprite creation
static ofMesh addRect(glm::vec3 pos, float w = 2.f, float h = 2.f) {
	float w_2 = w * 0.5;
	float h_2 = h * 0.5;
	ofMesh m;
	int Nv = m.getVertices().size();
	m.setMode(OF_PRIMITIVE_TRIANGLES);
	m.addVertex(pos + vec3(-w_2, -h_2, 0.));
	m.addVertex(pos + vec3(w_2, -h_2, 0.));
	m.addVertex(pos + vec3(w_2, h_2, 0.));
	m.addVertex(pos + vec3(-w_2, h_2, 0.));
	m.addTriangle(Nv + 0, Nv + 1, Nv + 2);
	m.addTriangle(Nv + 0, Nv + 2, Nv + 3);
	return m;

}
//--------------------------------------------------------------
//Handling room creation
auto addQuad(ofMesh& m, glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec3 d, ofFloatColor col = ofFloatColor(0)) -> void
{
	m.addVertex(a);
	m.addVertex(b);
	m.addVertex(c);
	m.addVertex(d);

	m.addTexCoord(vec2(1, 1));
	m.addTexCoord(vec2(0, 1));
	m.addTexCoord(vec2(0, 0));
	m.addTexCoord(vec2(1, 0));

	m.addTriangle(0, 1, 2);
	m.addTriangle(0, 2, 3);
};
//--------------------------------------------------------------
void ofApp::createRandLights(float amountOfLights)
{
	for (int i = 0; i < amountOfLights; ++i) {
		auto p = vec3(ofRandom(-8, 8), ofRandom(.1, 2), ofRandom(0, 4));
		MeshStruct _mesh;
		_mesh.mesh = (addRect(p, 0, 0));
		_mesh.size = (ofRandom(.05, .15));
		_mesh.r = ofRandom(.1);
		_mesh.g = _mesh.r;
		_mesh.b = _mesh.r;
		mesh.push_back(_mesh);
	}
}
//--------------------------------------------------------------
void ofApp::setup() {
	// Setup camera (for 3D rendering)
	cam.setPosition(vec3(0., 0., 2.));
	cam.setNearClip(0.05);
	cam.setFarClip(100.);
	// Other setup for 3D rendering
	ofEnableDepthTest();
	ofDisableArbTex();
	ofEnableNormalizedTexCoords();
	// Build shader (from GLSL code)
	build(shader, glslVertexLighting(), glslLighting() + glslFragmentLighting());
	addQuad(quadSideBackwall, vec3(-4, 0, 0), vec3(4, 0, 0), vec3(4, 2, 0), vec3(-4, 2, 0));
	addQuad(quadSideFloor, vec3(-4, 0, 0), vec3(4, 0, 0), vec3(4, 0, 2), vec3(-4, 0, 2));

	if (!image.load("stonewall.png"))
		std::cout << " Error loading image from file ." << std::endl;
	if (!image2.load("slimeCube.png"))
		std::cout << " Error loading image from file ." << std::endl;
	
	randLightShader = buildPointLightShader();
	createRandLights(50);
	piApprox = 3927 / 1250;
}
//--------------------------------------------------------------
ofBoxPrimitive ofApp::SlimeAnimation(float amp, float freq,float yPlacement,float _scale)
{
	ofPushMatrix();
		ofBoxPrimitive box{ _scale,_scale,_scale };
		ofPushMatrix();
				float scale = amp * sin(2 * piApprox * freq * deltaTime) + 2;
				//Places box at floor whilst being scaled
				ofTranslate(0, -box.getHeight() / 2 + yPlacement, 0); // Move Down
				//Scales Height and Width Differently to create a bouncing / squishing effect
				ofScale(scale, 3/scale, scale);
				ofTranslate(0, box.getHeight() / 2, 0); // Move Up
				box.draw();

		ofPopMatrix();  
	ofPopMatrix();
	return box;
}
//--------------------------------------------------------------
void ofApp::update() {
	//Map  mouse position to colours from 0-width/height to 0,1
	if (changingLighting)
	{
		mouseLightColour.x = ofMap(mouseX, 0, ofGetWidth(), 0, 1); // red
		mouseLightColour.y = ofMap(mouseY, 0, ofGetHeight(), 0, 1); // green
		if (lightingMode == 1)
			mouseLightColour.z = 0; // Constant value for the Value component // blue
		else if (lightingMode == 2) mouseLightColour.z = ofMap(mouseX, 0, ofGetWidth(), 0, 1);
		else if (lightingMode == 3) mouseLightColour.z = ofMap(mouseY, 0, ofGetHeight(), 0, 1);
	}
}
//--------------------------------------------------------------
void ofApp::drawRandParticles()
{
	//Rendering transparent particles
	glDepthMask(GL_FALSE);
	ofEnableBlendMode(OF_BLENDMODE_ADD);

	randLightShader.begin();
	float amp = .2, freq = .05, phase = 0, valueMinus = .2f;
	for (int i = 0; i < mesh.size(); i++)
	{
		ofSeedRandom(i);
		auto f_randomOffset = ofRandom(0,3);
		auto a_randomOffset = ofRandom(0, .1);
		phase = ofRandom(0, 100);
		//Sin animation for the opacity of the dust particles
		auto progressiveLightAnimation = ((amp+a_randomOffset) * sinf(2*piApprox * (freq + f_randomOffset) * deltaTime+ phase) + 1 )/ 2;
		progressiveLightAnimation -= valueMinus;
		randLightShader.setUniformMatrix4f("cameraMatrix", cam.getLocalTransformMatrix());
		randLightShader.setUniform1f("spriteRadius", mesh[i].size);
		randLightShader.setUniform1f("r", progressiveLightAnimation);
		randLightShader.setUniform1f("g", progressiveLightAnimation);
		randLightShader.setUniform1f("b", progressiveLightAnimation);
		vec3 colOffset;
		randLightShader.setUniform1f("appTime", ofGetFrameNum() / ofGetTargetFrameRate());
		f_randomOffset = ofRandom(0, 1);
		a_randomOffset = ofRandom(0, .1);
		//Controlling the Z animation for the movement of the particles
		auto zAnimation = ((.1 + a_randomOffset) * sinf(2 * piApprox * (freq + f_randomOffset) * deltaTime + phase));
		ofPushMatrix();
		ofTranslate(0, zAnimation, 0);
		mesh[i].mesh.draw();
		ofPopMatrix();
	}
	randLightShader.end();
	ofDisableBlendMode();
	glDepthMask(GL_TRUE);
}
//--------------------------------------------------------------
void ofApp::draw() {
	deltaTime += ofGetLastFrameTime();
	cam.begin();
	shader.begin();
	//Draw walls
	shader.setUniform3f("eye", cam.getPosition());
	shader.setUniformTexture("tex", image.getTexture(), 0);
	shader.setUniform1f("textureScale", .5);
	shader.setUniform3f("mouseLightColour", mouseLightColour);
	quadSideBackwall.draw();
	quadSideFloor.draw();
	ofScale(.5);
	//Draw Cube
	shader.setUniform1f("textureScale", 1);
	shader.setUniformTexture("tex", image2.getTexture(), 0);

	//In the future this should be changed into a function or lambda.
	//This is too many lines of code however I do not know the best way to go around this.
	//I will ask Lance for feedback of how to convert this into less code whilst creating the same result
	//My initial thoughts are to make a void function however it would have many arguments to pass in creating unreadable and
	//hard to use code.
	shader.setUniform1f("opacity", 0.5);
	ofPushMatrix();
	ofTranslate(slimeMovementX, 0, 0);
	ofTranslate(3, 0,2);
	SlimeAnimation();
	ofPopMatrix();

	ofPushMatrix();
	ofTranslate(slimeMovementX, 0, 0);
	ofTranslate(1.5, 0, 3.5);
	SlimeAnimation(.5,.4,.125,.25);
	ofPopMatrix();

	ofPushMatrix();
	ofTranslate(slimeMovementX, 0, 0);
	ofTranslate(-4, 0, 2);
	SlimeAnimation(.6, .2, .75, 1.5);
	ofPopMatrix();

	shader.end();
	//Draw Particles
	drawRandParticles();
	cam.end();
}
//--------------------------------------------------------------
void ofApp::keyPressed(int key) {
	if (key == OF_KEY_LEFT &&slimeMovementX > -8)
	{
		slimeMovementX -= .01;
	}
	if (key == OF_KEY_RIGHT && slimeMovementX < 8)
	{
		slimeMovementX += .01;
	}
	if (key == '1')
		lightingMode = 1;
	if (key == '2')
		lightingMode = 2;
	if (key == '3')
		lightingMode = 3;
	if (key == 'q')
		changingLighting = !changingLighting;
}
//--------------------------------------------------------------
void ofApp::keyReleased(int key) {

}
//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y) {

}
//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button) {

}
//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button) {

}
//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button) {

}
//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y) {

}
//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y) {

}
//--------------------------------------------------------------
void ofApp::windowResized(int w, int h) {

}
//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg) {

}
//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo) {

}
