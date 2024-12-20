#include "SoapyRFSASDR.hpp"
SoapyRFSASDR::SoapyRFSASDR(const SoapySDR::Kwargs& args)
{
	if (args.count("resname") != 0) {
		strncpy(_resname, args.at("resname").c_str(), MAX_RESNAME_LEN);
	}

	/* Initialize a session */
	printf("Opening device: %s\n", _resname);
	checkWarn(niRFSA_init(static_cast<ViRsrc>(_resname), VI_TRUE, VI_FALSE, &session));
	printf("Getting session: %x\n", session);
	checkWarn(niRFSA_ConfigureRefClock(session, "OnboardClock", 10e6));
	printf("Ref clock set.\n");
Error:
	if (error < VI_SUCCESS) {
		niRFSA_GetError(session, &lastErrorCode, MAX_ERROR_DESCRIPTION, errorMessage);
		printf("ERROR: ctorInit: %s\n", errorMessage);
	}
	else if (error > VI_SUCCESS) {
		niRFSA_GetError(session, &lastErrorCode, MAX_ERROR_DESCRIPTION, errorMessage);
		printf("WARNING: ctorInit: %s\n", errorMessage);
	}
}


SoapyRFSASDR::~SoapyRFSASDR(void){
	niRFSA_close(session);
}

/*******************************************************************
 * Identification API
 ******************************************************************/

std::string SoapyRFSASDR::getDriverKey( void ) const
{
	return "NI RFSA";
}

std::string SoapyRFSASDR::getHardwareKey( void ) const
{
	return "NI VST";
}

SoapySDR::Kwargs SoapyRFSASDR::getHardwareInfo( void ) const
{
	SoapySDR::Kwargs info;

	info["library_version"] = "0.0.0";
	info["backend_version"] = "0.0.0";
	return info;
}


/*******************************************************************
 * Channels API
 ******************************************************************/

size_t SoapyRFSASDR::getNumChannels( const int dir ) const
{
	return(1);
}

bool SoapyRFSASDR::getFullDuplex( const int direction, const size_t channel ) const
{
	return(false);
}

/*******************************************************************
 * Settings API

SoapySDR::ArgInfoList SoapyRFSASDR::getSettingInfo(void) const
{
	SoapySDR::ArgInfoList setArgs;

	return setArgs;
}

void SoapyRFSASDR::writeSetting(const std::string &key, const std::string &value)
{



}


std::string SoapyRFSASDR::readSetting(const std::string &key) const
{
	std::string info;

	return info;
}
 ******************************************************************/

/*******************************************************************
 * Antenna API

std::vector<std::string> SoapyRFSASDR::listAntennas( const int direction, const size_t channel ) const
{
	std::vector<std::string> options;
	if(direction == SOAPY_SDR_RX) options.push_back( "A_BALANCED" );
	if(direction == SOAPY_SDR_TX) options.push_back( "A" );
	return(options);
}

void SoapyRFSASDR::setAntenna( const int direction, const size_t channel, const std::string &name )
{
   if (direction == SOAPY_SDR_RX) {
       std::lock_guard<pluto_spin_mutex> lock(rx_device_mutex);
		iio_channel_attr_write(iio_device_find_channel(dev, "voltage0", false), "rf_port_select", name.c_str());
	}

	else if (direction == SOAPY_SDR_TX) {
        std::lock_guard<pluto_spin_mutex> lock(tx_device_mutex);
		iio_channel_attr_write(iio_device_find_channel(dev, "voltage0", true), "rf_port_select", name.c_str());

	} 
}


std::string SoapyRFSASDR::getAntenna( const int direction, const size_t channel ) const
{
	std::string options;

	if (direction == SOAPY_SDR_RX) {
		options = "A_BALANCED";
	}
	else if (direction == SOAPY_SDR_TX) {

		options = "A";
	}
	return options;
}
 ******************************************************************/

/*******************************************************************
 * Frontend corrections API

bool SoapyRFSASDR::hasDCOffsetMode( const int direction, const size_t channel ) const
{
	return(false);
}
******************************************************************/

/*******************************************************************
 * Gain API
 ******************************************************************/

std::vector<std::string> SoapyRFSASDR::listGains( const int direction, const size_t channel ) const
{
	std::vector<std::string> options;
	options.push_back("Reference Level");
	return(options);
}

bool SoapyRFSASDR::hasGainMode(const int direction, const size_t channel) const
{
	return false;
}

void SoapyRFSASDR::setGainMode( const int direction, const size_t channel, const bool automatic )
{
}

bool SoapyRFSASDR::getGainMode(const int direction, const size_t channel) const
{
	return false;
}

void SoapyRFSASDR::setGain( const int direction, const size_t channel, const double value )
{
	this->_reflevel = -value;
	if (this->session) {
		checkWarn(niRFSA_Abort(this->session));
	}
	checkWarn(niRFSA_ConfigureReferenceLevel(session, "", _reflevel));
	printf("setGain: Ref lvl: %f\n", _reflevel);
	checkWarn(niRFSA_Initiate(session));
	//printf("CB: Init session.\n");
Error:
	if (error < VI_SUCCESS) {
		niRFSA_GetError(session, &lastErrorCode, MAX_ERROR_DESCRIPTION, errorMessage);
		printf("ERROR: setGain: %s\n", errorMessage);
	}
	else if (error > VI_SUCCESS) {
		niRFSA_GetError(session, &lastErrorCode, MAX_ERROR_DESCRIPTION, errorMessage);
		printf("WARNING: setGain: %s\n", errorMessage);
	}
}

void SoapyRFSASDR::setGain( const int direction, const size_t channel, const std::string &name, const double value )
{
	this->setGain(direction,channel,value);
}

double SoapyRFSASDR::getGain(const int direction, const size_t channel, const std::string& name) const
{
	return -_reflevel;
}

SoapySDR::Range SoapyRFSASDR::getGainRange( const int direction, const size_t channel, const std::string &name ) const
{
	return(SoapySDR::Range(-20, 50));
}

/*******************************************************************
 * Frequency API
 ******************************************************************/

void SoapyRFSASDR::setFrequency( const int direction, const size_t channel, const std::string &name, const double frequency, const SoapySDR::Kwargs &args )
{
	this->_carrierFreq = frequency;
	if (this->session) {
		checkWarn(niRFSA_Abort(this->session));
	}
	checkWarn(niRFSA_ConfigureIQCarrierFrequency(session, "", _carrierFreq));
	printf("setFrequency: Carrier freq: %f\n", _carrierFreq);
	checkWarn(niRFSA_Initiate(session));
	//printf("CB: Init session.\n");
Error:
	if (error < VI_SUCCESS) {
		niRFSA_GetError(session, &lastErrorCode, MAX_ERROR_DESCRIPTION, errorMessage);
		printf("ERROR: setFrequency: %s\n", errorMessage);
	}
	else if (error > VI_SUCCESS) {
		niRFSA_GetError(session, &lastErrorCode, MAX_ERROR_DESCRIPTION, errorMessage);
		printf("WARNING: setFrequency: %s\n", errorMessage);
	}
}

double SoapyRFSASDR::getFrequency( const int direction, const size_t channel, const std::string &name ) const
{
	return double(this->_carrierFreq);
}

SoapySDR::ArgInfoList SoapyRFSASDR::getFrequencyArgsInfo(const int direction, const size_t channel) const
{

	SoapySDR::ArgInfoList freqArgs;

	return freqArgs;
}

std::vector<std::string> SoapyRFSASDR::listFrequencies( const int direction, const size_t channel ) const
{
	std::vector<std::string> names;
	names.push_back( "RF" );
	return(names);
}

SoapySDR::RangeList SoapyRFSASDR::getFrequencyRange( const int direction, const size_t channel, const std::string &name ) const
{
	return(SoapySDR::RangeList( 1, SoapySDR::Range( 70000000, 6000000000ull ) ) );

}

/*******************************************************************
 * Sample Rate API
 ******************************************************************/
void SoapyRFSASDR::setSampleRate( const int direction, const size_t channel, const double rate )
{
	this->_iqrate = rate;
	if (this->session) {
		checkWarn(niRFSA_Abort(this->session));
	}
	checkWarn(niRFSA_ConfigureIQRate(session, "", this->_iqrate));
	printf("setSampleRate: IQrate: %f\n", _iqrate);
	checkWarn(niRFSA_Initiate(session));
Error:
	if (error < VI_SUCCESS) {
		niRFSA_GetError(session, &lastErrorCode, MAX_ERROR_DESCRIPTION, errorMessage);
		printf("ERROR: setSampleRate: %s\n", errorMessage);
	}
	else if (error > VI_SUCCESS) {
		niRFSA_GetError(session, &lastErrorCode, MAX_ERROR_DESCRIPTION, errorMessage);
		printf("WARNING: setSampleRate: %s\n", errorMessage);
	}
}

double SoapyRFSASDR::getSampleRate( const int direction, const size_t channel ) const
{
	return this->_iqrate;

}

std::vector<double> SoapyRFSASDR::listSampleRates( const int direction, const size_t channel ) const
{
	std::vector<double> options;

	options.push_back(65105);//25M/48/8+1
	options.push_back(1e6);
	options.push_back(2e6);
	options.push_back(3e6);
	options.push_back(4e6);
	options.push_back(5e6);
	options.push_back(6e6);
	options.push_back(7e6);
	options.push_back(8e6);
	options.push_back(9e6);
	options.push_back(10e6);
	return(options);
}

SoapySDR::RangeList SoapyRFSASDR::getSampleRateRange( const int direction, const size_t channel ) const
{
	SoapySDR::RangeList results;
	results.push_back(SoapySDR::Range(2e6, 120e6));
	return results;
}

void SoapyRFSASDR::setBandwidth( const int direction, const size_t channel, const double bw )
{
	this->setSampleRate(direction, channel, bw);
}

double SoapyRFSASDR::getBandwidth( const int direction, const size_t channel ) const
{
	return double(this->_iqrate);
}

std::vector<double> SoapyRFSASDR::listBandwidths( const int direction, const size_t channel ) const
{
	std::vector<double> options;
	options.push_back(0.2e6);
	options.push_back(1e6);
	options.push_back(2e6);
	options.push_back(3e6);
	options.push_back(4e6);
	options.push_back(5e6);
	options.push_back(6e6);
	options.push_back(7e6);
	options.push_back(8e6);
	options.push_back(9e6);
	options.push_back(10e6);
	return(options);
}
