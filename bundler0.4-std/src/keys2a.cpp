/* keys2.cpp */
/* Class for SIFT keypoints */

#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include <zlib.h>

#include "keys2a.h"


/* This reads a keypoint file from a given filename and returns the list
 * of keypoints. */
keysInfo ReadKeyFile(int option, const char *filename, unsigned char **keys, keypt_t **info)
{
	//filename=./img.key
	FILE *file;
	file = fopen (filename, "r");

	keysInfo numLen;
	numLen.numKeys=0; numLen.descrLen=0;

	//if there is no such file
	if (! file) {
		/* Try to file a gzipped keyfile */
		char buf[1024];
		sprintf(buf, "%s.gz", filename);
		gzFile gzf = gzopen(buf, "rb");
		if (gzf == NULL) {
			printf("Could not open file: %s\n", filename);
		} 
		else {
			//read the archived file
			numLen = ReadKeysGzip(option, gzf, keys, info);
			gzclose(gzf);
		}
	}
	return numLen;
}

keysInfo ReadKeysGzip(int option, gzFile fp, unsigned char **keys, keypt_t **info)
{
	int i, num, len;
	keysInfo numLen;
	numLen.numKeys=0; numLen.descrLen=0;

	std::vector<Keypoint *> kps;
	char header[256];
	gzgets(fp, header, 256);

	if(option==1 || option==2){
		if (sscanf(header, "%d %d", &num, &len) != 2) {
			printf("Invalid keypoint file.\n");
			return numLen;
		}
	}
	else if(option==3){
		if (sscanf(header, "%d", &len) != 1) {
			printf("Invalid keypoint file.\n");
			return numLen;
		}
		gzgets(fp, header, 256);
		if (sscanf(header, "%d", &num) != 1) {
			printf("Invalid keypoint file.\n");
			return numLen;
		}
	}

	*keys = new unsigned char[len * num];
	
	if (info != NULL) 
		*info = new keypt_t[num];
	
	unsigned char *p = *keys; // value pointed by p = value pointed by keys 
	//(copy what is in the memory space that keys points to into the memory space 
	//that p points to)

	for (i = 0; i < num; i++) {
		/* Allocate memory for the keypoint. */
		float x, y, scale, ori;
		char buf[1024];
		gzgets(fp, buf, 1024);

		switch(option){
			case 1://"sift_lowe":
				if (sscanf(buf, "%f %f %f %f\n", &y, &x, &scale, &ori) != 4) {
					printf("Invalid keypoint file format.");
					return numLen;
				}
				break;
			case 2://"sift_vlfeat":
				if (sscanf(buf, "%f %f %f %f\n", &x, &y, &scale, &ori) != 4) {
					printf("Invalid keypoint file format.");
					return numLen;
				}
				break;
			case 3://"open_surf":
				if (sscanf(buf, "%f %f %f %f %*f\n", &scale, &x, &y, &ori) != 4) {
					printf("Invalid keypoint file format.");
					return numLen;
				}
				break;
			default:
				printf("Something's wrong!\n");
				return numLen;
		}
				
		if (info != NULL) {
			(*info)[i].x = x;
			(*info)[i].y = y;
			(*info)[i].scale = scale;
			(*info)[i].orient = ori;
		}

		if(option==1 || option==2){
			for (int line = 0; line < 7; line++) {
		        char *str = gzgets(fp, buf, 1024);
		        assert(str != Z_NULL);

		        if (line < 6) {
		            sscanf(buf, 
		                "%hhu %hhu %hhu %hhu %hhu %hhu %hhu %hhu %hhu %hhu "
		                "%hhu %hhu %hhu %hhu %hhu %hhu %hhu %hhu %hhu %hhu", 
		                p+0, p+1, p+2, p+3, p+4, p+5, p+6, p+7, p+8, p+9, 
		                p+10, p+11, p+12, p+13, p+14, 
		                p+15, p+16, p+17, p+18, p+19);

		            p += 20;
		        } 
		        else {
		            sscanf(buf, 
		                "%hhu %hhu %hhu %hhu %hhu %hhu %hhu %hhu",
		                p+0, p+1, p+2, p+3, p+4, p+5, p+6, p+7);
		            p += 8;
		        }
		    }
		}
		else if(option==3){
			for (int line = 0; line < 8; line++) {
			    char *str = gzgets(fp, buf, 1024);
			    assert(str != Z_NULL);

			    sscanf(buf, 
			        "%hhu %hhu %hhu %hhu %hhu %hhu %hhu %hhu", 
			        p+0, p+1, p+2, p+3, p+4, p+5, p+6, p+7);

			    p += 8;
			}
		}
	}
	assert(p == *keys + len * num);

	numLen.numKeys=num;
	numLen.descrLen=len;
	return numLen; // kps;
}

/* Create a search tree for the given set of keypoints */
ANNkd_tree *CreateSearchTree(int num_keys, int descr_len, unsigned char *keys)
{
	/* Create a new array of points */
	ANNpointArray pts = annAllocPts(num_keys, descr_len);

	for (int i = 0; i < num_keys; i++) {
		memcpy(pts[i], keys + descr_len * i, sizeof(unsigned char) * descr_len);
	}

	/* Create a search tree for k2 */
	ANNkd_tree *tree = new ANNkd_tree(pts, num_keys, descr_len, 16);

	return tree;
}

std::vector<KeypointMatch> MatchKeys(int num_keys1, int descr_len, unsigned char *k1, ANNkd_tree *tree2,
                                     double ratio, int max_pts_visit)
{
	annMaxPtsVisit(max_pts_visit);
	std::vector<KeypointMatch> matches;

	/* Now do the search */
	// clock_t start = clock();
	for (int i = 0; i < num_keys1; i++) {
		ANNidx nn_idx[2];
		ANNdist dist[2];

		tree2->annkPriSearch(k1 + descr_len * i, 2, nn_idx, dist, 0.0);

		if (((double) dist[0]) < ratio * ratio * ((double) dist[1])) {
			matches.push_back(KeypointMatch(i, nn_idx[0]));
		}
	}
	return matches;    
}

/* Compute likely matches between two sets of keypoints */
std::vector<KeypointMatch> MatchKeys(int num_keys1, unsigned char *k1, 
                                     int num_keys2, unsigned char *k2, 
                                     int descr_len,
                                     double ratio, int max_pts_visit) 
{
	annMaxPtsVisit(max_pts_visit);

	int num_pts = 0;
	std::vector<KeypointMatch> matches;

	num_pts = num_keys2;

	/* Create a new array of points */
	ANNpointArray pts = annAllocPts(num_pts, descr_len);

	for (int i = 0; i < num_pts; i++) {
		memcpy(pts[i], k2 + descr_len * i, sizeof(unsigned char) * descr_len);
	}

	/* Create a search tree for k2 */
	ANNkd_tree *tree = new ANNkd_tree(pts, num_pts, descr_len, 16);

	/* Now do the search */
	for (int i = 0; i < num_keys1; i++) {
		ANNidx nn_idx[2];
		ANNdist dist[2];

		tree->annkPriSearch(k1 + descr_len * i, 2, nn_idx, dist, 0.0);

		if (((double) dist[0]) < ratio * ratio * ((double) dist[1])) {
			matches.push_back(KeypointMatch(i, nn_idx[0]));
		}
	}
	
	/* Cleanup */
	annDeallocPts(pts);
	// annDeallocPt(axis_weights);

	delete tree;

	return matches;
}
