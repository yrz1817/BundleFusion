

#include "stdafx.h"

#include "FriedLiver.h"

RGBDSensor* getRGBDSensor()
{
	static RGBDSensor* g_sensor = NULL;
	if (g_sensor != NULL)	return g_sensor;

	if (GlobalAppState::get().s_sensorIdx == 0) {
#ifdef KINECT
		//static KinectSensor s_kinect;
		//return &s_kinect;
		g_sensor = new KinectSensor;
		return g_sensor;
#else 
		throw MLIB_EXCEPTION("Requires KINECT V1 SDK and enable KINECT macro");
#endif
	}

	if (GlobalAppState::get().s_sensorIdx == 1)	{
#ifdef OPEN_NI
		//static PrimeSenseSensor s_primeSense;
		//return &s_primeSense;
		g_sensor = new PrimeSenseSensor;
		return g_sensor;
#else 
		throw MLIB_EXCEPTION("Requires OpenNI 2 SDK and enable OPEN_NI macro");
#endif
	}
	else if (GlobalAppState::getInstance().s_sensorIdx == 2) {
#ifdef KINECT_ONE
		//static KinectOneSensor s_kinectOne;
		//return &s_kinectOne;
		g_sensor = new KinectOneSensor;
		return g_sensor;
#else
		throw MLIB_EXCEPTION("Requires Kinect 2.0 SDK and enable KINECT_ONE macro");
#endif
	}
	if (GlobalAppState::get().s_sensorIdx == 3) {
#ifdef BINARY_DUMP_READER
		//static BinaryDumpReader s_binaryDump;
		//return &s_binaryDump;
		g_sensor = new BinaryDumpReader;
		return g_sensor;
#else 
		throw MLIB_EXCEPTION("Requires BINARY_DUMP_READER macro");
#endif
	}
	//	if (GlobalAppState::get().s_sensorIdx == 4) {
	//		//static NetworkSensor s_networkSensor;
	//		//return &s_networkSensor;
	//		g_sensor = new NetworkSensor;
	//		return g_sensor;
	//}
	if (GlobalAppState::get().s_sensorIdx == 5) {
#ifdef INTEL_SENSOR
		//static IntelSensor s_intelSensor;
		//return &s_intelSensor;
		g_sensor = new IntelSensor;
		return g_sensor;
#else 
		throw MLIB_EXCEPTION("Requires INTEL_SENSOR macro");
#endif
	}
	if (GlobalAppState::get().s_sensorIdx == 6) {
#ifdef REAL_SENSE
		//static RealSenseSensor s_realSenseSensor;
		//return &s_realSenseSensor;
		g_sensor = new RealSenseSensor;
		return g_sensor;
#else
		throw MLIB_EXCEPTION("Requires Real Sense SDK and REAL_SENSE macro");
#endif
	}
	if (GlobalAppState::get().s_sensorIdx == 7) {
#ifdef STRUCTURE_SENSOR
		//static StructureSensor s_structureSensor;
		//return &s_structureSensor;
		g_sensor = new StructureSensor;
		return g_sensor;
#else
		throw MLIB_EXCEPTION("Requires STRUCTURE_SENSOR macro");
#endif
	}
	if (GlobalAppState::get().s_sensorIdx == 8) {
#ifdef SENSOR_DATA_READER
		//static SensorDataReader s_sensorDataReader;
		//return &s_sensorDataReader;
		g_sensor = new SensorDataReader;
		return g_sensor;
#else
		throw MLIB_EXCEPTION("Requires STRUCTURE_SENSOR macro");
#endif
	}

	throw MLIB_EXCEPTION("unkown sensor id " + std::to_string(GlobalAppState::get().s_sensorIdx));

	return NULL;
}


RGBDSensor* g_RGBDSensor = NULL;
CUDAImageManager* g_imageManager = NULL;
OnlineBundler* g_bundler = NULL;



void bundlingOptimization() {
	g_bundler->process(GlobalBundlingState::get().s_numLocalNonLinIterations, GlobalBundlingState::get().s_numLocalLinIterations,
		GlobalBundlingState::get().s_numGlobalNonLinIterations, GlobalBundlingState::get().s_numGlobalLinIterations);
	//g_bundler->resetDEBUG(false, false); // for no opt
}

void bundlingOptimizationThreadFunc() {

	DualGPU::get().setDevice(DualGPU::DEVICE_BUNDLING);

	bundlingOptimization();
}

void bundlingThreadFunc() {
	assert(g_RGBDSensor && g_imageManager);
	DualGPU::get().setDevice(DualGPU::DEVICE_BUNDLING);
	g_bundler = new OnlineBundler(g_RGBDSensor, g_imageManager);

	std::thread tOpt;

	while (1) {
		// opt
		if (g_RGBDSensor->isReceivingFrames()) {
			if (g_bundler->getCurrProcessedFrame() % 10 == 0) { // stop solve
				if (tOpt.joinable()) {
					tOpt.join();
				}
			}
			if (g_bundler->getCurrProcessedFrame() % 10 == 1) { // start solve
				MLIB_ASSERT(!tOpt.joinable());
				tOpt = std::thread(bundlingOptimizationThreadFunc);
			}
		}
		else { // stop then start solve
			if (tOpt.joinable()) {
				tOpt.join();
			}
			tOpt = std::thread(bundlingOptimizationThreadFunc);
		}
		//wait for a new input frame (LOCK IMAGE MANAGER)
		ConditionManager::lockImageManagerFrameReady(ConditionManager::Bundling);
		while (!g_imageManager->hasBundlingFrameRdy()) {
			ConditionManager::waitImageManagerFrameReady(ConditionManager::Bundling);
		}
		{
			ConditionManager::lockBundlerProcessedInput(ConditionManager::Bundling);
			while (g_bundler->hasProcssedInputFrame()) { //wait until depth sensing has confirmed the last one (WAITING THAT DEPTH SENSING RELEASES ITS LOCK)
				ConditionManager::waitBundlerProcessedInput(ConditionManager::Bundling);
			}
			{
				if (g_bundler->getExitBundlingThread()) {
					if (tOpt.joinable()) {
						tOpt.join();
					}
					ConditionManager::release(ConditionManager::Bundling);
					break;
				}
				g_bundler->processInput();						//perform sift and whatever
			}
			g_bundler->setProcessedInputFrame();			//let depth sensing know we have a frame (UNLOCK BUNDLING)
			ConditionManager::unlockAndNotifyBundlerProcessedInput(ConditionManager::Bundling);
		}
		g_imageManager->confirmRdyBundlingFrame();		//here it's processing with a new input frame  (GIVE DEPTH SENSING THE POSSIBLITY TO LOCK IF IT WANTS)
		ConditionManager::unlockAndNotifyImageManagerFrameReady(ConditionManager::Bundling);

		if (g_bundler->getExitBundlingThread()) {
			ConditionManager::release(ConditionManager::Bundling);
			break;
		}
	}
}

int main()
{
	ml::SensorData sd;
	sd.loadFromImages("D:/BundleFusion2/sens/output", "frame-", "jpg");
	sd.saveToFile("D:/BundleFusion/BuddleFusion-master/FriedLiver/x64/data/test2.sens");
}


