/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   mainRendering.h
 * Author: roohi
 *
 * Created on December 19, 2019, 1:53 PM
 */
#pragma once
#include "main_utility.h"
#include <rtrc/graphical/basicVkVideoPipeGroup.h>

namespace main{

std::mutex frameCM;
size_t frameCounter = 0;

template<typename appInstanceT>
auto createRenderingThread(
	const std::shared_ptr<appInstanceT> &appInstance,
	const std::shared_ptr<grPuller_t> &mRPuller,
	const std::shared_ptr<rtrc::vkPipes::basicVkVideoPipeGroup<appInstanceT>> &vidPipe)
{
	using namespace std;
	auto renderFunc = [](
		const std::shared_ptr<appInstanceT> appInstance,
		const std::shared_ptr<grPuller_t> mRPuller,
		const std::shared_ptr<rtrc::vkPipes::basicVkVideoPipeGroup<appInstanceT>> vidPipe)
	{
		size_t renderCounter = 0;
		constexpr size_t renderCounterMax = 1000;
		size_t maxTime = 0;
		size_t delayedCounter = 0;
		auto startTime = std::chrono::high_resolution_clock::now();
		while(continueRendering)
		{
			auto tmpStartTime = std::chrono::high_resolution_clock::now();
			++renderCounter;
			
			{
				std::lock_guard<std::mutex> rclk(renderCountersM);
				++allRenderCounter;
			}
			renderCounter = renderCounter % renderCounterMax;
			bool doPrint = renderCounter == 0;
			appInstance->getWindow()->pollInputEvents(glfwGetTime());
			{
				std::lock_guard<std::mutex> vplk(mainVPM);
				if(mainVP)mainVP->pollEvents();
			}
			mRPuller->graphicsPull(std::tuple{vidPipe->getDrawCallable()}, std::array<std::vector<VkSemaphore>,1>{}, 
				std::array<std::vector<VkPipelineStageFlags>,1>{}, std::tuple{vidPipe->getWriteCmd(mainVP)});
			size_t currCallTime = std::chrono::duration_cast<chrono::milliseconds>(std::chrono::high_resolution_clock::now() - tmpStartTime).count();
			if(currCallTime > maxTime) maxTime = currCallTime;
			if(currCallTime > 5) ++delayedCounter;
			
			if(doPrint)
			{
				auto endTime = std::chrono::high_resolution_clock::now();
				printf("app loop time : %f\n", std::chrono::duration_cast<chrono::milliseconds>(endTime - startTime).count()/(double)renderCounterMax);
				startTime = std::chrono::high_resolution_clock::now();
				mainLogger.logText(Rsnprintf<char,100>("max call time = %llu", maxTime).data);
				mainLogger.logText(Rsnprintf<char,100>("ratio of delayed frames was %llu/%llu", delayedCounter, renderCounterMax).data);
				maxTime = 0;
				delayedCounter = 0;
			}
			//std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}
	};
	
	return std::thread(renderFunc, appInstance, mRPuller, vidPipe);
}

}
