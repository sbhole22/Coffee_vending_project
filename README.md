# AuraMist Cafe - Smart IoT Vending Machine

**AuraMist Cafe** is a next-generation smart coffee vending machine prototype that bridges the gap between physical retail and digital convenience. Powered by an **ESP32-S3** and **LVGL**, it features a smartphone-class touch interface, real-time cloud integration with a mobile app, and interactive gamification to engage users while they wait.

## 🌟 Key Features

### 📱 Modern Touch Interface (LVGL)
*   **Fluid UI:** Built with **LVGL 8.x**, featuring smooth animations, gesture controls, and professional widgets.
*   **"Make it Yours" Engine:** A unique drag-and-drop interface where users visually construct their drink by stacking layers of Coffee, Milk, Foam, and Chocolate in a virtual glass.
*   **Mood Detection:** Algorithms analyze your custom drink recipe to assign a personality label (e.g., "The Alchemist", "Midnight Oil").

### ☁️ IoT & Cloud Integration
*   **Mobile Ordering:** A companion **React Native** app allows users to order remotely.
*   **Pickup System:** Users receive a 4-digit code on their phone, which they enter on the machine to "dispense" their order.
*   **Real-Time Sync:** The machine communicates with **Firebase Realtime Database** via SSL to fetch orders and update status (e.g., "Brewing" -> "Ready").

### 🎮 Gamification
*   **Tic-Tac-Toe:** Play against an AI opponent while your coffee brews.
*   **Bean Catcher:** A physics-based mini-game to catch falling ingredients.
*   **Rewards:** Winning games unlocks dynamic discounts (e.g., "WIN2" for 2% off) applied instantly to the current order.

---

## 🛠️ Hardware Requirements

*   **Microcontroller:** ESP32-S3 (Dual-core, WiFi/BLE)
*   **Display:** 4.0" IPS TFT LCD (ST7796S Driver) with Capacitive Touch (GT911)
*   **Resolution:** 480x320 pixels
*   **Connection:** SPI (Display) + I2C (Touch)

## 💻 Software Stack

### Firmware (Arduino/C++)
*   **Core:** Arduino for ESP32
*   **Graphics:** [LVGL 8.3](https://lvgl.io/)
*   **Display Driver:** [TFT_eSPI](https://github.com/Bodmer/TFT_eSPI) or [LCDWIKI](http://www.lcdwiki.com/)
*   **Cloud:** [FirebaseClient](https://github.com/mobizt/FirebaseClient) (by Mobizt)
*   **JSON:** [ArduinoJson](https://arduinojson.org/)

### Mobile App
*   **Framework:** React Native (Expo)
*   **Backend:** Firebase Realtime Database

---

## 🚀 Setup & Installation

### 1. Firmware Setup
1.  Install **Arduino IDE** and the **ESP32 Board Package**.
2.  Install required libraries via Library Manager:
    *   `lvgl`
    *   `TFT_eSPI`
    *   `FirebaseClient`
    *   `ArduinoJson`
3.  Configure `TFT_eSPI` User_Setup.h for your specific display pinout (ST7796S).
4.  Update `LVGL_coffee5.ino` with your WiFi and Firebase credentials:
    ```cpp
    #define WIFI_SSID "Your_SSID"
    #define WIFI_PASSWORD "Your_Password"
    #define API_KEY "Your_Firebase_API_Key"
    #define DATABASE_URL "https://your-project.firebaseio.com/"
    ```
5.  Upload to ESP32-S3.

### 2. Mobile App Setup
1.  Navigate to the `CoffeeApp` folder.
2.  Install dependencies: `npm install`
3.  Configure `firebaseConfig.js` with your project details.
4.  Run the app: `npx expo start`

---

## 📂 Project Structure

*   **`LVGL_coffee5/`**: The main firmware source code.
    *   `LVGL_coffee5.ino`: Main Arduino sketch, hardware setup, and network logic.
    *   `coffee_app.cpp`: All UI logic, LVGL screen definitions, and game code.
    *   `coffee_app.h`: Header file for the UI module.
*   **`CoffeeApp/`**: The React Native mobile application source.
*   **`libraries/`**: Custom or modified libraries used in the project.

---

## 📸 How It Works

1.  **Select:** Choose a drink from the menu or create a custom one.
2.  **Customize:** Adjust strength, size, or drag ingredients in "Make it Yours".
3.  **Order:** Pay locally or enter a Pickup Code from the mobile app.
4.  **Wait & Play:** While the progress bar fills (simulating brewing), play a game to win a coupon.
5.  **Enjoy:** The machine updates the cloud status to "Ready" and displays the final drink.

---

## 📜 License
This project is open-source and available for educational purposes.
