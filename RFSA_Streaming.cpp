#include "SoapyRFSASDR.hpp"
#include <memory>
#include <iostream>
#include <cstdint>
#include <cstring>
#include <iterator>
#include <algorithm>
#include <chrono>
#include <stdio.h>
#include <stdlib.h>

#define MTU (1 << 19)
#define MAX_ARG_LEN 255


std::vector<std::string> SoapyRFSASDR::getStreamFormats(const int direction, const size_t channel) const
{
	std::vector<std::string> formats;

	formats.push_back(SOAPY_SDR_CS16);
	formats.push_back(SOAPY_SDR_CF32);

	return formats;
}

std::string SoapyRFSASDR::getNativeStreamFormat(const int direction, const size_t channel, double &fullScale) const
{
	if (direction != SOAPY_SDR_RX) {
         throw std::runtime_error("RFSA driver is RX only, use SOAPY_SDR_RX. TX streaming is not supported for NI VST.");
    }
	fullScale = 16384;
	return SOAPY_SDR_CS16;
}

SoapySDR::ArgInfoList SoapyRFSASDR::getStreamArgsInfo(const int direction, const size_t channel) const
{
	SoapySDR::ArgInfoList streamArgs;

	return streamArgs;
}


bool SoapyRFSASDR::IsValidRxStreamHandle(SoapySDR::Stream* handle) const
{
    if (handle == nullptr) {
        return false;
    }

    if(session){
		return true;
	}

    return false;
}

bool SoapyRFSASDR::IsValidTxStreamHandle(SoapySDR::Stream* handle) const
{
    return false;
}

SoapySDR::Stream *SoapyRFSASDR::setupStream(
		const int direction,
		const std::string &format,
		const std::vector<size_t> &channels,
		const SoapySDR::Kwargs &args )
{
	if (direction != SOAPY_SDR_RX) {
         throw std::runtime_error("RFSA driver is RX only, use SOAPY_SDR_RX. TX streaming is not supported for NI VST.");
    }
	
	if (format == SOAPY_SDR_CF32) {
		SoapySDR_log(SOAPY_SDR_INFO, "Using format CF32.");
		streamFormat = RFSA_SDR_CF32;
	}
	else if (format == SOAPY_SDR_CS16) {
		SoapySDR_log(SOAPY_SDR_INFO, "Using format CS16.");
		streamFormat = RFSA_SDR_CS16;
	}
	else {
		throw std::runtime_error(
			"setupStream invalid format '" + format + "' -- Only CS16 and CF32 are supported by SoapyRFSASDR module.");
	}
	char argtmp[MAX_ARG_LEN];

	if (args.count("reflevel") != 0) {
		strncpy(argtmp, args.at("reflevel").c_str(), MAX_ARG_LEN);
		this->_reflevel = std::stod(argtmp);
	}
	else {
		printf("Init: ref level not specified.\n");
	}
	/* Configure NI-RFSA for a simple IQ acquisition */
	checkWarn(niRFSA_ConfigureReferenceLevel(session, "", _reflevel));
	printf("Ref lvl: %f\n", _reflevel);
	checkWarn(niRFSA_ConfigureAcquisitionType(session, NIRFSA_VAL_IQ));
	if (args.count("carrierFreq") != 0) {
		strncpy(argtmp, args.at("carrierFreq").c_str(), MAX_ARG_LEN);
		this->_carrierFreq = std::stod(argtmp);
	}
	else {
		printf("Init: carrier freq not specified.\n");
	}
	checkWarn(niRFSA_ConfigureIQCarrierFrequency(session, "", _carrierFreq));
	printf("Carrier freq: %f\n", _carrierFreq);
	ViInt32 numberOfSamples = 1000; // not used in continous mode
	checkWarn(niRFSA_ConfigureNumberOfSamples(
		session, "", VI_FALSE, numberOfSamples)); // continous mode
	if (args.count("iqrate") != 0) {
		strncpy(argtmp, args.at("iqrate").c_str(), MAX_ARG_LEN);
		this->_iqrate = std::stod(argtmp);
	}
	else {
		printf("Init: IQ rate not specified.\n");
	}
	checkWarn(niRFSA_ConfigureIQRate(session, "", _iqrate));
	printf("IQrate: %f\n", _iqrate);

	
	Error:
	if (error < VI_SUCCESS) {
		niRFSA_GetError(session, &lastErrorCode, MAX_ERROR_DESCRIPTION, errorMessage);
		printf("ERROR: Init: %s\n", errorMessage);
	}
	else if (error > VI_SUCCESS) {
		niRFSA_GetError(session, &lastErrorCode, MAX_ERROR_DESCRIPTION, errorMessage);
		printf("WARNING: Init: %s\n", errorMessage);
	}

	return reinterpret_cast<SoapySDR::Stream*>(&session);

}

void SoapyRFSASDR::closeStream( SoapySDR::Stream *handle)
{
	if (session)
		niRFSA_Abort(session);
	printf("aborted session: %x\n", session);
}

size_t SoapyRFSASDR::getStreamMTU( SoapySDR::Stream *handle) const
{

	if (IsValidTxStreamHandle(handle)) {
		return MTU;
	}

    return 0;
}

int SoapyRFSASDR::activateStream(
		SoapySDR::Stream *handle,
		const int flags,
		const long long timeNs,
		const size_t numElems )
{
	if (session)
		niRFSA_Abort(session);
		printf("aborted session: %x\n", session);
	checkWarn(niRFSA_Initiate(session));
	printf("Init session.\n");
Error:
	if (error < VI_SUCCESS) {
		niRFSA_GetError(session, &lastErrorCode, MAX_ERROR_DESCRIPTION, errorMessage);
		printf("ERROR: Init: %s\n", errorMessage);
		return lastErrorCode;
	}
	else if (error > VI_SUCCESS) {
		niRFSA_GetError(session, &lastErrorCode, MAX_ERROR_DESCRIPTION, errorMessage);
		printf("WARNING: Init: %s\n", errorMessage);
		return 0;
	}
	else {
		return 0;
	}
}

int SoapyRFSASDR::deactivateStream(
		SoapySDR::Stream *handle,
		const int flags,
		const long long timeNs )
{
	if (session)
		niRFSA_Abort(session);
	printf("aborted session: %x\n", session);
	return 0;
}

int SoapyRFSASDR::readStream(
		SoapySDR::Stream *handle,
		void * const *buffs,
		const size_t numElems,
		int &flags,
		long long &timeNs,
		const long timeoutUs )
{
	if (needReInit) {
		//printf("Reinit after error.\n");
		if (session) {
			niRFSA_Abort(session);
			niRFSA_Initiate(session);
		}
		needReInit = false;
		error = 0;
	}
	int nsample;
	if (streamFormat == RFSA_SDR_CS16) {
		if (numElems < 2) {
			return 0;
		}
		nsample = numElems / 2;
		checkWarn(niRFSA_FetchIQSingleRecordComplexI16(session,
			"",
			0,
			nsample,
			10.0, /* seconds */
			static_cast<NIComplexI16*>(static_cast<void*>(buffs[0])),
			&this->_wfmInfo
		));
	}
	else if (streamFormat == RFSA_SDR_CF32) {
		nsample = numElems;
		checkWarn(niRFSA_FetchIQSingleRecordComplexF32(session,
			"",
			0,
			nsample,
			10.0, /* seconds */
			static_cast<NIComplexNumberF32*>(static_cast<void*>(buffs[0])),
			&_wfmInfo
		));
	}
	else {
		throw std::runtime_error("Unknown stream format");
	}
Error:
	if (error < VI_SUCCESS) {
		if (error == 0xfffa5e85) {
			SoapySDR_log(SOAPY_SDR_SSI, "U");
		}
		else {
			niRFSA_GetError(session, &lastErrorCode, MAX_ERROR_DESCRIPTION, errorMessage);
			printf("ERROR: %x, %s\n", error, errorMessage);
		}
		needReInit = true;
	}
	else if (error > VI_SUCCESS) {
		if (error == 0x5b10b) {
			SoapySDR_log(SOAPY_SDR_SSI, "B");
		}
		else {
			niRFSA_GetError(session, &lastErrorCode, MAX_ERROR_DESCRIPTION, errorMessage);
			printf("WARNING: %x, %s\n", error, errorMessage);
		}
	}
	return nsample;
}

int SoapyRFSASDR::writeStream(
	SoapySDR::Stream* handle,
	const void* const* buffs,
	const size_t numElems,
	int& flags,
	const long long timeNs,
	const long timeoutUs)
{
	return SOAPY_SDR_NOT_SUPPORTED;
}

int SoapyRFSASDR::readStreamStatus(
		SoapySDR::Stream *stream,
		size_t &chanMask,
		int &flags,
		long long &timeNs,
		const long timeoutUs)
{
	return SOAPY_SDR_NOT_SUPPORTED;
}