#include "SoapyRFSASDR.hpp"
#include "visa.h"
#include <SoapySDR/Registry.hpp>
#include <sstream>
#include <chrono>
#include <thread>
#include <stdio.h>
#include <stdlib.h>

#define INST_LIST_MAX 255
#define INST_LEN_MAX 256

static std::vector<SoapySDR::Kwargs> results;
static std::vector<SoapySDR::Kwargs> find_RFSASDR(const SoapySDR::Kwargs &args) {
	if (!results.empty())
		return results;
	printf("listing Visa Resources\n");
	ViSession sesn;
	viOpenDefaultRM(&sesn);
	ViFindList fl;
	ViUInt32 cnt = 0;
	char* desc = (char*)malloc(INST_LEN_MAX); // mem leak...
	viFindRsrc(sesn, "/?*", &fl, &cnt, desc);
	SoapySDR::Kwargs options;
	options["device"] = desc;
	results.push_back(options);
	for (int n = 0; n < cnt-1; n++) {
		char* desc = (char*)malloc(INST_LEN_MAX); // mem leak...
		viFindNext(fl, desc);
		SoapySDR::Kwargs options;
		options["device"] = desc;
		results.push_back(options);
	}
	return results;
}

static SoapySDR::Device *make_RFSASDR(const SoapySDR::Kwargs &args)
{
	return new SoapyRFSASDR(args);
}

static SoapySDR::Registry register_rfsasdr("rfsa", &find_RFSASDR, &make_RFSASDR, SOAPY_SDR_ABI_VERSION);
