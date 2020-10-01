/*
 *    Copyright (C) 2016-2020 Grok Image Compression Inc.
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
 *    This source code incorporates work covered by the BSD 2-clause license.
 *    Please see the LICENSE file in the root directory for details.
 *
 */
#include "grk_config.h"
#include "common.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#ifdef _WIN32
#include "../common/windirent.h"
#else
#include <dirent.h>
#endif /* _WIN32 */

#ifdef _WIN32
#include <windows.h>
#else
#include <strings.h>
#define _stricmp strcasecmp
#define _strnicmp strncasecmp
#endif /* _WIN32 */

#define TCLAP_NAMESTARTSTRING "-"
#include "tclap/CmdLine.h"

using namespace TCLAP;
using namespace std;

#include "convert.h"
#include "grk_string.h"
#include <string>


typedef struct _dircnt {
	/** Buffer for holding images read from Directory*/
	char *filename_buf;
	/** Pointer to the buffer*/
	char **filename;
} dircnt;

typedef struct _img_folder {
	/** The directory path of the folder containing input images*/
	char *imgdirpath;
	/** Output format*/
	const char *out_format;
	/** Enable option*/
	bool set_imgdir;
	/** Enable Cod Format for output*/
	bool set_out_format;

	uint32_t flag;
} img_fol;


static uint32_t get_num_images(char *imgdirpath);
static int load_images(dircnt *dirptr, char *imgdirpath);
static char get_next_file(size_t imageno, dircnt *dirptr, img_fol *img_fol,
		grk_dparameters *parameters);
static int parse_cmdline_decoder(int argc, char **argv,
		grk_dparameters *parameters, img_fol *img_fol);

/* -------------------------------------------------------------------------- */
static void decode_help_display(void) {
	fprintf(stdout, "\nThis is the grk_dump utility from the Grok project.\n"
			"It dumps JPEG 2000 code stream info to stdout or a given file.\n"
			"It has been compiled against Grok library v%s.\n\n",
			grk_version());

	fprintf(stdout, "Parameters:\n");
	fprintf(stdout, "-----------\n");
	fprintf(stdout, "\n");
	fprintf(stdout, "  -ImgDir <directory>\n");
	fprintf(stdout, "	Image file Directory path \n");
	fprintf(stdout, "  -i <compressed file>\n");
	fprintf(stdout,
			"    REQUIRED only if an Input image directory not specified\n");
	fprintf(stdout,
			"    Currently accepts J2K-files and JP2-files. The file type\n");
	fprintf(stdout, "    is identified based on its suffix.\n");
	fprintf(stdout, "  -o <output file>\n");
	fprintf(stdout, "    OPTIONAL\n");
	fprintf(stdout, "    Output file where file info will be dump.\n");
	fprintf(stdout, "    By default it will be in the stdout.\n");
	fprintf(stdout, "  -v ");
	fprintf(stdout, "    OPTIONAL\n");
	fprintf(stdout, "    Enable informative messages\n");
	fprintf(stdout, "    By default verbose mode is off.\n");
	fprintf(stdout, "\n");
}

class GrokOutput: public StdOutput {
public:
	virtual void usage(CmdLineInterface &c) {
		(void) c;
		decode_help_display();
	}
};


/* -------------------------------------------------------------------------- */
static uint32_t get_num_images(char *imgdirpath) {
	DIR *dir;
	struct dirent *content;
	uint32_t num_images = 0;

	/*Reading the input images from given input directory*/

	dir = opendir(imgdirpath);
	if (!dir) {
		spdlog::error("Could not open Folder {}", imgdirpath);
		return 0;
	}

	while ((content = readdir(dir)) != nullptr) {
		if (strcmp(".", content->d_name) == 0
				|| strcmp("..", content->d_name) == 0)
			continue;
		num_images++;
	}
	closedir(dir);
	return num_images;
}

/* -------------------------------------------------------------------------- */
static int load_images(dircnt *dirptr, char *imgdirpath) {
	DIR *dir;
	struct dirent *content;
	int i = 0;

	/*Reading the input images from given input directory*/

	dir = opendir(imgdirpath);
	if (!dir) {
		spdlog::error("Could not open Folder {}", imgdirpath);
		return 1;
	}

	while ((content = readdir(dir)) != nullptr) {
		if (strcmp(".", content->d_name) == 0
				|| strcmp("..", content->d_name) == 0)
			continue;

		strcpy(dirptr->filename[i], content->d_name);
		i++;
	}
	closedir(dir);
	return 0;
}
/* -------------------------------------------------------------------------- */
static char get_next_file(size_t imageno, dircnt *dirptr, img_fol *img_fol,
		grk_dparameters *parameters) {
	char image_filename[GRK_PATH_LEN], infilename[3 * GRK_PATH_LEN],
			temp_ofname[GRK_PATH_LEN];
	char *temp_p, temp1[GRK_PATH_LEN] = "";

	strcpy(image_filename, dirptr->filename[imageno]);
	spdlog::info("File Number {} \"{}\"", imageno, image_filename);
	if (!grk::jpeg2000_file_format(image_filename, &parameters->decod_format))
		return 1;
	sprintf(infilename, "%s/%s", img_fol->imgdirpath, image_filename);
	if (grk::strcpy_s(parameters->infile, sizeof(parameters->infile),
			infilename) != 0) {
		return 1;
	}

	/*Set output file*/
	strcpy(temp_ofname, strtok(image_filename, "."));
	while ((temp_p = strtok(nullptr, ".")) != nullptr) {
		strcat(temp_ofname, temp1);
		sprintf(temp1, ".%s", temp_p);
	}
	if (img_fol->set_out_format) {
		char outfilename[3 * GRK_PATH_LEN];
		sprintf(outfilename, "%s/%s.%s", img_fol->imgdirpath, temp_ofname,
				img_fol->out_format);
		if (grk::strcpy_s(parameters->outfile, sizeof(parameters->outfile),
				outfilename) != 0) {
			return 1;
		}
	}
	return 0;
}


/* -------------------------------------------------------------------------- */
/**
 * Parse the command line
 */
/* -------------------------------------------------------------------------- */
static int parse_cmdline_decoder(int argc, char **argv,
		grk_dparameters *parameters, img_fol *img_fol) {


	try {
		CmdLine cmd("grk_dump command line", ' ', grk_version());

		// set the output
		GrokOutput output;
		cmd.setOutput(&output);

		ValueArg<string> inputArg("i", "input", "input file", false, "", "string",
				cmd);

		ValueArg<string> outputArg("o", "output", "output file", false, "", "string",
				cmd);

		ValueArg<string> imgDirArg("y", "ImgDir", "image directory", false, "", "string",
				cmd);

		SwitchArg verboseArg("v", "verbose", "verbose", cmd);
		ValueArg<uint32_t> flagArg("f", "flag",	"flag", false, 0, "unsigned integer", cmd);

		cmd.parse(argc, argv);

		if (inputArg.isSet()){
			const char *infile = inputArg.getValue().c_str();
			if (!grk::jpeg2000_file_format(infile,&parameters->decod_format )){
				spdlog::error(
						"Unknown input file format: {} \n"
								"        Known file formats are *.j2k, *.jp2 or *.jpc",
						infile);
				return 1;
			}
			if (grk::strcpy_s(parameters->infile, sizeof(parameters->infile),
					infile) != 0) {
				spdlog::error("Path is too long");
				return 1;
			}
		}

		if (outputArg.isSet()){
			if (grk::strcpy_s(parameters->outfile, sizeof(parameters->outfile),
					outputArg.getValue().c_str()) != 0) {
				spdlog::error("Path is too long");
				return 1;
			}
		}

		if (imgDirArg.isSet()){
			img_fol->imgdirpath = (char*) malloc(imgDirArg.getValue().length() + 1);
			if (!img_fol->imgdirpath)
				return 1;
			strcpy(img_fol->imgdirpath, imgDirArg.getValue().c_str());
			img_fol->set_imgdir = true;
		}

		if (verboseArg.isSet()){
			parameters->m_verbose = 1;
		}

		if (flagArg.isSet()){
			img_fol->flag = flagArg.getValue();
		}

	} catch (ArgException &e)  // catch any exceptions
	{
		cerr << "error: " << e.error() << " for arg " << e.argId() << endl;
		return 1;
	}


	/* check for possible errors */
	if (img_fol->set_imgdir) {
		if (!(parameters->infile[0] == 0)) {
			spdlog::error("options -ImgDir and -i cannot be used together.");
			return 1;
		}
		if (!img_fol->set_out_format) {
			spdlog::error(
					"When -ImgDir is used, -OutFor <FORMAT> must be used.");
			spdlog::error(
					"Only one format allowed.\n"
							"Valid format are PGM, PPM, PNM, PGX, BMP, TIF and RAW.");
			return 1;
		}
		if (!(parameters->outfile[0] == 0)) {
			spdlog::error("options -ImgDir and -o cannot be used together");
			return 1;
		}
	} else {
		if (parameters->infile[0] == 0) {
			spdlog::error("Required parameter is missing");
			spdlog::error("Example: {} -i image.j2k", argv[0]);
			spdlog::error("Help: {} -h", argv[0]);
			return 1;
		}
	}

	return 0;
}

/* -------------------------------------------------------------------------- */

/**
 sample error debug callback expecting no client object
 */
static void error_callback(const char *msg, void *client_data) {
	(void) client_data;
	spdlog::error(msg);
}
/**
 sample warning debug callback expecting no client object
 */
static void warning_callback(const char *msg, void *client_data) {
	(void) client_data;
	spdlog::warn(msg);
}
/**
 sample debug callback expecting no client object
 */
static void info_callback(const char *msg, void *client_data) {
	(void) client_data;
	spdlog::info(msg);
}

/* -------------------------------------------------------------------------- */
/**
 * GRK_DUMP MAIN
 */
/* -------------------------------------------------------------------------- */
int main(int argc, char *argv[]) {
	FILE *fout = nullptr;

	grk_dparameters parameters; /* Decompression parameters */
	grk_image *image = nullptr; /* Image structure */
	grk_codec l_codec = nullptr; /* Handle to a decompressor */
	grk_stream *l_stream = nullptr; /* Stream */
	grk_codestream_info_v2 *cstr_info = nullptr;
	grk_codestream_index *cstr_index = nullptr;

	size_t num_images, imageno;
	img_fol img_fol;
	dircnt *dirptr = nullptr;
	int rc = EXIT_SUCCESS;

	grk_initialize(nullptr, 0);

	grk_set_info_handler(info_callback, nullptr);
	grk_set_warning_handler(warning_callback, nullptr);
	grk_set_error_handler(error_callback, nullptr);

	/* Set decoding parameters to default values */
	grk_set_default_decompress_params(&parameters);

	/* Initialize img_fol */
	memset(&img_fol, 0, sizeof(img_fol));
	img_fol.flag = GRK_IMG_INFO | GRK_J2K_MH_INFO | GRK_J2K_MH_IND;

	/* Parse input and get user encoding parameters */
	if (parse_cmdline_decoder(argc, argv, &parameters, &img_fol) == 1) {
		rc = EXIT_FAILURE;
		goto cleanup;
	}

	/* Initialize reading of directory */
	if (img_fol.set_imgdir) {
		num_images = (size_t)get_num_images(img_fol.imgdirpath);
		if (num_images == 0) {
			spdlog::error("Folder is empty");
			rc = EXIT_FAILURE;
			goto cleanup;
		}

		dirptr = (dircnt*) malloc(sizeof(dircnt));
		if (dirptr) {
			dirptr->filename_buf = (char*) malloc(
					num_images * GRK_PATH_LEN * sizeof(char)); /* Stores at max 10 image file names*/
			if (!dirptr->filename_buf) {
				rc = EXIT_FAILURE;
				goto cleanup;
			}
			dirptr->filename = (char**) malloc(
					num_images * sizeof(char*));
			if (!dirptr->filename) {
				rc = EXIT_FAILURE;
				goto cleanup;
			}
			for (size_t i = 0; i < num_images; i++) {
				dirptr->filename[i] = dirptr->filename_buf + i * GRK_PATH_LEN;
			}
		}
		if (load_images(dirptr, img_fol.imgdirpath) == 1) {
			rc = EXIT_FAILURE;
			goto cleanup;
		}

	} else {
		num_images = 1;
	}

	/* Try to open for writing the output file if necessary */
	if (parameters.outfile[0] != 0) {
		fout = fopen(parameters.outfile, "w");
		if (!fout) {
			spdlog::error("failed to open {} for writing",
					parameters.outfile);
			rc = EXIT_FAILURE;
			goto cleanup;
		}
	} else
		fout = stdout;

	/* Read the header of each image one by one */
	for (imageno = 0; imageno < num_images; imageno++) {
		if (img_fol.set_imgdir) {
			if (get_next_file(imageno, dirptr, &img_fol, &parameters)) {
				continue;
			}
		}

		/* Read the input file and put it in memory */
		/* ---------------------------------------- */

		l_stream = grk_stream_create_file_stream(parameters.infile, 1024 * 1024,
				1);
		if (!l_stream) {
			spdlog::error("failed to create a stream from file {}",
					parameters.infile);
			rc = EXIT_FAILURE;
			goto cleanup;
		}

		/* Read the JPEG2000 stream */
		/* ------------------------ */

		switch (parameters.decod_format) {
		case GRK_J2K_FMT: { /* JPEG 2000 code stream */
			/* Get a decoder handle */
			l_codec = grk_create_decompress(GRK_CODEC_J2K, l_stream);
			break;
		}
		case GRK_JP2_FMT: { /* JPEG 2000 compressed image data */
			/* Get a decoder handle */
			l_codec = grk_create_decompress(GRK_CODEC_JP2, l_stream);
			break;
		}
		default:
			grk_stream_destroy(l_stream);
			l_stream = nullptr;
			continue;
		}

		/* Setup the decoder decoding parameters using user parameters */
		if (!grk_init_decompress(l_codec, &parameters)) {
			spdlog::error("grk_dump: failed to set up the decoder");
			rc = EXIT_FAILURE;
			goto cleanup;
		}

		/* Read the main header of the code stream and if necessary the JP2 boxes*/
		if (!grk_read_header(l_codec, nullptr, &image)) {
			spdlog::error("grk_dump: failed to read the header");
			rc = EXIT_FAILURE;
			goto cleanup;
		}

		grk_dump_codec(l_codec, img_fol.flag, fout);

		cstr_info = grk_get_cstr_info(l_codec);

		cstr_index = grk_get_cstr_index(l_codec);

		/* close the byte stream */
		if (l_stream) {
			grk_stream_destroy(l_stream);
			l_stream = nullptr;
		}

		/* free remaining structures */
		if (l_codec) {
			grk_destroy_codec(l_codec);
			l_codec = nullptr;
		}

		/* destroy the image header */
		if (image) {
			grk_image_destroy(image);
			image = nullptr;
		}

		/* destroy the code stream index */
		grk_destroy_cstr_index(&cstr_index);

		/* destroy the code stream info */
		grk_destroy_cstr_info(&cstr_info);

	}

	cleanup: if (dirptr) {
		if (dirptr->filename_buf)
			free(dirptr->filename_buf);
		if (dirptr->filename)
			free(dirptr->filename);
		free(dirptr);
	}

	/* close the byte stream */
	if (l_stream)
		grk_stream_destroy(l_stream);

	/* free remaining structures */
	if (l_codec) {
		grk_destroy_codec(l_codec);
	}

	/* destroy the image header */
	if (image)
		grk_image_destroy(image);

	if (fout)
		fclose(fout);

	grk_deinitialize();

	return rc;
}
