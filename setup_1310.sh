# This script helps setting up Contiki os development for TI Launchpad 1310 under apt based linux destros
# downloading necessary packages

# this command downloads files needed for cc26/13 development
git submodule update --init

# this command downloads necessery packages
sudo apt-get install srecord python-serial
