#pragma once



#include "ofMain.h"

#include "ofxOpenCv.h"

#define _USE_LIVE_VIDEO		// uncomment this to use a live camera
								// otherwise, we'll use a movie file

#define NUM_OF_ANTS 420000

class testApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();
		
		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);		

		#ifdef _USE_LIVE_VIDEO
		  ofVideoGrabber 		vidGrabber;
		#else
		  ofVideoPlayer 		vidPlayer;
		#endif

		ofxCvColorImage			colorImg;
		ofxCvGrayscaleImage 	grayImage;

		ofVboMesh mesh;

		ofVec2f velocities[NUM_OF_ANTS];
		ofVec3f rgb_velocities[NUM_OF_ANTS];

		int 				threshold;
		bool				bLearnBakground;
		int				sizeOfAnt;
		int min_x;
		int max_x;
		int min_y;
		int max_y;
		bool isFullScreen;
		float max_radius;
		float color_distance_factor;
		ofxCvColorImage pixel;
		ofCamera camera;
		int frame;

};

