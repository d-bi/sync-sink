//This prevents include loops. We recommend changing the macro to a name suitable for your plugin
#ifndef SYNCSINK_H_DEFINED
#define SYNCSINK_H_DEFINED

#include <ProcessorHeaders.h>
#include <vector>

//namespace must be an unique name for your plugin
namespace SyncSinkSpace
{
	class SyncSinkCanvas;
	class SyncSinkEditor;

	class SyncSink : public GenericProcessor
	{
	public:
		/** The class constructor, used to initialize any members. */
		SyncSink();

		/** The class destructor, used to deallocate memory */
		~SyncSink();

		/** Indicates if the processor has a custom editor. Defaults to false */
		bool hasEditor() const { return true; }

		/** If the processor has a custom editor, this method must be defined to instantiate it. */
		AudioProcessorEditor* createEditor() override;

		/** Optional method that informs the GUI if the processor is ready to function. If false acquisition cannot start. Defaults to true */
		//bool isReady();

		/** Defines the functionality of the processor.

		The process method is called every time a new data buffer is available.

		Processors can either use this method to add new data, manipulate existing
		data, or send data to an external target (such as a display or other hardware).

		Continuous signals arrive in the "buffer" variable, event data (such as TTLs
		and spikes) is contained in the "events" variable.
		*/
		void process(AudioSampleBuffer& buffer) override;

		/** Handles events received by the processor

		Called automatically for each received event whenever checkForEvents() is called from process()		
		*/
		void handleEvent(const EventChannel* eventInfo, const MidiMessage& event, int samplePosition) override;

		/** Handles spikes received by the processor

		Called automatically for each received event whenever checkForEvents(true) is called from process()
		*/
		void handleSpike(const SpikeChannel* spikeInfo, const MidiMessage& event, int samplePosition) override;

		/** The method that standard controls on the editor will call.
		It is recommended that any variables used by the "process" function
		are modified only through this method while data acquisition is active. */
		//void setParameter(int parameterIndex, float newValue) override;

		/** Saving custom settings to XML. */
		//void saveCustomParametersToXml(XmlElement* parentElement) override;

		/** Load custom settings from XML*/
		//void loadCustomParametersFromXml() override;

		/** Optional method called every time the signal chain is refreshed or changed in any way.

		Allows the processor to handle variations in the channel configuration or any other parameter
		passed down the signal chain. The processor can also modify here the settings structure, which contains
		information regarding the input and output channels as well as other signal related parameters. Said
		structure shouldn't be manipulated outside of this method.

		*/
		//void updateSettings() override;
		std::vector<double> getHistogram(int channel_idx, int sorted_id, int stim_class);
		int getNTrial();
		void setCanvas(SyncSinkCanvas* c);
		void setEditor(SyncSinkEditor* e);
		void addPSTHPlot(int channel_idx, int sorted_id, std::vector<int> stimClasses);
		void resetTensor();
		void rebin(int n_bins, int bin_size);
		String getStimClassLabel(int stim_class);
		int getNBins();
		int getBinSize();
		std::vector<int> getStimClasses();
		void clearVars();
		int numConditions = -1;
		SyncSinkCanvas* canvas = nullptr;
		SyncSinkEditor* thisEditor = nullptr;

	private:
		HashMap<String, String> conditionMap; // hashmap for image ids
		HashMap<String, int> conditionList; // hashmap for condition indexing
		HashMap<int, String> conditionListInverse; // hashmap for index to condition string
		HashMap<int, int> nTrialsByStimClass; // hashmap storing num trials for each condition
		std::vector<int> stimClasses;
		int currentStimClass = -1;
		int64 currentTrialStartTime = -1;
		bool inTrial = false;
		std::vector<std::vector<std::vector<std::vector<double>>>> spikeTensor; // n_channels * n_units * n_stim_classes * n_bins tensor for spike counts

		int nBins = 50; // default num bins
		int binSize = 10; // default bin sizes
		int nTrials = 0;
		void* context;
		void* socket;
		int dataport;
		
		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SyncSink);

	};
}

#endif