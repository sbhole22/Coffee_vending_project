#define ENABLE_DATABASE
#include <lvgl.h>
#include <TFT_eSPI.h> 
#include "touch.h"
#include "coffee_app.h" 
#include <WiFi.h>
#include <FirebaseClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>

// --- Network Credentials ---
#define WIFI_SSID "Your WiFi SSID here"
#define WIFI_PASSWORD "Your WiFi Password here"

// --- Firebase Credentials ---
#define API_KEY "Your API key here"
#define DATABASE_URL "Your Database URL here"

// --- Firebase Objects ---
WiFiClientSecure ssl_client;
AsyncClientClass client(ssl_client);

FirebaseApp app;
RealtimeDatabase Database;
AsyncResult result;
NoAuth noAuth;
bool firebaseInitialized = false;

static const uint16_t screenWidth  = 480;
static const uint16_t screenHeight = 320;
static lv_disp_draw_buf_t disp_buf;
static lv_color_t buf[screenWidth * 20];

TFT_eSPI my_lcd = TFT_eSPI();

#if LV_USE_LOG != 0
void my_print(const char * buf) {
    Serial.printf(buf);
    Serial.flush();
}
#endif

void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p) {
    uint32_t w = (area->x2 - area->x1 + 1);
    uint32_t h = (area->y2 - area->y1 + 1);
    my_lcd.startWrite();
    my_lcd.setAddrWindow(area->x1, area->y1, w, h);
    my_lcd.pushColors((uint16_t *)&color_p->full, w * h, true);
    my_lcd.endWrite();
    lv_disp_flush_ready(disp);
}

void my_touchpad_read(lv_indev_drv_t *indev_driver, lv_indev_data_t *data) {
    if (touch_has_signal()) {
        if (touch_touched()) {
            data->state = LV_INDEV_STATE_PR;
            data->point.x = touch_last_x;
            data->point.y = touch_last_y;
        } else if (touch_released()) {
            data->state = LV_INDEV_STATE_REL;
        }
    } else {
        data->state = LV_INDEV_STATE_REL;
    }
}

void setup() {
    Serial.begin(115200);
    
    my_lcd.init();
    my_lcd.setRotation(1);
    my_lcd.fillScreen(TFT_BLACK);
    
    touch_init(my_lcd.width(), my_lcd.height(), my_lcd.getRotation());
    
    lv_init();
    
#if LV_USE_LOG != 0
    lv_log_register_print_cb(my_print);
#endif

    lv_disp_draw_buf_init(&disp_buf, buf, NULL, screenWidth * 20);

    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = screenWidth;
    disp_drv.ver_res = screenHeight;
    disp_drv.flush_cb = my_disp_flush;
    disp_drv.draw_buf = &disp_buf;
    lv_disp_drv_register(&disp_drv);

    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = my_touchpad_read;
    lv_indev_drv_register(&indev_drv);

    // Initialize our custom Coffee App
    coffee_app_init();
    
    // --- Network Setup ---
    Serial.print("Connecting to Wi-Fi");
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    
    // We will handle Firebase initialization lazily when WiFi connects
    
    int try_count = 0;
    while (WiFi.status() != WL_CONNECTED && try_count < 20) {
        Serial.print(".");
        delay(300);
        try_count++;
    }
    if(WiFi.status() == WL_CONNECTED) {
        Serial.println("\nConnected to Wi-Fi");
        Serial.print("IP: "); Serial.println(WiFi.localIP());
    } else {
        Serial.println("\nWi-Fi Connection Failed (will retry in background)");
    }
    
    Serial.println("AuraMist Cafe Ready!");
}

// --- External Function Implementations ---

void init_firebase_if_needed() {
    if (!firebaseInitialized && WiFi.status() == WL_CONNECTED) {
        Serial.println("Initializing Firebase...");
        Firebase.printf("Firebase Client v%s\n", FIREBASE_CLIENT_VERSION);
        ssl_client.setInsecure();
        
        // Initialize App
        initializeApp(client, app, getAuth(noAuth));
        
        // Link Database to App (CRITICAL FIX for Error -117)
        app.getApp<RealtimeDatabase>(Database);
        
        Database.url(DATABASE_URL);
        firebaseInitialized = true;
        Serial.println("Firebase Initialized.");
    }
}

bool is_wifi_connected() {
    return WiFi.status() == WL_CONNECTED;
}

CloudOrder fetch_order_from_cloud(int orderId) {
    CloudOrder order;
    order.valid = false;
    
    if (WiFi.status() != WL_CONNECTED) return order;
    
    init_firebase_if_needed();

    String path = "/orders/" + String(orderId);
    Serial.print("Fetching order: "); Serial.println(path);
    
    // Use synchronous get
    Serial.println("Sending request to Firebase...");
    String jsonResult = Database.get<String>(client, path);
    
    // Retry if app was not assigned (Error -117)
    if (client.lastError().code() == -117) {
        Serial.println("Error -117 detected. Re-initializing Firebase...");
        firebaseInitialized = false;
        init_firebase_if_needed();
        Serial.println("Retrying request...");
        jsonResult = Database.get<String>(client, path);
    }
    
    Serial.print("Result Length: "); Serial.println(jsonResult.length());
    Serial.print("Result Content: "); Serial.println(jsonResult);
    Serial.print("Error Code: "); Serial.println(client.lastError().code());
    Serial.print("Error Msg: "); Serial.println(client.lastError().message());

    bool taskComplete = false;
    
    if (client.lastError().code() == 0 && jsonResult.length() > 0 && jsonResult != "null") {
        Serial.println("Order Found!");
        Serial.println(jsonResult);
        
        taskComplete = true; // Mark as complete for logic flow
    } else {
        Serial.print("Error: ");
        Serial.println(client.lastError().message());
    }
    
    if (taskComplete) {
        Serial.println("Order Found!");
        Serial.println(jsonResult);
        
        // Parse JSON
        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, jsonResult);
        
        if (!error) {
            order.valid = true;
            order.id = orderId;
            strlcpy(order.item_name, doc["item"] | "Unknown", sizeof(order.item_name));
            
            // Force conversion from String to float
            if (doc["price"].is<float>()) {
                order.price = doc["price"];
            } else if (doc["price"].is<String>()) {
                order.price = doc["price"].as<String>().toFloat();
            } else {
                order.price = doc["price"].as<float>();
            }
            
            Serial.print("Parsed Price: "); Serial.println(order.price);
            
            // Reset ingredients
            for(int i=0; i<8; i++) order.ingredients[i] = 0;
            
            // Parse ingredients array
            JsonArray ingArray = doc["ingredients"];
            for(JsonVariant v : ingArray) {
                const char* name = v["name"];
                float level = v["level"];
                
                // Map names to indices
                if(strcmp(name, "Espresso") == 0) order.ingredients[0] = level;
                else if(strcmp(name, "Whole Milk") == 0) order.ingredients[1] = level;
                else if(strcmp(name, "Almond Milk") == 0) order.ingredients[2] = level;
                else if(strcmp(name, "Oat Milk") == 0) order.ingredients[3] = level;
                else if(strcmp(name, "Foam") == 0) order.ingredients[4] = level;
                else if(strcmp(name, "Water") == 0) order.ingredients[5] = level;
                else if(strcmp(name, "Syrup") == 0) order.ingredients[6] = level;
                else if(strcmp(name, "Ice") == 0) order.ingredients[7] = level;
            }
        } else {
            Serial.print("JSON Parse Error: "); Serial.println(error.c_str());
        }
    } else {
        Serial.println("Order Not Found or Timeout");
    }
    
    return order;
}

void update_order_status_cloud(int orderId, const char* status) {
    if (WiFi.status() != WL_CONNECTED) return;
    
    init_firebase_if_needed();
    
    String path = "/orders/" + String(orderId);
    String json = "{\"status\": \"" + String(status) + "\"}";
    
    Database.update(client, path, object_t(json));
}

void loop() {
    app.loop();
    lv_timer_handler();
    delay(5);
}