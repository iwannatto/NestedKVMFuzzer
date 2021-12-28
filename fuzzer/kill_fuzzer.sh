set -eux

readonly afl_pid=$(cat afl_pid.txt)
sudo kill ${afl_pid}

# result directory is created by root permission
# because AFL is run with sudo
# so you should chmod to read these files through an editor run with user permission

sudo chmod -R 777 ./results