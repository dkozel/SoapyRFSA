#include <niRFSA.h>
#include <vector>
#include <mutex>
#include <thread>
#include <chrono>
#include <atomic>
#include <SoapySDR/Device.hpp>
#include <SoapySDR/Logger.hpp>
#include <SoapySDR/Types.hpp>
#include <SoapySDR/Formats.hpp>
#include <stdio.h>
#include <stdlib.h>

#define MAX_RESNAME_LEN 255
#define MAX_ERROR_DESCRIPTION 2500

typedef enum rfsaStreamFormat {
	RFSA_SDR_CF32,
	RFSA_SDR_CS16
} rfsaStreamFormat;


class SoapyRFSASDR : public SoapySDR::Device{

	public:
		SoapyRFSASDR( const SoapySDR::Kwargs & args );
		~SoapyRFSASDR();

		/*******************************************************************
		 * Identification API
		 ******************************************************************/

		std::string getDriverKey( void ) const;


		std::string getHardwareKey( void ) const;


		SoapySDR::Kwargs getHardwareInfo( void ) const;


		/*******************************************************************
		 * Channels API
		 ******************************************************************/

		size_t getNumChannels( const int ) const;


		bool getFullDuplex( const int direction, const size_t channel ) const;

		/*******************************************************************
		 * Stream API
		 ******************************************************************/

		std::vector<std::string> getStreamFormats(const int direction, const size_t channel) const;

		std::string getNativeStreamFormat(const int direction, const size_t channel, double &fullScale) const;

		SoapySDR::ArgInfoList getStreamArgsInfo(const int direction, const size_t channel) const;

		SoapySDR::Stream *setupStream(
				const int direction,
				const std::string &format,
				const std::vector<size_t> &channels = std::vector<size_t>(),
				const SoapySDR::Kwargs &args = SoapySDR::Kwargs() );

		void closeStream( SoapySDR::Stream *stream );

		size_t getStreamMTU( SoapySDR::Stream *stream ) const;

		int activateStream(
				SoapySDR::Stream *stream,
				const int flags = 0,
				const long long timeNs = 0,
				const size_t numElems = 0 );

		int deactivateStream(
				SoapySDR::Stream *stream,
				const int flags = 0,
				const long long timeNs = 0 );

		int readStream(
				SoapySDR::Stream *stream,
				void * const *buffs,
				const size_t numElems,
				int &flags,
				long long &timeNs,
				const long timeoutUs = 100000 );

		int writeStream(
				SoapySDR::Stream *stream,
				const void * const *buffs,
				const size_t numElems,
				int &flags,
				const long long timeNs = 0,
				const long timeoutUs = 100000);

		int readStreamStatus(
				SoapySDR::Stream *stream,
				size_t &chanMask,
				int &flags,
				long long &timeNs,
				const long timeoutUs
				);


		/*******************************************************************
 		 * Sensor API

		std::vector<std::string> listSensors(void) const;

		SoapySDR::ArgInfo getSensorInfo(const std::string &key) const;

		std::string readSensor(const std::string &key) const;
 		 ******************************************************************/


		/*******************************************************************
		 * Settings API

		SoapySDR::ArgInfoList getSettingInfo(void) const;


		void writeSetting(const std::string &key, const std::string &value);


		std::string readSetting(const std::string &key) const;
		 ******************************************************************/


		/*******************************************************************
		 * Antenna API

		std::vector<std::string> listAntennas( const int direction, const size_t channel ) const;


		void setAntenna( const int direction, const size_t channel, const std::string &name );


		std::string getAntenna( const int direction, const size_t channel ) const;
		 ******************************************************************/


		/*******************************************************************
		 * Frontend corrections API

		bool hasDCOffsetMode( const int direction, const size_t channel ) const;
		 ******************************************************************/


		/*******************************************************************
		 * Gain API
		 ******************************************************************/

		std::vector<std::string> listGains( const int direction, const size_t channel ) const;


		bool hasGainMode(const int direction, const size_t channel) const;


		void setGainMode( const int direction, const size_t channel, const bool automatic );


		bool getGainMode( const int direction, const size_t channel ) const;


		void setGain( const int direction, const size_t channel, const double value );


		void setGain( const int direction, const size_t channel, const std::string &name, const double value );


		double getGain( const int direction, const size_t channel, const std::string &name ) const;


		SoapySDR::Range getGainRange( const int direction, const size_t channel, const std::string &name ) const;


		/*******************************************************************
		 * Frequency API
		 ******************************************************************/

		void setFrequency( const int direction, const size_t channel, const std::string &name, const double frequency, const SoapySDR::Kwargs &args = SoapySDR::Kwargs() );


		double getFrequency( const int direction, const size_t channel, const std::string &name ) const;


		SoapySDR::ArgInfoList getFrequencyArgsInfo(const int direction, const size_t channel) const;


		std::vector<std::string> listFrequencies( const int direction, const size_t channel ) const;


		SoapySDR::RangeList getFrequencyRange( const int direction, const size_t channel, const std::string &name ) const;


		/*******************************************************************
		 * Sample Rate API
		 ******************************************************************/

		void setSampleRate( const int direction, const size_t channel, const double rate );


		double getSampleRate( const int direction, const size_t channel ) const;


		std::vector<double> listSampleRates( const int direction, const size_t channel ) const;


		void setBandwidth( const int direction, const size_t channel, const double bw );


		double getBandwidth( const int direction, const size_t channel ) const;


		std::vector<double> listBandwidths( const int direction, const size_t channel ) const;

		SoapySDR::RangeList getSampleRateRange(const int direction, const size_t channel) const;

	private:

        bool IsValidRxStreamHandle(SoapySDR::Stream* handle) const;
        bool IsValidTxStreamHandle(SoapySDR::Stream* handle) const;
       

		rfsaStreamFormat format;
		
		ViSession session = VI_NULL;
		//NIComplexI16* dataPtr = nullptr;
		niRFSA_wfmInfo _wfmInfo;
		ViStatus error = VI_SUCCESS;
		ViChar errorMessage[MAX_ERROR_DESCRIPTION];
		ViStatus lastErrorCode;
		rfsaStreamFormat streamFormat;

		bool needReInit = false;

		char* _resname = "RIO0";
		double _iqrate = 4e6;
		double _carrierFreq = 1e9;
		double _reflevel = 10;

        
};

