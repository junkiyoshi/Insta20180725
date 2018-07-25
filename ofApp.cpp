#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup() {

	ofBackground(39);
	ofSetWindowTitle("openframeworks");

	this->cap.open("D:\\video\\image.mp4");
	this->cap_size = cv::Size(640, 360);
	ofSetFrameRate(this->cap.get(CV_CAP_PROP_FPS));

	this->box2d.init();
	this->box2d.setGravity(0, 0);
	this->box2d.createBounds();
	this->box2d.setFPS(30);
	this->box2d.registerGrabbing();

	this->rect_size = 20;
	for (int x = 0; x < this->cap_size.width; x += this->rect_size) {

		for (int y = 0; y < this->cap_size.height; y += this->rect_size) {

			ofPoint start_point(x + ofGetWidth() * 0.5 - this->cap_size.width * 0.5, y + ofGetHeight() * 0.5 - this->cap_size.height * 0.5);

			this->rects.push_back(shared_ptr<ofxBox2dRect>(new ofxBox2dRect));
			this->rects.back().get()->setPhysics(1.0, 0.5, 5.0);
			this->rects.back().get()->setup(this->box2d.getWorld(), start_point.x, start_point.y, this->rect_size, this->rect_size);

			unique_ptr<ofImage> image(new ofImage());
			image->allocate(this->rect_size, this->rect_size, OF_IMAGE_COLOR);
			cv::Mat frame = cv::Mat(cv::Size(image->getWidth(), image->getHeight()), CV_MAKETYPE(CV_8UC3, image->getPixels().getNumChannels()), image->getPixels().getData(), 0);
			this->images.push_back(std::move(image));
			this->frames.push_back(frame);

			cv::Rect rect = cv::Rect(x, y, this->rect_size, this->rect_size);
			this->cv_rects.push_back(rect);

			this->start_point.push_back(start_point);
		}
	}

	this->fbo.allocate(ofGetWidth(), ofGetHeight());
}

//--------------------------------------------------------------
void ofApp::update() {

	cv::Mat src, frame;
	this->cap >> src;
	if (src.empty()) {

		cap.set(CV_CAP_PROP_POS_FRAMES, 1);
		return;
	}

	cv::resize(src, frame, this->cap_size);
	cv::cvtColor(frame, frame, CV_BGR2RGB);


	for (int i = 0; i < this->rects.size(); i++) {

		ofVec2f distance = this->start_point[i] - this->rects[i]->getPosition();
		this->rects[i]->addForce(distance, 0.01);

		cv::Mat tmp_box_image(frame, this->cv_rects[i]);
		tmp_box_image.copyTo(this->frames[i]);
		this->images[i]->update();
	}

	this->box2d.update();

	this->fbo.begin();
	ofClear(0);

	for (int i = 0; i < this->rects.size(); i++) {

		ofPushMatrix();
		ofTranslate(this->rects[i].get()->getPosition().x, this->rects[i].get()->getPosition().y);
		ofRotate(this->rects[i].get()->getRotation());

		this->images[i]->draw(-this->rect_size / 2, -this->rect_size / 2);

		ofPopMatrix();
	}

	this->fbo.end();
}

//--------------------------------------------------------------
void ofApp::draw() {

	this->fbo.draw(0, 0);
}

//========================================================================
int main() {

	ofSetupOpenGL(1280, 720, OF_WINDOW);
	ofRunApp(new ofApp());
}