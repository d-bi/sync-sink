#include <zmq.h>
#include "SyncSink.h"

using namespace SyncSinkSpace;

//Change all names for the relevant ones, including "Processor Name"
SyncSink::SyncSink() : GenericProcessor("SyncSink")
{

}

SyncSink::~SyncSink()
{

}

void SyncSink::process(AudioSampleBuffer& buffer)
{
	/** 
	If the processor needs to handle events, this method must be called only once per process call
	If spike processing is also needing, set the argument to true
	*/
	//checkForEvents(false);
	int numChannels = getNumOutputs();

	for (int chan = 0; chan < numChannels; chan++)
	{
		int numSamples = getNumSamples(chan);
		int64 timestamp = getTimestamp(chan);

		//Do whatever processing needed
	}
	 
}

