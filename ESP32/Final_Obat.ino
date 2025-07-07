#include <WiFi.h>
#include <HTTPClient.h>
#include "esp_camera.h"
#include "driver/ledc.h"
#include <DFRobotDFPlayerMini.h>
#include <ArduinoJson.h>

// ====== Serial dan DFPlayer Config ======
HardwareSerial mySerial(2);   // UART2: RX = GPIO14, TX = GPIO13
DFRobotDFPlayerMini myDFPlayer;

// ====== WiFi Config ======
const char* ssid     = "******";
const char* password = "*******";

// ====== Server Config ======
const char* serverUrl = "http://*********/upload";

// ====== RGB LED & Tombol ======
const int redPin    = 32;
const int greenPin  = 33;
const int bluePin   = 26;
const int buttonPin = 12;

// ====== RGB LED State ======
uint8_t currentRed   = 255;
uint8_t currentGreen = 145;
uint8_t currentBlue  = 255;

bool canPress = true;

// ====== Kamera Pin Mapping ======
#define PWDN_GPIO_NUM     -1
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM     21
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       19
#define Y4_GPIO_NUM       18
#define Y3_GPIO_NUM       5
#define Y2_GPIO_NUM       4
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

// ====== Fungsi: Kontrol LED RGB ======
void setLED(uint8_t r, uint8_t g, uint8_t b) {
    analogWrite(redPin, r);
    analogWrite(greenPin, g);
    analogWrite(bluePin, b);
    currentRed = r;
    currentGreen = g;
    currentBlue = b;
}

void restoreLED() {
    analogWrite(redPin, currentRed);
    analogWrite(greenPin, currentGreen);
    analogWrite(bluePin, currentBlue);
}

// ====== Fungsi: Inisialisasi Kamera ======
void setupCamera() {
    camera_config_t config;
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer   = LEDC_TIMER_0;
    config.pin_d0       = Y2_GPIO_NUM;
    config.pin_d1       = Y3_GPIO_NUM;
    config.pin_d2       = Y4_GPIO_NUM;
    config.pin_d3       = Y5_GPIO_NUM;
    config.pin_d4       = Y6_GPIO_NUM;
    config.pin_d5       = Y7_GPIO_NUM;
    config.pin_d6       = Y8_GPIO_NUM;
    config.pin_d7       = Y9_GPIO_NUM;
    config.pin_xclk     = XCLK_GPIO_NUM;
    config.pin_pclk     = PCLK_GPIO_NUM;
    config.pin_vsync    = VSYNC_GPIO_NUM;
    config.pin_href     = HREF_GPIO_NUM;
    config.pin_sscb_sda = SIOD_GPIO_NUM;
    config.pin_sscb_scl = SIOC_GPIO_NUM;
    config.pin_pwdn     = PWDN_GPIO_NUM;
    config.pin_reset    = RESET_GPIO_NUM;
    config.xclk_freq_hz = 25000000;
    config.pixel_format = PIXFORMAT_JPEG;
    config.frame_size   = FRAMESIZE_QVGA;
    config.jpeg_quality = 4;
    config.fb_count     = 1;

    if (esp_camera_init(&config) == ESP_OK) {
        Serial.println("Camera Init OK");
        sensor_t *s = esp_camera_sensor_get();

        // Pengaturan sensor tambahan
        s->set_brightness(s, -2);
        s->set_contrast(s, 2);
        s->set_saturation(s, -2);
        s->set_special_effect(s, 2);
        s->set_whitebal(s, 1);
        s->set_awb_gain(s, 1);
        s->set_wb_mode(s, 0);
        s->set_ae_level(s, -1);
        s->set_exposure_ctrl(s, 0);
        s->set_aec2(s, 0);
        s->set_aec_value(s, 1200);
        s->set_gain_ctrl(s, 1);
        s->set_agc_gain(s, 0);
        s->set_bpc(s, 0);
        s->set_wpc(s, 0);
        s->set_raw_gma(s, 0);
        s->set_lenc(s, 0);
        s->set_hmirror(s, 0);
        s->set_vflip(s, 0);
        s->set_dcw(s, 0);
        s->set_colorbar(s, 0);
    } else {
        Serial.println("Camera Init Failed");
    }
}

// ====== Fungsi: Ambil Gambar & Kirim ke Server ======
void sendPhotoAndProcessResponse() {
    Serial.println("Capturing photo...");
    setLED(0, 130, 255); // Biru: proses ambil gambar

    // Bersihkan buffer awal
    for (int i = 0; i < 3; i++) {
        camera_fb_t *temp_fb = esp_camera_fb_get();
        if (temp_fb) esp_camera_fb_return(temp_fb);
    }

    camera_fb_t *fb = esp_camera_fb_get();
    if (!fb) {
        Serial.println("Failed to capture photo");
        setLED(255, 130, 0); // Orange: gagal
        myDFPlayer.play(99);
        delay(2000);
        restoreLED();
        return;
    }

    Serial.printf("Captured photo size: %u bytes\n", fb->len);

    WiFiClient client;
    HTTPClient http;
    http.begin(client, serverUrl);
    http.addHeader("Content-Type", "image/jpeg");
    http.setTimeout(10000);

    Serial.println("Sending photo to server...");
    int httpResponseCode = http.POST(fb->buf, fb->len);

    if (httpResponseCode > 0) {
        String payload = http.getString();
        Serial.printf("HTTP Response Code: %d\n", httpResponseCode);
        Serial.println("Server Response: " + payload);

        // Parse JSON dari server
        StaticJsonDocument<200> doc;
        DeserializationError error = deserializeJson(doc, payload);

        if (error) {
            Serial.print("JSON parse error: ");
            Serial.println(error.f_str());
            setLED(255, 130, 0);
            myDFPlayer.play(99);
            delay(2000);
        } else {
            const char* status      = doc["status"];
            int predictedIndex      = doc["index"];
            float predictedScore    = doc["score"];

            if (strcmp(status, "success") == 0 && predictedIndex != -1) {
                // Suara hasil klasifikasi
                switch (predictedIndex) {
                    case 0: myDFPlayer.play(2); break;
                    case 1: myDFPlayer.play(3); break;
                    case 2: myDFPlayer.play(4); break;
                    default: myDFPlayer.play(5); break;
                }

                // Warna LED berdasarkan confidence
                if (predictedScore >= 0.85) {
                    setLED(0, 130, 0); // Hijau: akurat
                } else if (predictedScore >= 0.60) {
                    setLED(255, 130, 0); // Kuning: sedang
                    myDFPlayer.play(98);
                } else {
                    setLED(255, 130, 0); // Merah: tidak yakin
                    myDFPlayer.play(97);
                }
                delay(3000);
            } else {
                Serial.println("Invalid prediction index or failed status.");
                myDFPlayer.play(99);
                setLED(255, 130, 0);
                delay(2000);
            }
        }
    } else {
        // Jika gagal HTTP
        String errorString = http.errorToString(httpResponseCode);
        Serial.printf("HTTP Error: %d - %s\n", httpResponseCode, errorString.c_str());
        myDFPlayer.play(5);
        setLED(255, 125, 0);
        delay(3000);
    }

    http.end();
    esp_camera_fb_return(fb);
    restoreLED(); // Kembalikan LED ke idle
}

// ====== Setup Awal ======
void setup() {
    Serial.begin(115200);
    mySerial.begin(9600, SERIAL_8N1, 14, 13);
    Serial.println("Memulai DFPlayer...");
    delay(500);

    if (!myDFPlayer.begin(mySerial)) {
        Serial.println("DFPlayer Mini gagal.");
        while (1); // Hentikan program jika DFPlayer gagal
    }

    Serial.println("DFPlayer terhubung.");
    myDFPlayer.volume(30);
    myDFPlayer.play(1); // Suara siap
    delay(2000);

    // Konfigurasi pin
    pinMode(buttonPin, INPUT_PULLUP);
    pinMode(redPin, OUTPUT);
    pinMode(greenPin, OUTPUT);
    pinMode(bluePin, OUTPUT);
    setLED(255, 125, 255); // LED idle: putih muda

    // Koneksi WiFi
    Serial.print("Connecting to WiFi...");
    WiFi.begin(ssid, password);
    unsigned long wifiStart = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - wifiStart < 30000) {
        delay(500);
        Serial.print(".");
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("Connected!");
        Serial.println(WiFi.localIP());
        myDFPlayer.play(1);
        delay(1000);
    } else {
        Serial.println("WiFi failed");
        myDFPlayer.play(97);
        setLED(255, 125, 0); // Orange: gagal
        while (1); // Hentikan program
    }

    // Inisialisasi kamera
    setupCamera();
    Serial.println("Sistem siap. Tekan tombol.");
    myDFPlayer.play(1);
    delay(1000);
    setLED(255, 130, 255);
}

// ====== Loop Utama ======
void loop() {
    restoreLED(); // Pastikan LED idle tetap aktif

    if (digitalRead(buttonPin) == HIGH && canPress) {
        Serial.println("Tombol ditekan");
        canPress = false;
        sendPhotoAndProcessResponse();
        delay(5000); // Hindari bouncing & deteksi ganda
        canPress = true;
        Serial.println("Siap menerima penekanan tombol berikutnya");
    }
}
