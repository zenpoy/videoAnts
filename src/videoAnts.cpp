#include "videoAnts.h"
#include <cmath>


//--------------------------------------------------------------
void testApp::setup(){

#ifdef _USE_LIVE_VIDEO
	vidGrabber.setVerbose(true);
	vidGrabber.initGrabber(320,240);
#else
	vidPlayer.loadMovie("fingers.mov");
	vidPlayer.play();
#endif

	colorImg.allocate(320,240);
	grayImage.allocate(320,240);
	pixel.allocate(1,1);

	sizeOfAnt = 6;
	min_x = 100;
	max_x = 900;
	min_y = 50;
	max_y = 700;
	max_radius = 0.0025;
	color_distance_factor = 1;

	mesh.clear();
	mesh.setMode(OF_PRIMITIVE_TRIANGLES);
	for(int i=0; i<NUM_OF_ANTS; i += 3) {
			ofVec2f p = ofVec2f(ofRandom(0,1),ofRandom(0,1));
			ofVec2f p1 = p;
			ofVec2f p2 = p;

			mesh.addVertex(p);
			mesh.addVertex(p1);
			mesh.addVertex(p2);

			ofColor c = ofColor(ofRandom(0,255),ofRandom(0,255),ofRandom(0,255), 64);

			mesh.addColor(c);
			mesh.addColor(c);
			mesh.addColor(c);
			
			velocities[i] = ofVec2f(0, 0);
			velocities[i + 1] = ofVec2f(0, 0);
			velocities[i + 2] = ofVec2f(0, 0);
			
			rgb_velocities[i] = ofVec3f(0, 0, 0);
			rgb_velocities[i + 1] = ofVec3f(0, 0, 0);
			rgb_velocities[i + 2] = ofVec3f(0, 0, 0);
	}

	camera.setupPerspective();
	
	frame = 1;
	isFullScreen = false;
	isChangeColor = true;
}

//--------------------------------------------------------------
void testApp::update(){
	glPointSize(sizeOfAnt);     
	ofBackground(0,0,0);

	bool bNewFrame = false;

#ifdef _USE_LIVE_VIDEO
	vidGrabber.update();
	bNewFrame = vidGrabber.isFrameNew();
#else
	vidPlayer.update();
	bNewFrame = vidPlayer.isFrameNew();
#endif

	frame++;
	frame%=10000;
	camera.rotate(cosf(frame / 10.0f),sinf(frame / 21.0f),cosf(frame / 22.0f),sinf(frame / 23.0f));

	if (bNewFrame){

#ifdef _USE_LIVE_VIDEO
		colorImg.setFromPixels(vidGrabber.getPixels(), 320,240);
#else
		colorImg.setFromPixels(vidPlayer.getPixels(), 320,240);
#endif
		grayImage = colorImg;
	}
	colorImg.resetROI();

	ofVec3f *vertices = mesh.getVerticesPointer();
	ofFloatColor *colors = mesh.getColorsPointer();
	ofPixelsRef colorImgPixels = colorImg.getPixelsRef();
	
	#pragma omp parallel for
	for(int i=0;i<NUM_OF_ANTS;i += 3) {
		ofVec2f p = vertices[i];

		float distance = 3;


		if (bNewFrame) {
			ofFloatColor c = colorImgPixels.getColor(ofMap(p.x, 0, 1, 0, 320), ofMap(p.y, 0, 1, 0, 240));
			ofVec3f c1 = ofVec3f(ofClamp(c.r,0,1), ofClamp(c.g,0,1), ofClamp(c.b,0,1));
			ofVec3f c2 = ofVec3f(colors[i].r, colors[i].g, colors[i].b);
		
			distance = c1.distance(c2);

			ofVec3f old_c2 = c2;
			
			
			c2 += rgb_velocities[i];
			
			
			rgb_velocities[i] = (c2 - old_c2);

			float r_distance = abs(c2.x-c1.x);
			float g_distance = abs(c2.y-c1.y);
			float b_distance = abs(c2.z-c1.z);
			
			rgb_velocities[i].x += ofRandom(-r_distance,r_distance);
			rgb_velocities[i].y += ofRandom(-g_distance,g_distance);
			rgb_velocities[i].z += ofRandom(-b_distance,b_distance);

			rgb_velocities[i] *= color_distance_factor * distance;


			ofVec2f old_p = p;
			

			p += velocities[i];

			velocities[i] = (p - old_p);
			
			velocities[i].x += ofRandom(-max_radius,max_radius);
			velocities[i].y += ofRandom(-max_radius,max_radius);

			velocities[i] *= distance;



			if (p.x >= 1){
				p.x = p.x - (float)(int)p.x;
				velocities[i] *= 0.5;
				//velocities[i] = velocities[i].rotate(velocities[i].normalize().dot(ofVec2f(1,0)) * -1 + 90);
			}
			if (p.x < 0){
				p.x = p.x + (float)(int)(-p.x) + 1.0f;
				velocities[i] *= 0.5;
				//velocities[i] = velocities[i].rotate(velocities[i].normalize().dot(ofVec2f(1,0)) * -1 - 90);
			}

			if (p.y >= 1) {
				p.y =  p.y - (float)(int)p.y;
				velocities[i] *= 0.5;
				//velocities[i] = velocities[i].rotate(velocities[i].normalize().dot(ofVec2f(0,1)) * -1 - 90);
			}
			if (p.y < 0) {
				p.y =  p.y + (float)(int)(-p.y) + 1.0f;
				velocities[i] *= 0.5;
				//velocities[i] = velocities[i].rotate(velocities[i].normalize().dot(ofVec2f(0,1)) * -1 + 90);
			}


			ofVec2f p1 = p - velocities[i] + velocities[i].perpendiculared() *  velocities[i].length() * 0.1f; 
			ofVec2f p2 = p - velocities[i] + velocities[i].perpendiculared() * -velocities[i].length() * 0.1f;

			ofVec3f p3_0(p.x, p.y, distance);
			ofVec3f p3_1(p1.x, p1.y, vertices[i].z);
			ofVec3f p3_2(p2.x, p2.y, vertices[i].z);

			//mesh.setVertex(i, p3);
			#pragma omp critical
			{
				if (isChangeColor) {
					ofFloatColor c = ofFloatColor(ofClamp(c2.x,0,1),ofClamp(c2.y,0,1),ofClamp(c2.z,0,1), 0.3);
					colors[i] = c;
					colors[i + 1] = ofFloatColor(old_c2.x, old_c2.y, old_c2.z, 0.3);
					colors[i + 2] =  colors[i + 1];
				}
				vertices[i] = p3_0;
				vertices[i + 1] = p3_1;
				vertices[i + 2] = p3_2;
			}
		}	
	}
	mesh.setColor(0, mesh.getColor(0));
	mesh.setVertex(0, mesh.getVertex(0));
}

//--------------------------------------------------------------
void testApp::draw(){
	if (isFullScreen) {
		ofToggleFullscreen();
		isFullScreen = false;
	}

	// draw the incoming, and the grayscale
	ofSetHexColor(0xffffff);
	//colorImg.draw(20,20);
	//grayImage.draw(360,20);

	//draw our ants
	camera.begin();

	ofEnableAlphaBlending();
	ofEnablePointSprites();
	
	ofPushMatrix();
	ofTranslate(min_x, min_y, 1);
	ofScale(max_x, max_y, 200);
		mesh.drawFaces();
		mesh.drawVertices();
	ofPopMatrix();

	camera.end();

	ofDisablePointSprites();


	// finally, a report:
	ofSetHexColor(0xffffff);
	char reportStr[1024];
	sprintf(reportStr, "Press 'f' to toggle full screen\n\
					   Press 'q' / 'a' to change the size of ants. Current is: %d \n\
					   Press 'w' / 's' to change max radius. Current is: %f \n\
					   Press 'e' / 'd' to change the color distance factor. Current is: %f\n\
					   Press 'l' to toggle color change\n\
					   fps: %.3f"
					   , sizeOfAnt, max_radius, color_distance_factor, ofGetFrameRate());
	ofDrawBitmapString(reportStr, 20, 600);
}

//--------------------------------------------------------------
void testApp::keyPressed(int key){
	#pragma omp master
	switch (key){
	case 'q':
		sizeOfAnt ++;
		if (sizeOfAnt > 250) sizeOfAnt = 250;
		break;
	case 'a':
		sizeOfAnt --;
		if (sizeOfAnt < 1) sizeOfAnt = 1;
		break;
	case 'f':
		isFullScreen = !isFullScreen;
		break;
	case 's':
		max_radius*=0.99;
		if (max_radius <= 0)
			max_radius = 0;
		break;
	case 'w':
		max_radius*=1.01;
		break;
	case 'e':
		color_distance_factor *= 1.001;
		break;
	case 'd':
		color_distance_factor *= 0.999;
		break;
	case 'y':
		camera.rotate(0.1f, 1, 0, 0);
		break;
	case 'h':
		camera.rotate(-0.1f, 1, 0, 0);
		break;
	case 'g':
		camera.rotate(0.1f, 0, 1, 0);
		break;
	case 'j':
		camera.rotate(-0.1f, 0, 1, 0);
		break;
	case 'l':
		isChangeColor = !isChangeColor;
		break;
	}
}

//--------------------------------------------------------------
void testApp::keyReleased(int key){

}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void testApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void testApp::dragEvent(ofDragInfo dragInfo){ 

}
