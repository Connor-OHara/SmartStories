#include <wiringPi.h>
//#include "ws2812-rpi.h"
#include <iostream>
#include <unistd.h>  // for usleep
#include <alsa/asoundlib.h>
#include <stdlib.h>
#include <stdio.h>
#include <filesystem>
#include <iostream>
#include <vector>
#include <fstream>
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>

#include "pca9685.h"
#include <wiringPi.h>
#include <wiringPiI2C.h>



namespace fs = std::filesystem;
//first run boolean
bool s = true;

// pin setup for LEDs
#define PIN_BASE 300
#define PIN_BASE1 400
#define PIN_BASE2 500
#define MAX_PWM 4096
#define HERTZ 50


// GPIO pin for the button
#define BUTTON_PIN 17
#define PIN

bool lastButtonState = false;  // Last recorded state

//#define NUMPIXELS 24

// Function to be called when the button state changes
void buttonStateChanged() {
    if (digitalRead(BUTTON_PIN) == LOW) {
        if (!lastButtonState) {
            std::system("sudo arecord --device=hw:3,0 --format S16_LE --rate 44100 -c1 sample/controllerSample.wav &");
            lastButtonState = true;
        }
    } else {
        if (lastButtonState) {
            try {
                std::system("sudo pkill arecord");
                std::system("sudo pkill smartStories.py");        
            } catch (const std::exception& e) {
                // Leave empty
            }
            std::system("python SmartStories.py");
            lastButtonState = false;
            s = false;
        }
    }
}


void updateLEDs(int fd, int red, int green, int blue) {
    pwmWrite(PIN_BASE, red);
    pwmWrite(PIN_BASE + 1, green);
    pwmWrite(PIN_BASE + 2, blue);
}







int main(int argc, char* argv[]) {    
    if (std::string(argv[2]) == "T") {
        s = true;
    } else {
        s = false;
    }

    // Setup WiringPi
    if (wiringPiSetupGpio() == -1) {
        std::cerr << "Error initializing WiringPi." << std::endl;
        return 1;
    }
//    Adafruit_Neopixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);
    //Set up our LEDS:
    
    
    std::cout << "INIT STRIPS" << std::endl;
    int fd = pca9685Setup(PIN_BASE, 0x40, HERTZ);
	if (fd < 0)
	{
		printf("Error in setup\n");
		return fd;
	}
    int fd1 = pca9685Setup(PIN_BASE1, 0x41, HERTZ);
	if (fd1 < 0)
	{
		printf("Error in setup\n");
		return fd1;
	}
	int fd2 = pca9685Setup(PIN_BASE2, 0x42, HERTZ);
	if (fd2 < 0)
	{
		printf("Error in setup\n");
		return fd2;
	}
    std::cout << "STRIPS INITED, RESTTING" << std::endl;
	pca9685PWMReset(fd);
	pca9685PWMReset(fd1);
	pca9685PWMReset(fd2);

	int i, j;

    // Set up the button pin
    pinMode(BUTTON_PIN, INPUT);
    pullUpDnControl(BUTTON_PIN, PUD_UP);

//    unsigned long lastDebounceTime = 0;  // Last time the output was toggled

    // Attach the ISR (Interrupt Service Routine) to both rising and falling edges
    if (wiringPiISR(BUTTON_PIN, INT_EDGE_BOTH, &buttonStateChanged) < 0) {
        std::cerr << "Error setting up ISR." << std::endl;
        return 1;
    }

    // Jump loop for infinite running
    restart:
    std::cout << "IN RESTART" << std::endl;
    std::cout << "RESETING" << std::endl;
    pca9685PWMReset(fd);
    pca9685PWMReset(fd1);
    pca9685PWMReset(fd2);
    std::cout << "BEGIN ANIMATION" << std::endl;
    
    // Start waiting animation
    if (s) {
            std::system("eog --fullscreen waitingAnimation.gif &");
    }
    
    // If this is the first run off of boot we don't care about the 
    // Data that is already present. Wait until interrupt, which will start the script and get past this loop
    // The isr will give us data to be processed below.
    while(s) {
        //pwmWrite(PIN_BASE1, MAX_PWM);
        //pwmWrite(PIN_BASE1+1, MAX_PWM);
        //pwmWrite(PIN_BASE1+2, MAX_PWM - 500);
        
        /*
        pwmWrite(PIN_BASE1, MAX_PWM);
        delay(2000);
        pwmWrite(PIN_BASE1 + 4, MAX_PWM);
        delay(2000);
        pwmWrite(PIN_BASE1 + 8, MAX_PWM);
        delay(2000);
        pwmWrite(PIN_BASE1 + 9, MAX_PWM);
        delay(2000);
        pwmWrite(PIN_BASE1 + 13, MAX_PWM);
        delay(2000);
        pwmWrite(PIN_BASE2, MAX_PWM);
        delay(2000);
        pwmWrite(PIN_BASE2 + 4, MAX_PWM);
        delay(2000);
        pwmWrite(PIN_BASE2 + 8, MAX_PWM);
        delay(2000);
        pwmWrite(PIN_BASE2 + 9, MAX_PWM);
        delay(2000);
        pwmWrite(PIN_BASE2 + 13, MAX_PWM);
        */
        
        //Stolen Loading Animation from leds.c
       
        for (j = 0; j < 5; j++)
		{
			for (i = 0; i < MAX_PWM; i += 32)
			{
				pwmWrite(PIN_BASE + 16, i);
				pwmWrite(PIN_BASE1 + 16, i);
				pwmWrite(PIN_BASE2 + 16, i);
				delay(4);
			}

			for (i = 0; i < MAX_PWM; i += 32)
			{
				pwmWrite(PIN_BASE + 16, MAX_PWM - i);
				pwmWrite(PIN_BASE1 + 16, MAX_PWM - i);
				pwmWrite(PIN_BASE2 + 16, MAX_PWM - i);
				delay(4);
			}
		}

		pwmWrite(PIN_BASE + 16, 0);
		pwmWrite(PIN_BASE1 + 16, 0);
		pwmWrite(PIN_BASE2 + 16, 0);
		delay(500);

		for (j = 0; j < 5; j++)
		{
			for (i = 0; i < 16; i++)
			{
				pwmWrite(PIN_BASE + i, MAX_PWM);
				pwmWrite(PIN_BASE1 + i, MAX_PWM);
				pwmWrite(PIN_BASE2 + i, MAX_PWM);
				delay(20);
			}

			for (i = 0; i < 16; i++)
			{
				pwmWrite(PIN_BASE + i, 0);
				pwmWrite(PIN_BASE1 + i, 0);
				pwmWrite(PIN_BASE2 + i, 0);
				delay(20);
			}
		}

		pwmWrite(PIN_BASE + 16, 0);
		pwmWrite(PIN_BASE1 + 16, 0);
		pwmWrite(PIN_BASE2 + 16, 0);
		delay(500);
        
        // If you are in this loop you are waiting for the button interrupt to start a new script
    }
    
    // On secondary runs (after the interrupt)
    // Display the images, update the LEDs, and play the audio
    int subdirectoryCount = 0;
    
    // Determine number of loops needed
    for (const auto& entry : fs::directory_iterator("/home/smart/SmartStories/images")) {
        if (entry.is_directory()) {
            ++subdirectoryCount;
        }
    }
    
    // Intialize video settings
    sf::VideoMode desktopMode = sf::VideoMode::getDesktopMode();
    sf::RenderWindow window(sf::VideoMode(desktopMode.width, desktopMode.height), "Fullscreen Image", sf::Style::Fullscreen);
    
    for (int i = 0; i < subdirectoryCount; i++) {
        // Create or clear the audio buffer
        sf::SoundBuffer buffer;

        // This system call will ensure that the wav file from Google's API is in the correct format
        std::string formatCommand = "ffmpeg -i ./sound/speech_" + std::to_string(i) + ".wav -acodec pcm_s16le -ar 44100 ./sound/speech_formated_" + std::to_string(i) + ".wav";
        std::system(formatCommand.c_str());
        
        // Check the newly generated wav file, load it into the buffer, then start playing it
        std::string speechLocation = "./sound/speech_formated_" + std::to_string(i) + ".wav";
        std::cout << speechLocation << std::endl;
        if (!buffer.loadFromFile(speechLocation.c_str())) {
            std::cout << "Failed to load audio file!" << std::endl;
            return -1;
        }

        sf::Sound sound(buffer);
        sound.play();
        
        // Locate the playing wav file's corresponding image if it does not exist exit
        // If it does display it on the main screen
        sf::Texture texture;
        std::string imageLocation = "./images/image_" + std::to_string(i) + "/image_" + std::to_string(i) + ".png";
        if (!texture.loadFromFile(imageLocation.c_str())) {
            std::cout << "Failed to load image file!" << std::endl;
            return -1;
        }

        sf::Sprite sprite(texture);
        sprite.setScale(static_cast<float>(desktopMode.width) / texture.getSize().x, static_cast<float>(desktopMode.height) / texture.getSize().y);
        
        // Run the initial display of the picture out here so we can kill the animation premptively
        window.clear();
        window.draw(sprite);
        window.display();
        
        // Kill the waiting Animation if its on the first loop
        if (i == 0) {
            //std::system("sudo pkill eog");
        }
        
        // Read RGB data from the text file
        std::ifstream rgbFile("./images/image_" + std::to_string(i) + "/colorvalues/" + "color_values_" + std::to_string(i) + ".txt");
        std::string line;
        
        std::cout << "RESETTING IN PARSER" << std::endl;
        pca9685PWMReset(fd);
        pca9685PWMReset(fd1);
        pca9685PWMReset(fd2);
        
        while (std::getline(rgbFile, line)) {
            
            int red, green, blue, duration;
            // Update LEDs with RGB values during the specified duration
            //updateLEDs(fd, red, green, blue);  This will only target the first 3 pwm pins on the controller
            // Extract RGB values and duration from the line
            if (sscanf(line.c_str(), "%d,%d,%d,%d", &red, &green, &blue, &duration) == 4) {
            
                std::cout << "DEBUG FILE" << std::endl;
                std::cout << "Red: " << red << ", Green: " << green << ", Blue: " << blue << ", Duration: " << duration << std::endl;

                if (duration <= 4) {
                    std::cout << "WRITING IN CONTROLLER1" << std::endl;
                    std::cout << "Pixel " << duration << "red: " << (red * MAX_PWM) / 255 << std::endl;
                    pwmWrite(PIN_BASE + (duration * 3), (red * MAX_PWM) / 255);
                    std::cout << "Pixel " << duration << "green: " << (green * MAX_PWM) / 255 << std::endl;
                    pwmWrite(PIN_BASE + 1 + (duration * 3), (green * MAX_PWM) / 255);
                    std::cout << "Pixel " << duration << "blue: " << (blue * MAX_PWM) / 255 << std::endl;
                    pwmWrite(PIN_BASE + 2 + (duration * 3), ((blue * MAX_PWM) / 255) - 500);
                } else if (duration > 4 && duration <= 9) {
                    std::cout << "WRITING IN CONTROLLER2" << std::endl;
                    std::cout << "Pixel " << duration << "red: " << (red * MAX_PWM) / 255 << std::endl;
                    pwmWrite(PIN_BASE1 + ((duration - 5) * 3), (red * MAX_PWM) / 255);
                    std::cout << "Pixel " << duration << "green: " << (green * MAX_PWM) / 255 << std::endl;
                    pwmWrite(PIN_BASE1 + 1 + ((duration - 5) * 3), (green * MAX_PWM) / 255);
                    std::cout << "Pixel " << duration << "blue: " << (blue * MAX_PWM) / 255 << std::endl;
                    pwmWrite(PIN_BASE1 + 2 + ((duration - 5) * 3), ((blue * MAX_PWM) / 255) - 500);
                } else {
                    std::cout << "WRITING IN CONTROLLER3" << std::endl;
                    std::cout << "Pixel " << duration << "red: " << (red * MAX_PWM) / 255 << std::endl;
                    pwmWrite(PIN_BASE2 + ((duration - 10) * 3), (red * MAX_PWM) / 255);
                    std::cout << "Pixel " << duration << "green: " << (green * MAX_PWM) / 255 << std::endl;
                    pwmWrite(PIN_BASE2 + 1 + ((duration - 10) * 3), (green * MAX_PWM) / 255);
                    std::cout << "Pixel " << duration << "blue: " << (blue * MAX_PWM) / 255 << std::endl;
                    pwmWrite(PIN_BASE2 + 2 + ((duration - 10) * 3), ((blue * MAX_PWM) / 255) - 500);
                }
            } else {
                std::cerr << "Failed to parse line: " << line << std::endl;
            }
            // Wait for the specified duration
            //delay(duration);    The last value in the line is an index number not a duration

            // Clear LEDs after the duration
            //updateLEDs(fd, 0, 0, 0);   This is instantly clearing the LEDs they should stay lit until the next iteration of the loop
        }
        
        
        // Stay in this loop while the wav file is being played then iterate the for loop until its finished
        while (window.isOpen() && sound.getStatus() != sf::Sound::Stopped) {
            sf::Event event;
            while (window.pollEvent(event)) {
                if (event.type == sf::Event::Closed) {
                    window.close();
                }
            }
            
            window.clear();
            window.draw(sprite);
            window.display();
        }
    }
    

    s = true;
    goto restart;

    return 0;
}
