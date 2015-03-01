#!/bin/bash
#
# use_OpenSurf.sh
# Create a script for extracting sift features from a set of images

IMAGE_DIR=$1
SURF=$2

# if the executable exists, continue, otherwise throw an error
if [ -e $SURF ]
then 
:
else
    echo "[use_OpenSurf] Error: SURF not found.  Please install SURF to $SURF" > /dev/stderr
fi

# remove previous file
rm -f keypoints.txt

#surf 7 IMG.jpg IMG.key
for d in `ls -1 $IMAGE_DIR | egrep "jpg$"` #ls -1 lists one file per line; make sure they're the jpeg files
do 
    key_file=$IMAGE_DIR/`echo $d | sed 's/jpg$/key/'` #rename the files with the .key extension
    # run surf on the files to create the .key files, which are then zipped
    echo "echo \"Processing file $d\"" >> keypoints.txt
    echo "$SURF 7 $d $key_file; gzip -f $key_file" >> keypoints.txt
done

