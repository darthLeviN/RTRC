// rtrcC.cpp : Defines the entry point for the application.
//
#include "mainIncludes.h"
using namespace main;


int RTRC_MAIN(int argc, char *argv[])
{
	
	try
	{
	parse_main_input_options(argc, argv);
	
	//printf("fonts_dir =%s\n",fonts_dir.data);
	//printf("shaders_dir =%s\n", shaders_dir.data);
	
	auto appInstance = make_appInstance();
	// initialize video feed
	
	using vidPipe_t = rtrc::vkPipes::basicVkVideoPipeGroup<appInstance_t>;
	auto vidPipe = std::make_shared<vidPipe_t>(appInstance, feed_url, demuxer);
	
	// takes a sequence of graphics pipelines as argument.
	std::shared_ptr<grPuller_t> mRPuller(std::make_shared<grPuller_t>(appInstance, vidPipe->getPipeGroup()));
	vidPipe->issueLoad(mRPuller);
	initAndConfigureMainVP(appInstance, mRPuller);	
	vidPipe->startReadingVideo(mRPuller);
	
	std::thread renderingTh(createRenderingThread(appInstance, mRPuller, vidPipe));
	// configuring user inputs for window and starting the UI event polls that need to be in the main thread.
	rtrc::rglfwl::commonKeyCallbacks::configure(*appInstance->getWindow().get());
	size_t pollCount = 0;
	appInstance->getWindow()->show();
	pollUIEvents(appInstance, pollCount);
	// waiting for the rendering and video decoding threads.
	continueRendering = false;
	vidPipe->stopReadingVideo();
	renderingTh.join();
	
	appInstance->getWindow()->clearFunctionalCallbacks();
	}
	catch(rtrc::ravl::AVlibException &error)
	{
		printf(error.message.c_str());
		printf("error value : %d\n", error.value);
		getchar();
		exit(-1);
	}
	catch(rtrc::graphicalError &error)
	{
		printf(error.message.c_str());
		getchar();
		exit(-1);
	}
	catch(std::exception &error)
	{
		printf("%s\n", error.what());
	}
	return 0;
}
