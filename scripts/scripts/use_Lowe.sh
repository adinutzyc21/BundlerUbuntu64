#!/bin/bash
#
# use_Lowe.sh
# Create a script for extracting sift features from a set of images

IMAGE_DIR=$1
SIFT=$2


# if the executable exists, continue, otherwise throw an error
if [ -e $SIFT ]
then 
:
else
    echo "[use_Lowe] Error: SIFT not found.  Please install SIFT to $SIFT" > /dev/stderr
fi

# remove previous file
rm -f keypoints.txt

for d in `ls -1 $IMAGE_DIR | egrep "jpg$"` #ls -1 lists one file per line; make sure they're the jpeg files
do 
    pgm_file=$IMAGE_DIR/`echo $d | sed 's/jpg$/pgm/'` #rename the files with the .pgm extension
    key_file=$IMAGE_DIR/`echo $d | sed 's/jpg$/key/'` #rename the files with the .key extension
    # create the .pgm extension files and run sift on them to create the .key files, which are then zipped
    echo "echo \"Processing file $d\"" >> keypoints.txt
    echo "mogrify -format pgm $IMAGE_DIR/$d; $SIFT < $pgm_file > $key_file; rm $pgm_file; gzip -f $key_file" >> keypoints.txt
done

