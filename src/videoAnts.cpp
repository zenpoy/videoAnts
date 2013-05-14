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
	max_x = 660;
	min_y = 100;
	max_y = 500;
	max_radius = 0.0025;
	color_distance_factor = 1;

	mesh.clear();
	mesh.setMode(OF_PRIMITIVE_POINTS);
	for(int i=0;i<NUM_OF_ANTS;i++) {
		ofVec2f p = ofVec2f(ofRandom(0,1),ofRandom(0,1));
		mesh.addVertex(p);
		mesh.addColor(ofColor(ofRandom(0,255),ofRandom(0,255),ofRandom(0,255), 64));
		velocities[i] = ofVec2f(0, 0);
		rgb_velocities[i] = ofVec3f(0, 0, 0);
	}

	camera.setupPerspective();
	
	frame = 1;
	isFullScreen = false;
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

	

	for(int i=0;i<NUM_OF_ANTS;i++) {
		ofVec2f p = mesh.getVertex(i);

		float distance = 3;

		if (bNewFrame) {
			colorImg.setROI(ofMap(p.x, 0, 1, 0, 320), ofMap(p.y, 0, 1, 0, 240), 2, 2);
			pixel = colorImg.getRoiPixels();

			ofFloatColor c = pixel.getPixelsRef().getColor(0,0);
			ofVec3f c1 = ofVec3f(powf(ofClamp(c.r,0,1), 1.5), powf(ofClamp(c.g,0,1),1.5), powf(ofClamp(c.b,0,1), 1.5));
			ofVec3f c2 = ofVec3f(mesh.getColor(i).r, mesh.getColor(i).g, mesh.getColor(i).b);


			colorImg.resetROI();
			
			distance = c1.distance(c2);

			ofVec3f old_c2 = c2;
			
			
			c2 += rgb_velocities[i];
			mesh.setColor(i, ofFloatColor(ofClamp(c2.x,0,1),ofClamp(c2.y,0,1),ofClamp(c2.z,0,1), 0.3));
			
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
				p.x = 1;
				velocities[i] = velocities[i].rotate(velocities[i].normalize().dot(ofVec2f(1,0)) * -1 + 90) * 1.5;
			}
			if (p.x < 0){
				p.x = 0;
				velocities[i] = velocities[i].rotate(velocities[i].normalize().dot(ofVec2f(1,0)) * -1 - 90) * 1.5;
			}

			if (p.y >= 1) {
				p.y =  1;
				velocities[i] = velocities[i].rotate(velocities[i].normalize().dot(ofVec2f(0,1)) * -1 - 90) * 1.5;
			}
			if (p.y < 0) {
				p.y =  0;
				velocities[i] = velocities[i].rotate(velocities[i].normalize().dot(ofVec2f(0,1)) * -1 + 90) * 1.5;
			}

			ofVec3f p3(p.x, p.y, distance);

			mesh.setVertex(i, p3);
		}		
	}
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
	ofTranslate(min_x, min_y);
	ofScale(max_x, max_y, 300);
		mesh.drawFaces();
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
					   fps: %.3f"
					   , sizeOfAnt, max_radius, color_distance_factor, ofGetFrameRate());
	ofDrawBitmapString(reportStr, 20, 600);
}

//--------------------------------------------------------------
void testApp::keyPressed(int key){

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
