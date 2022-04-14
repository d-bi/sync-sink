#include <zmq.h>
#include <assert.h>
#include <string.h>
#include "SyncSink.h"
#include "SyncSinkEditor.h"
#include "SyncSinkCanvas.h"
#include <vector>
#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif

using namespace SyncSinkSpace;

//Change all names for the relevant ones, including "Processor Name"
SyncSink::SyncSink() : GenericProcessor("SyncSink")
{
	/**
	 * @brief initialize processor
	 * - open socket
	 */
	context = zmq_ctx_new();
	socket = zmq_socket(context, ZMQ_PUB);
	dataport = 5556;
	int rc = zmq_bind(socket, "tcp://*:5556");
	assert(rc == 0);
	std::cout << "socket opened" << std::endl;
	// for (int i = 0; i < 10; i++)
	// {
	// 	zmq_send(socket, "test message", 12, 0);
	// 	std::cout << "test message sent" << std::endl;
	// 	Sleep(100);
	// }
	// DynamicObject *obj = new DynamicObject();
	// obj->setProperty("data_type", "cts");
	// obj->setProperty("channel", 1);
	// obj->setProperty("timestamp", 1);
	// var json (obj);
	// String s = JSON::toString(json);
	// void *str = (void *) s.toRawUTF8();
	// std::cout << s << std::endl;
	// std::cout << (const char *) str << std::endl;
	// zmq_send(socket, str, strlen((const char *)str), 0);
	//zmq_send(socket, s, s.length(), 0);

	/**
	 * @brief confirm that client is connected to socket
	 * 
	 */
}

SyncSink::~SyncSink()
{
	/**
	 * @brief release processor
	 * - close socket
	 */
	zmq_close(socket);
	zmq_ctx_destroy(context);
}

AudioProcessorEditor* SyncSink::createEditor()
{
	editor = new SyncSinkEditor(this, true);
	return editor;
}


void SyncSink::process(AudioSampleBuffer& buffer)
{
	/** 
	If the processor needs to handle events, this method must be called only once per process call
	If spike processing is also needing, set the argument to true
	*/
	checkForEvents(true);
	int numChannels = buffer.getNumChannels();

	// std::cout << buffer.getNumSamples() << std::endl;

	for (int chan = 0; chan < numChannels; chan++)
	{
		// const float *sample = buffer.getReadPointer(chan);
		// int numSamples = getNumSamples(chan);
		// int64 timestamp = getTimestamp(chan);
		// const float sampleRate = getSampleRate(chan);

		// DynamicObject *obj = new DynamicObject();
		// obj->setProperty("data_type", "cts");
		// obj->setProperty("channel", chan);
		// obj->setProperty("timestamp", timestamp);
		// obj->setProperty("num_samples", numSamples);
		// const Array<var> raw(sample, numSamples);
		// obj->setProperty("content", raw);
		// obj->setProperty("sample_rate", sampleRate);
		// var json (obj);
		// String s = JSON::toString(json);
		// std::cout << s << std::endl;
		// void *str = (void *) s.toRawUTF8();
		// zmq_send(socket, str, strlen((const char *)str), 0);
		// zmq_send(socket, "data", 4, ZMQ_SNDMORE);
		// zmq_send(socket, sample, sizeof(float)*numSamples, 0);
		
		// for (int i = 0; i < numSamples; i++)
		// {
			// std::cout << sample[i] << std::endl;
		// } 
		//Do whatever processing needed
	}

}

void SyncSink::handleEvent(const EventChannel* eventInfo,
	const MidiMessage& event, int samplePosition)
{
	if (Event::getEventType(event) == EventChannel::TTL)
	{
		TTLEventPtr ttl = TTLEvent::deserializeFromMessage(event, eventInfo);
	}
	else if (Event::getEventType(event) == EventChannel::TEXT)
	{
		TextEventPtr txt = TextEvent::deserializeFromMessage(event, eventInfo);
		String text = txt->getText();
		const float sampleRate = txt->getChannelInfo()->getSampleRate();
		int64 timestamp = txt->getTimestamp();
		DynamicObject *obj = new DynamicObject();
		obj->setProperty("data_type", "event_text");
		obj->setProperty("timestamp", timestamp);
		obj->setProperty("content", text);
		obj->setProperty("sample_rate", sampleRate);
		var json (obj);
		String s = JSON::toString(json);
		void *str = (void *) s.toRawUTF8();
		zmq_send(socket, str, strlen((const char *)str), 0);
		
		/* Parse Kofiko */
		// Beginning of trial: add conditions
		//std::cout << text << std::endl;
		if (text.startsWith("ClearDesign"))
		{
			conditionMap.clear();
			conditionList.clear();
			conditionListInverse.clear();
			//nTrialsByStimClass.clear();
			spikeTensor.clear();
			stimClasses.clear();
			numConditions = 0;
			nTrials = 0;
		}
		else if (text.startsWith("AddCondition"))
		{
			StringArray tokens;
			tokens.addTokens(text, true);
			/* tokens[0] == AddCondition; tokens[1] == Name; tokens[2] == STIMCLASS;
			   tokens[3] == Visible; tokens[4] == 1; tokens[5] == TrialTypes; */
			for (int i = 6; i < tokens.size(); i++)
			{
				//std::cout << tokens[i] << std::endl;
				conditionMap.set(tokens[i], tokens[2]);

			}
			conditionList.set(tokens[2], numConditions);
			conditionListInverse.set(numConditions, tokens[2]);
			nTrialsByStimClass.set(numConditions, 0);
			stimClasses.push_back(numConditions);
			numConditions += 1;
			if (canvas != nullptr)
			{
				canvas->update();
			}
			for (int stimClass : stimClasses)
			{
				std::cout << stimClass << std::endl;
			}
			std::cout << text << std::endl;
		}
		else if (text.startsWith("TrialStart"))
		{
			StringArray tokens;
			tokens.addTokens(text, true);
			/* tokens[0] == TrialStart; tokens[1] == IMGID */
			if (conditionMap.contains(tokens[1]))
			{
				currentStimClass = conditionList[conditionMap[tokens[1]]];
				nTrials += 1;
				nTrialsByStimClass.set(currentStimClass, nTrialsByStimClass[currentStimClass] + 1);
			}
			else
			{
				std::cout << "Image ID " << tokens[1] << " not mappable to stimulus class!" << std::endl;
			}
			std::cout << "TrialStart for image " << tokens[1] << " at " << timestamp << std::endl;
			if (thisEditor != nullptr)
			{
				thisEditor->updateLegend();
			}
		}
		else if (text.startsWith("TrialAlign"))
		{
			std::cout << "TrialAlign at " << timestamp << std::endl;
			currentTrialStartTime = timestamp;
			inTrial = true;
		}
		else if (text.startsWith("TrialEnd"))
		{
			std::cout << "TrialEnd at " << timestamp << std::endl;
			if (canvas != nullptr) {
				//std::cout << "send update to canvas" << std::endl;
				canvas->updatePlots();
			}
			//if (!nTrialsByStimClass.contains(currentStimClass))
			//{
			//	std::cout << "unregistered stim class " << currentStimClass << std::endl;
			//	return;
			//}
			for (std::vector<std::vector<std::vector<double>>> channelTensor : spikeTensor)
			{
				for (std::vector<std::vector<double>> unitTensor : channelTensor)
				{
					//std::vector<double> conditionTensor = unitTensor[currentStimClass];
					int condition_idx = 0;
					for (int c = 0; c < unitTensor.size(); c++)//std::vector<double> conditionTensor : unitTensor)
					{
						std::vector<double> conditionTensor = unitTensor[c];
						for (double val : conditionTensor)
						{
							val *= double(nTrials) / double(nTrialsByStimClass[c] + 1);
//							std::cout << val << " ";
						}
						condition_idx += 1;
//						std::cout << std::endl;
					}
//					std::cout << std::endl;
				}
//				std::cout << std::endl;
			}
//			std::cout << std::endl;
			currentTrialStartTime = -1;
			currentStimClass = -1;
			inTrial = false;
		}
	}
}

void SyncSink::handleSpike(const SpikeChannel* spikeInfo,
	const MidiMessage& event, int samplePosition)
{
	SpikeEventPtr spike = SpikeEvent::deserializeFromMessage(event, spikeInfo);
	if (spike)
	{
		const SpikeChannel *chan = spike->getChannelInfo();
		int64 numChannels = chan->getNumChannels();
		int64 numSamples = chan->getTotalSamples();
		int64 timestamp = spike->getTimestamp();
		const float sampleRate = chan->getSampleRate();
		const float *waveform = spike->getDataPointer();
		const Array<var> raw(waveform, numSamples);
		DynamicObject *obj = new DynamicObject();
		obj->setProperty("data_type", "spike");
		obj->setProperty("sample_rate", sampleRate);
		obj->setProperty("num_samples", numSamples);
		int64 nodeID = chan->getCurrentNodeID();
		int64 spikeChannelIdx = getSpikeChannelIndex(spike);
		int64 sortedID = spike->getSortedID();
		obj->setProperty("sorted_id", sortedID);
		obj->setProperty("spike_channel_idx", spikeChannelIdx);
		// obj->setProperty("content", raw);
		
		/**
		 * @brief on sending spike information
		 * automatic clustering
		 * one channel: multiple units (neurons); clustering in PCA space
		 * - specify hyperparameters: num neurons, as well as min sep
		 * TODO: need to get pca space coordinates (?)
		 */
		
		// obj->setProperty("channel", );
		obj->setProperty("timestamp", timestamp);
		var json (obj);
		String s = JSON::toString(json);
		void *str = (void *) s.toRawUTF8();
		// std::cout << s << std::endl;
		zmq_send(socket, str, strlen((const char *)str), 0);

		if (!inTrial || numConditions < 0 || currentStimClass < 0 || currentTrialStartTime < 0)
		{
			return; // do not process spike when stimulus is not presented
		}

		/* Store in spikeTensor */
//		std::cout << "spike channel idx " << String(spikeChannelIdx) << std::endl;
		while (spikeTensor.size() < (spikeChannelIdx + 1))
		{
			std::vector<std::vector<std::vector<double>>> thisChannelSpikeTensor;
//			std::cout << "create channel tensor " << String(spikeTensor.size()) << std::endl;
			spikeTensor.push_back(thisChannelSpikeTensor);
		}

//		std::cout << "spike sorted id " << String(sortedID) << std::endl;
		while (spikeTensor[spikeChannelIdx].size() < (sortedID + 1))
		{
			std::vector<std::vector<double>> thisUnitSpikeTensor;
//			std::cout << "create unit tensor" << String(spikeTensor[spikeChannelIdx].size()) << std::endl;
			spikeTensor[spikeChannelIdx].push_back(thisUnitSpikeTensor);
		}
		
		while (spikeTensor[spikeChannelIdx][sortedID].size() < numConditions)
		{
			std::vector<double> thisConditionSpikeTensor(nBins, (double) 0);
//			std::cout << "create condition tensor" << String(spikeTensor[spikeChannelIdx][sortedID].size()) << std::endl;
			spikeTensor[spikeChannelIdx][sortedID].push_back(thisConditionSpikeTensor);
		}
//		std::vector<double> thisConditionSpikeTensor = thisUnitSpikeTensor[currentStimClass];
		
		double offset = double(timestamp - currentTrialStartTime) / double(sampleRate);
		int bin = floor(offset / ((double) binSize / (double) 1000));
		if (!nTrialsByStimClass.contains(currentStimClass))
		{
			std::cout << "unregistered stim class " << currentStimClass << std::endl;
			return;
		}
		if (bin < nBins)
		{
			spikeTensor[spikeChannelIdx][sortedID][currentStimClass][bin] += double(1) / double(nTrialsByStimClass[currentStimClass]); // assignment not working
//			std::cout << spikeTensor[spikeChannelIdx][sortedID][currentStimClass][bin] << " ";
		}
	}
}

std::vector<double> SyncSink::getHistogram(int channel_idx, int sorted_id, int stim_class)
{
	if (channel_idx >= spikeTensor.size()) {
		return std::vector<double>(nBins, 0);
	}
	if (sorted_id >= spikeTensor[channel_idx].size()) {
		return std::vector<double>(nBins, 0);
	}
	if (stim_class >= spikeTensor[channel_idx][sorted_id].size())
	{
		return std::vector<double>(nBins, 0);
	}
	return spikeTensor[channel_idx][sorted_id][stim_class];
}

int SyncSink::getNTrial()
{
	return nTrials;
}

void SyncSink::setCanvas(SyncSinkCanvas* c)
{
	canvas = c;
}

void SyncSinkSpace::SyncSink::setEditor(SyncSinkEditor* e)
{
	thisEditor = e;
}

void SyncSink::addPSTHPlot(int channel_idx, int sorted_id, std::vector<int> stimClasses)
{
	if (canvas != nullptr) {
		std::cout << "add plot to canvas: " << channel_idx << sorted_id;
		for (int stim_class : stimClasses)
		{
			std::cout << stim_class << "(" << conditionListInverse[stim_class] << ") ";

		}
		std::cout << std::endl;
		canvas->addPlot(channel_idx, sorted_id, stimClasses);
	}
}

void SyncSink::resetTensor()
{
	int ch_idx = 0;
	for (std::vector<std::vector<std::vector<double>>> channelTensor : spikeTensor)
	{
		int un_idx = 0;
		for (std::vector<std::vector<double>> unitTensor : spikeTensor[ch_idx])
		{
			int cond_idx = 0;
			for (std::vector<double> conditionTensor : spikeTensor[ch_idx][un_idx])
			{
				for (int i = 0; i < spikeTensor[ch_idx][un_idx][cond_idx].size(); i++)
				{
					spikeTensor[ch_idx][un_idx][cond_idx][i] = 0;
				}
				while (spikeTensor[ch_idx][un_idx][cond_idx].size() < nBins)
				{
					spikeTensor[ch_idx][un_idx][cond_idx].push_back(0);
				}
				cond_idx++;
			}
			un_idx++;
		}
		ch_idx++;
	}
	nTrials = 0;
	if (canvas != nullptr)
	{
		canvas->updatePlots();
		canvas->update();
	}
	if (thisEditor != nullptr)
	{
		thisEditor->updateLegend();
	}
}

void SyncSink::rebin(int n_bins, int bin_size)
{
	nBins = n_bins;
	binSize = bin_size;
}

String SyncSink::getStimClassLabel(int stim_class)
{
	if (conditionListInverse.contains(stim_class))
	{
		return conditionListInverse[stim_class];
	}
	return "";
}

int SyncSink::getNBins()
{
	return nBins;
}

int SyncSink::getBinSize()
{
	return binSize;
}

std::vector<int> SyncSink::getStimClasses()
{
	return stimClasses;
}
