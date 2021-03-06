/*
 * Feature2D.h
 *
 *  Created on: Sep 28, 2016
 *      Author: kevinsheridan
 */

#ifndef PAUVSI_M7_PAUVSI_VIO_INCLUDE_PAUVSI_VIO_VIOFEATURE2D_HPP_
#define PAUVSI_M7_PAUVSI_VIO_INCLUDE_PAUVSI_VIO_VIOFEATURE2D_HPP_

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/calib3d.hpp>
#include <opencv2/features2d.hpp>
#include "opencv2/core/core.hpp"
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/xfeatures2d.hpp"
#include "opencv2/video.hpp"
#include <vector>
#include <deque>
#include <string>
#include <ros/ros.h>

#define DEAFAULT_FEATURE_SIZE 1.0
#define DEFAULT_FETAURE_DEPTH 0.5
#define DEFAULT_FEATURE_DEPTH_VARIANCE 1000

class VIOFeature2D
{

private:
	int id;
	cv::KeyPoint feature; // response = quality
	cv::Point2f undistort_feature; // this is the direction vector of this feature
	//cv::Point2f normalized_feature;
	cv::Mat description; // vector 1X32
	bool described;
	bool matched;
	bool undistorted;
	float quality;
	std::deque<int> matchedFeatureIndexes;
	std::deque<int> matchedFeatureIDs;
	float distanceFromFrameCenter;
	cv::Mat K;

	//depth kalman
	double depth;
	double depth_variance;

	//bool convertedTo3D;

public:

	bool forwardMatched;
	int forwardMatchIndex;
	int forwardMatchID;

	VIOFeature2D()
	{

	}

	/*
	 * create a feature with a position and match
	 */
	VIOFeature2D(cv::Point2f pt, int matchedID, int matchedIndex, std::deque<int>&& matchedID_deque, std::deque<int>&& matchedIndex_deque, int _id, double depth, double depth_variance){
		this->setFeaturePosition(pt);
		id = _id;
		described = false; // the feature has not been described with this constructor
		matched = true;

		matchedFeatureIDs = (matchedID_deque); // give this feature the old features ID buffer
		matchedFeatureIndexes = (matchedIndex_deque); // give this feature the old features Index buffer

		matchedFeatureIDs.push_front(matchedID); // add the old feature to this features ID buffer at front
		matchedFeatureIndexes.push_front(matchedIndex); // add the old feature to this features index buffer
		quality = -1.0;

		this->depth = depth;
		this->depth_variance = depth_variance;
	}
	/*
	 * create a feature with a position, description and match
	 */
	VIOFeature2D(cv::Point2f pt, int matchedID, int matchedIndex, std::deque<int>&& matchedID_deque, std::deque<int>&& matchedIndex_deque, cv::Mat desc, int _id, double depth, double depth_variance){
		this->setFeaturePosition(pt);
		id = _id;
		described = true; // the feature has been described with this constructor
		description = desc;
		matched = true;

		matchedFeatureIDs = (matchedID_deque); // give this feature the old features ID buffer
		matchedFeatureIndexes = (matchedIndex_deque); // give this feature the old features Index buffer

		matchedFeatureIDs.push_front(matchedID); // add the old feature to this features ID buffer at front
		matchedFeatureIndexes.push_front(matchedIndex); // add the old feature to this features index buffer
		quality = -1.0;

		this->depth = depth;
		this->depth_variance = depth_variance;
	}
	/*
	 * creates a feature
	 * without description
	 * not matched
	 */
	VIOFeature2D(cv::KeyPoint _corner, int _id, double depth = DEFAULT_FETAURE_DEPTH, double depth_variance = DEFAULT_FEATURE_DEPTH_VARIANCE){
		feature = _corner;
		id = _id;
		described = false; // the feature has not been described with this constructor
		matched = false;
		//convertedTo3D = false;
		quality = -1.0;
		this->depth = depth;
		this->depth_variance = depth_variance;
	}

	/*
	 * creates a feature with a description
	 * not matched
	 */
	VIOFeature2D(cv::KeyPoint _corner, cv::Mat _description, int _id, double depth = DEFAULT_FETAURE_DEPTH, double depth_variance = DEFAULT_FEATURE_DEPTH_VARIANCE){
		feature = _corner;
		id = _id;
		description = _description;
		described = true; // the feature has not been described with this constructor
		matched = false;
		//convertedTo3D = false; // the feature
		quality = -1.0;
		this->depth = depth;
		this->depth_variance = depth_variance;
	}

	void updateDepth(double depth_measurement, double variance)
	{
		ROS_ASSERT(this->depth_variance > 0 && variance > 0);
		double K = this->depth_variance / (this->depth_variance + variance);

		this->depth = this->depth + K * (depth_measurement - this->depth);

		this->depth_variance = (1 - K) * this->depth_variance;
	}

	cv::KeyPoint getFeature(){
		return this->feature;
	}

	cv::Point2f getFeaturePosition(){
		return this->feature.pt;
	}

	cv::Mat getFeatureDescription(){
		return description;
	}

	float getResponse()
	{
		return feature.response;
	}


	bool isFeatureDescribed(){
		return described;
	}

	bool isMatched(){
		return matched;
	}

	int getFeatureID(){
		return id;
	}

	double getFeatureDepth(){
		return this->depth;
	}

	double getFeatureDepthVariance(){
		return this->depth_variance;
	}

	float getQuality(){
		return quality;
	}

	float getDistanceFromFrameCenter(){
		return distanceFromFrameCenter;
	}

	int getMatchedID(int i){
		return this->matchedFeatureIDs.at(i);
	}

	int getMatchedIndex(int i){
		return this->matchedFeatureIndexes.at(i);
	}
	int getMatchedID(){
		return this->matchedFeatureIDs.at(0);
	}

	int getMatchedIndex(){
		return this->matchedFeatureIndexes.at(0);
	}

	std::deque<int> getMatchedIDDeque(){
		return this->matchedFeatureIDs;
	}

	std::deque<int> getMatchedIndexDeque(){
		return this->matchedFeatureIndexes;
	}

	/*
	 * if normalized true returns normalized undistorted point else
	 * returns undistorted point
	 */
	cv::Point2f getUndistorted(bool normalized = true)
	{
		ROS_ASSERT(this->undistorted);
		if(normalized)
			return this->undistort_feature;
		else
			return cv::Point2f(K.at<float>(0, 0) * this->undistort_feature.x + K.at<float>(2, 0), K.at<float>(1, 1) * this->undistort_feature.y + K.at<float>(2, 1)); // the non normed point
	}

	bool isUndistorted(){
		return this->undistorted;
	}

	/*
	 * undistorts this feature using K and D
	 * the new K is the ident
	 * sets the undistorted feature
	 */
	void undistort(cv::Mat K, cv::Mat D)
	{
		std::vector<cv::Point2f> in;
		in.push_back(this->feature.pt);

		std::vector<cv::Point2f> out;

		cv::fisheye::undistortPoints(in, out, K, D);

		this->undistort_feature = out.at(0);
		this->undistorted = true;
		this->K = K;
	}

	void setFeatureDepth(double d)
	{
		this->depth = d;
	}

	void setQuality(float _quality){
		this->quality = _quality;
	}

	void setDistanceFromFrameCenter(float d){
		distanceFromFrameCenter = d;
	}

	/*
	 * sets the corner and position of feature
	 */
	void setFeature(cv::KeyPoint kp){
		feature = kp;
	}

	void setFeaturePosition(cv::Point2f pt){
		feature = cv::KeyPoint(pt, DEAFAULT_FEATURE_SIZE);
	}

	/*
	 * sets the feature by creating it with a szie and pos
	 */
	void setFeature(cv::Point2f pt, float size){
		feature = cv::KeyPoint(pt, size);
	}

	/*
	 * sets the description and sets the feature to described
	 */
	void setFeatureDescription(cv::Mat desc){
		description = desc;
		described = true;
	}

	void setFeatureID(int _id){
		id = _id;
	}

};



#endif /* PAUVSI_M7_PAUVSI_VIO_INCLUDE_PAUVSI_VIO_VIOFEATURE2D_HPP_ */
