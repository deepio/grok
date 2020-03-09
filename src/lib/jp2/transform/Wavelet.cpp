#include "grok_includes.h"

#include "dwt_utils.h"
#include "dwt53.h"
#include "dwt97.h"
#include "WaveletForward.h"
#include "dwt.h"

namespace grk {

Wavelet::Wavelet() {
}

Wavelet::~Wavelet() {
}

bool Wavelet::encode(TileComponent *tile_comp, uint8_t qmfbid){
	if (qmfbid == 1) {
		WaveletForward<dwt53> dwt;
		return dwt.run(tile_comp);
	} else if (qmfbid == 0) {
		WaveletForward<dwt97> dwt;
		return dwt.run(tile_comp);
	}
	return false;
}

bool Wavelet::decode(TileProcessor *p_tcd,  TileComponent* tilec,
                             uint32_t numres, uint8_t qmfbid){

	if (qmfbid == 1) {
		return decode_53(p_tcd,tilec,numres);
	} else if (qmfbid == 0) {
		return decode_97(p_tcd,tilec,numres);
	}
	return false;
}

}
