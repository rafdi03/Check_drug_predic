#include <WiFi.h>
#include <HTTPClient.h>
#include "esp_camera.h"
#include "driver/ledc.h" // Untuk kontrol LED PWM
#include <DFRobotDFPlayerMini.h>
#include <ArduinoJson.h> // <<< TAMBAHKAN INI UNTUK PARSING JSON

HardwareSerial mySerial(2);   // UART2, pastikan pin 14 (RX) dan 13 (TX)
DFRobotDFPlayerMini myDFPlayer;

// ====== WiFi Config ======
const char* ssid = "";
const char* password = "";

// ====== Server Config ======
const char* serverUrl = "http://192.168.***.***:***/upload"; // Ganti dengan URL endpoint server Flask Anda

// ====== Pin Config ======
const int redPin = 32;
const int greenPin = 33;
const int bluePin = 26;
const int buttonPin = 12;

bool canPress = true; // Flag untuk debounce tombol. Logika awal Anda menggunakan ini dengan delay.

// ====== Camera Pins (ESP32-CAM default) ======
#define PWDN_GPIO_NUM -1
#define RESET_GPIO_NUM -1
#define XCLK_GPIO_NUM 21
#define SIOD_GPIO_NUM 26
#define SIOC_GPIO_NUM 27
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

// --- Fungsi Kontrol LED ---
void setLED(uint8_t r, uint8_t g, uint8_t b) {
    analogWrite(redPin, r);
    analogWrite(greenPin, g);
    analogWrite(bluePin, b);
}

// --- Inisialisasi Kamera ---
void setupCamera() {
    camera_config_t config;
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    config.pin_d0 = Y2_GPIO_NUM;
    config.pin_d1 = Y3_GPIO_NUM;
    config.pin_d2 = Y4_GPIO_NUM;
    config.pin_d3 = Y5_GPIO_NUM;
    config.pin_d4 = Y6_GPIO_NUM;
    config.pin_d5 = Y7_GPIO_NUM;
    config.pin_d6 = Y8_GPIO_NUM;
    config.pin_d7 = Y9_GPIO_NUM;
    config.pin_xclk = XCLK_GPIO_NUM;
    config.pin_pclk = PCLK_GPIO_NUM;
    config.pin_vsync = VSYNC_GPIO_NUM;
    config.pin_href = HREF_GPIO_NUM;
    config.pin_sscb_sda = SIOD_GPIO_NUM;
    config.pin_sscb_scl = SIOC_GPIO_NUM;
    config.pin_pwdn = PWDN_GPIO_NUM;
    config.pin_reset = RESET_GPIO_NUM;
    config.xclk_freq_hz = 25000000;
    config.pixel_format = PIXFORMAT_JPEG;
    config.frame_size = FRAMESIZE_QVGA;        // QVGA (320x240)
    config.jpeg_quality = 4;                 // Quality level (0-63, lower is better quality)
    config.fb_count = 1;

    if (esp_camera_init(&config) == ESP_OK) {
        Serial.println("Camera Init OK");
        sensor_t *s = esp_camera_sensor_get();
        // Pengaturan sensor kamera Anda (sesuai yang Anda berikan)
        s->set_brightness(s, -2);
        s->set_contrast(s, 2);
        s->set_saturation(s, -2);
        s->set_special_effect(s, 2);        // 1 = Grayscale, 2 = Negative
        s->set_whitebal(s, 1);               // AWB ON
        s->set_awb_gain(s, 1);               // AWB Gain ON
        s->set_wb_mode(s, 0);                // WB Mode Auto
        s->set_ae_level(s, -1);
        s->set_exposure_ctrl(s, 0);          // AEC ON (Auto Exposure)
        s->set_aec2(s, 0);                   // AEC2 OFF
        s->set_aec_value(s, 1200);           // Exposure Value
        s->set_gain_ctrl(s, 1);              // AGC ON (Auto Gain Control)
        s->set_agc_gain(s, 0);               // AGC Gain
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

// --- Fungsi Kirim Foto dan Proses Respons ---
// --- Fungsi Kirim Foto dan Proses Respons ---
void sendPhotoAndProcessResponse() {
    Serial.println("Capturing photo...");
    setLED(0, 0, 255); // LED Biru: Mengambil foto

    // Flush beberapa frame awal agar dapat frame terbaru
    for (int i = 0; i < 3; i++) {
        camera_fb_t * temp_fb = esp_camera_fb_get();
        if (temp_fb) esp_camera_fb_return(temp_fb);
    }

    camera_fb_t * fb = esp_camera_fb_get();
    if (!fb) {
        Serial.println("Failed to capture photo");
        setLED(255, 0, 0); // LED Merah: Gagal capture
        myDFPlayer.play(99); // Suara error, misalnya 0099.mp3
        delay(2000);
        setLED(255, 150, 255); // Kembali ke putih
        return;
    }

    Serial.printf("Captured photo size: %u bytes\n", fb->len);

    WiFiClient client;
    HTTPClient http;

    http.begin(client, serverUrl);
    http.addHeader("Content-Type", "image/jpeg");

    // === MENAMBAHKAN TIMEOUT DI SINI ===
    http.setTimeout(10000); // Set timeout ke 10 detik (10000 milidetik)

    Serial.println("Sending photo to server...");
    int httpResponseCode = http.POST(fb->buf, fb->len);

    if (httpResponseCode > 0) {
        String payload = http.getString();
        Serial.printf("HTTP Response Code: %d\n", httpResponseCode);
        Serial.print("Server Response: ");
        Serial.println(payload);

        // --- Parse JSON response ---
        // *** PERBAIKI BARIS INI ***
        StaticJsonDocument<200> doc; // <<< PASTIKAN UKURANNYA CUKUP UNTUK JSON ANDA
        DeserializationError error = deserializeJson(doc, payload);

        if (error) {
            Serial.print(F("deserializeJson() failed: "));
            Serial.println(error.f_str());
            setLED(255, 0, 0); // LED Merah: Gagal parsing JSON
            myDFPlayer.play(99); // Suara error
            delay(2000);
        } else {
            const char* status = doc["status"];
            int predictedIndex = doc["index"];
            float predictedScore = doc["score"];

            Serial.printf("Status from server: %s\n", status);
            Serial.printf("Predicted Index: %d\n", predictedIndex);
            Serial.printf("Predicted Score: %.4f\n", predictedScore);

            if (strcmp(status, "success") == 0) {
                // Logika pemutaran suara DFPlayer berdasarkan indeks
                // dan indikasi LED berdasarkan score
                if (predictedIndex != -1) { // Pastikan indeks valid
                    Serial.print("Playing sound for index: ");
                    Serial.println(predictedIndex);

                    // Peta indeks ke nomor track DFPlayer yang Anda berikan:
                    // Index 0 (omeprazole) -> Track 4
                    // Index 1 (panadol)    -> Track 2
                    // Index 2 (procold_flu) -> Track 3
                    switch (predictedIndex) {
                        case 0: // omeprazole
                            myDFPlayer.play(2); // Sesuai permintaan Anda
                            break;
                        case 1: // panadol 2
                            myDFPlayer.play(3); // Sesuai permintaan Anda
                            break;
                        case 2: // procold_flu
                            myDFPlayer.play(4); // Sesuai permintaan Anda
                            break;
                        default:
                            Serial.println("Unknown index received. Playing default error sound.");
                            myDFPlayer.play(5); // Suara error/tidak dikenal (misal 0099.mp3)
                            break;
                    }
                    // Tambahkan kondisi LED untuk confidence score
                    if (predictedScore >= 0.85) { // Confidence tinggi
                        setLED(0, 255, 0); // Hijau
                    } else if (predictedScore >= 0.60) { // Confidence sedang
                        setLED(255, 255, 0); // Kuning
                        myDFPlayer.play(98); // Suara "kurang yakin", misal 0098.mp3 (opsional)
                    } else { // Confidence rendah
                        setLED(255, 0, 0); // Merah
                        myDFPlayer.play(97); // Suara "tidak yakin", misal 0097.mp3 (opsional)
                    }
                    delay(3000); // Beri waktu suara diputar dan LED menyala
                } else {
                    Serial.println("Server responded with success but invalid index (-1).");
                    myDFPlayer.play(99); // Suara error
                    setLED(255, 0, 0); // LED Merah: Indeks tidak valid
                    delay(2000);
                }
            } else {
                // Perbaiki juga baris ini karena doc["message"] akan berfungsi setelah doc dideklarasikan dengan benar
                Serial.printf("Server processing error: %s\n", doc["message"].as<const char*>());
                myDFPlayer.play(99); // Suara error
                setLED(255, 0, 0); // LED Merah: Server error
                delay(2000);
            }
        }
    } else {
        // === LOGIKA TIMEOUT DITAMBAHKAN DI SINI ===
        String errorString = http.errorToString(httpResponseCode);
        Serial.printf("Failed to send photo or receive response. HTTP Error: %d - %s\n", httpResponseCode, errorString.c_str());
        myDFPlayer.play(5);
        if (errorString.indexOf("timeout") != -1) { // Mengecek apakah error mengandung kata "timeout"
            Serial.println("Server connection timed out! Playing 'gagal kirim' sound.");
            myDFPlayer.play(5); // Suara "gagal kirim" (track 5)
            setLED(255, 0, 0); // LED Merah: Timeout
            delay(3000); // Beri waktu suara diputar
        } else {
            myDFPlayer.play(99); // Suara error komunikasi umum
            setLED(255, 0, 0); // LED Merah: Gagal komunikasi umum
            delay(2000);
        }
    }

    http.end();
    esp_camera_fb_return(fb); // Sangat penting: kembalikan buffer frame!
    setLED(255, 150, 255); // Kembali ke LED Putih setelah proses
}

void setup() {
    Serial.begin(115200);
    Serial.println(); // Baris baru untuk kebersihan

    // === Inisialisasi DFPlayer Mini ===
    // Pastikan pin 14 (RX) dan 13 (TX) tidak bentrok dengan fungsi lain di ESP32-CAM!
    // Misalnya, pin 13 seringkali terhubung ke LED Flash.
    // Jika bentrok, Anda mungkin perlu mengubah pin UART2 atau menggunakan SoftwareSerial jika diperlukan.
    mySerial.begin(9600, SERIAL_8N1, 14, 13);
    Serial.println("Memulai DFPlayer...");
    delay(500); // Beri waktu inisialisasi serial
    if (!myDFPlayer.begin(mySerial)) {
        Serial.println("Gagal menginisialisasi DFPlayer Mini.");
        Serial.println("Pastikan kabel dan kartu SD terpasang dengan benar.");
        while (1); // Stuck jika DFPlayer gagal
    }
    Serial.println("DFPlayer Mini berhasil terhubung.");
    myDFPlayer.volume(30);   // Volume 0-30

    // NEW: Mainkan suara "DFPlayer Terhubung"
    myDFPlayer.play(1); // Track 1: connection
    delay(2000); // Beri waktu suara diputar

    // === Inisialisasi Pin & LED ===
    pinMode(buttonPin, INPUT_PULLUP);
    // Mengatur pin sebagai OUTPUT untuk LED
    pinMode(redPin, OUTPUT);
    pinMode(greenPin, OUTPUT);
    pinMode(bluePin, OUTPUT);
    setLED(255, 150, 255); // LED putih nyala

    // === Inisialisasi WiFi ===
    Serial.print("Connecting to WiFi...");
    WiFi.begin(ssid, password);
    unsigned long wifiConnectStart = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - wifiConnectStart < 30000) { // Coba selama 30 detik
        delay(500);
        Serial.print(".");
    }
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("Connected!");
        Serial.print("IP Address: ");
        Serial.println(WiFi.localIP());
        // NEW: Mainkan suara "WiFi Terhubung"
        myDFPlayer.play(1); // Track 1: (assuming track 1 for "connected" message, not 2 as previously)
        delay(1000);
    } else {
        Serial.println("Failed to connect to WiFi.");
        myDFPlayer.play(97); // Suara "WiFi failed", misalnya track 97
        setLED(255, 0, 0); // LED Merah: Gagal WiFi
        while(1) delay(1000); // Stuck jika WiFi gagal
    }
    
    // === Inisialisasi Kamera ===
    setupCamera();
    Serial.println("Sistem siap. Tekan tombol.");
    // NEW: Mainkan suara "Sistem Siap"
    myDFPlayer.play(1); // Track 1: (assuming track 1 for "system ready" message, not 3 as previously)
    delay(1000); // Beri waktu suara diputar

    setLED(255, 150, 255); // Kembali ke LED Putih setelah semua inisialisasi
}

void loop() {
    setLED(255, 150, 255); // Pastikan LED default putih saat idle

    // === Logika Penekanan Tombol Anda yang Asli ===
    if (digitalRead(buttonPin) == HIGH && canPress) {
        Serial.println("Button Pressed!");

        canPress = false; // Nonaktifkan penekanan sampai proses selesai

        sendPhotoAndProcessResponse(); // Panggil fungsi utama yang kini memproses respons

        // Delay 5 detik Anda yang asli setelah proses selesai
        delay(5000); // Tunggu 5 detik sebelum bisa ditekan lagi
        canPress = true; // Aktifkan kembali penekanan tombol
        Serial.println("Siap menerima penekanan tombol berikutnya");
    }
}