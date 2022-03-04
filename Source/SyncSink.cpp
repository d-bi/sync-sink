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
		const float *sample = buffer.getReadPointer(chan);
		int numSamples = getNumSamples(chan);
		int64 timestamp = getTimestamp(chan);

		DynamicObject *obj = new DynamicObject();
		obj->setProperty("data_type", "cts");
		obj->setProperty("channel", chan);
		obj->setProperty("timestamp", timestamp);
		obj->setProperty("num_samples", numSamples);
		const Array<var> raw(sample, numSamples);
		obj->setProperty("content", raw);
		var json (obj);
		String s = JSON::toString(json);
		void *str = (void *) s.toRawUTF8();
		zmq_send(socket, str, strlen((const char *)str), 0);
		// zmq_send(socket, "data", 4, ZMQ_SNDMORE);
		// zmq_send(socket, sample, sizeof(float)*numSamples, 0);
		
		// for (int i = 0; i < numSamples; i++)
		// {
			// std::cout << sample[i] << std::endl;
		// } 
		//Do whatever processing needed
	}

}

