#include <zmq.h>
#include <assert.h>
#include <string.h>
#include "SyncSink.h"
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
		obj->setProperty("content", raw);
		
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
	}
}
