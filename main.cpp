#include <opencv2/features2d.hpp>
#include <opencv2/xfeatures2d.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/calib3d.hpp>
#include <opencv2/highgui.hpp>
#include <iostream>
using namespace std;
using namespace cv;

const double akazeThresh = 3e-4; // AKAZE detection threshold set to locate about 1000 keypoints
const double ransacThresh = 2.5f; // RANSAC inlier threshold
const double nnRatioThresh = 0.8f; // Nearest-neighbour matching ratio
const int bbMinInliers = 15; // Minimal number of inliers to draw bounding box

int main(int argc, char** argv) {
	Mat imageToCatch, sceneImage, coloredSceneImg, coloredImgToCatch;
	//Set the parameter to (0) if you want to use your default camera
	VideoCapture camOrVid("Hobbit.mp4");
	if (!camOrVid.isOpened()) { //Checks if camera or video opened accordingly
		cout << "Camera or file missing";
		return 1;
	}
	//Gets image to catch in chosen video or camera
	coloredImgToCatch = imread("Hobbit.jpg", IMREAD_COLOR);

	if (coloredImgToCatch.empty()) { //Checks if any image was found
		cout << "Nenhuma imagem para captura";
		return 1;
	}
	resize(coloredImgToCatch, coloredImgToCatch, Size(), 0.3, 0.3);
	cvtColor(coloredImgToCatch, imageToCatch, COLOR_BGR2GRAY);//Converts image to grayscale so it's better to detect matches

	//Choose which feature detection algorithm you want to use by uncommenting the lines
	//SIFT is chosen by default

	//Define AKAZE
	/*Ptr<AKAZE> detector = AKAZE::create();
	detector->setThreshold(akazeThresh);*/

	int minHessian = 400;
	//Define SIFT
	Ptr<Feature2D> detector = xfeatures2d::SIFT::create(minHessian);

	//Define SUFT
	//Ptr<Feature2D> detector = xfeatures2d::SURF::create(minHessian);

	//Define Matcher using flann
	Ptr<DescriptorMatcher> matcher = DescriptorMatcher::create(DescriptorMatcher::FLANNBASED);

	//Define keypoints and descriptors
	vector<KeyPoint> kpToCatch, kpScene;
	Mat descriptorToCatch, descriptorScene;

	//Detect and Compute object keypoints and descriptos
	detector->detectAndCompute(imageToCatch, Mat(), kpToCatch, descriptorToCatch);
	
	if (descriptorToCatch.empty() || kpToCatch.size() < 2) {
		cout << "Image to catch descriptors or/and keypoints are empty"; //If descriptors or/and keypoints are empty there's no reason to continue
		return -1;
	}
	if (descriptorToCatch.type() != CV_32F)
		descriptorToCatch.convertTo(descriptorToCatch, CV_32F); //

	//Define object bounding box for later drawing in sceneImage;
	vector<Point2f> objCorner(4), sceneCorner(4);
	objCorner[0] = Point2f(0, 0);
	objCorner[1] = Point2f(imageToCatch.cols, 0);
	objCorner[2] = Point2f(imageToCatch.cols, imageToCatch.rows);
	objCorner[3] = Point2f(0, imageToCatch.rows);

	while (true) {
		//Read sceneImage from cam or video input
		camOrVid >> coloredSceneImg;
		if (coloredSceneImg.empty()) { //Checks for end of video or if no image is being captured
			cout << "Scene image empty or end of video";
			return 1;
		}
		resize(coloredSceneImg, coloredSceneImg, Size(), 0.5, 0.5);
		cvtColor(coloredSceneImg, sceneImage, COLOR_BGR2GRAY);//Converts image to grayscale so it's better to detect matches
		//Detect and Compute scene keypoints and descriptos
		detector->detectAndCompute(sceneImage, noArray(), kpScene, descriptorScene);
		//If descriptors or/and keypoints are empty continue to next frame
		if (descriptorScene.empty() || kpScene.size() < 2) {
			continue;
		}
		if (descriptorScene.type() != CV_32F)
			descriptorScene.convertTo(descriptorScene, CV_32F);

		//Start of matching process
		Mat homography, inlierMask;
		vector<vector<DMatch>> matches;
		vector<DMatch> inlierMatches;
		vector<KeyPoint> objectMatched, sceneMatched, objectInliers, sceneInliers;
		vector<Point2f> objectPoints, scenePoints;
		
		//Use flannbased matcher and algorithm K-Nearest Neighbours to match both descriptors
		matcher->knnMatch(descriptorToCatch, descriptorScene, matches, 2);

		//This iteration checks for good matches among the found matches
		//If a match is considered good, then we'll obtain both KeyPoints and its coordinates for object and scene
		for (int i = 0, j = 0; i < matches.size(); ++i) {
			if (matches[i][0].distance < nnRatioThresh * matches[i][1].distance) {
				objectMatched.push_back(kpToCatch[matches[i][0].queryIdx]);
				sceneMatched.push_back(kpScene[matches[i][0].trainIdx]);

				objectPoints.push_back(objectMatched[j].pt);
				scenePoints.push_back(sceneMatched[j].pt);
				++j;
			}
		}
		
		if (objectMatched.size() >= 4) {
			homography = findHomography(objectPoints, scenePoints, RANSAC, ransacThresh, inlierMask);
		}
		else {
			//Go to next frame
			continue;
		}

		if (homography.empty()) {
			//Go to next frame
			continue;
		}
		else {
			//Good matches which provide correct estimation are called inliers
			for (int i = 0; i < objectMatched.size(); i++) {
				if (inlierMask.at<uchar>(i)) {
					int matchIdx = objectInliers.size();
					objectInliers.push_back(objectMatched[i]);
					sceneInliers.push_back(sceneMatched[i]);
					inlierMatches.push_back(DMatch(matchIdx, matchIdx, 0));
				}
			}
			//Perspective transform of object bounding box to scene coordinates
			perspectiveTransform(objCorner, sceneCorner, homography);
			if (objectInliers.size() >= bbMinInliers) {
				//Draw object bounding box after perspective transformation
				for (long long int i = 0; i < sceneCorner.size() - 1; ++i) {
					line(coloredSceneImg, sceneCorner[i], sceneCorner[i + 1], Scalar(0, 0, 255), 2);
				}
				line(coloredSceneImg, sceneCorner[sceneCorner.size() - 1], sceneCorner[0], Scalar(0, 0, 255), 2);
			}
			//Draw matches and show
			Mat imgMatches;
			drawMatches(coloredImgToCatch, objectInliers, coloredSceneImg, sceneInliers, inlierMatches, imgMatches,
				Scalar(255, 0, 0), Scalar(255, 0, 0), vector<char>(), DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS);
			imshow("Good Matches & Object detection", imgMatches);
			if (waitKey(1) == 27) {
				break;
			}
		}
	}
	return 0;
}