Dependencies:
Make sure you have cmake installed.
sudo apt install libcurl4-openssl-dev
git clone https://github.com/open-source-parsers/jsoncpp.git
cd jsoncpp; sudo make install
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/lib

Building:
(In root project dir) cmake -S .
make

Running:
There is a cpplights_env file that needs to be sourced before the project
is run. This will allow the user to configure parameters like hue lights
simulator URL, port, username for REST API, and poll time for the HTTP 
status checks.

This is intended to be run with hue-simulator which can be installed with
npm. The env file is configured to have the simulator run on localhost 
port 8080, so running the simulator would look like this for that:

hue-simulator --port=8080