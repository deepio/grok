/*
*    Copyright (C) 2016-2019 Grok Image Compression Inc.
*
*    This source code is free software: you can redistribute it and/or  modify
*    it under the terms of the GNU Affero General Public License, version 3,
*    as published by the Free Software Foundation.
*
*    This source code is distributed in the hope that it will be useful,
*    but WITHOUT ANY WARRANTY; without even the implied warranty of
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*    GNU Affero General Public License for more details.
*
*    You should have received a copy of the GNU Affero General Public License
*    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
*
*    This source code incorporates work covered by the following copyright and
*    permission notice:
*
 * The copyright in this software is being made available under the 2-clauses
 * BSD License, included below. This software may be subject to other third
 * party and contributor rights, including patent rights, and no such rights
 * are granted under this license.
 *
 * Copyright (c) 2002-2014, Universite catholique de Louvain (UCL), Belgium
 * Copyright (c) 2002-2014, Professor Benoit Macq
 * Copyright (c) 2001-2003, David Janssens
 * Copyright (c) 2002-2003, Yannick Verschueren
 * Copyright (c) 2003-2007, Francois-Olivier Devaux
 * Copyright (c) 2003-2014, Antonin Descampe
 * Copyright (c) 2005, Herve Drolon, FreeImage Team
 * Copyright (c) 2006-2007, Parvatha Elangovan
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS `AS IS'
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <cstdio>
#include <cstdlib>
#include "opj_apps_config.h"
#include "openjpeg.h"
#include "RAWFormat.h"
#include "convert.h"
#include "common.h"

// swap endian for 16 bit int
template<typename T> static inline T swap(T x)
{
	return (T)(((x >> 8) & 0x00ff) | ((x & 0x00ff) << 8));
}
// no-op specialization for 8 bit
template<> inline uint8_t swap(uint8_t x)
{
	return x;
}
// no-op specialization for 8 bit
template<> inline int8_t swap(int8_t x)
{
	return x;
}
template<typename T> static inline T endian(T x, bool fromBigEndian){

#ifdef GROK_BIG_ENDIAN
	if (!fromBigEndian)
	   return swap<T>(x);
#else
	if (fromBigEndian)
	   return swap<T>(x);
#endif
	return x;
}

bool RAWFormat::encode(opj_image_t* image, const char* filename, int compressionParam, bool verbose) {
	(void)compressionParam;
	return imagetoraw(image, filename, bigEndian,verbose) ? true : false;
}
opj_image_t* RAWFormat::decode(const char* filename, opj_cparameters_t *parameters) {
	return rawtoimage(filename, parameters, bigEndian);
}

template<typename T> static bool readBytes(FILE *rawFile, bool fromBigEndian,
										int32_t* ptr,
										uint64_t nloop){
	const size_t bufSize = 4096;
	uint8_t buf[bufSize];

	for (uint64_t i = 0; i < nloop; i+=bufSize) {
		size_t ct = fread(buf, 1, bufSize, rawFile);
		if (!ct)
			return false;
		uint8_t *inPtr = buf;
		for (size_t j = 0; j < ct; j += sizeof(T)){
		    T* value = (T*)(inPtr);
		    *(ptr++) = endian<T>(*value, fromBigEndian);
		    inPtr += sizeof(T);
		}
	}

	return true;
}

opj_image_t* RAWFormat::rawtoimage(const char *filename,
										opj_cparameters_t *parameters,
										bool big_endian)
{
	bool readFromStdin = grk::useStdio(filename);
	raw_cparameters_t *raw_cp = &parameters->raw_cp;
	uint32_t subsampling_dx = parameters->subsampling_dx;
	uint32_t subsampling_dy = parameters->subsampling_dy;

	FILE *f = nullptr;
	uint32_t i, compno, numcomps, w, h;
	OPJ_COLOR_SPACE color_space;
	opj_image_cmptparm_t *cmptparm;
	opj_image_t * image = nullptr;
	unsigned short ch;
	bool success = true;

	if (!(raw_cp->rawWidth && raw_cp->rawHeight && raw_cp->rawComp && raw_cp->rawBitDepth)) {
		fprintf(stderr, "[ERROR] invalid raw image parameters\n");
		fprintf(stderr, "Please use the Format option -F:\n");
		fprintf(stderr, "-F <width>,<height>,<ncomp>,<bitdepth>,{s,u}@<dx1>x<dy1>:...:<dxn>x<dyn>\n");
		fprintf(stderr, "If subsampling is omitted, 1x1 is assumed for all components\n");
		fprintf(stderr, "Example: -i image.raw -o image.j2k -F 512,512,3,8,u@1x1:2x2:2x2\n");
		fprintf(stderr, "         for raw 512x512 image with 4:2:0 subsampling\n");
		return nullptr;
	}

	if (readFromStdin) {
		if (!grok_set_binary_mode(stdin))
			return nullptr;
		f = stdin;
	}
	else {
		f = fopen(filename, "rb");
		if (!f) {
			fprintf(stderr, "[ERROR] Failed to open %s for reading !!\n", filename);
			success = false;
			goto cleanup;
		}
	}
	numcomps = raw_cp->rawComp;
	if (numcomps == 1) {
		color_space = OPJ_CLRSPC_GRAY;
	}
	else if ((numcomps >= 3) && (parameters->tcp_mct == 0)) {
		color_space = OPJ_CLRSPC_SYCC;
	}
	else if ((numcomps >= 3) && (parameters->tcp_mct != 2)) {
		color_space = OPJ_CLRSPC_SRGB;
	}
	else {
		color_space = OPJ_CLRSPC_UNKNOWN;
	}
	w = raw_cp->rawWidth;
	h = raw_cp->rawHeight;
	cmptparm = (opj_image_cmptparm_t*)calloc(numcomps, sizeof(opj_image_cmptparm_t));
	if (!cmptparm) {
		fprintf(stderr, "[ERROR] Failed to allocate image components parameters !!\n");
		success = false;
		goto cleanup;
	}
	/* initialize image components */
	for (i = 0; i < numcomps; i++) {
		cmptparm[i].prec = raw_cp->rawBitDepth;
		cmptparm[i].sgnd = raw_cp->rawSigned;
		cmptparm[i].dx = subsampling_dx * raw_cp->rawComps[i].dx;
		cmptparm[i].dy = subsampling_dy * raw_cp->rawComps[i].dy;
		cmptparm[i].w = w;
		cmptparm[i].h = h;
	}
	/* create the image */
	image = opj_image_create(numcomps, &cmptparm[0], color_space);
	free(cmptparm);
	if (!image) {
		success = false;
		goto cleanup;
	}
	/* set image offset and reference grid */
	image->x0 = parameters->image_offset_x0;
	image->y0 = parameters->image_offset_y0;
	image->x1 = parameters->image_offset_x0 + (w - 1) *	subsampling_dx + 1;
	image->y1 = parameters->image_offset_y0 + (h - 1) * subsampling_dy + 1;

	if (raw_cp->rawBitDepth <= 8) {
		for (compno = 0; compno < numcomps; compno++) {
			int32_t *ptr = image->comps[compno].data;
			uint64_t nloop = ((uint64_t)w*h) / (raw_cp->rawComps[compno].dx*raw_cp->rawComps[compno].dy);
			bool rc;
			if (raw_cp->rawSigned)
				rc = readBytes<int8_t>(f, big_endian, ptr,nloop);
			else
				rc = readBytes<uint8_t>(f, big_endian, ptr,nloop);
			if (!rc){
				fprintf(stderr, "[ERROR] Error reading raw file. End of file probably reached.\n");
				success = false;
				goto cleanup;
			}
		}
	}
	else if (raw_cp->rawBitDepth <= 16) {
		for (compno = 0; compno < numcomps; compno++) {
			auto ptr = image->comps[compno].data;
			uint64_t nloop = ((uint64_t)w*h*sizeof(uint16_t)) / (raw_cp->rawComps[compno].dx*raw_cp->rawComps[compno].dy);
			bool rc;
			if (raw_cp->rawSigned)
				rc = readBytes<int16_t>(f, big_endian, ptr,nloop);
			else
				rc = readBytes<uint16_t>(f, big_endian, ptr,nloop);
			if (!rc){
				fprintf(stderr, "[ERROR] Error reading raw file. End of file probably reached.\n");
				success = false;
				goto cleanup;
			}
		}
	}
	else {
		fprintf(stderr, "[ERROR] Grok cannot encode raw components with bit depth higher than 16 bits.\n");
		success = false;
		goto cleanup;
	}

	if (fread(&ch, 1, 1, f)) {
		if (parameters->verbose)
			fprintf(stdout, "[WARNING] End of raw file not reached... processing anyway\n");
	}
cleanup:
	if (f && !readFromStdin){
		if (!grk::safe_fclose(f)){
			opj_image_destroy(image);
			image = nullptr;
		}
	}
	if (!success){
		opj_image_destroy(image);
		image = nullptr;
	}
	return image;
}

template<typename T> static bool writeBytes(FILE *rawFile, bool fromBigEndian,
										int32_t* ptr,
										uint32_t w, uint32_t h,
										int32_t lower, int32_t upper){
	const size_t bufSize = 4096;
	uint8_t buf[bufSize];
	uint8_t *outPtr = buf;
	size_t outCount = 0;

	for (uint32_t line = 0; line < h; line++) {
		for (uint32_t row = 0; row < w; row++) {
			int32_t curr = *ptr++;
			if (curr > upper)
				curr = upper;
			else if (curr < lower)
				curr = lower;
			T uc = endian<T>((T)(curr), fromBigEndian);
			uint8_t* currPtr = (uint8_t*)&uc;
			for (uint32_t i = 0; i < sizeof(T) && outCount < bufSize; ++i){
				*outPtr++ = *currPtr++;
				outCount++;
			}
			if (outCount == bufSize) {
				size_t res = fwrite(buf, 1, bufSize, rawFile);
				if (res != bufSize)
					return false;
				outCount = 0;
				outPtr = buf;
			}
		}
	}
	//flush
	if (outCount) {
		size_t res = fwrite(buf, 1, outCount, rawFile);
		if (res != outCount)
			return false;
	}

	return true;
}

int RAWFormat::imagetoraw(opj_image_t * image, 
							const char *outfile,
							bool big_endian,
							bool verbose)
{
	bool writeToStdout = grk::useStdio(outfile);
	FILE *rawFile = nullptr;
	unsigned int compno, numcomps;
	int fails;
	if ((image->numcomps * image->x1 * image->y1) == 0) {
		fprintf(stderr, "[ERROR] invalid raw image parameters\n");
		return 1;
	}

	numcomps = image->numcomps;

	if (numcomps > 4) {
		numcomps = 4;
	}

	for (compno = 1; compno < numcomps; ++compno) {
		if (image->comps[0].dx != image->comps[compno].dx) {
			break;
		}
		if (image->comps[0].dy != image->comps[compno].dy) {
			break;
		}
		if (image->comps[0].prec != image->comps[compno].prec) {
			break;
		}
		if (image->comps[0].sgnd != image->comps[compno].sgnd) {
			break;
		}
	}
	if (compno != numcomps) {
		fprintf(stderr, "[ERROR] imagetoraw_common: All components shall have the same subsampling, same bit depth, same sign.\n");
		return 1;
	}

	if (writeToStdout) {
		if (!grok_set_binary_mode(stdout))
			return 1;
		rawFile = stdout;
	}
	else {
		rawFile = fopen(outfile, "wb");
		if (!rawFile) {
			fprintf(stderr, "[ERROR] Failed to open %s for writing !!\n", outfile);
			return 1;
		}
	}

	fails = 1;
	if (verbose)
		fprintf(stdout, "Raw image characteristics: %d components\n", image->numcomps);

	for (compno = 0; compno < image->numcomps; compno++) {
		if (verbose)
			fprintf(stdout, "Component %u characteristics: %dx%dx%d %s\n", compno, image->comps[compno].w,
				image->comps[compno].h, image->comps[compno].prec, image->comps[compno].sgnd == 1 ? "signed" : "unsigned");

		auto w = image->comps[compno].w;
		auto h = image->comps[compno].h;
		bool sgnd = image->comps[compno].sgnd ;
		auto prec = image->comps[compno].prec;

		int32_t lower = sgnd ? -(1 << (prec-1)) : 0;
		int32_t upper = sgnd? -lower -1 : (1 << image->comps[compno].prec) - 1;
		int32_t *ptr = image->comps[compno].data;

		bool rc;
		if (prec <= 8) {
			if (sgnd)
				rc = writeBytes<int8_t>(rawFile, big_endian, ptr, w,h,lower,upper);
			else
				rc = writeBytes<uint8_t>(rawFile, big_endian, ptr, w,h,lower,upper);
			if (!rc)
				fprintf(stderr, "[ERROR] failed to write bytes for %s\n", outfile);
		}
		else if (prec <= 16) {
			if (sgnd)
				rc = writeBytes<int16_t>(rawFile, big_endian,ptr, w,h,lower,upper);
			else
				rc = writeBytes<uint16_t>(rawFile, big_endian,ptr, w,h,lower,upper);
			if (!rc)
				fprintf(stderr, "[ERROR] failed to write bytes for %s\n", outfile);
		}
		else if (image->comps[compno].prec <= 32) {
			fprintf(stderr, "[ERROR] More than 16 bits per component no handled yet\n");
			goto beach;
		}
		else {
			fprintf(stderr, "[ERROR] invalid precision: %d\n", image->comps[compno].prec);
			goto beach;
		}
	}
	fails = 0;
beach:
	if (!writeToStdout && rawFile){
		if (!grk::safe_fclose(rawFile))
			fails = 1;
	}
	return fails;
}
