#!/bin/bash

# Define source and destination directories
SOURCE="/mnt/home/downloads_linux/USB_fun/DEVL_php/VIBECode_Updates/"
DESTINATION="/opt/lampp/htdocs/"

# Copy the folder and its contents to the destination
cp -r "$SOURCE" "$DESTINATION"

echo "${DESTINATION}VIBECode_Updates/" 
# Change directory to the copied folder
cd "${DESTINATION}VIBECode_Updates/" 

sleep 5

cd /opt/lampp/htdocs/

./make_files_folder_777.sh VIBECode_Updates

cd /opt/lampp/htdocs/VIBECode_Updates/


echo "Folder copied and changed directory successfully."

mv /opt/lampp/htdocs/index.php /opt/lampp/htdocs/index2.php

/opt/lampp/manager-linux-x64.run &

