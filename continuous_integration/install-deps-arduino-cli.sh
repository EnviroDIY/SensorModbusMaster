#!/bin/bash

# Makes the bash script print out every command before it is executed, except echo
trap '[[ $BASH_COMMAND != echo* ]] && echo $BASH_COMMAND' DEBUG

# Exit with nonzero exit code if anything fails
set -e

echo "\n\e[32mCurrent Arduino CLI version:\e[0m"
arduino-cli version

echo "::group::Installing Platforms and Frameworks"
echo "\n\e[32mUpdating the core index\e[0m"
arduino-cli --config-file continuous_integration/arduino_cli.yaml core update-index

echo "\n\e[32mInstalling the Arduino AVR Core\e[0m"
arduino-cli --config-file continuous_integration/arduino_cli.yaml core install arduino:avr

echo "\n\e[32mInstalling the EnviroDIY AVR Core\e[0m"
arduino-cli --config-file continuous_integration/arduino_cli.yaml core install EnviroDIY:avr

echo "\n\e[32mInstalling the Arduino SAMD Core\e[0m"
arduino-cli --config-file continuous_integration/arduino_cli.yaml core install arduino:samd

echo "\n\e[32mInstalling the Adafruit SAMD Core\e[0m"
arduino-cli --config-file continuous_integration/arduino_cli.yaml core install adafruit:samd

echo "\n\e[32mUpdating the core index\e[0m"
arduino-cli --config-file continuous_integration/arduino_cli.yaml core update-index

echo "\n\e[32mUpgrading all cores\e[0m"
arduino-cli --config-file continuous_integration/arduino_cli.yaml core upgrade

echo "\n\e[32mCurrently installed cores:\e[0m"
arduino-cli --config-file continuous_integration/arduino_cli.yaml core list
echo "::endgroup::"


echo "::group::Installing Libraries"
echo "\n\e[32mUpdating the library index\e[0m"
arduino-cli --config-file continuous_integration/arduino_cli.yaml lib update-index


echo "::group::Current globally installed libraries"
echo "\n\e[32mCurrently installed libraries:\e[0m"
arduino-cli --config-file continuous_integration/arduino_cli.yaml lib update-index
arduino-cli --config-file continuous_integration/arduino_cli.yaml lib list
echo "::endgroup::"
