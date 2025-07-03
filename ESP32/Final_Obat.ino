#include <WiFi.h>           // Library for WiFi connectivity
#include <HTTPClient.h>     // Library for making HTTP requests
#include "esp_camera.h"     // ESP32 Camera driver
#include "driver/ledc.h"    // LED Control library for PWM (though not used directly in setLED, good to include if future PWM control is needed)
#include <DFRobotDFPlayerMini.h> // Library for DFPlayer Mini MP3 module
#include <ArduinoJson.h>    // Library for JSON parsing and serialization

// --- Global Objects ---
HardwareSerial mySerial(2); // Initialize SoftwareSerial for DFPlayer Mini on UART2.
                            // Ensure these pins (RX: GPIO16, TX: GPIO17 by default for UART2 on ESP32-CAM)
                            // are correctly wired to the DFPlayer Mini's TX and RX.
                            // Note: The original code mentioned pin 14 (RX) and 13 (TX) which are UART2 default on some ESP32 boards,
                            // but for ESP32-CAM, UART2 usually uses GPIO16/17. Double-check your specific ESP32-CAM board's pinout.
DFRobotDFPlayerMini myDFPlayer; // DFPlayer Mini object

// --- Network Configuration ---
// It's recommended to store sensitive information like SSID and password outside the code,
// for example, in a `secrets.h` file or using a configuration portal.
// For now, placeholders are used. Replace with your actual WiFi credentials.
const char* ssid = "****";     // Your WiFi SSID
const char* password = "*****"; // Your WiFi Password

// --- Server Configuration ---
const char* serverUrl = "http://*******/upload"; // Replace with your Flask server's image upload endpoint URL

// --- Pin Definitions ---
// Define pins for the RGB LED. These are standard GPIOs on the ESP32.
const int redPin = 32;
const int int greenPin = 33;
const int bluePin = 26;
// Define the pin for the pushbutton.
const int buttonPin = 12;

// --- State Variables ---
bool canPress = true; // Flag for button debouncing. Controls if the button can be pressed again.

// --- Camera Pin Definitions (ESP32-CAM default) ---
// These definitions are standard for most ESP32-CAM modules (e.g., AI-Thinker).
// PWDN and RESET are often set to -1 as they are not always accessible or necessary.
#define PWDN_GPIO_NUM -1
#define RESET_GPIO_NUM -1
#define XCLK_GPIO_NUM 21
#define SIOD_GPIO_NUM 26 // SDA
#define SIOC_GPIO_NUM 27 // SCL
#define Y9_GPIO_NUM 35
#define Y8_GPIO_NUM 34
#define Y7_GPIO_NUM 39
#define Y6_GPIO_NUM 36
#define Y5_GPIO_NUM 19
#define Y4_GPIO_NUM 18
#define Y3_GPIO_NUM 5
#define Y2_GPIO_NUM 4
#define VSYNC_GPIO_NUM 25
#define HREF_GPIO_NUM 23
#define PCLK_GPIO_NUM 22

// --- Function Prototypes ---
void setLED(uint8_t r, uint8_t g, uint8_t b);
void setupCamera();
void sendPhotoAndProcessResponse();

// --- LED Control Function ---
/**
 * @brief Sets the color of the RGB LED.
 * Uses analogWrite to control the brightness of each color component,
 * simulating PWM for an RGB LED.
 * @param r Red component brightness (0-255)
 * @param g Green component brightness (0-255)
 * @param b Blue component brightness (0-255)
 */
void setLED(uint8_t r, uint8_t g, uint8_t b) {
    analogWrite(redPin, r);
    analogWrite(greenPin, g);
    analogWrite(bluePin, b);
}

// --- Camera Initialization Function ---
/**
 * @brief Initializes the ESP32-CAM module with specified configurations.
 * Configures pixel format, frame size, JPEG quality, and various image
 * sensor settings like brightness, contrast, saturation, white balance,
 * exposure, and gain control.
 * Prints status to Serial.
 */
void setupCamera() {
    camera_config_t config;
    // LEDC channel and timer configuration (usually for camera's flash LED, not the external RGB LED)
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    // Data bus pins
    config.pin_d0 = Y2_GPIO_NUM;
    config.pin_d1 = Y3_GPIO_NUM;
    config.pin_d2 = Y4_GPIO_NUM;
    config.pin_d3 = Y5_GPIO_NUM;
    config.pin_d4 = Y6_GPIO_NUM;
    config.pin_d5 = Y7_GPIO_NUM;
    config.pin_d6 = Y8_GPIO_NUM;
    config.pin_d7 = Y9_GPIO_NUM;
    // Clock and sync pins
    config.pin_xclk = XCLK_GPIO_NUM;
    config.pin_pclk = PCLK_GPIO_NUM;
    config.pin_vsync = VSYNC_GPIO_NUM;
    config.pin_href = HREF_GPIO_NUM;
    // I2C communication pins for sensor configuration
    config.pin_sscb_sda = SIOD_GPIO_NUM;
    config.pin_sscb_scl = SIOC_GPIO_NUM;
    // Power down and reset pins
    config.pin_pwdn = PWDN_GPIO_NUM;
    config.pin_reset = RESET_GPIO_NUM;
    // Clock frequency for the camera
    config.xclk_freq_hz = 25000000; // 25MHz XCLK
    // Image format and resolution
    config.pixel_format = PIXFORMAT_JPEG; // JPEG format for efficient transmission
    config.frame_size = FRAMESIZE_QVGA;   // QVGA (320x240) resolution
    config.jpeg_quality = 4;              // JPEG quality (0-63, lower is higher quality but larger file size)
    config.fb_count = 1;                  // Number of frame buffers to allocate

    // Attempt to initialize the camera
    esp_err_t err = esp_camera_init(&config);
    if (err == ESP_OK) {
        Serial.println("Camera initialized successfully.");
        // Get camera sensor object to apply detailed settings
        sensor_t *s = esp_camera_sensor_get();
        if (s) {
            // Apply custom sensor settings for image quality (tuning might be needed)
            s->set_brightness(s, -2);       // -2 to 2
            s->set_contrast(s, 2);          // -2 to 2
            s->set_saturation(s, -2);       // -2 to 2
            s->set_special_effect(s, 2);    // 0: No Effect, 1: Grayscale, 2: Negative, 3: Sepia, 4: Reddish, 5: Greenish, 6: Blueish
            s->set_whitebal(s, 1);          // 0: disable, 1: enable AWB
            s->set_awb_gain(s, 1);          // 0: disable, 1: enable AWB gain control
            s->set_wb_mode(s, 0);           // 0: Auto, 1: Sunny, 2: Cloudy, 3: Office, 4: Home
            s->set_ae_level(s, -1);         // -2 to 2
            s->set_exposure_ctrl(s, 0);     // 0: AEC ON (Auto Exposure Control)
            s->set_aec2(s, 0);              // 0: disable, 1: enable AEC2 (enhances low light)
            s->set_aec_value(s, 1200);      // Exposure value (lower means less exposure)
            s->set_gain_ctrl(s, 1);         // 0: AGC OFF, 1: AGC ON (Auto Gain Control)
            s->set_agc_gain(s, 0);          // 0 to 30 (AGC gain, only if AGC is OFF)
            s->set_bpc(s, 0);               // 0: disable, 1: enable (Bad Pixel Correction)
            s->set_wpc(s, 0);               // 0: disable, 1: enable (White Pixel Correction)
            s->set_raw_gma(s, 0);           // 0: disable, 1: enable (Gamma Correction)
            s->set_lenc(s, 0);              // 0: disable, 1: enable (Lens Correction)
            s->set_hmirror(s, 0);           // 0: disable, 1: enable (Horizontal Mirror)
            s->set_vflip(s, 0);             // 0: disable, 1: enable (Vertical Flip)
            s->set_dcw(s, 0);               // 0: disable, 1: enable (Denoising and Color Wider)
            s->set_colorbar(s, 0);          // 0: disable, 1: enable (Colorbar test pattern)
        } else {
            Serial.println("Failed to get camera sensor object.");
        }
    } else {
        Serial.printf("Camera initialization failed with error 0x%x\n", err);
        // Indicate camera failure visually and audibly
        setLED(255, 130, 0); // Red LED for camera error
        myDFPlayer.play(99); // Play error sound (assuming 99 is an error track)
        delay(3000); // Hold error state for visibility
        ESP.restart(); // Restart the ESP32 to try and reinitialize
    }
}

// --- Photo Capture, Send, and Response Processing Function ---
/**
 * @brief Captures an image from the camera, sends it to the configured
 * Flask server via HTTP POST, and processes the JSON response.
 * Based on the server's response, it controls the RGB LED and plays
 * sounds using the DFPlayer Mini module.
 */
void sendPhotoAndProcessResponse() {
    Serial.println("Attempting to capture photo...");
    setLED(0, 130, 255); // Blue LED: Indicates photo capture in progress

    // Flush initial frames to ensure a fresh image is captured.
    // The camera might have old data in its buffer.
    for (int i = 0; i < 3; i++) {
        camera_fb_t *temp_fb = esp_camera_fb_get();
        if (temp_fb) {
            esp_camera_fb_return(temp_fb); // Return the buffer to the camera driver
        } else {
            Serial.println("Warning: Could not get temporary frame buffer during flush.");
        }
    }

    camera_fb_t *fb = esp_camera_fb_get(); // Get the latest frame buffer
    if (!fb) {
        Serial.println("Failed to capture photo frame.");
        setLED(255, 130, 0); // Orange LED: Failed to capture
        myDFPlayer.play(99); // Play a generic error sound
        delay(2000);
        setLED(255, 130, 255); // Return to idle white LED
        return; // Exit function if photo capture failed
    }

    Serial.printf("Captured photo size: %u bytes\n", fb->len);

    WiFiClient client; // Create a WiFiClient object
    HTTPClient http;   // Create an HTTPClient object

    // Begin HTTP connection with the server URL
    http.begin(client, serverUrl);
    // Set content type header for JPEG image upload
    http.addHeader("Content-Type", "image/jpeg");
    // Set a timeout for the HTTP request to prevent indefinite waiting
    http.setTimeout(15000); // Set timeout to 15 seconds (increased from 10s for potentially slower networks/servers)

    Serial.println("Sending photo to server...");
    // Perform HTTP POST request with the captured image data
    int httpResponseCode = http.POST(fb->buf, fb->len);

    if (httpResponseCode > 0) {
        // HTTP response received successfully
        String payload = http.getString(); // Get the response payload as a String
        Serial.printf("HTTP Response Code: %d\n", httpResponseCode);
        Serial.print("Server Response: ");
        Serial.println(payload);

        // --- Parse JSON response from the server ---
        // StaticJsonDocument is allocated on the stack. The size needs to be sufficient
        // to hold the expected JSON response. 200 bytes is a good starting point for
        // simple responses, but adjust if your JSON grows.
        StaticJsonDocument<256> doc; // Increased size slightly for safety

        // Attempt to deserialize the JSON payload
        DeserializationError error = deserializeJson(doc, payload);

        if (error) {
            // JSON deserialization failed
            Serial.print(F("deserializeJson() failed: "));
            Serial.println(error.f_str());
            setLED(255, 130, 0); // Orange LED: JSON parsing error
            myDFPlayer.play(99); // Play error sound
            delay(2000);
        } else {
            // JSON parsed successfully. Extract values.
            // Use .as<const char*>() for string values for safety.
            const char* status = doc["status"].as<const char*>();
            int predictedIndex = doc["index"].as<int>();
            float predictedScore = doc["score"].as<float>();

            Serial.printf("Status from server: %s\n", status);
            Serial.printf("Predicted Index: %d\n", predictedIndex);
            Serial.printf("Predicted Score: %.4f\n", predictedScore);

            // Check if the server's status is "success"
            if (strcmp(status, "success") == 0) {
                if (predictedIndex != -1) { // Ensure a valid index was returned by the server
                    Serial.print("Playing sound for index: ");
                    Serial.println(predictedIndex);

                    // Logic to play specific DFPlayer Mini tracks based on the predicted index
                    switch (predictedIndex) {
                        case 0: // Example: omeprazole
                            myDFPlayer.play(2); // Track 2
                            break;
                        case 1: // Example: panadol
                            myDFPlayer.play(3); // Track 3
                            break;
                        case 2: // Example: procold_flu
                            myDFPlayer.play(4); // Track 4
                            break;
                        default:
                            Serial.println("Unknown index received. Playing default error sound.");
                            myDFPlayer.play(5); // Track 5: Generic unknown item sound
                            break;
                    }

                    // Adjust LED color based on the prediction confidence score
                    if (predictedScore >= 0.85) { // High confidence
                        setLED(0, 255, 0); // Green LED
                        Serial.println("High confidence prediction.");
                    } else if (predictedScore >= 0.60) { // Medium confidence
                        setLED(255, 165, 0); // Orange LED
                        Serial.println("Medium confidence prediction.");
                    } else { // Low confidence
                        setLED(255, 0, 0); // Red LED
                        Serial.println("Low confidence prediction.");
                        myDFPlayer.play(99); // Play error sound for low confidence
                    }
                } else {
                    Serial.println("Server responded with success but an invalid index (-1).");
                    myDFPlayer.play(99); // Play error sound
                    setLED(255, 0, 0); // Red LED for server logic error
                    delay(2000);
                }
            } else {
                // Server reported an error status
                Serial.printf("Server error: %s\n", doc["message"].as<const char*>());
                myDFPlayer.play(99); // Play error sound
                setLED(255, 0, 0); // Red LED for server-side error
                delay(2000);
            }
        }
    } else {
        // Failed to send photo or receive any HTTP response
        String errorString = http.errorToString(httpResponseCode);
        Serial.printf("Failed to send photo or receive response. HTTP Error: %d - %s\n", httpResponseCode, errorString.c_str());
        setLED(255, 130, 0); // Orange LED: Communication error
        myDFPlayer.play(5); // Play a generic communication error sound
    }

    http.end(); // Close the HTTP connection
    esp_camera_fb_return(fb); // IMPORTANT: Return the frame buffer to prevent memory leaks
    setLED(255, 130, 255); // Return to idle white LED after processing
}

// --- Setup Function ---
/**
 * @brief Initializes serial communication, DFPlayer Mini, RGB LED,
 * connects to WiFi, and sets up the camera.
 * This function runs once when the ESP32 starts.
 */
void setup() {
    Serial.begin(115200); // Initialize serial communication at 115200 baud
    Serial.println("\n--- ESP32-CAM Object Recognition System ---"); // Clear line and print header

    // --- DFPlayer Mini Initialization ---
    // Start serial communication for DFPlayer Mini (UART2: GPIO16/17 for ESP32-CAM)
    // Note: The original code mentioned pin 14 and 13. Ensure your wiring matches.
    mySerial.begin(9600); // DFPlayer Mini typically communicates at 9600 baud
    Serial.println("Initializing DFPlayer Mini...");
    delay(500); // Give DFPlayer time to power up and stabilize

    if (!myDFPlayer.begin(mySerial)) { // Check if DFPlayer Mini successfully initialized
        Serial.println("ERROR: Failed to initialize DFPlayer Mini. Please check wiring.");
        setLED(255, 0, 0); // Red LED for critical error
        // Optionally, try to play a very basic sound or blink an LED pattern here
        // if the DFPlayer itself is not the problem but only communication is.
        while (true) { // Halt execution if DFPlayer fails, as it's a critical component
            delay(1000);
            Serial.println("Stuck: DFPlayer not found.");
        }
    }
    Serial.println("DFPlayer Mini connected successfully.");
    myDFPlayer.volume(30); // Set volume (0-30)
    myDFPlayer.play(1);    // Play track 1 (e.g., "System ready" sound)

    // --- Pin Initialization ---
    pinMode(buttonPin, INPUT_PULLUP); // Configure button pin with internal pull-up resistor
    pinMode(redPin, OUTPUT);          // Set RGB LED pins as outputs
    pinMode(greenPin, OUTPUT);
    pinMode(bluePin, OUTPUT);
    setLED(255, 135, 255); // Set LED to a default "idle" white color

    // --- WiFi Connection ---
    Serial.print("Connecting to WiFi: ");
    Serial.println(ssid);
    WiFi.begin(ssid, password); // Initiate WiFi connection
    unsigned long wifiConnectStart = millis();
    // Wait for WiFi connection with a timeout
    while (WiFi.status() != WL_CONNECTED && millis() - wifiConnectStart < 30000) { // Try for 30 seconds
        delay(500);
        Serial.print(".");
        setLED(0, 0, 255); // Blue LED while connecting
        delay(500);
        setLED(255, 130, 255); // Briefly flash white
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nWiFi Connected!");
        Serial.print("IP Address: ");
        Serial.println(WiFi.localIP());
        myDFPlayer.play(98); // Play "WiFi connected" sound (assuming track 98)
    } else {
        Serial.println("\nERROR: Failed to connect to WiFi.");
        myDFPlayer.play(97); // Play "WiFi error" sound (assuming track 97)
        setLED(255, 130, 0); // Orange LED for WiFi failure
        while (true) { // Halt execution if WiFi fails, as it's critical for server communication
            delay(1000);
            Serial.println("Stuck: WiFi connection failed.");
        }
    }

    // --- Camera Initialization ---
    setupCamera(); // Call the camera setup function
    Serial.println("System ready. Press the button to start recognition.");
    setLED(255, 130, 255); // Set LED to idle white once system is ready
}

// --- Loop Function ---
/**
 * @brief Main loop function that runs repeatedly after setup().
 * Monitors the button press and triggers photo capture and processing
 * when the button is pressed, with a debouncing mechanism.
 */
void loop() {
    // Keep LED at idle white when not actively processing, to give visual feedback
    setLED(255, 130, 255);

    // Check if the button is pressed and if it's currently allowed to be pressed (debounce)
    if (digitalRead(buttonPin) == LOW && canPress) { // Button is active LOW with INPUT_PULLUP
        Serial.println("Button pressed!");
        canPress = false; // Prevent immediate re-pressing

        // Trigger the main process of sending photo and getting response
        sendPhotoAndProcessResponse();

        // Implement a delay after processing to prevent rapid consecutive presses
        // and allow the system to settle. This also acts as a cooldown.
        delay(5000); // 5-second cooldown

        canPress = true; // Allow button to be pressed again after the cooldown
    }

    // Small delay to prevent the loop from running too fast, though not strictly necessary
    // for this button-triggered logic, it's good practice for general MCU stability.
    delay(10);
}