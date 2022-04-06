#include <zmq.h>
#include <assert.h>
#include <string.h>
#include "SyncSink.h"
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
		// std::cout << s << std::endl;
		zmq_send(socket, str, strlen((const char *)str), 0);
		
		/* Parse Kofiko */
		// Beginning of trial: add conditions
		if (text.startsWith("ClearDesign"))
		{
			conditionMap.clear();
			numConditions = 0;
		}
		else if (text.startsWith("AddCondition"))
		{
			StringArray tokens;
			tokens.addTokens(text, true);
			/* tokens[0] == AddCondition; tokens[1] == Name; tokens[2] == STIMCLASS;
			   tokens[3] == Visible; tokens[4] == 1; tokens[5] == TrialTypes; */
			for (int i = 6; i < tokens.size(); i++)
			{
				std::cout << tokens[i] << std::endl;
				conditionMap.set(tokens[i], tokens[2]);
				conditionList.set(tokens[2], numConditions);
			}
			numConditions += 1;
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
			}
			else
			{
				std::cout << "Image ID " << tokens[1] << " not mappable to stimulus class!" << std::endl;
			}
			std::cout << "TrialStart for image " << tokens[1] << " at " << timestamp << std::endl;
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
			currentTrialStartTime = -1;
			currentStimClass = -1;
			inTrial = false;
			for (std::vector<std::vector<std::vector<double>>> channelTensor : spikeTensor)
			{
				for (std::vector<std::vector<double>> unitTensor : channelTensor)
				{
					for (std::vector<double> conditionTensor : unitTensor)
					{
						for (double val : conditionTensor)
						{
							val *= double(nTrials) / double(nTrials + 1);
							std::cout << val << " ";
						}
						std::cout << std::endl;
					}
					std::cout << std::endl;
				}
				std::cout << std::endl;
			}
			std::cout << std::endl;
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


		if (!inTrial)
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
			std::vector<double> thisConditionSpikeTensor(nBins);
//			std::cout << "create condition tensor" << String(spikeTensor[spikeChannelIdx][sortedID].size()) << std::endl;
			spikeTensor[spikeChannelIdx][sortedID].push_back(thisConditionSpikeTensor);
		}
//		std::vector<double> thisConditionSpikeTensor = thisUnitSpikeTensor[currentStimClass];
		
		double offset = double(timestamp - currentTrialStartTime) / double(sampleRate);
		int bin = floor(offset / 0.01);
		if (bin < nBins)
		{
			spikeTensor[spikeChannelIdx][sortedID][currentStimClass][bin] = double(1) / double(nTrials); // assignment not working
//			std::cout << spikeTensor[spikeChannelIdx][sortedID][currentStimClass][bin] << " ";
		}
	}
}
