#include "ofApp.h"

#include "ofxPubSubOsc.h"

using namespace ofxCv;
using namespace cv;

void ofApp::setup() {
    ofBackground(0);
	camera.setDeviceID(0);
	camera.setup(320, 240);
	
    gui.setup();
    
    gui.add(lkMaxLevel.set("lkMaxLevel", 3, 0, 8));
    gui.add(lkMaxFeatures.set("lkMaxFeatures", 200, 1, 1000));
    gui.add(lkQualityLevel.set("lkQualityLevel", 0.01, 0.001, .02));
    gui.add(lkMinDistance.set("lkMinDistance", 4, 1, 16));
    gui.add(lkWinSize.set("lkWinSize", 8, 4, 64));
    gui.add(usefb.set("Use Farneback", false));
	gui.add(fbPyrScale.set("fbPyrScale", .5, 0, .99));
	gui.add(fbLevels.set("fbLevels", 4, 1, 8));
    gui.add(fbIterations.set("fbIterations", 2, 1, 8));
    gui.add(fbPolyN.set("fbPolyN", 7, 5, 10));
    gui.add(fbPolySigma.set("fbPolySigma", 1.5, 1.1, 2));
    gui.add(fbUseGaussian.set("fbUseGaussian", false));
    gui.add(fbWinSize.set("winSize", 32, 4, 64));
	gui.add(flowPower.set("FlowPower", 0., 0., 100.));
	gui.add(playBack.set("PlayBack", 0., 0., 1000.));
	gui.add(filter.set("filter", 0.1, 0., 1.));

	ofxPublishOsc("localhost", 9000, "/playback", flowPower);

	curFlow = &fb;
}

void ofApp::update(){
	camera.update();
	
	if(camera.isFrameNew()) {
		
		if(usefb) {
			curFlow = &fb;
			fb.setPyramidScale(fbPyrScale);
			fb.setNumLevels(fbLevels);
			fb.setWindowSize(fbWinSize);
			fb.setNumIterations(fbIterations);
			fb.setPolyN(fbPolyN);
			fb.setPolySigma(fbPolySigma);
            fb.setUseGaussian(fbUseGaussian);
		} else {
			curFlow = &lk;
			lk.setMaxFeatures(lkMaxFeatures);
			lk.setQualityLevel(lkQualityLevel);
			lk.setMinDistance(lkMinDistance);
			lk.setWindowSize(lkWinSize);
			lk.setMaxLevel(lkMaxLevel);
		}
		
		// you can use Flow polymorphically
		curFlow->calcOpticalFlow(camera);
		float fp = flowPower;
		if (usefb) {
			flowPower = 0;
			int count = 0;
			auto motion = fb.getAverageFlow();
			flowPower = motion.length();

		}
		else {
			flowPower = 0;
			int count = 0;
			for (int i = 0; i < lk.getMotion().size(); i++) {
				auto m = lk.getMotion().at(i);
				float mm = m.length();
				if (0 < mm && mm < 200) {
					count++;
					flowPower += m.length();
				}
			}
			if(count > 0)
				flowPower /= count;
		}
		flowPower = flowPower * filter + fp * (1 - filter);
		playBack += flowPower * 100;
		if (playBack > playBack.getMax() - 0.01f) playBack = 0;

	}
}

void ofApp::draw(){
    ofPushMatrix();
    ofTranslate(250, 100);
	camera.draw(0,0,640,480);
    curFlow->draw(0,0,640,480);
    ofDrawBitmapStringHighlight(ofToString((int) ofGetFrameRate()) + "fps", 10, 20);
    ofPopMatrix();
    gui.draw();
	ofDrawRectangle(0, ofGetHeight() - 200, ofMap(playBack, 0, playBack.getMax(), 0, ofGetWidth()), 200);
}