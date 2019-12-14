#include <opencv2/features2d.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/calib3d.hpp>
#include <opencv2/highgui.hpp>      //for imshow
#include <iostream>
using namespace std;
using namespace cv;

const double akazeThresh = 3e-4; // AKAZE detection threshold set to locate about 1000 keypoints
const double ransacThresh = 2.5f; // RANSAC inlier threshold
const double nnRatioThresh = 0.8f; // Nearest-neighbour matching ratio
const int bbMinInliers = 15; // Minimal number of inliers to draw bounding box

int main(int argc, char** argv) {
	namedWindow("Teste", WINDOW_AUTOSIZE);
	Mat imageToCatch, sceneImage;
	VideoCapture camOrVid(0);
	if (!camOrVid.isOpened()) { //verifica se cap abriu como esperado
		cout << "Camera ou arquivo em falta.";
		return 1;
	}

	imageToCatch = imread("C:/Users/Shark Byte/Downloads/cover2.jpg", IMREAD_COLOR);
	//resize(imageToCatch, imageToCatch, Size(), 0.5, 0.5);
	if (imageToCatch.empty()) { //verifica a imagem para captura
		cout << "Nenhuma imagem para captura";
		return 1;
	}
	//Define AKAZE
	Ptr<AKAZE> detector = AKAZE::create();
	detector->setThreshold(akazeThresh);
	//Ptr<ORB> detector = ORB::create();
	//Define Matcher
	Ptr<DescriptorMatcher> matcher = DescriptorMatcher::create("BruteForce-Hamming");
	//Define keypoints and descriptors
	vector<KeyPoint> kpToCatch, kpScene;
	Mat descriptorToCatch, descriptorScene;
	//Define object bounding box;
	vector<Point2f> objCorner(4), sceneCorner(4);
	objCorner[0] = Point2f(0, 0);
	objCorner[1] = Point2f(imageToCatch.cols, 0);
	objCorner[2] = Point2f(imageToCatch.cols, imageToCatch.rows);
	objCorner[3] = Point2f(0, imageToCatch.rows);
	//Detect and Compute object keypoints and descriptos
	Point* ptMask = new cv::Point[objCorner.size()];
	const Point* ptContain = { &ptMask[0] };
	int iSize = static_cast<int>(objCorner.size());
	for (size_t i = 0; i < objCorner.size(); i++) {
		ptMask[i].x = static_cast<int>(objCorner[i].x);
		ptMask[i].y = static_cast<int>(objCorner[i].y);
	}
	Mat matMask = Mat::zeros(imageToCatch.size(), CV_8UC1);
	cv::fillPoly(matMask, &ptContain, &iSize, 1, Scalar::all(255));
	detector->detectAndCompute(imageToCatch, matMask, kpToCatch, descriptorToCatch);
	delete[] ptMask;
	//detector->detectAndCompute(imageToCatch, objCorner, kpToCatch, descriptorToCatch);

	while (true) {
		camOrVid >> sceneImage;
		if (sceneImage.empty()) {
			cout << "Imagem de cena vazia";
			return 1;
		}
		//resize(sceneImage, sceneImage, Size(), 0.4, 0.4);
		//cvtColor(sceneImage, sceneImage, COLOR_BGR2GRAY);//coloca em grayscale
		detector->detectAndCompute(sceneImage, noArray(), kpScene, descriptorScene);
		//detector->setMaxFeatures(kpScene.size());
		vector<vector<DMatch>> matches;
		vector<KeyPoint> objectMatched, sceneMatched;
		matcher->knnMatch(descriptorToCatch, descriptorScene, matches, 2);

		for (int i = 0; i < matches.size(); ++i) {
			if (matches[i][0].distance < nnRatioThresh * matches[i][1].distance) {
				objectMatched.push_back(kpToCatch[matches[i][0].queryIdx]);
				sceneMatched.push_back(kpScene[matches[i][0].trainIdx]);
			}
		}

		//Good matches which provide correct estimation are called inliers
		Mat inlierMask, homography;
		vector<KeyPoint> objectInliers, sceneInliers;
		vector<DMatch> inlierMatches;
		if (objectMatched.size() >= 4) {
			vector<Point2f> objectPoints, scenePoints;
			for (int i = 0; i < objectMatched.size(); ++i) {
				objectPoints.push_back(objectMatched[i].pt);
				scenePoints.push_back(sceneMatched[i].pt);
			}

			homography = findHomography(objectPoints, scenePoints, RANSAC, ransacThresh, inlierMask);
		}
		if (objectMatched.size() < 4 || homography.empty()) {
			//Show this frame
			continue;
		}
		else {
			for (int i = 0; i < objectMatched.size(); i++) {
				if (inlierMask.at<uchar>(i)) {
					int newI = static_cast<int>(objectInliers.size());
					objectInliers.push_back(objectMatched[i]);
					sceneInliers.push_back(sceneMatched[i]);
					inlierMatches.push_back(DMatch(newI, newI, 0));
				}
			}
			perspectiveTransform(objCorner, sceneCorner, homography);
			if (objectInliers.size() >= bbMinInliers) {
				//Draw object bounding box after perspective transformation
				for (long long int i = 0; i < sceneCorner.size() - 1; ++i) {
					line(sceneImage, sceneCorner[i], sceneCorner[i + 1], Scalar(0, 0, 255), 2);
				}
				line(sceneImage, sceneCorner[sceneCorner.size() - 1], sceneCorner[0], Scalar(0, 0, 255), 2);
			}
			Mat imgMatches;
			drawMatches(imageToCatch, objectInliers, sceneImage, sceneInliers, inlierMatches, imgMatches,
				Scalar(255, 0, 0), Scalar(255, 0, 0));
			imshow("Good Matches & Object detection", imgMatches);
			//imshow("teste", sceneImage);
			//imshow("Teste", imageAux);
			if (waitKey(1) == 27) {
				break;
			}
		}
	}
	return 0;
}
/*#include <opencv2/core/core.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/xfeatures2d.hpp>
#include <opencv2/highgui.hpp>
#include <iostream>
#include <opencv2/imgproc.hpp>
#include <opencv2/features2d.hpp>
#include <opencv2/calib3d.hpp>

using namespace cv;
using namespace std;

#include <opencv2/core/core.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/xfeatures2d.hpp>
#include <opencv2/highgui.hpp>
#include <iostream>
#include <opencv2/imgproc.hpp>
#include <opencv2/features2d.hpp>
#include <opencv2/calib3d.hpp>

using namespace cv;
using namespace std;
const float ratioThresh = 0.75f;

int main(int argc, char* argv[]) {
	namedWindow("Teste", WINDOW_AUTOSIZE);
	Mat imageToCatch, sceneImage, imageAux;
	VideoCapture camOrVid(0);
	if (!camOrVid.isOpened()) { //verifica se cap abriu como esperado
		cout << "Camera ou arquivo em falta.";
		return 1;
	}

	imageToCatch = imread("C:/Users/igors/Desktop/Projeto-Realidade-Aumentada/Projeto-PG/hobbit.jpg", IMREAD_GRAYSCALE);

	if (imageToCatch.empty()) { //verifica a imagem para captura
		cout << "Nenhuma imagem para captura";
		return 1;
	}

	int minHessian = 400;
	Ptr<Feature2D> detector = xfeatures2d::SIFT::create(minHessian);

	vector<KeyPoint> kpToCatch, kpScene;
	Mat descriptorToCatch, descriptorScene;

	detector->detectAndCompute(imageToCatch, Mat(), kpToCatch, descriptorToCatch);

	while (true) {
		camOrVid >> sceneImage;
		if (sceneImage.empty()) {
			cout << "Imagem de cena vazia";
			return 1;
		}
		cvtColor(sceneImage, sceneImage, COLOR_BGR2GRAY);//coloca em grayscale
		detector->detectAndCompute(sceneImage, Mat(), kpScene, descriptorScene);
		if (descriptorScene.empty())
			continue;
		Ptr<DescriptorMatcher> matcher = DescriptorMatcher::create(DescriptorMatcher::FLANNBASED);
		vector<vector<DMatch>> knnMatches;
		matcher->knnMatch(descriptorToCatch, descriptorScene, knnMatches, 2);

		vector<DMatch> goodMatches;
		for (int i = 0; i < knnMatches.size(); ++i) {
			if (knnMatches[i][0].distance < ratioThresh * knnMatches[i][1].distance)
				goodMatches.push_back(knnMatches[i][0]);
		}

		Mat imgMatches;
		drawMatches(imageToCatch, kpToCatch, sceneImage, kpScene, goodMatches, imgMatches,
			Scalar::all(-1), Scalar::all(-1), vector<char>(), DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS);

		if (goodMatches.size() > 3) {

			vector<Point2f> object, scene;

			for (int i = 0; i < goodMatches.size(); i++) {
				object.push_back(kpToCatch[goodMatches[i].queryIdx].pt);
				scene.push_back(kpScene[goodMatches[i].trainIdx].pt);
			}

			vector<Point2f> objCorner(4), sceneCorner(4);
			objCorner[0] = Point2f(0, 0);
			objCorner[1] = Point2f(imageToCatch.cols, 0);
			objCorner[2] = Point2f(imageToCatch.cols, imageToCatch.rows);
			objCorner[3] = Point2f(0, imageToCatch.rows);


			try {
				Mat H = findHomography(object, scene, RANSAC);
				if(!H.empty())
					perspectiveTransform(objCorner, sceneCorner, H);
			}
			catch (cv::Exception e) {
				continue;
			}

			line(imgMatches, sceneCorner[0] + Point2f((float)imageToCatch.cols, 0), sceneCorner[1] + Point2f((float)imageToCatch.cols, 0), Scalar(0, 255, 0), 4);

			line(imgMatches, sceneCorner[1] + Point2f((float)imageToCatch.cols, 0), sceneCorner[2] + Point2f((float)imageToCatch.cols, 0), Scalar(0, 255, 0), 4);

			line(imgMatches, sceneCorner[2] + Point2f((float)imageToCatch.cols, 0), sceneCorner[3] + Point2f((float)imageToCatch.cols, 0), Scalar(0, 255, 0), 4);

			line(imgMatches, sceneCorner[3] + Point2f((float)imageToCatch.cols, 0), sceneCorner[0] + Point2f((float)imageToCatch.cols, 0), Scalar(0, 255, 0), 4);
		}
		imshow("Good Matches & Object detection", imgMatches);
		//imshow("teste", sceneImage);
		//imshow("Teste", imageAux);
		if (waitKey(1) == 27) {
			break;
		}
	}
	return 0;
}
*/